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
#include <ISceneNode.h>
#include <ISceneUserDataSerializer.h>
#include <string>
#include <vector>

// Forward Declarations
namespace tinyxml2 {
	class XMLElement;
}
class _Object;
struct TemplateStruct;
struct SpawnStruct;
struct ConstraintStruct;

// Handle user data from .irr file
class _UserDataLoader : public irr::scene::ISceneUserDataSerializer {
	void OnCreateNode(irr::scene::ISceneNode *Node) { }
	void OnReadUserData(irr::scene::ISceneNode *ForSceneNode, irr::io::IAttributes *UserData);
	irr::io::IAttributes* createUserData(irr::scene::ISceneNode *ForSceneNode) { return 0; }
};

// Classes
class _Level {
	friend class _UserDataLoader;

	public:

		int Init(const std::string &LevelName, bool HeaderOnly=false);
		int Close();

		// Properties
		const std::string &GetLevelName() { return LevelName; }
		const std::string &GetLevelNiceName() { return LevelNiceName; }
		int GetLevelVersion() { return LevelVersion; }
		const irr::video::SColor &GetClearColor() { return ClearColor; }
		float GetFastestTime() { return FastestTime; }

		// Objects
		void SpawnObjects();
		_Object *CreateObject(const SpawnStruct &Object);
		_Object *CreateConstraint(const ConstraintStruct &Object);

		// Templates
		TemplateStruct *GetTemplate(const std::string &Name);
		TemplateStruct *GetTemplateFromID(int ID);

		// Scripts
		void RunScripts();

	private:

		// Loading
		int GetTemplateProperties(tinyxml2::XMLElement *TemplateElement, TemplateStruct &Object);
		int GetObjectSpawnProperties(tinyxml2::XMLElement *ObjectElement, SpawnStruct &ObjectSpawn);

		// Level
		std::string LevelName, LevelNiceName;
		int LevelVersion;
		bool IsCustomLevel;
		std::string GameVersion;
		irr::video::SColor ClearColor;
		_UserDataLoader UserDataLoader;
		float FastestTime;

		// Resources
		std::vector<std::string> Scripts;
		std::vector<std::string> Sounds;

		// Objects
		std::vector<TemplateStruct *> Templates;
		std::vector<SpawnStruct *> ObjectSpawns;
};

// Singletons
extern _Level Level;
