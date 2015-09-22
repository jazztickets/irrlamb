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
#include <engine/scripting.h>
#include <engine/interface.h>
#include <engine/random.h>
#include <engine/log.h>
#include <engine/objectmanager.h>
#include <engine/level.h>
#include <engine/campaign.h>
#include <engine/audio.h>
#include <objects/template.h>
#include <objects/player.h>
#include <objects/orb.h>
#include <objects/zone.h>
#include <objects/constraint.h>
#include <objects/springjoint.h>
#include <play.h>
#include <menu.h>

_Scripting Scripting;

// Controls
luaL_Reg _Scripting::CameraFunctions[] = {
	{"SetYaw", &_Scripting::CameraSetYaw},
	{"SetPitch", &_Scripting::CameraSetPitch},
	{NULL, NULL}
};

// Functions for creating objects
luaL_Reg _Scripting::ObjectFunctions[] = {
	{"GetPointer", &_Scripting::ObjectGetPointer},
	{"GetName", &_Scripting::ObjectGetName},
	{"SetPosition", &_Scripting::ObjectSetPosition},
	{"GetPosition", &_Scripting::ObjectGetPosition},
	{"Stop", &_Scripting::ObjectStop},
	{"SetAngularVelocity", &_Scripting::ObjectSetAngularVelocity},
	{"SetLifetime", &_Scripting::ObjectSetLifetime},
	{"Delete", &_Scripting::ObjectDelete},
	{NULL, NULL}
};

// Functions for manipulating orbs
luaL_Reg _Scripting::OrbFunctions[] = {
	{"Deactivate", &_Scripting::OrbDeactivate},
	{NULL, NULL}
};


// Functions for timers
luaL_Reg _Scripting::TimerFunctions[] = {
	{"Stamp", &_Scripting::TimerStamp},
	{"DelayedFunction", &_Scripting::TimerDelayedFunction},
	{NULL, NULL}
};

// Functions for changing levels
luaL_Reg _Scripting::LevelFunctions[] = {
	{"Lose", &_Scripting::LevelLose},
	{"Win", &_Scripting::LevelWin},
	{"GetTemplate", &_Scripting::LevelGetTemplate},
	{"CreateObject", &_Scripting::LevelCreateObject},
	{"CreateConstraint", &_Scripting::LevelCreateConstraint},
	{"CreateSpring", &_Scripting::LevelCreateSpring},
	{NULL, NULL}
};

// Functions for the GUI
luaL_Reg _Scripting::GUIFunctions[] = {
	{"TutorialText", &_Scripting::GUITutorialText},
	{NULL, NULL}
};

// Functions for audio
luaL_Reg _Scripting::AudioFunctions[] = {
	{"Play", &_Scripting::AudioPlay},
	{NULL, NULL}
};

// Functions for random number generation
luaL_Reg _Scripting::RandomFunctions[] = {
	{"Seed", &_Scripting::RandomSeed},
	{"GetFloat", &_Scripting::RandomGetFloat},
	{"GetInt", &_Scripting::RandomGetInt},
	{NULL, NULL}
};

// Functions for manipulating zones
luaL_Reg _Scripting::ZoneFunctions[] = {

	{NULL, NULL}
};

// Lua library functions
int luaopen_Camera(lua_State *State) {
	luaL_newlib(State, _Scripting::CameraFunctions);
	return 1;
}

int luaopen_Object(lua_State *State) {
	luaL_newlib(State, _Scripting::ObjectFunctions);
	return 1;
}

int luaopen_Orb(lua_State *State) {
	luaL_newlib(State, _Scripting::OrbFunctions);
	return 1;
}

int luaopen_Timer(lua_State *State) {
	luaL_newlib(State, _Scripting::TimerFunctions);
	return 1;
}

int luaopen_Level(lua_State *State) {
	luaL_newlib(State, _Scripting::LevelFunctions);
	return 1;
}

int luaopen_GUI(lua_State *State) {
	luaL_newlib(State, _Scripting::GUIFunctions);
	return 1;
}

int luaopen_Audio(lua_State *State) {
	luaL_newlib(State, _Scripting::AudioFunctions);
	return 1;
}

int luaopen_Random(lua_State *State) {
	luaL_newlib(State, _Scripting::RandomFunctions);
	return 1;
}

int luaopen_Zone(lua_State *State) {
	luaL_newlib(State, _Scripting::ZoneFunctions);
	return 1;
}

