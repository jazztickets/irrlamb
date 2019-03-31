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
#include <replay.h>
#include <save.h>
#include <log.h>
#include <config.h>
#include <level.h>
#include <framework.h>
#include <sstream>

_Replay Replay;

// Start recording a replay
void _Replay::StartRecording() {
	if(State != STATE_NONE)
		return;

	// Set up state
	State = STATE_RECORDING;
	Time = 0;

	// Get header information
	ReplayVersion = REPLAY_VERSION;
	LevelVersion = Level.LevelVersion;
	LevelName = Level.LevelName;

	// Create replay file for object data
	ReplayDataFile = Save.ReplayPath + "replay.dat";
	File.open(ReplayDataFile.c_str(), std::ios::out | std::ios::binary);
	if(!File.is_open())
		Log.Write("Unable to open: %s", ReplayDataFile.c_str());
}

// Stops the recording process
void _Replay::StopRecording() {

	if(State == STATE_RECORDING) {
		State = STATE_NONE;
		File.close();
		remove(ReplayDataFile.c_str());
	}
}

// Saves the current replay out to a file
bool _Replay::SaveReplay(const std::string &PlayerDescription, bool Autosave, bool Won) {
	Description = PlayerDescription;
	Timestamp = time(nullptr);
	FinishTime = Time;

	// Flush current replay file
	File.flush();

	// Get new file name
	std::stringstream ReplayFilePath;
	ReplayFilePath << Save.ReplayPath << (uint32_t)Timestamp << "-" << Level.LevelName << ".replay";

	// Open new file
	std::fstream NewFile(ReplayFilePath.str().c_str(), std::ios::out | std::ios::binary);
	if(!NewFile) {
		Log.Write("Unable to open for writing: %s", ReplayFilePath.str().c_str());
		return false;
	}

	// Write platform
	char Platform = PLATFORM;
	WriteChunk(NewFile, PACKET_PLATFORM, (char *)&Platform, sizeof(Platform));

	// Write replay version
	WriteChunk(NewFile, PACKET_REPLAYVERSION, (char *)&ReplayVersion, sizeof(ReplayVersion));

	// Write level version
	WriteChunk(NewFile, PACKET_LEVELVERSION, (char *)&LevelVersion, sizeof(LevelVersion));

	// Write timestep value
	WriteChunk(NewFile, PACKET_TIMESTEP, (char *)&Framework.GetTimeStep(), sizeof(Framework.GetTimeStep()));

	// Write level file
	WriteChunk(NewFile, PACKET_LEVELFILE, LevelName.c_str(), LevelName.length());

	// Write player's description of replay
	WriteChunk(NewFile, PACKET_DESCRIPTION, Description.c_str(), Description.length());

	// Write time stamp
	WriteChunk(NewFile, PACKET_DATE, (char *)&Timestamp, sizeof(Timestamp));

	// Write finish time
	WriteChunk(NewFile, PACKET_FINISHTIME, (char *)&FinishTime, sizeof(FinishTime));

	// Write autosave value
	WriteChunk(NewFile, PACKET_AUTOSAVE, (char *)&Autosave, sizeof(Autosave));

	// Write won value
	WriteChunk(NewFile, PACKET_WON, (char *)&Won, sizeof(Won));

	// Finished with header
	NewFile.put(PACKET_OBJECTDATA);
	uint32_t Dummy = 0;
	NewFile.write((char *)&Dummy, sizeof(Dummy));

	// Copy current data to new replay file
	std::ifstream CurrentReplayFile(ReplayDataFile.c_str(), std::ios::in | std::ios::binary);
	char Buffer[4096];
	std::streamsize BytesRead;
	while(!CurrentReplayFile.eof()) {
		CurrentReplayFile.read(Buffer, 4096);
		BytesRead = CurrentReplayFile.gcount();

		if(BytesRead)
			NewFile.write(Buffer, (uint32_t)BytesRead);
	}

	CurrentReplayFile.close();
	NewFile.close();

	return true;
}

