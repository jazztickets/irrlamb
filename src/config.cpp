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
#include <config.h>
#include <tinyxml2.h>
#include <globals.h>
#include <save.h>
#include <input.h>
#include <constants.h>

using namespace irr;

_Config Config;

using namespace tinyxml2;

// Initializes the config system
int _Config::Init() {

	Reset();

	return 1;
}

// Closes the config system
int _Config::Close() {

	return 1;
}

// Resets the configuration to the default values
void _Config::Reset() {

	// Video
	DriverType = video::EDT_OPENGL;
	ScreenWidth = 800;
	ScreenHeight = 600;
	Fullscreen = false;
	Shadows = true;
	MultipleLights = true;
	Shaders = true;
	TrilinearFiltering = true;
	AnisotropicFiltering = 0;
	AntiAliasing = 0;
	Vsync = false;
	MaxFPS = MAXFPS;
	ShowFPS = false;
	Caching = false;
	ShowTutorial = true;

	// Audio
	SoundVolume = 1.0;

	// Input
	JoystickEnabled = true;
	JoystickIndex = -1;

	MouseScaleX = 1.0f;
	MouseScaleY = 1.0f;
	MouseSensitivity = 1.0f;

	InvertMouse = false;
	InvertGamepadY = false;
	DeadZone = ACTIONS_DEADZONE;

	// Replays
	AutosaveNewRecords = true;

#ifdef PANDORA
	DriverType = EDT_OGLES1;
	ScreenHeight = 480;
	Fullscreen = true;
	Shaders = false;
	TrilinearFiltering = false;
	MouseScaleX = 5.0f;
	MouseScaleY = 5.0f;
#endif
}

