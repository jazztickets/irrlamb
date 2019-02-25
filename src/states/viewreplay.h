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
#pragma once

// Libraries
#include <state.h>
#include <replay.h>
#include <vector3d.h>

// Forward Declarations
class _Object;
class _Player;
class _Camera;

// Classes
class _ViewReplayState : public _State {

	public:

		enum GUIElements {
			MAIN_RESTART,
			MAIN_PAUSE,
			MAIN_SKIP,
			MAIN_INCREASE,
			MAIN_DECREASE,
			MAIN_EXIT,
		};

		_ViewReplayState() : ShowHUD(true) {}

		int Init();
		int Close();

		bool HandleKeyPress(int Key);
		bool HandleMousePress(int Button, int MouseX, int MouseY);
		void HandleMouseLift(int Button, int MouseX, int MouseY);
		void HandleMouseWheel(float Direction);
		void HandleGUI(irr::gui::EGUI_EVENT_TYPE EventType, irr::gui::IGUIElement *Element);
		bool HandleAction(int InputType, int Action, float Value);

		void Update(float FrameTime);
		void Draw();

		void SetCurrentReplay(const std::string &File) { CurrentReplay = File; }

	private:

		void SetupGUI();
		void ChangeReplaySpeed(float Amount);
		void Pause();
		void Skip(float Amount);
		float GetTimeIncrement();

		// States
		std::string CurrentReplay;
		float Timer;
		bool ShowHUD;
		bool FreeCamera;

		// Objects
		_Camera *Camera;
		irr::core::vector3df PlayerPosition;

		// Replay information
		_ReplayEvent NextEvent;
		float PauseSpeed;

		// Events
		int NextPacketType;

		// GUI
		irr::gui::IGUIElement *Layout;
};

extern _ViewReplayState ViewReplayState;
