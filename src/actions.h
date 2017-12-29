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
#include <input.h>
#include <string>
#include <list>

// Constants
const int ACTIONS_MAXINPUTS = 256;
const float ACTIONS_SCALE = 1.0f;
const float ACTIONS_DEADZONE = 0.05f;

// Forward declarations
namespace tinyxml2 {
	class XMLDocument;
	class XMLElement;
}

struct _ActionMap {
	_ActionMap(int Action, float Scale, float DeadZone) : Action(Action), DeadZone(DeadZone), Scale(Scale) { }

	int Action;
	float DeadZone;
	float Scale;
};

struct _ActionState {
	float Value;
	int Source;
};

// Handles actions
class _Actions {

	public:

		enum Types {
			MOVE_LEFT,
			MOVE_RIGHT,
			MOVE_FORWARD,
			MOVE_BACK,
			JUMP,
			RESET,
			CAMERA_LEFT,
			CAMERA_RIGHT,
			CAMERA_UP,
			CAMERA_DOWN,
			MENU_LEFT,
			MENU_RIGHT,
			MENU_UP,
			MENU_DOWN,
			MENU_GO,
			MENU_BACK,
			MENU_PAUSE,
			CURSOR_LEFT,
			CURSOR_RIGHT,
			CURSOR_UP,
			CURSOR_DOWN,
			MENU_PAGEUP,
			MENU_PAGEDOWN,
			COUNT,
		};

		_Actions();

		void ResetState();
		void ClearMappings(int InputType);
		void ClearMappingsForAction(int InputType, int Action);

		// Actions
		float GetState(int Action);
		const std::string &GetName(int Action) { return Names[Action]; }

		// Maps
		void AddInputMap(int InputType, int Input, int Action, float Scale=1.0f, float DeadZone=-1.0f, bool IfNone=true);
		int GetInputForAction(int InputType, int Action);

		// Handlers
		void InputEvent(int InputType, int Input, float Value);

		// Config
		void Serialize(int InputType, tinyxml2::XMLDocument &Document, tinyxml2::XMLElement *InputElement);
		void Unserialize(tinyxml2::XMLElement *InputElement, float MasterDeadZone);

	private:

		std::list<_ActionMap> InputMap[_Input::INPUT_COUNT][ACTIONS_MAXINPUTS];

		_ActionState State[COUNT];
		std::string Names[COUNT];
};

extern _Actions Actions;
