/******************************************************************************
* irrlamb - https://github.com/jazztickets/irrlamb
* Copyright (C) 2019  Alan Witkowski
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/
#include <states/play.h>
#include <globals.h>
#include <input.h>
#include <log.h>
#include <graphics.h>
#include <audio.h>
#include <config.h>
#include <physics.h>
#include <scripting.h>
#include <objectmanager.h>
#include <interface.h>
#include <camera.h>
#include <framework.h>
#include <level.h>
#include <campaign.h>
#include <fader.h>
#include <actions.h>
#include <save.h>
#include <objects/player.h>
#include <menu.h>
#include <states/viewreplay.h>
#include <states/null.h>
#include <ISceneManager.h>
#include <IFileSystem.h>

const float PAUSE_FADE_AMOUNT = 0.85f;

using namespace irr;

_PlayState PlayState;

// Initializes the state
int _PlayState::Init() {
	HighScoreIndex = -1;
	Timer = 0.0f;
	InputReplay = nullptr;
	Interface.ChangeSkin(_Interface::SKIN_GAME);

	// Add camera
	Camera = new _Camera();

	// Setup input replay
	InputReplay = new _Replay();
	if(ReplayInputs) {

		// Open replay
		if(!InputReplay->LoadReplay(InputReplayFilename, true)) {
			Log.Write("Cannot load replay: %s", InputReplayFilename.c_str());
			Framework.SetDone(true);
			return 0;
		}

		// Get level name
		TestLevel = InputReplay->GetLevelName();
	}

	// Get level name
	std::string LevelFile;
	if(TestLevel != "") {
		LevelFile = TestLevel;
	}
	else {
		LevelFile = Campaign.GetLevel(CurrentCampaign, CampaignLevel);
		Save.UnlockLevel(LevelFile);
	}

	// Load level
	if(!Level.Init(LevelFile))
		return 0;

	// Check version
	if(ReplayInputs && Level.LevelVersion != InputReplay->GetLevelVersion()) {
		Log.Write("Level version mismatch: %d vs %d", Level.LevelVersion, InputReplay->GetLevelVersion());
		Framework.SetDone(true);
		return 0;
	}

	// Reset level
	ResetLevel();
	FirstLoad = true;

	return 1;
}

// Shuts the state down
int _PlayState::Close() {

	// Stop the replay
	Replay.StopRecording();

	// Close the system down
	delete InputReplay;
	delete Camera;
	Level.Close();
	ObjectManager.ClearObjects();
	Interface.Clear();
	irrScene->clear();
	Audio.StopSounds();
	Physics.Close();

	// Save stats
	if(TestLevel == "") {
		Save.LevelStats[Level.LevelName].PlayTime += Timer;
		Save.SaveLevelStats(Level.LevelName);
	}

	return 1;
}

// Resets the level
void _PlayState::ResetLevel() {
	Framework.SetTimeScale(1.0f);
	HighScoreIndex = -1;
	FirstLoad = false;
	Jumped = false;

	// Handle saves
	if(TestLevel == "") {
		Save.LevelStats[Level.LevelName].LoadCount++;
		Save.LevelStats[Level.LevelName].PlayTime += Timer;
		Save.SaveLevelStats(Level.LevelName);
	}

	// Get first event for input replay
	if(ReplayInputs) {
		InputReplay->StopReplay();
		InputReplay->LoadReplay(InputReplayFilename);
		InputReplay->ReadEvent(NextEvent);
	}

	// Stop sounds
	Audio.StopSounds();

	// Stop recording
	Replay.StopRecording();

	// Set up GUI
	Menu.InitPlay();
	Interface.Clear();
	Timer = 0.0f;

	// Set up camera
	Camera->SetRotation(0.0f, 30.0f);
	Camera->SetDistance(5.0f);
	Camera->SetFOV(Config.FOV);

	// Clear objects
	ObjectManager.ClearObjects();
	Physics.Reset();

	// Start replay recording
	Replay.StartRecording();

	// Load level objects
	Level.SpawnEntities();
	Level.RunScripts();
	Graphics.SetLightCount();

	// Get the player
	Player = static_cast<_Player *>(ObjectManager.GetObjectByType(_Object::PLAYER));
	if(Player == nullptr) {
		Log.Write("Cannot find player object");
		return;
	}
	Player->SetCamera(Camera);

	// Record camera in replay
	glm::vec3 Position = Player->GetPosition();
	Camera->Update(core::vector3df(Position[0], Position[1], Position[2]));
	Camera->RecordReplay();

	// Reset game timer
	Framework.ResetTimer();
	Fader.Start(FADE_SPEED);
	Resetting = false;
}

// Handle new actions
bool _PlayState::HandleAction(int InputType, int Action, float Value) {
	if(Resetting)
		return false;

	//printf("%d %f\n", Action, Value);
	bool Processed = false;
	if(!IsPaused()) {
		switch(Action) {
			case _Actions::JUMP:
				if(Value && !ReplayInputs) {
					Player->Jump();
					Jumped = true;
				}
			break;
			case _Actions::RESET:
				if(Value)
					StartReset();
			break;
			case _Actions::MENU_PAUSE:
				if(!Value)
					return false;

				if(ReplayInputs) {
					NullState.State = _Menu::STATE_REPLAYS;
					Framework.ChangeState(&NullState);
				}
				else if(TestLevel != "")
					Framework.SetDone(true);
				else
					Menu.InitPause();

				return true;
			break;
			case _Actions::CAMERA_LEFT:
				if(Camera && !ReplayInputs) {
					if(InputType == _Input::JOYSTICK_AXIS)
						Value *= Framework.GetLastFrameTime();
					Camera->HandleMouseMotion(-Value, 0);
				}
			break;
			case _Actions::CAMERA_RIGHT:
				if(Camera && !ReplayInputs) {
					if(InputType == _Input::JOYSTICK_AXIS)
						Value *= Framework.GetLastFrameTime();
					Camera->HandleMouseMotion(Value, 0);
				}
			break;
			case _Actions::CAMERA_UP:
				if(Camera && !ReplayInputs) {
					if(InputType == _Input::JOYSTICK_AXIS)
						Value *= Framework.GetLastFrameTime();
					Camera->HandleMouseMotion(0, -Value);
				}
			break;
			case _Actions::CAMERA_DOWN:
				if(Camera && !ReplayInputs) {
					if(InputType == _Input::JOYSTICK_AXIS)
						Value *= Framework.GetLastFrameTime();
					Camera->HandleMouseMotion(0, Value);
				}
			break;
		}
	}
	else {
		Processed = Menu.HandleAction(InputType, Action, Value);
	}

	//printf("action press %d %f\n", Action, Value);

	return Processed;
}

// Key presses
bool _PlayState::HandleKeyPress(int Key) {
	if(Resetting)
		return true;

	bool LuaProcessed = false;
	if(Menu.State == _Menu::STATE_NONE) {

		switch(Key) {
			case KEY_KEY_1:
			case KEY_KEY_2:
			case KEY_KEY_3:
			case KEY_KEY_4:
			case KEY_KEY_5:
				if(ReplayInputs)
					Framework.SetTimeScale(1 << (Key - KEY_KEY_1));
			break;
			case KEY_F1:
				Menu.InitPause();
			break;
			case KEY_F2:
				Config.InvertMouse = !Config.InvertMouse;

				if(Config.InvertMouse)
					Interface.SetShortMessage("Mouse inverted", INTERFACE_SHORTMESSAGE_X, INTERFACE_SHORTMESSAGE_Y);
				else
					Interface.SetShortMessage("Mouse normal", INTERFACE_SHORTMESSAGE_X, INTERFACE_SHORTMESSAGE_Y);
			break;
			case KEY_F3:
				if(Input.GetKeyState(KEY_RSHIFT))
					ObjectManager.PrintObjectOrientations();
				else if(Input.GetKeyState(KEY_RCONTROL))
					Physics.Dump();
				else
					Player->PrintOrientation();
			break;
			case KEY_F5:
				Framework.ChangeState(&PlayState);
			break;
			case KEY_F10:
				Config.SoundVolume = !Config.SoundVolume;
				Audio.SetGain(Config.SoundVolume);

				if(Config.SoundVolume)
					Interface.SetShortMessage("Audio on", INTERFACE_SHORTMESSAGE_X, INTERFACE_SHORTMESSAGE_Y);
				else
					Interface.SetShortMessage("Audio off", INTERFACE_SHORTMESSAGE_X, INTERFACE_SHORTMESSAGE_Y);
			break;
			case KEY_F11:
				Interface.DrawHUD = !Interface.DrawHUD;
			break;
			case KEY_F12:
				Graphics.SaveScreenshot(Level.LevelName);
			break;
		}

		// Send key presses to Lua
		LuaProcessed = Scripting.HandleKeyPress(Key);
	}

	bool Processed = Menu.HandleKeyPress(Key);

	return Processed || LuaProcessed;
}

// Mouse buttons
bool _PlayState::HandleMousePress(int Button, int MouseX, int MouseY) {
	if(Resetting)
		return false;

	if(!IsPaused()) {
		Scripting.HandleMousePress(Button, MouseX, MouseY);
	}

	return false;
}

// Mouse buttons
void _PlayState::HandleMouseLift(int Button, int MouseX, int MouseY) {
	if(Resetting)
		return;
}

// Mouse wheel
void _PlayState::HandleMouseWheel(float Direction) {

}

// GUI events
void _PlayState::HandleGUI(irr::gui::EGUI_EVENT_TYPE EventType, gui::IGUIElement *Element) {
	if(Resetting)
		return;

	Menu.HandleGUI(EventType, Element);
}

// Updates the current state
void _PlayState::Update(float FrameTime) {

	if(Resetting) {
		if(Fader.IsDoneFading()) {
			ResetLevel();
		}

		return;
	}

	// Check if paused
	if(!IsPaused()) {

		// Update time
		Timer += FrameTime;

		// Update replay
		Replay.Update(FrameTime);

		// Update game logic
		ObjectManager.BeginFrame();

		// Handle movement
		if(ReplayInputs)
			GetInputFromReplay();
		else
			Player->HandleInput();

		// Update physics
		Physics.Update(FrameTime);
		ObjectManager.Update(FrameTime);
		Interface.Update(FrameTime);
		Scripting.UpdateTimedCallbacks();

		// Handle end of updates
		ObjectManager.EndFrame();

		// Update audio
		glm::vec3 Position = Player->GetPosition();
		Audio.SetPosition(Position[0], Position[1], Position[2]);

		// Update camera for replay
		Camera->Update(core::vector3df(Position[0], Position[1], Position[2]));
		Camera->RecordReplay();

		// Record state for replay
		RecordPlayerSpeed();
		RecordInput();

		// Reset jump state
		Jumped = false;
	}
}

// Interpolate object positions
void _PlayState::UpdateRender(float BlendFactor) {
	if(Resetting)
		return;

	if(!IsPaused()) {
		ObjectManager.InterpolateOrientations(BlendFactor);
		glm::vec3 DrawPosition = Player->GetDrawPosition();
		Camera->Update(core::vector3df(DrawPosition[0], DrawPosition[1], DrawPosition[2]));
	}
}

// Draws the current state
void _PlayState::Draw() {

	// Draw HUD
	Interface.RenderHUD(Timer, FirstLoad);

	// Draw fps
	if(Config.ShowFPS)
		Interface.RenderFPS(irrDriver->getScreenSize().Width - 140 * Interface.GetUIScale(), 10 * Interface.GetUIScale());

	// Darken the screen
	if(IsPaused())
		Interface.FadeScreen(PAUSE_FADE_AMOUNT);

	// Draw irrlicht GUI
	Menu.Draw();
}

// Returns true if the game is paused
bool _PlayState::IsPaused() {

	return Menu.State != _Menu::STATE_NONE;
}

// Start resetting the level
void _PlayState::StartReset() {
	if(Resetting)
		return;

	Fader.Start(-FADE_SPEED);
	Resetting = true;
}

// Win the level and update stats
void _PlayState::WinLevel(bool HideNextLevel) {

	Log.Write("Won %s %fs", Level.LevelName.c_str(), PlayState.Timer);

	// Skip stats if just testing a level
	if(PlayState.TestLevel == "") {

		// Increment win count
		Save.LevelStats[Level.LevelName].WinCount++;

		// Add high score
		HighScoreIndex = Save.AddScore(Level.LevelName, PlayState.Timer);

		// Autosave replays for new records
		if(HighScoreIndex == 0)
			Replay.SaveReplay(std::string("New record for ") + Level.LevelNiceName, true, true);

		// Unlock next level
		uint32_t NewCampaign = PlayState.CurrentCampaign;
		uint32_t NewLevel = PlayState.CampaignLevel;
		if(!HideNextLevel && Campaign.GetNextLevel(NewCampaign, NewLevel, true)) {
			std::string NextLevelFile = Campaign.GetLevel(NewCampaign, NewLevel);
			Save.UnlockLevel(NextLevelFile);
		}

		// Save stats to a file
		Save.SaveLevelStats(Level.LevelName);
	}

	// Show win screen
	Menu.InitWin(HideNextLevel);
}

// Lose the level and update stats
void _PlayState::LoseLevel() {

	Log.Write("Lose %s %fs", Level.LevelName.c_str(), PlayState.Timer);

	// Skip stats if just testing a level
	if(PlayState.TestLevel == "") {

		// Increment win count
		Save.LevelStats[Level.LevelName].LoseCount++;

		// Save stats to a file
		Save.SaveLevelStats(Level.LevelName);
	}

	// Show lose screen
	Menu.InitLose();
}

// Record input
void _PlayState::RecordInput() {
	if(ReplayInputs)
		return;

	// Get input direction
	core::vector3df Push(0.0f, 0.0f, 0.0f);
	Player->GetPushDirection(Push);

	// Get camera
	float Yaw = Camera->GetYaw();
	float Pitch = Camera->GetPitch();

	// Write replay event
	std::fstream &ReplayFile = Replay.GetFile();
	Replay.WriteEvent(_Replay::PACKET_INPUT);
	ReplayFile.write((char *)&Push.X, sizeof(Push.X));
	ReplayFile.write((char *)&Push.Z, sizeof(Push.Z));
	ReplayFile.write((char *)&Yaw, sizeof(Yaw));
	ReplayFile.write((char *)&Pitch, sizeof(Pitch));
	ReplayFile.write((char *)&Jumped, sizeof(Jumped));
}

// Record player speed to replay
void _PlayState::RecordPlayerSpeed() {
	if(!Player)
		return;

	// Get speed
	float Speed = glm::length(Player->GetLinearVelocity()) + glm::length(Player->GetAngularVelocity());

	// Write replay event
	std::fstream &ReplayFile = Replay.GetFile();
	Replay.WriteEvent(_Replay::PACKET_PLAYERSPEED);
	ReplayFile.write((char *)&Speed, sizeof(Speed));
}

// Control game from replay inputs
void _PlayState::GetInputFromReplay() {
	if(!ReplayInputs)
		return;

	char Buffer[1024];
	std::fstream &ReplayFile = InputReplay->GetFile();
	while(!InputReplay->ReplayStopped() && Timer >= NextEvent.Timestamp) {
		//printf("Processing header packet: type=%d time=%f\n", NextEvent.Type, NextEvent.Timestamp);

		switch(NextEvent.Type) {
			case _Replay::PACKET_MOVEMENT: {
				int16_t ObjectCount;
				ReplayFile.read((char *)&ObjectCount, sizeof(ObjectCount));
				for(int i = 0; i < ObjectCount; i++)
					ReplayFile.read(Buffer, 2 + 4 * 3 + 4 * 3);
			} break;
			case _Replay::PACKET_CREATE: {
				ReplayFile.read(Buffer, 2 + 2);
				int PositionType = ReplayFile.get();
				if(PositionType == 1)
					ReplayFile.read(Buffer, 4 * 4);
				else
					ReplayFile.read(Buffer, 4 * 3);
				ReplayFile.read(Buffer, 4 * 3);
			} break;
			case _Replay::PACKET_DELETE:
				ReplayFile.read(Buffer, 2);
			break;
			case _Replay::PACKET_CAMERA:
				ReplayFile.read(Buffer, 4 * 3 + 4 * 3);
			break;
			case _Replay::PACKET_ORBDEACTIVATE:
				ReplayFile.read(Buffer, 2 + 4);
			break;
			case _Replay::PACKET_INPUT: {

				// Read replay
				core::vector3df Push(0.0f, 0.0f, 0.0f);
				bool Jumping;
				float Yaw;
				float Pitch;
				ReplayFile.read((char *)&Push.X, sizeof(Push.X));
				ReplayFile.read((char *)&Push.Z, sizeof(Push.Z));
				ReplayFile.read((char *)&Yaw, sizeof(Yaw));
				ReplayFile.read((char *)&Pitch, sizeof(Pitch));
				ReplayFile.read((char *)&Jumping, sizeof(Jumping));

				// Inject input
				Camera->SetYaw(Yaw);
				Camera->SetPitch(Pitch);
				Player->HandlePush(Push);
				if(Jumping)
					Player->Jump();

				//printf("t=%f x=%f z=%f yaw=%f pitch=%f jumping=%d\n", NextEvent.Timestamp, Push.X, Push.Z, Yaw, Pitch, Jumping);
			}
			break;
			case _Replay::PACKET_PLAYERSPEED:
				ReplayFile.read(Buffer, 4);
			break;
			default:
			break;
		}

		InputReplay->ReadEvent(NextEvent);
	}

	if(InputReplay->ReplayStopped()) {
		Log.Write("Validation stopped %fs", PlayState.Timer);

		Menu.InitPause();
	}
}
