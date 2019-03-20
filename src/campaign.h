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
#include <string>

// Structures
struct _LevelInfo {
	std::string File;
	std::string DataPath;
	std::string NiceName;
	int Unlocked;
};

struct _CampaignInfo {
	std::string Name;
	bool Show;
	int Column;
	int Row;
	std::vector<_LevelInfo> Levels;
};

// Classes
class _Campaign {

	public:

		int Init();
		int Close();

		const std::vector<_CampaignInfo> &GetCampaigns() { return Campaigns; }
		const _CampaignInfo &GetCampaign(int Index) { return Campaigns[Index]; }

		bool GetNextLevel(uint32_t &Campaign, uint32_t &Level, bool Update=false);
		const std::string &GetLevel(int Campaign, int Level) { return Campaigns[Campaign].Levels[Level].File; }
		const std::string &GetLevelNiceName(int Campaign, int Level) { return Campaigns[Campaign].Levels[Level].NiceName; }

	private:

		std::vector<_CampaignInfo> Campaigns;
};

// Singletons
extern _Campaign Campaign;
