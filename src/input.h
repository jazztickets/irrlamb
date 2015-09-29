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
#include <string>
#include <IEventReceiver.h>
#include <irrArray.h>

// Classes
class _Input : public irr::IEventReceiver  {

	public:

		enum MouseButtonType {
			MOUSE_LEFT,
			MOUSE_RIGHT,
			MOUSE_MIDDLE,
			MOUSE_COUNT,
		};

		enum InputType {
			KEYBOARD,
			MOUSE_BUTTON,
			MOUSE_AXIS,
			JOYSTICK_BUTTON,
			JOYSTICK_AXIS,
			INPUT_COUNT,
		};

		_Input();
		bool OnEvent(const irr::SEvent &Event);

		void ResetInputState();
		void InitializeJoysticks(bool ShowLog=false);
		bool GetKeyState(int Key) const { return Keys[Key]; }
		bool GetMouseState(int Button) const { return MouseButtons[Button]; }

		void SetMouseLocked(bool Value);
		bool GetMouseLocked() const { return MouseLocked; }

		float GetMouseX() const { return MouseX; }
		float GetMouseY() const { return MouseY; }
		void SetMouseX(float Value) { MouseX = Value; }
		void SetMouseY(float Value) { MouseY = Value; }

		bool HasJoystick() const { return Joysticks.size() > 0; }
		uint32_t GetJoystickCount() const { return Joysticks.size(); }
		const irr::SEvent::SJoystickEvent &GetJoystickState();
		const irr::SJoystickInfo &GetJoystickInfo(int Index=0);
		float GetAxis(int Axis);
		void DriveMouse(int Action, float Value);
		irr::core::stringc GetCleanJoystickName(uint32_t Index=0);

		const char *GetKeyName(int Key);

	private:

		void SetKeyState(int Key, bool State) { Keys[Key] = State; }
		void SetMouseState(int Button, bool State) { MouseButtons[Button] = State; }

		// Input
		bool Keys[irr::KEY_KEY_CODES_COUNT], MouseButtons[MOUSE_COUNT];

		// Joystick
		irr::core::array<irr::SJoystickInfo> Joysticks;
		irr::SEvent::SJoystickEvent JoystickState;
		uint32_t LastJoystickButtonState;

		// States
		bool MouseLocked;
		float MouseX, MouseY;
		bool VirtualMouseMoved;
};

// Singletons
extern _Input Input;