// Constructor
_Scripting::_Scripting()
:	LuaObject(NULL) {

}

// Initializes the scripting interface
int _Scripting::Init() {

	// Restart scripting system
	Reset();

	return 1;
}

// Shuts down the scripting interface
int _Scripting::Close() {

	// Close old lua state
	if(LuaObject != NULL)
		lua_close(LuaObject);

	return 1;
}

// Resets the scripting state
void _Scripting::Reset() {

	// Close old lua state
	if(LuaObject != NULL)
		lua_close(LuaObject);

	// Initialize Lua object
	LuaObject = luaL_newstate();
	luaopen_base(LuaObject);
	luaopen_math(LuaObject);

	// Register C++ functions used by Lua
	luaL_requiref(LuaObject, "Camera", luaopen_Camera, 1);
	luaL_requiref(LuaObject, "Object", luaopen_Object, 1);
	luaL_requiref(LuaObject, "Orb", luaopen_Orb, 1);
	luaL_requiref(LuaObject, "Timer", luaopen_Timer, 1);
	luaL_requiref(LuaObject, "Level", luaopen_Level, 1);
	luaL_requiref(LuaObject, "GUI", luaopen_GUI, 1);
	luaL_requiref(LuaObject, "Audio", luaopen_Audio, 1);
	luaL_requiref(LuaObject, "Random", luaopen_Random, 1);
	luaL_requiref(LuaObject, "Zone", luaopen_Zone, 1);

	// Clean up
	KeyCallbacks.clear();
	TimedCallbacks.clear();
}

// Loads a Lua file
int _Scripting::LoadFile(const std::string &FilePath) {
	if(FilePath == "")
		return 0;

	// Load the file
	if(luaL_dofile(LuaObject, FilePath.c_str()) != 0) {
		Log.Write("_Scripting::LoadFile - failed to load script %s", FilePath.c_str());
		Log.Write("%s", lua_tostring(LuaObject, -1));
		return 0;
	}

	return 1;
}

// Defines a variable in Lua
void _Scripting::DefineLuaVariable(const char *VariableName, const char *Value) {

	lua_pushstring(LuaObject, Value);
	lua_setglobal(LuaObject, VariableName);
}

// Checks for a valid argument count
bool _Scripting::CheckArguments(lua_State *LuaObject, int Required) {
	int ArgumentCount = lua_gettop(LuaObject);

	// Check for arguments
	if(ArgumentCount != Required) {

		lua_Debug Record;
		lua_getstack(LuaObject, 0, &Record);
		lua_getinfo(LuaObject, "nl", &Record);

		Log.Write("Function %s requires %d arguments\n", Record.name, Required);
		return false;
	}

	return true;
}

// Calls a Lua function by name
void _Scripting::CallFunction(const std::string &FunctionName) {

	// Check for the function name
	lua_getglobal(LuaObject, FunctionName.c_str());
	if(!lua_isfunction(LuaObject, -1)) {
		lua_pop(LuaObject, 1);
		return;
	}

	lua_call(LuaObject, 0, 0);
}

// Passes collision events to Lua
void _Scripting::CallCollisionHandler(const std::string &FunctionName, _Object *BaseObject, _Object *OtherObject) {

	// Check for the function name
	lua_getglobal(LuaObject, FunctionName.c_str());
	if(!lua_isfunction(LuaObject, -1)) {
		lua_pop(LuaObject, 1);
		return;
	}

	lua_pushlightuserdata(LuaObject, static_cast<void *>(BaseObject));
	lua_pushlightuserdata(LuaObject, static_cast<void *>(OtherObject));
	lua_call(LuaObject, 2, 0);
}

// Calls a zone enter/exit event
void _Scripting::CallZoneHandler(const std::string &FunctionName, int Type, _Object *Zone, _Object *Object) {

	// Check for the function name
	lua_getglobal(LuaObject, FunctionName.c_str());
	if(!lua_isfunction(LuaObject, -1)) {
		lua_pop(LuaObject, 1);
		return;
	}

	// Set parameters
	lua_pushinteger(LuaObject, Type);
	lua_pushlightuserdata(LuaObject, static_cast<void *>(Zone));
	lua_pushlightuserdata(LuaObject, static_cast<void *>(Object));
	lua_call(LuaObject, 3, 1);

	// Get return value for disabling the zone
	int Disable = (int)lua_tointeger(LuaObject, -1);
	if(Disable) {
		_Zone *ZoneObject = static_cast<_Zone *>(Zone);
		ZoneObject->SetActive(false);
	}
}

