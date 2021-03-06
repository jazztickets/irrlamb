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
#include <states/viewreplay.h>
#include <globals.h>
#include <input.h>
#include <graphics.h>
#include <config.h>
#include <physics.h>
#include <level.h>
#include <objectmanager.h>
#include <replay.h>
#include <camera.h>
#include <audio.h>
#include <framework.h>
#include <interface.h>
#include <objects/orb.h>
#include <objects/player.h>
#include <objects/template.h>
#include <font/CGUITTFont.h>
#include <states/play.h>
#include <menu.h>
#include <states/null.h>
#include <ISceneManager.h>

const float REPLAY_TIME_INCREMENT = 0.1f;

using namespace irr;

_ViewReplayState ViewReplayState;

// Initializes the state
int _ViewReplayState::Init() {
	Menu.ClearCurrentLayout();
	Layout = new CGUIEmptyElement(irrGUI, irrGUI->getRootGUIElement());
	Layout->drop();
	Camera = nullptr;
	Player = nullptr;
	FreeCamera = false;

	// Set up state
	PauseSpeed = 1.0f;
	Framework.SetTimeScale(1.0f);
	Timer = 0.0f;
	Input.SetMouseLocked(false);
	Interface.ChangeSkin(_Interface::SKIN_GAME);

	// Set up physics world
	Physics.SetEnabled(false);

	// Load replay
	if(!Replay.LoadReplay(CurrentReplay.c_str()))
		return 0;

	// Read first event
	Replay.ReadEvent(NextEvent);

	// Load the level
	if(!Level.Init(Replay.GetLevelName()))
		return 0;

	// Update number of lights
	Graphics.SetLightCount();

	// Add camera
	Camera = new _Camera();

	// Turn off graphics until camera is positioned
	Graphics.SetDrawScene(false);

	// Initialize controls
	SetupGUI();

	// Set fog background color
	Graphics.SetClearColor(Level.ClearColor);

	return 1;
}

// Shuts the state down
int _ViewReplayState::Close() {

	// Stop replay
	Replay.StopReplay();

	// Clear objects
	delete Camera;
	Level.Close();
	ObjectManager.ClearObjects();
	Interface.Clear();
	irrScene->clear();
	Layout->remove();

	return 1;
}

// Key presses
bool _ViewReplayState::HandleKeyPress(int Key) {

	bool Processed = true;
	switch(Key) {
		case KEY_ESCAPE:
			NullState.State = _Menu::STATE_REPLAYS;
			Framework.ChangeState(&NullState);
		break;
		case KEY_F1:
			NullState.State = _Menu::STATE_REPLAYS;
			Framework.ChangeState(&NullState);
		break;
		case KEY_F11:
			ShowHUD = !ShowHUD;
		break;
		case KEY_F12:
			Graphics.SaveScreenshot(Replay.GetLevelName());
		break;
		case KEY_SPACE:
			Pause();
		break;
		case KEY_RIGHT:
			Skip(1.0f);
		break;
		case KEY_KEY_1:
			Framework.SetTimeScale(0.5);
		break;
		case KEY_KEY_2:
			Framework.SetTimeScale(1.0);
		break;
		case KEY_KEY_3:
			Framework.SetTimeScale(2.0);
		break;
		case KEY_KEY_4:
			Framework.SetTimeScale(4.0);
		break;
		case KEY_KEY_5:
			Framework.SetTimeScale(8.0);
		break;
		case KEY_UP:
			ChangeReplaySpeed(GetTimeIncrement());
		break;
		case KEY_DOWN:
			ChangeReplaySpeed(-GetTimeIncrement());
		break;
		default:
			Processed = false;
		break;
	}

	return Processed;
}

// Mouse press
bool _ViewReplayState::HandleMousePress(int Button, int MouseX, int MouseY) {

	// RMB enables free camera mode
	if(Button == 1) {
		FreeCamera = true;
		Input.SetMouseLocked(true);
	}

	return false;
}

// Mouse lift
void _ViewReplayState::HandleMouseLift(int Button, int MouseX, int MouseY) {
	if(Button == 1) {
		FreeCamera = false;
		Input.SetMouseLocked(false);
	}
}

