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
#include <actions.h>

// Classes
class _Config {

	public:

		int Init();
		int Close();

		void Reset();
		void AddDefaultActionMap(bool Force=false);
		int ReadConfig();
		int WriteConfig();

		int ReadJoystickConfig();
		int WriteJoystickConfig();

		// Video
		int DriverType;
		int ScreenWidth, ScreenHeight;
		int MaxFPS;
		bool Fullscreen;
		bool Shadows;
		bool TrilinearFiltering;
		bool Shaders;
		bool MultipleLights;
		bool Vsync;
		bool ShowFPS;
		bool ShowTutorial;
		int AnisotropicFiltering;
		int AntiAliasing;

		// Audio
		bool AudioEnabled;
		float SoundVolume, MusicVolume;

		// Input
		float MouseSensitivity, MouseScaleX, MouseScaleY;
		bool InvertMouse, InvertGamepadY;
		bool JoystickEnabled;
		int JoystickIndex;
		float DeadZone;

		// Replays
		bool AutosaveNewRecords;

	private:

};

// Singletons
extern _Config Config;
