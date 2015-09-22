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
#pragma once

// Libraries
#include <engine/state.h>
#include <string>

// Forward Declarations
class _Object;
class _Player;
class _Camera;

// Classes
class _PlayState : public _State {
	friend class _Menu;

	public:

		int Init();
		int Close();

		bool HandleAction(int InputType, int Action, float Value);
		bool HandleKeyPress(int Key);
		bool HandleMousePress(int Button, int MouseX, int MouseY);
		void HandleMouseLift(int Button, int MouseX, int MouseY);
		void HandleMouseWheel(float Direction);
		void HandleGUI(irr::gui::EGUI_EVENT_TYPE EventType, irr::gui::IGUIElement *Element);

		void Update(float FrameTime);
		void UpdateRender(float TimeStepRemainder);
		void Draw();

		bool IsPaused();
		void StartReset();
		void ResetLevel();
		void WinLevel();
		void LoseLevel();

		void SetTestLevel(const std::string &Level) { TestLevel = Level; }
		void SetCampaign(int Value) { CurrentCampaign = Value; }
		void SetCampaignLevel(int Value) { CampaignLevel = Value; }

		_Camera *GetCamera() { return Camera; }
		float GetTimer() { return Timer; }

	private:

		// States
		std::string TestLevel;
		float Timer;
		bool Resetting;

		// Campaign
		int CurrentCampaign, CampaignLevel;

		// Objects
		_Player *Player;
		_Camera *Camera;
};

extern _PlayState PlayState;
