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
#include <ISceneNode.h>
#include <ISceneUserDataSerializer.h>
#include <string>
#include <vector>

// Forward Declarations
namespace tinyxml2 {
	class XMLElement;
}
class _Object;
struct _Template;
struct _ObjectSpawn;
struct _ConstraintSpawn;

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

		// Objects
		void SpawnEntities();
		_Object *CreateObject(const _ObjectSpawn &Object);
		_Object *CreateConstraint(const _ConstraintSpawn &Object);

		// Templates
		_Template *GetTemplate(const std::string &Name);
		_Template *GetTemplateFromID(int ID);

		// Scripts
		void RunScripts();

		// Attributes
		std::string LevelName;
		std::string LevelNiceName;
		int LevelVersion;
		bool IsCustomLevel;
		std::string GameVersion;
		irr::video::SColor ClearColor;
		_UserDataLoader UserDataLoader;
		float FastestTime;

	private:

		// Loading
		int GetTemplateProperties(tinyxml2::XMLElement *TemplateElement, _Template &Template);
		int GetObjectSpawnProperties(tinyxml2::XMLElement *ObjectElement, _ObjectSpawn &ObjectSpawn);
		int GetConstraintSpawnProperties(tinyxml2::XMLElement *ConstraintElement, _ConstraintSpawn &ConstraintSpawn);

		// Custom levels
		std::string CustomDataPath;

		// Resources
		std::vector<std::string> Scripts;
		std::vector<std::string> Sounds;

		// Objects
		std::vector<_Template *> Templates;
		std::vector<_ObjectSpawn *> ObjectSpawns;
		std::vector<_ConstraintSpawn *> ConstraintSpawns;
};

// Singletons
extern _Level Level;