// Add default actions
void _Config::AddDefaultActionMap(bool Force) {

	if(Force) {
		for(int i = 0; i < _Input::INPUT_COUNT; i++)
			Actions.ClearMappings(i);
	}

	// Add keyboard
#ifdef PANDORA
	Actions.AddInputMap(_Input::KEYBOARD, KEY_UP, _Actions::MOVE_FORWARD);
	Actions.AddInputMap(_Input::KEYBOARD, KEY_DOWN, _Actions::MOVE_BACK);
	Actions.AddInputMap(_Input::KEYBOARD, KEY_LEFT, _Actions::MOVE_LEFT);
	Actions.AddInputMap(_Input::KEYBOARD, KEY_RIGHT, _Actions::MOVE_RIGHT);
	Actions.AddInputMap(_Input::KEYBOARD, KEY_RSHIFT, _Actions::JUMP);
	Actions.AddInputMap(_Input::KEYBOARD, KEY_RCONTROL, _Actions::JUMP);
	Actions.AddInputMap(_Input::KEYBOARD, KEY_NEXT, _Actions::JUMP);
#else
	Actions.AddInputMap(_Input::KEYBOARD, KEY_KEY_E, _Actions::MOVE_FORWARD);
	Actions.AddInputMap(_Input::KEYBOARD, KEY_KEY_D, _Actions::MOVE_BACK);
	Actions.AddInputMap(_Input::KEYBOARD, KEY_KEY_S, _Actions::MOVE_LEFT);
	Actions.AddInputMap(_Input::KEYBOARD, KEY_KEY_F, _Actions::MOVE_RIGHT);
	Actions.AddInputMap(_Input::KEYBOARD, KEY_SPACE, _Actions::JUMP);
#endif
	Actions.AddInputMap(_Input::KEYBOARD, KEY_KEY_X, _Actions::RESET);
	Actions.AddInputMap(_Input::KEYBOARD, KEY_ESCAPE, _Actions::MENU_PAUSE);
	Actions.AddInputMap(_Input::KEYBOARD, KEY_ESCAPE, _Actions::MENU_BACK);
	Actions.AddInputMap(_Input::MOUSE_AXIS, 0, _Actions::CAMERA_LEFT);
	Actions.AddInputMap(_Input::MOUSE_AXIS, 1, _Actions::CAMERA_RIGHT);
	Actions.AddInputMap(_Input::MOUSE_AXIS, 2, _Actions::CAMERA_UP);
	Actions.AddInputMap(_Input::MOUSE_AXIS, 3, _Actions::CAMERA_DOWN);

	Actions.AddInputMap(_Input::MOUSE_AXIS, 4, _Actions::MENU_PAGEUP);
	Actions.AddInputMap(_Input::MOUSE_AXIS, 5, _Actions::MENU_PAGEDOWN);

	float AxisScaleX = 130.0f;
	float AxisScaleY = 100.0f;
	float CursorSpeed = 400.0f;
	float Scale = ACTIONS_SCALE;
	float DeadZone = ACTIONS_DEADZONE;

	// Get controller name
	std::string Name = "";
	if(Input.HasJoystick())
		Name = Input.GetCleanJoystickName(0).c_str();

	// Handle steam controller
	if(Name == "wireless_steam_controller") {
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 0, _Actions::MOVE_LEFT, Scale, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 1, _Actions::MOVE_RIGHT, Scale, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 2, _Actions::MOVE_FORWARD, Scale, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 3, _Actions::MOVE_BACK, Scale, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 0, _Actions::CURSOR_LEFT, CursorSpeed, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 1, _Actions::CURSOR_RIGHT, CursorSpeed, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 2, _Actions::CURSOR_UP, CursorSpeed, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 3, _Actions::CURSOR_DOWN, CursorSpeed, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 4, _Actions::CAMERA_LEFT, AxisScaleX, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 5, _Actions::CAMERA_RIGHT, AxisScaleX, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 6, _Actions::CAMERA_UP, AxisScaleY, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 7, _Actions::CAMERA_DOWN, AxisScaleY, DeadZone);

		Actions.AddInputMap(_Input::JOYSTICK_BUTTON, 2, _Actions::JUMP);
		Actions.AddInputMap(_Input::JOYSTICK_BUTTON, 15, _Actions::JUMP, 1.0f, -1.0f, false);
		Actions.AddInputMap(_Input::JOYSTICK_BUTTON, 2, _Actions::MENU_GO);
		Actions.AddInputMap(_Input::JOYSTICK_BUTTON, 3, _Actions::MENU_BACK);
		Actions.AddInputMap(_Input::JOYSTICK_BUTTON, 6, _Actions::MENU_PAGEUP);
		Actions.AddInputMap(_Input::JOYSTICK_BUTTON, 7, _Actions::MENU_PAGEDOWN);
		Actions.AddInputMap(_Input::JOYSTICK_BUTTON, 10, _Actions::RESET);
		Actions.AddInputMap(_Input::JOYSTICK_BUTTON, 16, _Actions::RESET, 1.0f, -1.0f, false);
		Actions.AddInputMap(_Input::JOYSTICK_BUTTON, 11, _Actions::MENU_PAUSE);
	}
	else {
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 0, _Actions::MOVE_LEFT, Scale, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 1, _Actions::MOVE_RIGHT, Scale, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 2, _Actions::MOVE_FORWARD, Scale, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 3, _Actions::MOVE_BACK, Scale, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 0, _Actions::CURSOR_LEFT, CursorSpeed, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 1, _Actions::CURSOR_RIGHT, CursorSpeed, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 2, _Actions::CURSOR_UP, CursorSpeed, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 3, _Actions::CURSOR_DOWN, CursorSpeed, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 6, _Actions::CAMERA_LEFT, AxisScaleX, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 7, _Actions::CAMERA_RIGHT, AxisScaleX, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 8, _Actions::CAMERA_UP, AxisScaleY, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_AXIS, 9, _Actions::CAMERA_DOWN, AxisScaleY, DeadZone);
		Actions.AddInputMap(_Input::JOYSTICK_BUTTON, 0, _Actions::JUMP);
		Actions.AddInputMap(_Input::JOYSTICK_BUTTON, 0, _Actions::MENU_GO);
		Actions.AddInputMap(_Input::JOYSTICK_BUTTON, 1, _Actions::MENU_BACK);
		Actions.AddInputMap(_Input::JOYSTICK_BUTTON, 4, _Actions::MENU_PAGEUP);
		Actions.AddInputMap(_Input::JOYSTICK_BUTTON, 5, _Actions::MENU_PAGEDOWN);

		// Add mappings depending on controller used
		if(Name == "sony_playstation(r)3_controller") {
			Actions.AddInputMap(_Input::JOYSTICK_BUTTON, 8, _Actions::RESET);
			Actions.AddInputMap(_Input::JOYSTICK_BUTTON, 9, _Actions::MENU_PAUSE);
		}
		else {
			Actions.AddInputMap(_Input::JOYSTICK_BUTTON, 6, _Actions::RESET);
			Actions.AddInputMap(_Input::JOYSTICK_BUTTON, 7, _Actions::MENU_PAUSE);
		}
	}
}