// Load header data
void _Replay::LoadHeader() {
	bool Debug = false;

	// Write replay version
	char PacketType;
	uint32_t PacketSize;
	bool Done = false;
	char Buffer[1024];
	while(!File.eof() && !Done) {
		PacketType = File.get();
		File.read((char *)&PacketSize, sizeof(PacketSize));
		switch(PacketType) {
			case PACKET_REPLAYVERSION:
				File.read((char *)&ReplayVersion, sizeof(ReplayVersion));
				if(ReplayVersion != REPLAY_VERSION)
					Done = true;

				if(Debug)
					Log.Write("ReplayVersion=%d, PacketSize=%d sizeof=%d", ReplayVersion, PacketSize, sizeof(ReplayVersion));
			break;
			case PACKET_LEVELVERSION:
				File.read((char *)&LevelVersion, sizeof(LevelVersion));

				if(Debug)
					Log.Write("LevelVersion=%d, PacketSize=%d sizeof=%d", LevelVersion, PacketSize, sizeof(LevelVersion));
			break;
			case PACKET_LEVELFILE:
				if(PacketSize > 1024)
					PacketSize = 1024;
				File.read(Buffer, PacketSize);
				Buffer[PacketSize] = 0;
				LevelName = Buffer;

				if(Debug)
					Log.Write("LevelName=%s, PacketSize=%d", Buffer, PacketSize);
			break;
			case PACKET_DESCRIPTION:
				if(PacketSize > 1024)
					PacketSize = 1024;
				File.read(Buffer, PacketSize);
				Buffer[PacketSize] = 0;
				Description = Buffer;

				if(Debug)
					Log.Write("Description=%s, PacketSize=%d", Buffer, PacketSize);
			break;
			case PACKET_DATE:
				if(PacketSize > 8)
					PacketSize = 8;
				File.read((char *)&Timestamp, PacketSize);

				if(Debug)
					Log.Write("Timestamp=%d, PacketSize=%d sizeof=%d", Timestamp, PacketSize, sizeof(Timestamp));
			break;
			case PACKET_FINISHTIME:
				File.read((char *)&FinishTime, sizeof(FinishTime));

				if(Debug)
					Log.Write("FinishTime=%f, PacketSize=%d sizeof=%d", FinishTime, PacketSize, sizeof(FinishTime));
			break;
			case PACKET_TIMESTEP:
				File.read((char *)&TimeStep, sizeof(TimeStep));

				if(Debug)
					Log.Write("TimeStep=%f, PacketSize=%d sizeof=%d", TimeStep, PacketSize, sizeof(TimeStep));
			break;
			case PACKET_AUTOSAVE:
				Autosave = File.get();

				if(Debug)
					Log.Write("Autosave=%d, PacketSize=%d", Autosave, PacketSize);
			break;
			case PACKET_WON:
				Won = File.get();

				if(Debug)
					Log.Write("Won=%d, PacketSize=%d", Won, PacketSize);
			break;
			case PACKET_PLATFORM:
				Platform = File.get();
			break;
			case PACKET_OBJECTDATA:
				Done = true;
			break;
			default:
				File.ignore(PacketSize);
			break;
		}
	}
}

// Write a replay chunk
void _Replay::WriteChunk(std::fstream &OutFile, char Type, const char *Data, uint32_t Size) {
   OutFile.put(Type);
   OutFile.write((char *)&Size, sizeof(Size));
   OutFile.write(Data, Size);
}

// Updates the replay timer
void _Replay::Update(float FrameTime) {
	Time += FrameTime;
}

// Determines if a packet is required
bool _Replay::NeedsPacket() {

	return State == STATE_RECORDING;
}

// Starts replay
bool _Replay::LoadReplay(const std::string &ReplayFile, bool HeaderOnly) {
	LevelName = "";
	LevelVersion = 0;
	ReplayVersion = 0;
	Autosave = false;
	Won = false;
	Platform = 0;

	// Try absolute path
	File.open(ReplayFile.c_str(), std::ios::in | std::ios::binary);
	if(!File) {

		// Pass only file name
		File.open((Save.ReplayPath + ReplayFile).c_str(), std::ios::in | std::ios::binary);
		if(!File)
			return false;
	}

	// Read header
	LoadHeader();

	// Read only the header
	if(HeaderOnly)
		File.close();

	return true;
}

// Stops replay
void _Replay::StopReplay() {

	State = STATE_NONE;
	File.close();
}

// Returns true if the replay is done playing
bool _Replay::ReplayStopped() {

	return File.eof();
}

// Write replay event
void _Replay::WriteEvent(uint8_t Type) {
	File.put(Type);
	File.write((char *)&Time, sizeof(Time));
}

// Reads a packet header
void _Replay::ReadEvent(_ReplayEvent &Packet) {
	Packet.Type = File.get();
	File.read((char *)&Packet.Timestamp, sizeof(Packet.Timestamp));
}
