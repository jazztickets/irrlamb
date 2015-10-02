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
#include <campaign.h>
#include <globals.h>
#include <log.h>
#include <game.h>
#include <level.h>
#include <tinyxml/tinyxml2.h>

_Campaign Campaign;

using namespace tinyxml2;

// Loads the campaign data
int _Campaign::Init() {
	Campaigns.clear();

	Log.Write("_Campaign::Init - Loading file irrlamb.xml");

	// Open the XML file
	std::string LevelFile = std::string("levels/main.xml");
	XMLDocument Document;
	if(Document.LoadFile(LevelFile.c_str()) != XML_NO_ERROR) {
		Log.Write("Error loading level file with error id = %d", Document.ErrorID());
		Log.Write("Error string 1: %s", Document.GetErrorStr1());
		Log.Write("Error string 2: %s", Document.GetErrorStr2());
		Close();
		return 0;
	}

	// Check for level tag
	XMLElement *CampaignsElement = Document.FirstChildElement("campaigns");
	if(!CampaignsElement) {
		Log.Write("Could not find campaigns tag");
		return 0;
	}

	// Load campaigns
	XMLElement *CampaignElement = CampaignsElement->FirstChildElement("campaign");
	for(; CampaignElement != 0; CampaignElement = CampaignElement->NextSiblingElement("campaign")) {

		_CampaignInfo Campaign;
		Campaign.Name = CampaignElement->Attribute("name");

		// Get levels
		XMLElement *LevelElement = CampaignElement->FirstChildElement("level");
		for(; LevelElement != 0; LevelElement = LevelElement->NextSiblingElement("level")) {
			_LevelInfo Level;
			Level.File = LevelElement->GetText();
			Level.DataPath = Game.GetWorkingPath() + "levels/" + Level.File + "/";
			Level.Unlocked = 0;
			LevelElement->QueryIntAttribute("unlocked", &Level.Unlocked);

			::Level.Init(Level.File, true);
			Level.NiceName = ::Level.LevelNiceName;

			Campaign.Levels.push_back(Level);
		}

		Campaigns.push_back(Campaign);
	}

	return 1;
}

// Closes the campaign system
int _Campaign::Close() {

	Campaigns.clear();

	return 1;
}

// Get next level or 1st level in next campaign, return false if no next level
bool _Campaign::GetNextLevel(uint32_t &Campaign, uint32_t &Level, bool Update) {
	uint32_t NewCampaign = Campaign;
	uint32_t NewLevel = Level;
	if(NewCampaign >= Campaigns.size())
	   return false;

	if(NewLevel+1 >= Campaigns[NewCampaign].Levels.size()) {
		if(NewCampaign+1 >= Campaigns.size())
			return false;
		else {
			NewCampaign++;
			NewLevel = 0;
		}
	}
	else {
		NewLevel++;
	}

	// Return values
	if(Update) {
		Campaign = NewCampaign;
		Level = NewLevel;
	}

	return true;
}
