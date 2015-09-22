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
#include <engine/actions.h>
#include <engine/game.h>
#include <engine/state.h>
#include <engine/config.h>
#include <tinyxml/tinyxml2.h>

_Actions Actions;

using namespace tinyxml2;

// Constructor
_Actions::_Actions() {
	ResetState();

	Names[MOVE_LEFT] = "Move Left";
	Names[MOVE_RIGHT] = "Move Right";
	Names[MOVE_FORWARD] = "Move Forward";
	Names[MOVE_BACK] = "Move Back";
	Names[JUMP] = "Jump";
	Names[RESET] = "Restart Level";
	Names[CAMERA_LEFT] = "Camera Left";
	Names[CAMERA_RIGHT] = "Camera Right";
	Names[CAMERA_UP] = "Camera Up";
	Names[CAMERA_DOWN] = "Camera Down";
	Names[MENU_LEFT] = "Menu Left";
	Names[MENU_RIGHT] = "Menu Right";
	Names[MENU_UP] = "Menu Up";
	Names[MENU_DOWN] = "Menu Down";
	Names[MENU_GO] = "Menu Go";
	Names[MENU_BACK] = "Menu Back";
	Names[MENU_PAUSE] = "Pause";
	Names[CURSOR_LEFT] = "Cursor Left";
	Names[CURSOR_RIGHT] = "Cursor Right";
	Names[CURSOR_UP] = "Cursor Up";
	Names[CURSOR_DOWN] = "Cursor Down";
}

// Reset the state
void _Actions::ResetState() {
	for(int i = 0; i < COUNT; i++) {
		State[i].Value = 0.0f;
		State[i].Source = -1;
	}
}

// Clear all mappings
void _Actions::ClearMappings(int InputType) {
	for(int i = 0; i < ACTIONS_MAXINPUTS; i++)
		InputMap[InputType][i].clear();
}

// Remove a mapping for an action
void _Actions::ClearMappingsForAction(int InputType, int Action) {
	for(int i = 0; i < ACTIONS_MAXINPUTS; i++) {
		for(std::list<_ActionMap>::iterator MapIterator = InputMap[InputType][i].begin(); MapIterator != InputMap[InputType][i].end(); ) {
			if(MapIterator->Action == Action) {
				MapIterator = InputMap[InputType][i].erase(MapIterator);
			}
			else
				++MapIterator;
		}
	}
}

// Get action
float _Actions::GetState(int Action) {
	if(Action < 0 || Action >= COUNT)
		return 0.0f;

	return State[Action].Value;
}

// Add an input mapping
void _Actions::AddInputMap(int InputType, int Input, int Action, float Scale, bool IfNone) {
	if(Action < 0 || Action >= COUNT || Input < 0 || Input >= ACTIONS_MAXINPUTS)
		return;

	if(!IfNone || (IfNone && GetInputForAction(InputType, Action) == -1)) {
		InputMap[InputType][Input].push_back(_ActionMap(Action, Scale));
	}
}

// Returns the first input for an action
int _Actions::GetInputForAction(int InputType, int Action) {
	for(int i = 0; i < ACTIONS_MAXINPUTS; i++) {
		for(std::list<_ActionMap>::iterator MapIterator = InputMap[InputType][i].begin(); MapIterator != InputMap[InputType][i].end(); MapIterator++) {
			if(MapIterator->Action == Action) {
				return i;
			}
		}
	}

	return -1;
}

// Inject an input into the action handler
void _Actions::InputEvent(int InputType, int Input, float Value) {
	if(Input < 0 || Input >= ACTIONS_MAXINPUTS)
		return;

	for(std::list<_ActionMap>::iterator MapIterator = InputMap[InputType][Input].begin(); MapIterator != InputMap[InputType][Input].end(); MapIterator++) {

		// Only let joystick overwrite action state if the keyboard isn't being used
		if(InputType != _Input::JOYSTICK_AXIS || (InputType == _Input::JOYSTICK_AXIS && (State[MapIterator->Action].Source == -1 || State[MapIterator->Action].Source == _Input::JOYSTICK_AXIS))) {

			// If key was released, set source to -1 so that joystick can overwrite it
			if(InputType == _Input::KEYBOARD && Value == 0.0f)
				State[MapIterator->Action].Source = -1;
			else
				State[MapIterator->Action].Source = InputType;

			State[MapIterator->Action].Value = Value;
		}

		// Apply input scale to action
		float InputValue = Value * MapIterator->Scale;

		// Invert gamepad camera Y
		if(Config.InvertGamepadY && InputType == _Input::JOYSTICK_AXIS && (MapIterator->Action == _Actions::CAMERA_UP || MapIterator->Action == _Actions::CAMERA_DOWN)) {
			InputValue = -InputValue;
		}

		// If true is returned, stop handling the same key
		if(Game.GetState()->HandleAction(InputType, MapIterator->Action, InputValue))
			break;
	}
}

// Write to config file
void _Actions::Serialize(int InputType, XMLDocument &Document, XMLElement *InputElement) {
	for(int i = 0; i < ACTIONS_MAXINPUTS; i++) {
		for(std::list<_ActionMap>::iterator MapIterator = InputMap[InputType][i].begin(); MapIterator != InputMap[InputType][i].end(); MapIterator++) {

			// Insert action name
			XMLComment *Comment = Document.NewComment(Names[MapIterator->Action].c_str());
			InputElement->InsertEndChild(Comment);

			// Add input map line
			XMLElement *Element = Document.NewElement("map");
			Element->SetAttribute("type", InputType);
			Element->SetAttribute("input", i);
			Element->SetAttribute("action", MapIterator->Action);
			Element->SetAttribute("scale", MapIterator->Scale);
			InputElement->InsertEndChild(Element);
		}
	}
}

// Unserialize
void _Actions::Unserialize(XMLElement *InputElement) {
	int Type, Input, Action;
	float Scale;

	// Get input mapping
	for(XMLElement *Element = InputElement->FirstChildElement("map"); Element != 0; Element = Element->NextSiblingElement("map")) {
		if(Element->QueryIntAttribute("type", &Type) != XML_NO_ERROR
			|| Element->QueryIntAttribute("input", &Input) != XML_NO_ERROR
			|| Element->QueryIntAttribute("action", &Action) != XML_NO_ERROR
			|| Element->QueryFloatAttribute("scale", &Scale) != XML_NO_ERROR)
			continue;

		Actions.AddInputMap(Type, Input, Action, Scale, false);
	}
}