// Calls a Lua function from a given keyname
bool _Scripting::HandleKeyPress(int Key) {

	// Find function
	KeyCallbacksIterator = KeyCallbacks.find(Key);
	if(KeyCallbacksIterator != KeyCallbacks.end()) {
		CallFunction(KeyCallbacksIterator->second);
		return true;
	}

	return false;
}

// Passes mouse clicks to Lua
void _Scripting::HandleMousePress(int Button, int MouseX, int MouseY) {

	// Get Lua function
	lua_getglobal(LuaObject, "OnMousePress");
	if(!lua_isfunction(LuaObject, -1)) {
		lua_pop(LuaObject, 1);
		return;
	}

	// Pass parameters and call function
	lua_pushnumber(LuaObject, Button);
	lua_pushnumber(LuaObject, MouseX);
	lua_pushnumber(LuaObject, MouseY);
	lua_call(LuaObject, 3, 0);
}

// Sets the camera's yaw value
int _Scripting::CameraSetYaw(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 1))
		return 0;

	float Yaw = (float)lua_tonumber(LuaObject, 1);

	if(PlayState.GetCamera())
		PlayState.GetCamera()->SetYaw(Yaw);

	return 0;
}

// Sets the camera's yaw value
int _Scripting::CameraSetPitch(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 1))
		return 0;

	float Pitch = (float)lua_tonumber(LuaObject, 1);

	if(PlayState.GetCamera())
		PlayState.GetCamera()->SetPitch(Pitch);

	return 0;
}

// Gets a pointer to an object from a name
int _Scripting::ObjectGetPointer(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 1))
		return 0;

	// Get parameters
	std::string Name = lua_tostring(LuaObject, 1);
	_Object *Object = ObjectManager.GetObjectByName(Name);

	// Pass pointer
	lua_pushlightuserdata(LuaObject, (void *)Object);

	return 1;
}

// Gets the name of an object
int _Scripting::ObjectGetName(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 1))
		return 0;

	_Object *Object = (_Object *)(lua_touserdata(LuaObject, 1));

	if(Object != NULL)
		lua_pushstring(LuaObject, Object->GetName().c_str());

	return 1;
}

// Sets the object's position
int _Scripting::ObjectSetPosition(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 4))
		return 0;

	// Get parameters
	_Object *Object = (_Object *)(lua_touserdata(LuaObject, 1));
	float PositionX = (float)lua_tonumber(LuaObject, 2);
	float PositionY = (float)lua_tonumber(LuaObject, 3);
	float PositionZ = (float)lua_tonumber(LuaObject, 4);

	// Set position
	if(Object != NULL)
		Object->SetPosition(btVector3(PositionX, PositionY, PositionZ));

	return 0;
}

// Gets the object's position
int _Scripting::ObjectGetPosition(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 1))
		return 0;

	// Get object
	_Object *Object = (_Object *)(lua_touserdata(LuaObject, 1));
	if(Object == NULL)
		return 0;

	// Send position to Lua
	lua_pushnumber(LuaObject, Object->GetPosition()[0]);
	lua_pushnumber(LuaObject, Object->GetPosition()[1]);
	lua_pushnumber(LuaObject, Object->GetPosition()[2]);

	return 3;
}

// Sets the object's lifetime
int _Scripting::ObjectSetLifetime(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 2))
		return 0;

	// Get parameters
	_Object *Object = (_Object *)(lua_touserdata(LuaObject, 1));
	float Lifetime = (float)lua_tonumber(LuaObject, 2);

	// Set lifetime
	if(Object != NULL)
		Object->SetLifetime(Lifetime);

	return 0;
}

// Stops an object's movement
int _Scripting::ObjectStop(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 1))
		return 0;

	// Stop object
	_Object *Object = (_Object *)(lua_touserdata(LuaObject, 1));
	if(Object != NULL)
		Object->Stop();

	return 0;
}

// Sets an object's angular velocity
int _Scripting::ObjectSetAngularVelocity(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 4))
		return 0;

	// Get parameters
	_Object *Object = (_Object *)(lua_touserdata(LuaObject, 1));
	float X = (float)lua_tonumber(LuaObject, 2);
	float Y = (float)lua_tonumber(LuaObject, 3);
	float Z = (float)lua_tonumber(LuaObject, 4);

	if(Object != NULL)
		Object->SetAngularVelocity(btVector3(X, Y, Z));

	return 0;
}