// Handle action inputs
bool _ViewReplayState::HandleAction(int InputType, int Action, float Value) {
	if(Input.HasJoystick())
		Input.DriveMouse(Action, Value);

	if(FreeCamera) {
		switch(Action) {
			case _Actions::CAMERA_LEFT:
				if(Camera) {
					if(InputType == _Input::JOYSTICK_AXIS)
						Value *= Framework.GetLastFrameTime();
					Camera->HandleMouseMotion(-Value, 0);
				}
			break;
			case _Actions::CAMERA_RIGHT:
				if(Camera) {
					if(InputType == _Input::JOYSTICK_AXIS)
						Value *= Framework.GetLastFrameTime();
					Camera->HandleMouseMotion(Value, 0);
				}
			break;
			case _Actions::CAMERA_UP:
				if(Camera) {
					if(InputType == _Input::JOYSTICK_AXIS)
						Value *= Framework.GetLastFrameTime();
					Camera->HandleMouseMotion(0, -Value);
				}
			break;
			case _Actions::CAMERA_DOWN:
				if(Camera) {
					if(InputType == _Input::JOYSTICK_AXIS)
						Value *= Framework.GetLastFrameTime();
					Camera->HandleMouseMotion(0, Value);
				}
			break;
		}
	}

	return false;
}

// Mouse wheel
void _ViewReplayState::HandleMouseWheel(float Direction) {
	if(Direction < 0)
		ChangeReplaySpeed(-GetTimeIncrement());
	else
		ChangeReplaySpeed(GetTimeIncrement());
}

// GUI events
void _ViewReplayState::HandleGUI(irr::gui::EGUI_EVENT_TYPE EventType, gui::IGUIElement *Element) {

	switch(EventType) {
		case gui::EGET_BUTTON_CLICKED:
			switch(Element->getID()) {
				case MAIN_EXIT:
					NullState.State = _Menu::STATE_REPLAYS;
					Framework.ChangeState(&NullState);
				break;
				case MAIN_DECREASE:
					ChangeReplaySpeed(-GetTimeIncrement());
				break;
				case MAIN_INCREASE:
					ChangeReplaySpeed(GetTimeIncrement());
				break;
				case MAIN_PAUSE:
					Pause();
				break;
				case MAIN_RESTART:
					Framework.ChangeState(this);
				break;
				case MAIN_SKIP:
					Skip(1.0f);
				break;
			}
		break;
		default:
		break;
	}
}

// Updates the current state
void _ViewReplayState::Update(float FrameTime) {

	// Update the replay
	Timer += FrameTime;
	while(!Replay.ReplayStopped() && Timer >= NextEvent.Timestamp) {
		//printf("Processing header packet: type=%d time=%f\n", NextEvent.Type, NextEvent.Timestamp);

		switch(NextEvent.Type) {
			case _Replay::PACKET_MOVEMENT:
				ObjectManager.UpdateFromReplay();
			break;
			case _Replay::PACKET_CREATE: {
				_ObjectSpawn Spawn;

				// Read replay
				std::fstream &ReplayFile = Replay.GetFile();

				// Get template
				int16_t TemplateID;
				ReplayFile.read((char *)&TemplateID, sizeof(TemplateID));
				Spawn.Template = Level.GetTemplateFromID(TemplateID);

				// Get object id
				int16_t ObjectID;
				ReplayFile.read((char *)&ObjectID, sizeof(ObjectID));

				// Get orientation
				int PositionType = ReplayFile.get();
				if(PositionType == 1)
					ReplayFile.read((char *)&Spawn.Plane, sizeof(float) * 4);
				else
					ReplayFile.read((char *)&Spawn.Position, sizeof(float) * 3);
				ReplayFile.read((char *)&Spawn.Rotation, sizeof(float) * 3);

				// Create spawn object
				if(Spawn.Template != nullptr) {
					_Object *NewObject = Level.CreateObject(Spawn);
					NewObject->SetID(ObjectID);

					// Get player
					if(NewObject->GetType() == _Object::PLAYER)
						Player = (_Player *)NewObject;
				}

				// Update number of lights
				Graphics.SetLightCount();
			}
			break;
			case _Replay::PACKET_DELETE: {

				// Read replay
				std::fstream &ReplayFile = Replay.GetFile();
				int16_t ObjectID;
				ReplayFile.read((char *)&ObjectID, sizeof(ObjectID));

				// Delete object
				ObjectManager.DeleteObjectByID(ObjectID);
			}
			break;
			case _Replay::PACKET_CAMERA: {

				// Read replay
				core::vector3df Position, LookAt;
				std::fstream &ReplayFile = Replay.GetFile();
				ReplayFile.read((char *)&Position.X, sizeof(float) * 3);
				ReplayFile.read((char *)&LookAt.X, sizeof(float) * 3);

				// Update audio
				Audio.SetPosition(LookAt.X, LookAt.Y, LookAt.Z);

				// Set camera orientation
				if(!FreeCamera) {
					Camera->GetNode()->setPosition(Position);
					Camera->GetNode()->setTarget(LookAt);

					// Set yaw and pitch of free camera
					core::vector3df Direction = (Position - LookAt).normalize();
					Camera->SetYaw(-std::atan2(Direction.X, -Direction.Z) * core::RADTODEG);
					Camera->SetPitch(std::asin(Direction.Y) * core::RADTODEG);
				}
				Graphics.SetDrawScene(true);

				//printf("Camera Position=%f %f %f Target=%f %f %f\n", Position.X, Position.Y, Position.Z, LookAt.X, LookAt.Y, LookAt.Z);
			}
			break;
			case _Replay::PACKET_ORBDEACTIVATE: {

				// Read replay
				std::fstream &ReplayFile = Replay.GetFile();
				int16_t ObjectID;
				float Length;
				ReplayFile.read((char *)&ObjectID, sizeof(ObjectID));
				ReplayFile.read((char *)&Length, sizeof(Length));

				// Deactivate orb
				_Orb *Orb = static_cast<_Orb *>(ObjectManager.GetObjectByID(ObjectID));
				Orb->StartDeactivation("", Length);

				// Update number of lights
				Graphics.SetLightCount();
			}
			break;
			case _Replay::PACKET_INPUT: {
				core::vector3df Push(0.0f, 0.0f, 0.0f);
				bool Jumping;
				float Yaw;
				float Pitch;

				// Read replay
				std::fstream &ReplayFile = Replay.GetFile();
				ReplayFile.read((char *)&Push.X, sizeof(Push.X));
				ReplayFile.read((char *)&Push.Z, sizeof(Push.Z));
				ReplayFile.read((char *)&Yaw, sizeof(Yaw));
				ReplayFile.read((char *)&Pitch, sizeof(Pitch));
				ReplayFile.read((char *)&Jumping, sizeof(Jumping));

				//printf("x=%f z=%f yaw=%f pitch=%f jumping=%d\n", Push.X, Push.Z, Yaw, Pitch, Jumping);
			}
			break;
			case _Replay::PACKET_PLAYERSPEED: {

				// Read replay
				float Speed;
				std::fstream &ReplayFile = Replay.GetFile();
				ReplayFile.read((char *)&Speed, sizeof(Speed));

				// Update player audio
				if(Player) {
					core::vector3df Position = Player->GetNode()->getPosition();
					Player->UpdateAudio(glm::vec3(Position.X, Position.Y, Position.Z), Speed);
				}
			}
			break;
			default:
			break;
		}

		Replay.ReadEvent(NextEvent);
	}

	ObjectManager.UpdateReplay(FrameTime);
	Interface.Update(FrameTime);
}