// Reads the config file
int _Config::ReadConfig() {

	// Open the XML file
	XMLDocument Document;
	if(Document.LoadFile(Save.ConfigFile.c_str()) != XML_SUCCESS) {
		return 0;
	}

	// Check for config tag
	XMLElement *ConfigElement = Document.FirstChildElement("config");
	if(!ConfigElement)
		return 0;

	// Check for a video tag
	XMLElement *VideoElement = ConfigElement->FirstChildElement("video");
	if(VideoElement) {
		XMLElement *Element;

		// Get driver
		//VideoElement->QueryIntAttribute("driver", (int *)(&DriverType));

		// Check for the screen tag
		Element = VideoElement->FirstChildElement("screen");
		if(Element) {
			Element->QueryIntAttribute("width", &ScreenWidth);
			Element->QueryIntAttribute("height", &ScreenHeight);
			Element->QueryBoolAttribute("fullscreen", &Fullscreen);
		}

		// Check for the filtering tag
		Element = VideoElement->FirstChildElement("filtering");
		if(Element) {
			Element->QueryBoolAttribute("trilinear", &TrilinearFiltering);
			Element->QueryIntAttribute("anisotropic", &AnisotropicFiltering);
			Element->QueryIntAttribute("antialiasing", &AntiAliasing);
		}

		// Check for the shadow tag
		Element = VideoElement->FirstChildElement("shadows");
		if(Element) {
			Element->QueryBoolAttribute("enabled", &Shadows);
		}

		// Check for the shader tag
		Element = VideoElement->FirstChildElement("shaders");
		if(Element) {
			Element->QueryBoolAttribute("enabled", &Shaders);
		}

		// Check for the shader tag
		Element = VideoElement->FirstChildElement("multiplelights");
		if(Element) {
			Element->QueryBoolAttribute("enabled", &MultipleLights);
		}

		// Check for the vsync tag
		Element = VideoElement->FirstChildElement("vsync");
		if(Element) {
			Element->QueryBoolAttribute("enabled", &Vsync);
		}

		// Check max fps tag
		Element = VideoElement->FirstChildElement("maxfps");
		if(Element) {
			Element->QueryIntAttribute("value", &MaxFPS);
		}
	}

	// Check for the gameplay tag
	XMLElement *GameplayElement = ConfigElement->FirstChildElement("gameplay");
	if(GameplayElement) {
		GameplayElement->QueryBoolAttribute("showfps", &ShowFPS);
		GameplayElement->QueryBoolAttribute("showtutorial", &ShowTutorial);
		GameplayElement->QueryBoolAttribute("caching", &Caching);
	}

	// Check for the audio tag
	XMLElement *AudioElement = ConfigElement->FirstChildElement("audio");
	if(AudioElement) {
		AudioElement->QueryFloatAttribute("sound_volume", &SoundVolume);
	}

	// Check for the replay tag
	XMLElement *ReplayElement = ConfigElement->FirstChildElement("replay");
	if(ReplayElement) {
		ReplayElement->QueryBoolAttribute("autosave", &AutosaveNewRecords);
	}

	// Get input element
	XMLElement *InputElement = ConfigElement->FirstChildElement("input");
	if(InputElement) {
		InputElement->QueryFloatAttribute("mouse_sensitivity", &MouseSensitivity);
		InputElement->QueryFloatAttribute("mousex", &MouseScaleX);
		InputElement->QueryFloatAttribute("mousey", &MouseScaleY);
		InputElement->QueryBoolAttribute("invert_mouse", &InvertMouse);
		InputElement->QueryBoolAttribute("invert_gamepad_y", &InvertGamepadY);
		InputElement->QueryBoolAttribute("joystick_enabled", &JoystickEnabled);
		InputElement->QueryFloatAttribute("deadzone", &DeadZone);
	}

	// Add action maps
	Actions.ClearMappings(_Input::KEYBOARD);
	Actions.ClearMappings(_Input::MOUSE_BUTTON);
	Actions.ClearMappings(_Input::MOUSE_AXIS);
	Actions.Unserialize(InputElement, DeadZone);

	return 1;
}

