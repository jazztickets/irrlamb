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
#include <lua.hpp>
#include <list>
#include <string>
#include <map>

// Structures
struct TimedCallbackStruct {
	float TimeStamp;
	std::string Function;
};

// Forward Declarations
class _Object;

// Classes
class _Scripting {

	public:

		_Scripting();
		~_Scripting() { }

		int Init();
		int Close();

		void Reset();
		int LoadFile(const std::string &FilePath);

		void DefineLuaVariable(const char *VariableName, const char *Value);

		void CallFunction(const std::string &FunctionName);
		void CallCollisionHandler(const std::string &FunctionName, _Object *BaseObject, _Object *OtherObject);
		void CallZoneHandler(const std::string &FunctionName, int Type, _Object *Zone, _Object *Object);

		bool HandleKeyPress(int Key);
		void HandleMousePress(int Button, int MouseX, int MouseY);
		void UpdateTimedCallbacks();

		static luaL_Reg CameraFunctions[], ObjectFunctions[], OrbFunctions[], TimerFunctions[], LevelFunctions[],
						GUIFunctions[], AudioFunctions[], RandomFunctions[], ZoneFunctions[];

	private:

		static bool CheckArguments(lua_State *LuaObject, int Required);

		static int CameraSetYaw(lua_State *LuaObject);
		static int CameraSetPitch(lua_State *LuaObject);

		static int ObjectGetPointer(lua_State *LuaObject);
		static int ObjectGetName(lua_State *LuaObject);
		static int ObjectSetPosition(lua_State *LuaObject);
		static int ObjectGetPosition(lua_State *LuaObject);
		static int ObjectStop(lua_State *LuaObject);
		static int ObjectSetAngularVelocity(lua_State *LuaObject);
		static int ObjectSetLifetime(lua_State *LuaObject);
		static int ObjectDelete(lua_State *LuaObject);

		static int OrbDeactivate(lua_State *LuaObject);

		static int TimerDelayedFunction(lua_State *LuaObject);
		static int TimerStamp(lua_State *LuaObject);

		static int LevelLose(lua_State *LuaObject);
		static int LevelWin(lua_State *LuaObject);
		static int LevelGetTemplate(lua_State *LuaObject);
		static int LevelCreateObject(lua_State *LuaObject);
		static int LevelCreateConstraint(lua_State *LuaObject);
		static int LevelCreateSpring(lua_State *LuaObject);

		static int GUITutorialText(lua_State *LuaObject);

		static int AudioPlay(lua_State *LuaObject);

		static int RandomSeed(lua_State *LuaObject);
		static int RandomGetFloat(lua_State *LuaObject);
		static int RandomGetInt(lua_State *LuaObject);

		void AddTimedCallback(const std::string &FunctionName, float Time);
		void AttachKeyToFunction(int Key, const std::string &FunctionName);

		std::list<TimedCallbackStruct> TimedCallbacks;

		std::map<int, std::string> KeyCallbacks;
		std::map<int, std::string>::iterator KeyCallbacksIterator;

		lua_State *LuaObject;

};

// Singletons
extern _Scripting Scripting;
