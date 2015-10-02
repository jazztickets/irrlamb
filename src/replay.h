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
#include <fstream>

// Event packet structure
struct _ReplayEvent {
	uint8_t Type;
	float TimeStamp;
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

			// Object updates
			PACKET_OBJECTDATA = 127,
			PACKET_CAMERA,
			PACKET_MOVEMENT,
			PACKET_CREATE,
			PACKET_DELETE,
			PACKET_ORBDEACTIVATE,
		};

		enum StateType {
			STATE_NONE,
			STATE_RECORDING,
			STATE_REPLAYING,
		};

		// Recording functions
		void StartRecording();
		void StopRecording();
		bool SaveReplay(const std::string &PlayerDescription, bool Autosave=false);

		// Playback functions
		bool LoadReplay(const std::string &ReplayFile, bool HeaderOnly=false);
		void StartReplay() { State = STATE_REPLAYING; }
		void StopReplay();
		bool ReplayStopped();

		void Update(float FrameTime);

		bool IsRecording() const { return State == STATE_RECORDING; }
		bool IsReplaying() const { return State == STATE_REPLAYING; }
		bool NeedsPacket();
		void ResetNextPacketTimer();

		std::fstream &GetFile() { return File; }
		void WriteEvent(uint8_t Type);
		void ReadEvent(_ReplayEvent &Packet);

		const std::string &GetLevelName() { return LevelName; }
		const std::string &GetDescription() { return Description; }
		int GetVersion() { return ReplayVersion; }
		float GetFinishTime() { return FinishTime; }
		time_t &GetTimeStamp() { return TimeStamp; }
		bool GetAutosave() { return Autosave; }

	private:

		void LoadHeader();
		void WriteChunk(std::fstream &OutFile, char Type, const char *Data, uint32_t Size);

		// Header
		int32_t ReplayVersion;
		int32_t LevelVersion;
		std::string LevelName;
		std::string Description;
		time_t TimeStamp;
		float FinishTime;
		bool Autosave;

		// Replay data file name
		std::string ReplayDataFile;

		// File stream
		std::fstream File;

		// Time management
		float Time, NextPacketTime;

		// State
		StateType State;

};

// Singletons
extern _Replay Replay;
