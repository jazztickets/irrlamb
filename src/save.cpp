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
#include <save.h>
#include <globals.h>
#include <log.h>
#include <database.h>
#include <constants.h>
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	#include <sys/stat.h>
#endif

_Save Save;

// Initializes the save system
int _Save::Init() {

	#ifdef _WIN32
		SavePath = std::string(getenv("APPDATA")) + "/irrlamb/";
		CreateDirectory(SavePath.c_str(), nullptr);
	#else
		SavePath = std::string(getenv("HOME")) + std::string("/.local/");
		mkdir(SavePath.c_str(), 0755);
		SavePath += "share/";
		mkdir(SavePath.c_str(), 0755);
		SavePath += "irrlamb/";
		mkdir(SavePath.c_str(), 0755);
	#endif

	ReplayPath = SavePath + std::string("replays/");
	ScreenshotsPath = SavePath + std::string("screenshots/");
	CustomLevelsPath = SavePath + std::string("customlevels/");

	// Get files
	ConfigFile = SavePath + std::string("config.xml");
	StatsFile = SavePath + std::string("stats.dat");

	// Create directories
	#ifdef _WIN32
		CreateDirectory(SavePath.c_str(), nullptr);
		CreateDirectory(ReplayPath.c_str(), nullptr);
		CreateDirectory(ScreenshotsPath.c_str(), nullptr);
		CreateDirectory(CustomLevelsPath.c_str(), nullptr);
	#else
		mkdir(SavePath.c_str(), S_IRWXU | S_IXGRP | S_IRGRP | S_IXOTH | S_IROTH);
		mkdir(ReplayPath.c_str(), S_IRWXU | S_IXGRP | S_IRGRP | S_IXOTH | S_IROTH);
		mkdir(ScreenshotsPath.c_str(), S_IRWXU | S_IXGRP | S_IRGRP | S_IXOTH | S_IROTH);
		mkdir(CustomLevelsPath.c_str(), S_IRWXU | S_IXGRP | S_IRGRP | S_IXOTH | S_IROTH);
	#endif

	// Create database instance
	Database = new _Database();

	return 1;
}

// Closes the save system
int _Save::Close() {

	delete Database;

	return 1;
}

// Init the stats database file
int _Save::InitStatsDatabase() {
	int DatabaseVersion = -1;

	// Open stats database and get file version
	if(Database->OpenDatabase(StatsFile.c_str())) {
		int Result = Database->RunDataQuery("SELECT Version from DatabaseInfo");
		if(Result)
			DatabaseVersion = Database->GetInt(0);
		Database->CloseQuery();

		// Upgrade old database versions
		if(DatabaseVersion == 0) {
		}
	}

	// Create new database if it doesn't exist
	if(DatabaseVersion == -1) {

		// Create new database
		Log.Write("_Save::InitStatsDatabase - Creating new stats database...\n");
		if(!Database->OpenDatabaseCreate(StatsFile.c_str())) {
			return 0;
		}

		// Setup database
		Database->RunQuery("PRAGMA journal_mode = 'OFF'");
		Database->RunQuery("BEGIN TRANSACTION");

		// Create version table
		Database->RunQuery("CREATE TABLE DatabaseInfo('Version' INTEGER)");

		// Create stats table
		Database->RunQuery(
			"CREATE TABLE Stats(\n"
			"ID INTEGER PRIMARY KEY,\n"
			"LevelFile TEXT,\n"
			"Unlocked INTEGER DEFAULT(0),\n"
			"Difficulty INTEGER DEFAULT(0),\n"
			"Loads INTEGER DEFAULT(0),\n"
			"Wins INTEGER DEFAULT(0),\n"
			"Loses INTEGER DEFAULT(0),\n"
			"PlayTime FLOAT DEFAULT(0)\n"
			")\n");

		// Create highscores table
		Database->RunQuery(
			"CREATE TABLE HighScores(\n"
			"ID INTEGER PRIMARY KEY,\n"
			"StatsID INTEGER DEFAULT(0),\n"
			"Time FLOAT,\n"
			"Date INTEGER\n"
			")");

		// Create indexes
		Database->RunQuery("CREATE INDEX StatsLevelFile on Stats (LevelFile ASC)");

		// Add version number
		char Buffer[256];
		sprintf(Buffer, "INSERT INTO DatabaseInfo(Version) VALUES(%d)", STATS_VERSION);
		Database->RunQuery(Buffer);

		// End transaction
		Database->RunQuery("END TRANSACTION");
	}

	return 1;
}

