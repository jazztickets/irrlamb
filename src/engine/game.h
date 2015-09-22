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
#pragma once
#include <string>
#include <irrTypes.h>

// Forward Declarations
class _State;

class _Game {

	public:

		enum ManagerStateType {
			STATE_INIT,
			STATE_UPDATE,
			STATE_CLOSE
		};

		int Init(int Count, char **Arguments);
		void Update();
		void Close();

		bool IsDone() { return Done; }
		void SetDone(bool Value) { Done = Value; }

		ManagerStateType GetManagerState() { return ManagerState; }
		void ChangeState(_State *State);
		_State *GetState() { return State; }

		float GetTimeStep() { return TimeStep; }
		float GetTimeScale() { return TimeScale; }
		float GetLastFrameTime() { return LastFrameTime; }
		void SetTimeScale(float Value) { TimeScale = Value; }
		void UpdateTimeStepAccumulator(float Value) { TimeStepAccumulator += Value; }
		void ResetTimer();

		void EnableAudio();
		void DisableAudio();

		const std::string &GetWorkingPath() { return WorkingPath; }

	private:

		void ResetGraphics();

		// States
		ManagerStateType ManagerState;
		_State *State, *NewState;
		bool PreviousWindowActive, WindowActive;

		// Flags
		bool Done, MouseWasLocked;

		// Time
		irr::u32 TimeStamp;
		float SleepRate;

		// Physics
		float LastFrameTime;
		float TimeStep, TimeStepAccumulator, TimeScale;

		// Misc
		std::string WorkingPath;
};

// Singletons
extern _Game Game;
