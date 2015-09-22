/*************************************************************************************
*	irrlamb - https://github.com/jazztickets/irrlamb
*	Copyright (C) 2015  Alan Witkowski
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************************/
#include <null.h>
#include <engine/constants.h>
#include <engine/globals.h>
#include <engine/input.h>
#include <engine/log.h>
#include <engine/graphics.h>
#include <engine/audio.h>
#include <engine/config.h>
#include <engine/physics.h>
#include <engine/scripting.h>
#include <engine/objectmanager.h>
#include <engine/replay.h>
#include <engine/interface.h>
#include <engine/camera.h>
#include <engine/game.h>
#include <engine/level.h>
#include <engine/campaign.h>
#include <engine/fader.h>
#include <engine/actions.h>
#include <engine/save.h>
#include <objects/player.h>
#include <menu.h>
#include <viewreplay.h>

using namespace irr;

_NullState NullState;

// Initializes the state
int _NullState::Init() {
	Interface.ChangeSkin(_Interface::SKIN_MENU);

	Menu.FirstStateLoad = true;
	switch(State) {
		case _Menu::STATE_LEVELS:
			Menu.InitLevels();
		break;
		case _Menu::STATE_REPLAYS:
			Menu.InitReplays();
		break;
		default:
			Menu.InitMain();
		break;
	}

	return 1;
}

// Shuts the state down
int _NullState::Close() {

	return 1;
}

// Handle new actions
bool _NullState::HandleAction(int InputType, int Action, float Value) {

	return Menu.HandleAction(InputType, Action, Value);
}

// Key presses
bool _NullState::HandleKeyPress(int Key) {

	return Menu.HandleKeyPress(Key);
}

// GUI events
void _NullState::HandleGUI(gui::EGUI_EVENT_TYPE EventType, gui::IGUIElement *Element) {

	Menu.HandleGUI(EventType, Element);
}

// Updates the current state
void _NullState::Update(float FrameTime) {

}

// Interpolate object positions
void _NullState::UpdateRender(float TimeStepRemainder) {
}

// Draws the current state
void _NullState::Draw() {
	Menu.Draw();
}