// Draws the current state
void _ViewReplayState::Draw() {
	if(FreeCamera && Player)
		Camera->Update(Player->GetNode()->getPosition());

	if(!ShowHUD)
		return;

	// Draw box
	char Buffer[256];
	int Left = 10 * Interface.GetUIScale();
	int Top = 10 * Interface.GetUIScale();
	int Width = 460 * Interface.GetUIScale();
	int Height = 150 * Interface.GetUIScale();
	Interface.DrawTextBox(Left + Width/2, Top + Height/2, Width, Height, video::SColor(150, 255, 255, 255));

	// Draw timer
	float DisplayTime;
	if(!Replay.ReplayStopped())
		DisplayTime = Timer;
	else
		DisplayTime = Replay.GetFinishTime();

	// Draw time
	int X = Left + Width/2 - 20 * Interface.GetUIScale();
	int Y = Top + 15 * Interface.GetUIScale();
	int Padding = 16 * Interface.GetUIScale();
	Interface.RenderText("Time", X - Padding, Y, _Interface::ALIGN_RIGHT, _Interface::FONT_MEDIUM);
	Interface.ConvertSecondsToString(DisplayTime, Buffer);
	Interface.RenderText(Buffer, X + Padding, Y, _Interface::ALIGN_LEFT, _Interface::FONT_MEDIUM);

	// Draw current speed
	Y += 60 * Interface.GetUIScale();
	Interface.RenderText("Speed", X - Padding, Y, _Interface::ALIGN_RIGHT, _Interface::FONT_MEDIUM);
	sprintf(Buffer, "%.2f", Framework.GetTimeScale());
	Interface.RenderText(Buffer, X + Padding , Y, _Interface::ALIGN_LEFT, _Interface::FONT_MEDIUM);

	// Draw fps
	if(Config.ShowFPS)
		Interface.RenderFPS(10 * Interface.GetUIScale(), irrDriver->getScreenSize().Height - 50 * Interface.GetUIScale());

	// Draw buttons
	irrGUI->drawAll();
}