// Deletes an object
int _Scripting::ObjectDelete(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 1))
		return 0;

	// Delete object
	_Object *Object = (_Object *)(lua_touserdata(LuaObject, 1));
	if(Object != NULL)
		ObjectManager.DeleteObject(Object);

	return 0;
}

// Deactivates an orb
int _Scripting::OrbDeactivate(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 3))
		return 0;

	// Get parameters
	_Orb *Orb = (_Orb *)(lua_touserdata(LuaObject, 1));
	std::string FunctionName = lua_tostring(LuaObject, 2);
	float Time = (float)lua_tonumber(LuaObject, 3);

	// Deactivate orb
	if(Orb != NULL && Orb->IsStillActive()) {
		Orb->StartDeactivation(FunctionName, Time);
	}

	return 0;
}

// Returns a time stamp from the start of the level
int _Scripting::TimerStamp(lua_State *LuaObject) {

	lua_pushnumber(LuaObject, PlayState.GetTimer());

	return 1;
}

// Adds a timed callback
int _Scripting::TimerDelayedFunction(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 2))
		return 0;

	// Get parameters
	std::string FunctionName = lua_tostring(LuaObject, 1);
	float Time = (float)lua_tonumber(LuaObject, 2);

	// Add function to list
	Scripting.AddTimedCallback(FunctionName, Time);

	return 0;
}

// Restarts the level
int _Scripting::LevelLose(lua_State *LuaObject) {
	int ArgumentCount = lua_gettop(LuaObject);

	if(ArgumentCount == 1) {
		std::string Message = lua_tostring(LuaObject, 1);
		Menu.SetLoseMessage(Message);
	}

	PlayState.LoseLevel();

	return 0;
}

// Wins the level
int _Scripting::LevelWin(lua_State *LuaObject) {

	PlayState.WinLevel();

	return 0;
}

// Gets a template from a name
int _Scripting::LevelGetTemplate(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 1))
		return 0;

	// Get parameters
	std::string TemplateName = lua_tostring(LuaObject, 1);

	// Send template to Lua
	lua_pushlightuserdata(LuaObject, Level.GetTemplate(TemplateName));

	return 1;
}

// Creates an object from a template
int _Scripting::LevelCreateObject(lua_State *LuaObject) {

	// Get argument count
	int ArgumentCount = lua_gettop(LuaObject);

	// Check for arguments
	if(ArgumentCount != 5 && ArgumentCount != 8) {
		Log.Write("Function Level.CreateObject requires either 5 or 8 arguments\n");
		return false;
	}

	// Get parameters
	std::string ObjectName = lua_tostring(LuaObject, 1);
	TemplateStruct *Template = (TemplateStruct *)(lua_touserdata(LuaObject, 2));
	float PositionX = (float)lua_tonumber(LuaObject, 3);
	float PositionY = (float)lua_tonumber(LuaObject, 4);
	float PositionZ = (float)lua_tonumber(LuaObject, 5);
	float RotationX = 0.0f;
	float RotationY = 0.0f;
	float RotationZ = 0.0f;
	if(ArgumentCount > 5) {
		RotationX = (float)lua_tonumber(LuaObject, 6);
		RotationY = (float)lua_tonumber(LuaObject, 7);
		RotationZ = (float)lua_tonumber(LuaObject, 8);
	}

	// Set up spawn struct
	SpawnStruct Spawn;
	Spawn.Name = ObjectName;
	Spawn.Template = Template;
	Spawn.Position.setValue(PositionX, PositionY, PositionZ);
	Spawn.Rotation.setValue(RotationX, RotationY, RotationZ);

	// Create object
	_Object *Object = Level.CreateObject(Spawn);

	// Send new object to Lua
	lua_pushlightuserdata(LuaObject, static_cast<void *>(Object));

	return 1;
}

// Creates a constraint
int _Scripting::LevelCreateConstraint(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 4))
		return 0;

	// Set up constraint struct
	ConstraintStruct Constraint;
	Constraint.Name = lua_tostring(LuaObject, 1);
	Constraint.Template = (TemplateStruct *)(lua_touserdata(LuaObject, 2));
	Constraint.BodyA = (_Object *)(lua_touserdata(LuaObject, 3));
	Constraint.BodyB = (_Object *)(lua_touserdata(LuaObject, 4));

	// Create object
	_Object *Object = Level.CreateConstraint(Constraint);

	// Send new object to Lua
	lua_pushlightuserdata(LuaObject, static_cast<void *>(Object));

	return 1;
}

