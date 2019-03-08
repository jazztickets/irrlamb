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
#include <vector>
#include <map>
#include <string>
#include <ctime>

// Forward Declarations
class _Database;

// Struct for one highscore
struct _HighScore {
	_HighScore() : Time(0), DateStamp(0) { }
	_HighScore(float Time, int DateStamp) : Time(Time), DateStamp(DateStamp) { }

	bool operator<(const _HighScore &Value) {
		return Time < Value.Time;
	}

	float Time;
	time_t DateStamp;
};

// Struct for one level stat
struct _LevelStat {
	_LevelStat() : ID(0), Unlocked(0), LoadCount(0), LoseCount(0), WinCount(0), PlayTime(0) { }

	int ID;
	int Unlocked;
	int LoadCount;
	int LoseCount;
	int WinCount;
	float PlayTime;
	std::vector<_HighScore> HighScores;
};

// Classes
class _Save {

	public:

		int Init();
		int Close();

		int InitStatsDatabase();

		int LoadLevelStats();
		void SaveLevelStats(const std::string &Level);

		int AddScore(const std::string &Level, float Time);
		void UnlockLevel(const std::string &Level);

		// Paths
		std::string SavePath;
		std::string ReplayPath;
		std::string ScreenshotsPath;
		std::string CustomLevelsPath;
		std::string CachePath;
		std::string ConfigFile;
		std::string StatsFile;

		// Stats
		std::map<std::string, _LevelStat> LevelStats;

	private:

		// Database
		_Database *Database;
};

// Singletons
extern _Save Save;