// Setup GUI controls
void _ViewReplayState::SetupGUI() {
	float Padding = 74;

	// Exit
	gui::IGUIButton *ButtonExit = irrGUI->addButton(Interface.GetRightRectPercent(0.994, 0.05, BUTTON_SMALL_SIZE_X, BUTTON_SMALL_SIZE_Y), Layout, MAIN_EXIT, L"Exit");
	ButtonExit->setImage(Interface.Images[_Interface::IMAGE_BUTTON_SMALL]);
	ButtonExit->setUseAlphaChannel(true);
	ButtonExit->setDrawBorder(false);
	ButtonExit->setOverrideFont(Interface.Fonts[_Interface::FONT_BUTTON]);
	ButtonExit->setScaleImage(true);

	irr::core::recti Bounds = Interface.GetRightRectPercent(0.994, 0.05, BUTTON_ICON_SIZE, BUTTON_ICON_SIZE);

	// Skip ahead
	Bounds.UpperLeftCorner.X -= (Padding + 180) * Interface.GetUIScale();
	Bounds.LowerRightCorner.X -= (Padding + 180) * Interface.GetUIScale();
	gui::IGUIButton *ButtonSkip = irrGUI->addButton(Bounds, Layout, MAIN_SKIP);
	ButtonSkip->setImage(Interface.Images[_Interface::IMAGE_FASTFORWARD]);
	ButtonSkip->setUseAlphaChannel(true);
	ButtonSkip->setDrawBorder(false);
	ButtonSkip->setScaleImage(true);

	// Pause
	Bounds.UpperLeftCorner.X -= Padding * Interface.GetUIScale();
	Bounds.LowerRightCorner.X -= Padding * Interface.GetUIScale();
	gui::IGUIButton *ButtonPause = irrGUI->addButton(Bounds, Layout, MAIN_PAUSE);
	ButtonPause->setImage(Interface.Images[_Interface::IMAGE_PAUSE]);
	ButtonPause->setUseAlphaChannel(true);
	ButtonPause->setDrawBorder(false);
	ButtonPause->setScaleImage(true);

	// Restart replay
	Bounds.UpperLeftCorner.X -= Padding * Interface.GetUIScale();
	Bounds.LowerRightCorner.X -= Padding * Interface.GetUIScale();
	gui::IGUIButton *ButtonRewind = irrGUI->addButton(Bounds, Layout, MAIN_RESTART);
	ButtonRewind->setImage(Interface.Images[_Interface::IMAGE_REWIND]);
	ButtonRewind->setUseAlphaChannel(true);
	ButtonRewind->setDrawBorder(false);
	ButtonRewind->setScaleImage(true);

	// Increase replay speed
	Bounds.UpperLeftCorner.X -= Padding * Interface.GetUIScale();
	Bounds.LowerRightCorner.X -= Padding * Interface.GetUIScale();
	gui::IGUIButton *ButtonIncrease = irrGUI->addButton(Bounds, Layout, MAIN_INCREASE);
	ButtonIncrease->setImage(Interface.Images[_Interface::IMAGE_INCREASE]);
	ButtonIncrease->setUseAlphaChannel(true);
	ButtonIncrease->setDrawBorder(false);
	ButtonIncrease->setScaleImage(true);

	// Decrease replay speed
	Bounds.UpperLeftCorner.X -= Padding * Interface.GetUIScale();
	Bounds.LowerRightCorner.X -= Padding * Interface.GetUIScale();
	gui::IGUIButton *ButtonDecrease = irrGUI->addButton(Bounds, Layout, MAIN_DECREASE);
	ButtonDecrease->setImage(Interface.Images[_Interface::IMAGE_DECREASE]);
	ButtonDecrease->setUseAlphaChannel(true);
	ButtonDecrease->setDrawBorder(false);
	ButtonDecrease->setScaleImage(true);
}

// Change replay speed
void _ViewReplayState::ChangeReplaySpeed(float Amount) {
	float Scale = Framework.GetTimeScale();

	Scale += Amount;
	if(Scale >= 10.0f)
		Scale = 10.0f;
	else if(Scale <= 0.0f)
		Scale = 0.0f;

	Framework.SetTimeScale(Scale);
}

// Pause the replay
void _ViewReplayState::Pause() {

	// Pause or play
	if(Framework.GetTimeScale() == 0.0f) {
		Framework.SetTimeScale(PauseSpeed);
	}
	else {
		PauseSpeed = Framework.GetTimeScale();
		Framework.SetTimeScale(0.0f);
	}
}

// Skip ahead
void _ViewReplayState::Skip(float Amount) {
	Framework.UpdateTimeStepAccumulator(Amount);
}

// Get how much time to adjust replay speed
float _ViewReplayState::GetTimeIncrement() {
	float Scale = REPLAY_TIME_INCREMENT;
	if(Input.GetKeyState(KEY_LSHIFT))
		Scale *= 0.1f;

	return Scale;
}
