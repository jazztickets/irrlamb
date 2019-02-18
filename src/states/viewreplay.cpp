/******************************************************************************
* irrlamb - https://github.com/jazztickets/irrlamb
* Copyright (C) 2015  Alan Witkowski
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
#include <game.h>
#include <interface.h>
#include <constants.h>
#include <objects/orb.h>
#include <objects/template.h>
#include <font/CGUITTFont.h>
#include <states/play.h>
#include <menu.h>
#include <states/null.h>
#include <ISceneManager.h>

using namespace irr;

_ViewReplayState ViewReplayState;

// Initializes the state
int _ViewReplayState::Init() {
	Menu.ClearCurrentLayout();
	Layout = new CGUIEmptyElement(irrGUI, irrGUI->getRootGUIElement());
	Layout->drop();
	Camera = nullptr;

	// Set up state
	PauseSpeed = 1.0f;
	Game.SetTimeScale(1.0f);
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
			Game.ChangeState(&NullState);
		break;
		case KEY_F1:
			NullState.State = _Menu::STATE_REPLAYS;
			Game.ChangeState(&NullState);
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
		case KEY_UP:
			ChangeReplaySpeed(REPLAY_TIME_INCREMENT);
		break;
		case KEY_DOWN:
			ChangeReplaySpeed(-REPLAY_TIME_INCREMENT);
		break;
		default:
			Processed = false;
		break;
	}

	return Processed;
}

// Handle action inputs
bool _ViewReplayState::HandleAction(int InputType, int Action, float Value) {
	if(Input.HasJoystick())
		Input.DriveMouse(Action, Value);

	return false;
}

// Mouse wheel
void _ViewReplayState::HandleMouseWheel(float Direction) {

	if(Direction < 0)
		ChangeReplaySpeed(-REPLAY_TIME_INCREMENT);
	else
		ChangeReplaySpeed(REPLAY_TIME_INCREMENT);
}

// GUI events
void _ViewReplayState::HandleGUI(irr::gui::EGUI_EVENT_TYPE EventType, gui::IGUIElement *Element) {

	switch(EventType) {
		case gui::EGET_BUTTON_CLICKED:
			switch(Element->getID()) {
				case MAIN_EXIT:
					NullState.State = _Menu::STATE_REPLAYS;
					Game.ChangeState(&NullState);
				break;
				case MAIN_DECREASE:
					ChangeReplaySpeed(-REPLAY_TIME_INCREMENT);
				break;
				case MAIN_INCREASE:
					ChangeReplaySpeed(REPLAY_TIME_INCREMENT);
				break;
				case MAIN_PAUSE:
					Pause();
				break;
				case MAIN_RESTART:
					Game.ChangeState(this);
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
	while(!Replay.ReplayStopped() && Timer >= NextEvent.TimeStamp) {
		//printf("Processing header packet: type=%d time=%f\n", NextEvent.Type, NextEvent.TimeStamp);

		switch(NextEvent.Type) {
			case _Replay::PACKET_MOVEMENT:
				ObjectManager.UpdateFromReplay();
			break;
			case _Replay::PACKET_CREATE: {
				_ObjectSpawn Spawn;

				// Read replay
				std::fstream &ReplayFile = Replay.GetFile();
				int16_t TemplateID;
				int16_t ObjectID;
				ReplayFile.read((char *)&TemplateID, sizeof(TemplateID));
				ReplayFile.read((char *)&ObjectID, sizeof(ObjectID));
				ReplayFile.read((char *)&Spawn.Position, sizeof(btScalar) * 3);
				ReplayFile.read((char *)&Spawn.Rotation, sizeof(btScalar) * 3);

				// Create spawn object
				Spawn.Template = Level.GetTemplateFromID(TemplateID);
				if(Spawn.Template != nullptr) {
					_Object *NewObject = Level.CreateObject(Spawn);
					NewObject->SetID(ObjectID);
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

				// Set camera orientation
				Camera->GetNode()->setPosition(Position);
				Camera->GetNode()->setTarget(LookAt);
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
	char Buffer[256];

	// Draw box
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
	sprintf(Buffer, "%.2f", Game.GetTimeScale());
	Interface.RenderText(Buffer, X + Padding , Y, _Interface::ALIGN_LEFT, _Interface::FONT_MEDIUM);

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
	float Scale = Game.GetTimeScale();

	Scale += Amount;
	if(Scale >= 10.0f)
		Scale = 10.0f;
	else if(Scale <= 0.0f)
		Scale = 0.0f;

	Game.SetTimeScale(Scale);
}

// Pause the replay
void _ViewReplayState::Pause() {

	// Pause or play
	if(Game.GetTimeScale() == 0.0f) {
		Game.SetTimeScale(PauseSpeed);
	}
	else {
		PauseSpeed = Game.GetTimeScale();
		Game.SetTimeScale(0.0f);
	}
}

// Skip ahead
void _ViewReplayState::Skip(float Amount) {
	Game.UpdateTimeStepAccumulator(Amount);
}