// Writes the config file
int _Config::WriteConfig() {

	XMLDocument Document;
	Document.InsertEndChild(Document.NewDeclaration());

	// Config
	XMLElement *ConfigElement = Document.NewElement("config");
	ConfigElement->SetAttribute("version", "1.0");
	Document.InsertEndChild(ConfigElement);

	// Create video element
	XMLElement *VideoElement = Document.NewElement("video");
	ConfigElement->LinkEndChild(VideoElement);

	// Screen settings
	XMLElement *ScreenElement = Document.NewElement("screen");
	ScreenElement->SetAttribute("width", ScreenWidth);
	ScreenElement->SetAttribute("height", ScreenHeight);
	ScreenElement->SetAttribute("fullscreen", Fullscreen);
	VideoElement->LinkEndChild(ScreenElement);

	// Filtering
	XMLElement *FilteringElement = Document.NewElement("filtering");
	FilteringElement->SetAttribute("trilinear", TrilinearFiltering);
	FilteringElement->SetAttribute("anisotropic", AnisotropicFiltering);
	FilteringElement->SetAttribute("antialiasing", AntiAliasing);
	VideoElement->LinkEndChild(FilteringElement);

	// Shadows
	XMLElement *ShadowsElement = Document.NewElement("shadows");
	ShadowsElement->SetAttribute("enabled", Shadows);
	VideoElement->LinkEndChild(ShadowsElement);

	// Shaders
	XMLElement *ShadersElement = Document.NewElement("shaders");
	ShadersElement->SetAttribute("enabled", Shaders);
	VideoElement->LinkEndChild(ShadersElement);

	// Multiple Lights
	XMLElement *MultipleLightsElement = Document.NewElement("multiplelights");
	MultipleLightsElement->SetAttribute("enabled", MultipleLights);
	VideoElement->LinkEndChild(MultipleLightsElement);

	// Vsync
	XMLElement *VsyncElement = Document.NewElement("vsync");
	VsyncElement->SetAttribute("enabled", Vsync);
	VideoElement->LinkEndChild(VsyncElement);

	// Max fps
	XMLElement *MaxFPSElement = Document.NewElement("maxfps");
	MaxFPSElement->SetAttribute("value", MaxFPS);
	VideoElement->LinkEndChild(MaxFPSElement);

	// Create gameplay element
	XMLElement *GameplayElement = Document.NewElement("gameplay");
	GameplayElement->SetAttribute("showfps", ShowFPS);
	GameplayElement->SetAttribute("showtutorial", ShowTutorial);
	GameplayElement->SetAttribute("caching", Caching);
	ConfigElement->LinkEndChild(GameplayElement);

	// Create audio element
	XMLElement *AudioElement = Document.NewElement("audio");
	AudioElement->SetAttribute("sound_volume", SoundVolume);
	ConfigElement->LinkEndChild(AudioElement);

	// Create replay element
	XMLElement *ReplayElement = Document.NewElement("replay");
	ReplayElement->SetAttribute("autosave", AutosaveNewRecords);
	ConfigElement->LinkEndChild(ReplayElement);

	// Input
	XMLElement *InputElement = Document.NewElement("input");
	InputElement->SetAttribute("mouse_sensitivity", MouseSensitivity);
	InputElement->SetAttribute("mousex", MouseScaleX);
	InputElement->SetAttribute("mousey", MouseScaleY);
	InputElement->SetAttribute("invert_mouse", InvertMouse);
	InputElement->SetAttribute("invert_gamepad_y", InvertGamepadY);
	InputElement->SetAttribute("joystick_enabled", JoystickEnabled);
	InputElement->SetAttribute("deadzone", DeadZone);
	ConfigElement->LinkEndChild(InputElement);

	// Write action map
	Actions.Serialize(_Input::KEYBOARD, Document, InputElement);
	Actions.Serialize(_Input::MOUSE_BUTTON, Document, InputElement);
	Actions.Serialize(_Input::MOUSE_AXIS, Document, InputElement);

	// Write file
	Document.SaveFile(Save.ConfigFile.c_str());

	return 1;
}

// Read the current joystick's mapping
int _Config::ReadJoystickConfig() {
	int HasJoystickConfig = 0;

	// Loop over all joysticks
	for(uint32_t i = 0; i < Input.GetJoystickCount(); i++) {

		// Get joystick name
		std::string Name = Input.GetCleanJoystickName(i).c_str();
		std::string Path = Save.SavePath + Name + ".xml";

		// Open the XML file
		XMLDocument Document;
		if(Document.LoadFile(Path.c_str()) != XML_SUCCESS)
			continue;

		// Get input element
		XMLElement *InputMapElement = Document.FirstChildElement("inputmap");
		if(InputMapElement) {
			HasJoystickConfig = 1;

			// Check if enabled
			int Enabled = 1;
			InputMapElement->QueryIntAttribute("enabled", &Enabled);
			if(Enabled) {
				JoystickIndex = i;

				// Add action maps
				Actions.ClearMappings(_Input::JOYSTICK_BUTTON);
				Actions.ClearMappings(_Input::JOYSTICK_AXIS);
				Actions.Unserialize(InputMapElement, DeadZone);

				// Quit after first valid joystick
				break;
			}
		}
	}

	return HasJoystickConfig;
}

// Write the current joystick's mapping
int _Config::WriteJoystickConfig() {

	// Loop over all joysticks
	for(uint32_t i = 0; i < Input.GetJoystickCount(); i++) {

		XMLDocument Document;
		Document.InsertEndChild(Document.NewDeclaration());

		// Config
		XMLElement *InputMapElement = Document.NewElement("inputmap");
		InputMapElement->SetAttribute("name", Input.GetJoystickInfo(i).Name.c_str());
		InputMapElement->SetAttribute("enabled", "1");
		Document.InsertEndChild(InputMapElement);

		// Write action map
		Actions.Serialize(_Input::JOYSTICK_BUTTON, Document, InputMapElement);
		Actions.Serialize(_Input::JOYSTICK_AXIS, Document, InputMapElement);

		// Write file
		std::string Name = Input.GetCleanJoystickName(i).c_str();
		Document.SaveFile((Save.SavePath + Name + ".xml").c_str());
	}

	return 1;
}
