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
#include <engine/replay.h>
#include <engine/save.h>
#include <engine/log.h>
#include <engine/config.h>
#include <engine/constants.h>
#include <engine/level.h>
#include <engine/game.h>
#include <sstream>

_Replay Replay;

// Start recording a replay
void _Replay::StartRecording() {
	if(State != STATE_NONE)
		return;

	// Set up state
	State = STATE_RECORDING;
	NextPacketTime = 1.0f;
	Time = 0;

	// Get header information
	ReplayVersion = REPLAY_VERSION;
	LevelVersion = Level.GetLevelVersion();
	LevelName = Level.GetLevelName();

	// Create replay file for object data
	ReplayDataFile = Save.GetReplayPath() + "replay.dat";
	if(!ReplayStream.OpenForWrite(ReplayDataFile.c_str())) {
		Log.Write("_Replay::StartRecording - Unable to open %s!", ReplayDataFile.c_str());
	}
}

// Stops the recording process
void _Replay::StopRecording() {

	if(State == STATE_RECORDING) {
		State = STATE_NONE;
		ReplayStream.Close();
		remove(ReplayDataFile.c_str());
	}
}

// Saves the current replay out to a file
bool _Replay::SaveReplay(const std::string &PlayerDescription) {
	Description = PlayerDescription;
	TimeStamp = time(NULL);
	FinishTime = Time;

	// Flush current replay file
	ReplayStream.Flush();

	// Get new file name
	std::stringstream ReplayFilePath;
	ReplayFilePath << Save.GetReplayPath() << (unsigned int)TimeStamp << ".replay";

	// Open new file
	_File ReplayFile;
	if(!ReplayFile.OpenForWrite(ReplayFilePath.str().c_str())) {
		Log.Write("_Replay::SaveReplay - Unable to open %s for writing!", ReplayFilePath.str().c_str());
		return false;
	}

	// Write replay version
	ReplayFile.WriteChar(PACKET_REPLAYVERSION);
	ReplayFile.WriteInt(sizeof(ReplayVersion));
	ReplayFile.WriteInt(ReplayVersion);

	// Write level version
	ReplayFile.WriteChar(PACKET_LEVELVERSION);
	ReplayFile.WriteInt(sizeof(LevelVersion));
	ReplayFile.WriteInt(LevelVersion);

	// Write timestep value
	ReplayFile.WriteChar(PACKET_TIMESTEP);
	ReplayFile.WriteInt(sizeof(Game.GetTimeStep()));
	ReplayFile.WriteFloat(Game.GetTimeStep());

	// Write level file
	ReplayFile.WriteChar(PACKET_LEVELFILE);
	ReplayFile.WriteInt(LevelName.length());
	ReplayFile.WriteString(LevelName.c_str(), REPLAY_STRINGSIZE);

	// Write player's description of replay
	ReplayFile.WriteChar(PACKET_DESCRIPTION);
	ReplayFile.WriteInt(Description.length());
	ReplayFile.WriteString(Description.c_str(), REPLAY_STRINGSIZE);

	// Write time stamp
	ReplayFile.WriteChar(PACKET_DATE);
	ReplayFile.WriteInt(sizeof(TimeStamp));
	ReplayFile.WriteInt((int)TimeStamp);

	// Write finish time
	ReplayFile.WriteChar(PACKET_FINISHTIME);
	ReplayFile.WriteInt(sizeof(FinishTime));
	ReplayFile.WriteFloat(FinishTime);

	// Finished with header
	ReplayFile.WriteChar(PACKET_OBJECTDATA);
	ReplayFile.WriteInt(0);

	// Copy current data to new replay file
	std::ifstream CurrentReplayFile(ReplayDataFile.c_str(), std::ios::in | std::ios::binary);
	char Buffer[4096];
	std::streamsize BytesRead;
	while(!CurrentReplayFile.eof()) {
		CurrentReplayFile.read(Buffer, 4096);
		BytesRead = CurrentReplayFile.gcount();

		if(BytesRead) {
			//printf("BytesRead=%d\n", BytesRead);
			ReplayFile.WriteData(Buffer, (unsigned int)BytesRead);
		}
	}
	CurrentReplayFile.close();
	ReplayFile.Close();

	return true;
}

// Load header data
void _Replay::LoadHeader() {

	// Write replay version
	int PacketType;
	int PacketSize;
	bool Done = false;
	char Buffer[256];
	while(!ReplayStream.Eof() && !Done) {
		PacketType = ReplayStream.ReadChar();
		PacketSize = ReplayStream.ReadInt();
		switch(PacketType) {
			case PACKET_REPLAYVERSION:
				ReplayVersion = ReplayStream.ReadInt();
			break;
			case PACKET_LEVELVERSION:
				LevelVersion = ReplayStream.ReadInt();
			break;
			case PACKET_LEVELFILE:
				ReplayStream.ReadString(Buffer, REPLAY_STRINGSIZE);
				LevelName = Buffer;
			break;
			case PACKET_DESCRIPTION:
				ReplayStream.ReadString(Buffer, REPLAY_STRINGSIZE);
				Description = Buffer;
			break;
			case PACKET_DATE:
				TimeStamp = ReplayStream.ReadInt();
			break;
			case PACKET_FINISHTIME:
				FinishTime = ReplayStream.ReadFloat();
			break;
			case PACKET_OBJECTDATA:
				Done = true;
			break;
			default:
				ReplayStream.Ignore(PacketSize);
			break;
		}
	}
}

// Updates the replay timer
void _Replay::Update(float FrameTime) {
	Time += FrameTime;
	NextPacketTime += FrameTime;
}

// Determines if a packet is required
bool _Replay::NeedsPacket() {

	return State == STATE_RECORDING;
}

// Resets the next packet timer
void _Replay::ResetNextPacketTimer() {

	if(NeedsPacket())
		NextPacketTime = 0;
}

// Starts replay
bool _Replay::LoadReplay(const std::string &ReplayFile, bool HeaderOnly) {
	LevelName = "";
	LevelVersion = 0;
	ReplayVersion = 0;

	// Get file name
	std::string FilePath = Save.GetReplayPath() + ReplayFile;

	// Open the replay
	if(!ReplayStream.OpenForRead(FilePath.c_str()))
		return false;

	// Read header
	LoadHeader();

	// Read only the header
	if(HeaderOnly)
		ReplayStream.Close();

	return true;
}

// Stops replay
void _Replay::StopReplay() {

	State = STATE_NONE;
	ReplayStream.Close();
}

// Returns true if the replay is done playing
bool _Replay::ReplayStopped() {

	return ReplayStream.Eof();
}

// Write replay event
void _Replay::WriteEvent(int Type) {
	ReplayStream.WriteChar(Type);
	ReplayStream.WriteFloat(Time);

	//printf("WriteEvent Type=%d Time=%f\n", Type, Time);
}

// Reads a packet header
void _Replay::ReadEvent(ReplayEventStruct &Packet) {
	Packet.Type = ReplayStream.ReadChar();
	Packet.TimeStamp = ReplayStream.ReadFloat();
}