// Populates the level stats map
int _Save::LoadLevelStats() {
	Log.Write("_Save::LoadLevelStats - Loading save file");
	LevelStats.clear();
	char Buffer[256];

	// Get level stats
	Database->RunQuery("PRAGMA journal_mode = 'OFF'");
	Database->RunQuery("BEGIN TRANSACTION");
	Database->RunDataQuery("SELECT * FROM Stats");

	// Get data
	while(Database->FetchRow()) {
		_LevelStat Stat;
		std::string LevelFile;

		// Get record data
		Stat.ID = Database->GetInt(0);
		LevelFile = Database->GetString(1);
		Stat.Unlocked = Database->GetInt(2);
		Stat.LoadCount = Database->GetInt(4);
		Stat.WinCount = Database->GetInt(5);
		Stat.LoseCount = Database->GetInt(6);
		Stat.PlayTime = Database->GetFloat(7);

		// Get highscores
		sprintf(Buffer, "SELECT * FROM HighScores where StatsID = %d", Stat.ID);
		Database->RunDataQuery(Buffer, 1);
		while(Database->FetchRow(1)) {
			_HighScore HighScore;
			HighScore.Time = Database->GetFloat(2, 1);
			HighScore.DateStamp = Database->GetInt(3, 1);
			Stat.HighScores.push_back(HighScore);
		}
		Database->CloseQuery(1);

		// Add record to map
		LevelStats[LevelFile] = Stat;
	}

	// Close query
	Database->RunQuery("END TRANSACTION");
	Database->CloseQuery();

	return 1;
}

// Update the stats database for one level
void _Save::SaveLevelStats(const std::string &Level) {

	auto LevelStatsIterator = LevelStats.find(Level);
	if(LevelStatsIterator == LevelStats.end())
		return;

	// Get stat from map
	_LevelStat &Stat = LevelStatsIterator->second;

	// Update database
	char Query[512];
	sprintf(Query, "UPDATE Stats SET "
					"Loads = %d"
					", Wins = %d"
					", Loses = %d"
					", PlayTime = %f"
					" WHERE ID = %d",
					Stat.LoadCount,
					Stat.WinCount,
					Stat.LoseCount,
					Stat.PlayTime,
					Stat.ID);

	Database->RunQuery(Query);
}

// Adds a score to the list if it's high enough
int _Save::AddScore(const std::string &Level, float Time) {

	// Get level stats
	_LevelStat &Stats = LevelStats[Level];

	// Add score
	int InsertIndex = -1;
	if(Stats.HighScores.size() == 0) {
		InsertIndex = 0;
	}
	else {

		// Find score place
		for(std::size_t i = 0; i < Stats.HighScores.size(); i++) {
			if(Time < Stats.HighScores[i].Time) {
				InsertIndex = i;
				break;
			}
		}

		// Add to end
		if(InsertIndex == -1 && Stats.HighScores.size() < STATS_MAXSCORES) {
			InsertIndex = Stats.HighScores.size();
		}
	}

	// Insert high score
	if(InsertIndex != -1) {
		Stats.HighScores.insert(Stats.HighScores.begin() + InsertIndex, _HighScore(Time, (int)time(nullptr)));

		// Cut off old scores
		int ExcessScores = Stats.HighScores.size() - STATS_MAXSCORES;
		if(ExcessScores > 0)
			Stats.HighScores.erase(Stats.HighScores.end() - ExcessScores, Stats.HighScores.end());

		// Update database
		Database->RunQuery("BEGIN TRANSACTION");

		// Delete old scores
		char Query[256];
		sprintf(Query, "DELETE FROM HighScores WHERE StatsID = %d", Stats.ID);
		Database->RunQuery(Query);

		// Insert new scores
		for(std::size_t i = 0; i < Stats.HighScores.size(); i++) {
			sprintf(Query, "INSERT INTO HighScores(StatsID, Time, Date) VALUES(%d, %f, %d)", Stats.ID, Stats.HighScores[i].Time, (int)Stats.HighScores[i].DateStamp);
			Database->RunQuery(Query);
		}

		Database->RunQuery("END TRANSACTION");
	}

	return InsertIndex;
}

// Unlocks a level
void _Save::UnlockLevel(const std::string &Level) {

	// Get stat from map
	_LevelStat &Stats = LevelStats[Level];

	// Unlock level
	Stats.Unlocked = 1;

	// Check to see if level exists in database
	char Query[512];
	snprintf(Query, 512, "SELECT ID FROM Stats WHERE LevelFile = '%s'", Level.c_str());
	int Count = Database->RunCountQuery(Query);

	// Insert
	if(Count == 0) {
		sprintf(Query, "INSERT INTO Stats(LevelFile, Unlocked, Difficulty) VALUES('%s', %d, %d)", Level.c_str(), Stats.Unlocked, 0);
		Database->RunQuery(Query);
		Stats.ID = (int)Database->GetLastInsertID();
	}
}
