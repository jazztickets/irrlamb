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
#include <fstream>

// Constants
const int REPLAY_VERSION = 4;

// Event packet structure
struct _ReplayEvent {
	uint8_t Type;
	float Timestamp;
};

// Classes
class _Replay {

	public:

		enum PacketType {

			// Header
			PACKET_REPLAYVERSION,
			PACKET_LEVELVERSION,
			PACKET_LEVELFILE,
			PACKET_DESCRIPTION,
			PACKET_DATE,
			PACKET_FINISHTIME,
			PACKET_TIMESTEP,
			PACKET_AUTOSAVE,
			PACKET_WON,
			PACKET_PLATFORM,

			// Object updates
			PACKET_OBJECTDATA = 127,
			PACKET_CAMERA,
			PACKET_MOVEMENT,
			PACKET_CREATE,
			PACKET_DELETE,
			PACKET_ORBDEACTIVATE,
			PACKET_INPUT,
		};

		enum StateType {
			STATE_NONE,
			STATE_RECORDING,
			STATE_REPLAYING,
		};

		// Recording functions
		void StartRecording();
		void StopRecording();
		bool SaveReplay(const std::string &PlayerDescription, bool Autosave=false, bool Won=false);

		// Playback functions
		bool LoadReplay(const std::string &ReplayFile, bool HeaderOnly=false);
		void StartReplay() { State = STATE_REPLAYING; }
		void StopReplay();
		bool ReplayStopped();

		void Update(float FrameTime);

		bool IsRecording() const { return State == STATE_RECORDING; }
		bool IsReplaying() const { return State == STATE_REPLAYING; }
		bool NeedsPacket();

		std::fstream &GetFile() { return File; }
		void WriteEvent(uint8_t Type);
		void ReadEvent(_ReplayEvent &Packet);

		const std::string &GetLevelName() { return LevelName; }
		const std::string &GetDescription() { return Description; }
		int32_t GetVersion() { return ReplayVersion; }
		int32_t GetLevelVersion() { return LevelVersion; }
		float GetTimeStep() { return TimeStep; }
		float GetFinishTime() { return FinishTime; }
		time_t &GetTimestamp() { return Timestamp; }
		char GetPlatform() { return Platform; }
		bool GetAutosave() { return Autosave; }
		bool GetWon() { return Won; }

	private:

		void LoadHeader();
		void WriteChunk(std::fstream &OutFile, char Type, const char *Data, uint32_t Size);

		// Header
		int32_t ReplayVersion;
		int32_t LevelVersion;
		std::string LevelName;
		std::string Description;
		time_t Timestamp;
		float FinishTime;
		float TimeStep;
		char Platform;
		bool Autosave;
		bool Won;

		// Replay data file name
		std::string ReplayDataFile;

		// File stream
		std::fstream File;

		// Time management
		float Time;

		// State
		StateType State;

};

// Singletons
extern _Replay Replay;
