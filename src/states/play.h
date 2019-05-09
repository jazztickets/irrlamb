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
#include <string>

// Forward Declarations
class _Object;
class _Player;
class _Camera;

// Classes
class _PlayState : public _State {
	friend class _Menu;

	public:

		_PlayState() : CurrentCampaign(0), CampaignLevel(0), ReplayInputs(false) { }

		int Init();
		int Close();

		bool HandleAction(int InputType, int Action, float Value);
		bool HandleKeyPress(int Key);
		bool HandleMousePress(int Button, int MouseX, int MouseY);
		void HandleMouseLift(int Button, int MouseX, int MouseY);
		void HandleMouseWheel(float Direction);
		void HandleGUI(irr::gui::EGUI_EVENT_TYPE EventType, irr::gui::IGUIElement *Element);

		void Update(float FrameTime);
		void UpdateRender(float BlendFactor);
		void Draw();

		bool IsPaused();
		void StartReset();
		void ResetLevel();
		void WinLevel(bool HideNextLevel=false);
		void LoseLevel();

		void SetTestLevel(const std::string &Level) { TestLevel = Level; }
		void SetValidateReplay(const std::string &Replay) { InputReplayFilename = Replay; ReplayInputs = Replay != ""; }
		void SetCampaign(int Value) { CurrentCampaign = Value; }
		void SetCampaignLevel(int Value) { CampaignLevel = Value; }

		_Camera *GetCamera() { return Camera; }
		float GetTimer() { return Timer; }

	private:

		// Replays
		void RecordInput();
		void RecordPlayerSpeed();
		void GetInputFromReplay();

		// States
		std::string TestLevel;
		float Timer;
		int HighScoreIndex;
		bool FirstLoad;
		bool Resetting;
		bool Jumped;

		// Campaign
		uint32_t CurrentCampaign, CampaignLevel;

		// Objects
		_Player *Player;
		_Camera *Camera;

		// Replays
		std::string InputReplayFilename;
		bool ReplayInputs;
		_Replay *InputReplay;
		_ReplayEvent NextEvent;
};

extern _PlayState PlayState;