// Creates a spring constraint
int _Scripting::LevelCreateSpring(lua_State *LuaObject) {

	return 1;
}

// Sets the tutorial text
int _Scripting::GUITutorialText(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 2))
		return 0;

	// Get parameters
	std::string Text(lua_tostring(LuaObject, 1));
	float Length = (float)lua_tonumber(LuaObject, 2);

	// Show text
	Interface.SetTutorialText(Text, Length);

	return 0;
}

// Plays a sound buffer
int _Scripting::AudioPlay(lua_State *LuaObject) {

	// Validate arguments
	int ArgumentCount = lua_gettop(LuaObject);
	if(ArgumentCount < 4 || ArgumentCount > 7)
		return 0;

	// Get parameters
	std::string File(lua_tostring(LuaObject, 1));
	float PositionX = (float)lua_tonumber(LuaObject, 2);
	float PositionY = (float)lua_tonumber(LuaObject, 3);
	float PositionZ = (float)lua_tonumber(LuaObject, 4);

	int Looping = 0;
	if(ArgumentCount > 4)
		Looping = (int)lua_tonumber(LuaObject, 5);

	float MinGain = 0.0f;
	if(ArgumentCount > 5)
		MinGain = (float)lua_tonumber(LuaObject, 6);

	float MaxGain = 1.0f;
	if(ArgumentCount > 6)
		MaxGain = (float)lua_tonumber(LuaObject, 7);

	// Play sound
	_AudioSource *Source = new _AudioSource(Audio.GetBuffer(File), Looping, MinGain, MaxGain);
	Audio.Play(Source, PositionX, PositionY, PositionZ);

	// Return audio source
	lua_pushlightuserdata(LuaObject, static_cast<void *>(Source));

	return 1;
}

// Sets the random seed
int _Scripting::RandomSeed(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 1))
		return 0;

	// Get parameter
	irr::u32 Seed = (irr::u32)lua_tointeger(LuaObject, 1);

	// Set seed
	Random.SetSeed(Seed);

	return 0;
}

// Generates a random float
int _Scripting::RandomGetFloat(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 2))
		return 0;

	// Get parameters
	float Min = (float)lua_tonumber(LuaObject, 1);
	float Max = (float)lua_tonumber(LuaObject, 2);

	// Send random number to Lua
	lua_pushnumber(LuaObject, Random.GenerateRange(Min, Max));

	return 1;
}

// Generates a random integer
int _Scripting::RandomGetInt(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 2))
		return 0;

	// Get parameters
	int Min = (int)lua_tointeger(LuaObject, 1);
	int Max = (int)lua_tointeger(LuaObject, 2);

	// Send random number to Lua
	lua_pushnumber(LuaObject, Random.GenerateRange(Min, Max));

	return 1;
}

// Adds a timed callback function to the list
void _Scripting::AddTimedCallback(const std::string &FunctionName, float Time) {

	// Check time
	if(Time <= 0.0)
		return;

	// Create callback structure
	TimedCallbackStruct Callback;
	Callback.TimeStamp = PlayState.GetTimer() + Time;
	Callback.Function = FunctionName;

	// Insert in order
	std::list<TimedCallbackStruct>::iterator Iterator;
	for(Iterator = TimedCallbacks.begin(); Iterator != TimedCallbacks.end(); ++Iterator) {
		if(Callback.TimeStamp < (*Iterator).TimeStamp)
			break;
	}

	// Add callback
	TimedCallbacks.insert(Iterator, Callback);
}

// Updates the timed callback functions
void _Scripting::UpdateTimedCallbacks() {

	for(std::list<TimedCallbackStruct>::iterator Iterator(TimedCallbacks.begin()); Iterator != TimedCallbacks.end(); ++Iterator) {
		if(PlayState.GetTimer() >= (*Iterator).TimeStamp) {
			Scripting.CallFunction((*Iterator).Function);

			// Remove callback
			Iterator = TimedCallbacks.erase(Iterator);
			if(Iterator == TimedCallbacks.end())
				break;
		}
		else
			return;
	}
}

// Attaches a key to a Lua function
void _Scripting::AttachKeyToFunction(int Key, const std::string &FunctionName) {

	// Install callback
	KeyCallbacksIterator = KeyCallbacks.find(Key);
	if(KeyCallbacksIterator == KeyCallbacks.end())
		KeyCallbacks.insert(std::pair<int, std::string>(Key, FunctionName));
}
