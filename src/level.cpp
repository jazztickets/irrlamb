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
#include <level.h>
#include <globals.h>
#include <game.h>
#include <objectmanager.h>
#include <scripting.h>
#include <graphics.h>
#include <save.h>
#include <replay.h>
#include <log.h>
#include <physics.h>
#include <input.h>
#include <audio.h>
#include <filestream.h>
#include <config.h>
#include <objects/template.h>
#include <objects/player.h>
#include <objects/sphere.h>
#include <objects/box.h>
#include <objects/orb.h>
#include <objects/cylinder.h>
#include <objects/terrain.h>
#include <objects/zone.h>
#include <objects/collision.h>
#include <objects/constraint.h>
#include <objects/springjoint.h>
#include <tinyxml/tinyxml2.h>
#include <ISceneManager.h>
#include <IFileSystem.h>

_Level Level;

using namespace irr;
using namespace tinyxml2;

// Handle user data from .irr file
void _UserDataLoader::OnReadUserData(irr::scene::ISceneNode *ForSceneNode, irr::io::IAttributes *UserData) {

	for(uint32_t i = 0; i < UserData->getAttributeCount(); i++) {
		core::stringc Name(UserData->getAttributeName(i));

		if(Name == "BackgroundColor") {
			video::SColorf FloatColor(UserData->getAttributeAsColorf(i));
			Level.ClearColor.set(255, (uint32_t)(255 * FloatColor.r), (uint32_t)(255 * FloatColor.g), (uint32_t)(255 * FloatColor.b));
		}
	}

	return;
}

// Loads a level file
int _Level::Init(const std::string &LevelName, bool HeaderOnly) {
	if(!HeaderOnly)
		Log.Write("_Level::Init - Loading level: %s", LevelName.c_str());

	// Get fastest time
	FastestTime = 0.0f;
	const SaveLevelStruct *Stats = Save.GetLevelStats(LevelName);
	if(Stats->HighScores.size() > 0)
		FastestTime = Stats->HighScores[0].Time;

	// Get paths
	this->LevelName = LevelName;
	LevelNiceName = "";
	std::string LevelFile = LevelName + "/" + LevelName + ".xml";
	std::string FilePath = Game.GetWorkingPath() + std::string("levels/") + LevelFile;
	std::string CustomFilePath = Save.GetCustomLevelsPath() + LevelFile;
	std::string DataPath = Game.GetWorkingPath() + std::string("levels/") + LevelName + "/";

	// See if custom level exists first
	IsCustomLevel = false;
	std::ifstream CustomLevelExists(CustomFilePath.c_str());
	if(CustomLevelExists) {
		IsCustomLevel = true;
		FilePath = CustomFilePath;
		DataPath = Save.GetCustomLevelsPath() + LevelName + "/";
	}
	CustomLevelExists.close();

	// Open the XML file
	XMLDocument Document;
	if(Document.LoadFile(FilePath.c_str()) != XML_NO_ERROR) {
		Log.Write("Error loading level file with error id = %d", Document.ErrorID());
		Log.Write("Error string 1: %s", Document.GetErrorStr1());
		Log.Write("Error string 2: %s", Document.GetErrorStr2());
		Close();
		return 0;
	}

	// Check for level tag
	XMLElement *LevelElement = Document.FirstChildElement("level");
	if(!LevelElement) {
		Log.Write("Could not find level tag");
		Close();
		return 0;
	}

	// Level version
	if(LevelElement->QueryIntAttribute("version", &LevelVersion) == XML_NO_ATTRIBUTE) {
		Log.Write("Could not find level version");
		Close();
		return 0;
	}

	// Check required game version
	GameVersion = LevelElement->Attribute("gameversion");
	if(GameVersion == "") {
		Log.Write("Could not find game version attribute");
		Close();
		return 0;
	}

	// Load level info
	XMLElement *InfoElement = LevelElement->FirstChildElement("info");
	if(InfoElement) {
		XMLElement *NiceNameElement = InfoElement->FirstChildElement("name");
		if(NiceNameElement) {
			LevelNiceName = NiceNameElement->GetText();
		}
	}

	// Return after header is read
	if(HeaderOnly) {
		Close();
		return true;
	}

	// Load default lua script
	Scripts.clear();
	Scripts.push_back(Game.GetWorkingPath() + "scripts/default.lua");

	// Options
	bool Fog = false;
	bool EmitLight = false;
	Level.ClearColor.set(255, 0, 0, 0);

	// Load options
	XMLElement *OptionsElement = LevelElement->FirstChildElement("options");
	if(OptionsElement) {

		// Does the player emit light?
		XMLElement *EmitLightElement = OptionsElement->FirstChildElement("emitlight");
		if(EmitLightElement) {
			EmitLightElement->QueryBoolAttribute("enabled", &EmitLight);
		}
	}

	// Load world
	XMLElement *ResourcesElement = LevelElement->FirstChildElement("resources");
	if(ResourcesElement) {

		// Load scenes
		for(XMLElement *SceneElement = ResourcesElement->FirstChildElement("scene"); SceneElement != 0; SceneElement = SceneElement->NextSiblingElement("scene")) {

			// Get file
			std::string File = SceneElement->Attribute("file");
			if(File == "") {
				Log.Write("Could not find file attribute on scene");
				Close();
				return 0;
			}

			// Reset fog
			irrDriver->setFog(video::SColor(0, 0, 0, 0), video::EFT_FOG_EXP, 0, 0, 0.0f);

			// Load scene
			if(IsCustomLevel) {
				irrFile->changeWorkingDirectoryTo(DataPath.c_str());
				irrScene->loadScene(File.c_str(), &UserDataLoader);
				irrFile->changeWorkingDirectoryTo(Game.GetWorkingPath().c_str());
			}
			else {
				irrScene->loadScene((DataPath + File).c_str(), &UserDataLoader);
			}

			// Set texture filters on meshes in the scene
			core::array<irr::scene::ISceneNode *> MeshNodes;
			irrScene->getSceneNodesFromType(scene::ESNT_MESH, MeshNodes);
			for(uint32_t i = 0; i < MeshNodes.size(); i++) {
				if(EmitLight && Config.Shaders) {
					video::SMaterial &Material = MeshNodes[i]->getMaterial(0);
					int ShaderType = 0;
					if(Material.MaterialType == video::EMT_TRANSPARENT_ALPHA_CHANNEL) {
						ShaderType = 1;
					}
					MeshNodes[i]->setMaterialType((video::E_MATERIAL_TYPE)Graphics.GetCustomMaterial(ShaderType));
				}

				MeshNodes[i]->setMaterialFlag(video::EMF_TRILINEAR_FILTER, Config.TrilinearFiltering);
				for(uint32_t j = 0; j < MeshNodes[i]->getMaterialCount(); j++) {
					for(int k = 0; k < 4; k++) {
						MeshNodes[i]->getMaterial(j).TextureLayer[k].AnisotropicFilter = Config.AnisotropicFiltering;
						if(MeshNodes[i]->getMaterial(j).FogEnable)
							Fog = true;
					}
				}
			}
		}

		// Load collision
		for(XMLElement *CollisionElement = ResourcesElement->FirstChildElement("collision"); CollisionElement != 0; CollisionElement = CollisionElement->NextSiblingElement("collision")) {

			// Get file
			std::string File = CollisionElement->Attribute("file");
			if(File == "") {
				Log.Write("Could not find file attribute on collision");
				Close();
				return 0;
			}

			// Create template
			TemplateStruct *Template = new TemplateStruct;
			Template->CollisionFile = DataPath + File;
			Template->Type = _Object::COLLISION;
			Template->Mass = 0.0f;
			Templates.push_back(Template);

			// Create spawn
			SpawnStruct *ObjectSpawn = new SpawnStruct;
			ObjectSpawn->Template = Template;
			ObjectSpawns.push_back(ObjectSpawn);
		}

		// Load scripts
		for(XMLElement *ScriptElement = ResourcesElement->FirstChildElement("script"); ScriptElement != 0; ScriptElement = ScriptElement->NextSiblingElement("script")) {

			// Get file
			std::string File = ScriptElement->Attribute("file");
			if(File == "") {
				Log.Write("Could not find file attribute on script");
				Close();
				return 0;
			}

			Scripts.push_back(DataPath + File);
		}

		// Load sounds
		Sounds.clear();
		for(XMLElement *SoundElement = ResourcesElement->FirstChildElement("sound"); SoundElement != 0; SoundElement = SoundElement->NextSiblingElement("sound")) {

			// Get file
			std::string File = SoundElement->Attribute("file");
			if(File == "") {
				Log.Write("Could not find file attribute on sound");
				Close();
				return 0;
			}

			// Attempt to load sound
			if(Audio.LoadBuffer(File))
				Sounds.push_back(File);
		}
	}

	// Load templates
	XMLElement *TemplatesElement = LevelElement->FirstChildElement("templates");
	if(TemplatesElement) {
		int TemplateID = 0;
		for(XMLElement *TemplateElement = TemplatesElement->FirstChildElement(); TemplateElement != 0; TemplateElement = TemplateElement->NextSiblingElement()) {

			// Create a template
			TemplateStruct *Template = new TemplateStruct;
			Template->Fog = Fog;

			// Get the template properties
			if(!GetTemplateProperties(TemplateElement, *Template))
				return 0;

			// Assign options
			Template->TemplateID = TemplateID;
			if(EmitLight) {

				// Use shaders on materials that receive light
				if(Config.Shaders)
					Template->CustomMaterial = Graphics.GetCustomMaterial(0);

				// Set the player to emit light
				if(Template->Type == _Object::PLAYER || Template->Type == _Object::ORB)
					Template->EmitLight = true;
			}
			TemplateID++;

			// Store for later
			Templates.push_back(Template);
		}
	}

	// Load object spawns
	XMLElement *ObjectsElement = LevelElement->FirstChildElement("objects");
	if(ObjectsElement) {
		for(XMLElement *ObjectElement = ObjectsElement->FirstChildElement(); ObjectElement != 0; ObjectElement = ObjectElement->NextSiblingElement()) {

			// Create an object spawn
			SpawnStruct *ObjectSpawn = new SpawnStruct;

			// Get the object properties
			if(!GetObjectSpawnProperties(ObjectElement, *ObjectSpawn))
				return 0;

			// Store for later
			ObjectSpawns.push_back(ObjectSpawn);
		}
	}

	return 1;
}

// Closes the level
int _Level::Close() {

	// Clear scripts
	Scripts.clear();

	// Clear sounds
	for(size_t i = 0; i < Sounds.size(); i++) {
		Audio.CloseBuffer(Sounds[i]);
	}
	Sounds.clear();

	// Delete templates
	for(size_t i = 0; i < Templates.size(); i++) {
		delete Templates[i];
	}
	Templates.clear();

	// Delete object spawn data
	for(size_t i = 0; i < ObjectSpawns.size(); i++) {
		delete ObjectSpawns[i];
	}
	ObjectSpawns.clear();

	return 1;
}

// Processes a template tag
int _Level::GetTemplateProperties(XMLElement *TemplateElement, TemplateStruct &Object) {
	XMLElement *Element;
	const char *String;

	// Get type
	std::string ObjectType = TemplateElement->Value();

	// Get name
	String = TemplateElement->Attribute("name");
	if(!String) {
		Log.Write("Template is missing name");
		return 0;
	}
	Object.Name = String;

	// Get attributes
	TemplateElement->QueryFloatAttribute("lifetime", &Object.Lifetime);
	TemplateElement->QueryIntAttribute("smooth", &Object.Smooth);

	// Get scale
	Element = TemplateElement->FirstChildElement("shape");
	if(Element) {
		Element->QueryFloatAttribute("w", &Object.Shape[0]);
		Element->QueryFloatAttribute("h", &Object.Shape[1]);
		Element->QueryFloatAttribute("l", &Object.Shape[2]);
		Element->QueryFloatAttribute("r", &Object.Radius);
	}

	// Get mesh properties
	Element = TemplateElement->FirstChildElement("mesh");
	if(Element) {
		String = Element->Attribute("file");
		if(String)
			Object.Mesh = String;

		Element->QueryFloatAttribute("scale", &Object.Scale);
	}

	// Get physical attributes
	Element = TemplateElement->FirstChildElement("physics");
	if(Element) {
		Element->QueryIntAttribute("sleep", &Object.Sleep);
		Element->QueryFloatAttribute("mass", &Object.Mass);
		Element->QueryFloatAttribute("friction", &Object.Friction);
		Element->QueryFloatAttribute("restitution", &Object.Restitution);
	}

	// Get damping
	Element = TemplateElement->FirstChildElement("damping");
	if(Element) {
		Element->QueryFloatAttribute("linear", &Object.LinearDamping);
		Element->QueryFloatAttribute("angular", &Object.AngularDamping);
	}

	// Get limits for constraints. 0-1 is linear, 2-3 is angular
	Element = TemplateElement->FirstChildElement("limit");
	if(Element) {
		int Type = 0;
		Element->QueryIntAttribute("type", &Type);
		Element->QueryFloatAttribute("x", &Object.ConstraintData[Type][0]);
		Element->QueryFloatAttribute("y", &Object.ConstraintData[Type][1]);
		Element->QueryFloatAttribute("z", &Object.ConstraintData[Type][2]);
	}

	// Get pivot for constraints
	Element = TemplateElement->FirstChildElement("pivot");
	if(Element) {
		Element->QueryFloatAttribute("x", &Object.ConstraintData[0][0]);
		Element->QueryFloatAttribute("y", &Object.ConstraintData[0][1]);
		Element->QueryFloatAttribute("z", &Object.ConstraintData[0][2]);
	}

	// Get axis for constraints
	Element = TemplateElement->FirstChildElement("axis");
	if(Element) {
		Element->QueryFloatAttribute("x", &Object.ConstraintData[1][0]);
		Element->QueryFloatAttribute("y", &Object.ConstraintData[1][1]);
		Element->QueryFloatAttribute("z", &Object.ConstraintData[1][2]);
	}

	// Get textures
	for(XMLElement *Element = TemplateElement->FirstChildElement("texture"); Element != 0; Element = TemplateElement->NextSiblingElement("texture")) {

		// Get texture index
		int Index = 0;
		Element->QueryIntAttribute("index", &Index);
		if(Index > 3) {
			Log.Write("Texture index out of bounds! %d > 3", Index);
			return 0;
		}

		// Get texture scale
		Element->QueryFloatAttribute("scale", &Object.TextureScale[Index]);

		// Get filename
		const char *Filename = Element->Attribute("file");
		if(Filename) {
			Object.Textures[Index] = Game.GetWorkingPath() + std::string("textures/") + Filename;
			if(!irrFile->existFile(Object.Textures[Index].c_str())) {
				Log.Write("Texture file does not exist: %s", Filename);
				return 0;
			}
		}
	}

	// Validate objects
	if(ObjectType == "player") {
		Object.Type = _Object::PLAYER;
		Object.CollisionGroup &= ~_Physics::FILTER_CAMERA;
		Object.Fog = false;
	}
	else if(ObjectType == "orb") {
		Object.Type = _Object::ORB;
	}
	else if(ObjectType == "sphere") {
		Object.Type = _Object::SPHERE;
	}
	else if(ObjectType == "box") {
		Object.Type = _Object::BOX;
	}
	else if(ObjectType == "cylinder") {
		Object.Type = _Object::CYLINDER;
	}
	else if(ObjectType == "terrain") {
		Object.Type = _Object::TERRAIN;
		Object.Mass = 0.0f;
	}
	else if(ObjectType == "zone") {
		Object.Type = _Object::ZONE;
		Object.Mass = 0.0f;
		Object.CollisionGroup = _Physics::FILTER_ZONE;
		Object.CollisionMask = _Physics::FILTER_RIGIDBODY | _Physics::FILTER_KINEMATIC;
	}
	else if(ObjectType == "d6") {
		Object.Type = _Object::CONSTRAINT_D6;
	}
	else if(ObjectType == "hinge") {
		Object.Type = _Object::CONSTRAINT_HINGE;
	}

	// If a rigidbody's mass is zero, set group to static
	if(Object.Mass == 0.0f && Object.CollisionGroup == _Physics::FILTER_RIGIDBODY) {
		Object.CollisionGroup = _Physics::FILTER_STATIC | _Physics::FILTER_CAMERA;

		// Prevent collision with other static object
		Physics.RemoveFilter(Object.CollisionMask, _Physics::FILTER_STATIC);
	}

	return 1;
}

// Processes an object tag
int _Level::GetObjectSpawnProperties(XMLElement *ObjectElement, SpawnStruct &ObjectSpawn) {
	XMLElement *Element;

	// Get name
	ObjectSpawn.Name = ObjectElement->Attribute("name");
	if(ObjectSpawn.Name == "") {
		Log.Write("Object is missing name");
		return 0;
	}

	// Get template name
	std::string TemplateName = ObjectElement->Attribute("template");
	if(TemplateName == "") {
		Log.Write("Object is missing template name");
		return 0;
	}

	// Get template data
	TemplateStruct *Template = GetTemplate(TemplateName);
	if(Template == NULL) {
		Log.Write("Cannot find template %s", TemplateName.c_str());
		return 0;
	}
	ObjectSpawn.Template = Template;

	// Get position
	Element = ObjectElement->FirstChildElement("position");
	if(Element) {
		Element->QueryFloatAttribute("x", &ObjectSpawn.Position[0]);
		Element->QueryFloatAttribute("y", &ObjectSpawn.Position[1]);
		Element->QueryFloatAttribute("z", &ObjectSpawn.Position[2]);
	}

	// Get euler rotation
	Element = ObjectElement->FirstChildElement("rotation");
	if(Element) {
		Element->QueryFloatAttribute("x", &ObjectSpawn.Rotation[0]);
		Element->QueryFloatAttribute("y", &ObjectSpawn.Rotation[1]);
		Element->QueryFloatAttribute("z", &ObjectSpawn.Rotation[2]);
	}

	return 1;
}

// Spawns all of the objects in the level
void _Level::SpawnObjects() {

	for(size_t i = 0; i < ObjectSpawns.size(); i++) {
		SpawnStruct *Spawn = ObjectSpawns[i];
		CreateObject(*Spawn);
	}
}

// Creates an object from a spawn struct
_Object *_Level::CreateObject(const SpawnStruct &Object) {

	// Add object
	_Object *NewObject = NULL;
	switch(Object.Template->Type) {
		case _Object::PLAYER:
			NewObject = ObjectManager.AddObject(new _Player(Object));
		break;
		case _Object::ORB:
			NewObject = ObjectManager.AddObject(new _Orb(Object));
		break;
		case _Object::COLLISION:
			NewObject = ObjectManager.AddObject(new _Collision(Object));
		break;
		case _Object::SPHERE:
			NewObject = ObjectManager.AddObject(new _Sphere(Object));
		break;
		case _Object::BOX:
			NewObject = ObjectManager.AddObject(new _Box(Object));
		break;
		case _Object::CYLINDER:
			NewObject = ObjectManager.AddObject(new _Cylinder(Object));
		break;
		case _Object::TERRAIN:
			NewObject = ObjectManager.AddObject(new _Terrain(Object));
		break;
		case _Object::ZONE:
			NewObject = ObjectManager.AddObject(new _Zone(Object));
		break;
	}

	// Record replay event
	if(Replay.IsRecording() && Object.Template->TemplateID != -1) {

		// Write replay information
		_File &ReplayStream = Replay.GetReplayStream();
		Replay.WriteEvent(_Replay::PACKET_CREATE);
		ReplayStream.WriteShortInt(Object.Template->TemplateID);
		ReplayStream.WriteShortInt(NewObject->GetID());
		ReplayStream.WriteData((void *)&Object.Position, sizeof(btScalar) * 3);
		ReplayStream.WriteData((void *)&Object.Rotation, sizeof(btScalar) * 3);
	}

	return NewObject;
}

// Creates a constraint from a template
_Object *_Level::CreateConstraint(const ConstraintStruct &Object) {

	// Add object
	_Object *NewObject = ObjectManager.AddObject(new _Constraint(Object));

	return NewObject;
}

// Gets a template by name
TemplateStruct *_Level::GetTemplate(const std::string &Name) {

	// Search templates by name
	for(size_t i = 0; i < Templates.size(); i++) {
		if(Templates[i]->Name == Name) {
			return Templates[i];
		}
	}

	return NULL;
}

// Gets a template by name
TemplateStruct *_Level::GetTemplateFromID(int ID) {

	// Search templates by id
	for(size_t i = 0; i < Templates.size(); i++) {
		if(Templates[i]->TemplateID == ID) {
			return Templates[i];
		}
	}

	return NULL;
}

// Runs the level's scripts
void _Level::RunScripts() {

	// Reset Lua state
	Scripting.Reset();

	// Add key names to lua scope
	Scripting.DefineLuaVariable("KEY_FORWARD", Input.GetKeyName(Actions.GetInputForAction(_Input::KEYBOARD, _Actions::MOVE_FORWARD)));
	Scripting.DefineLuaVariable("KEY_BACK", Input.GetKeyName(Actions.GetInputForAction(_Input::KEYBOARD, _Actions::MOVE_BACK)));
	Scripting.DefineLuaVariable("KEY_LEFT", Input.GetKeyName(Actions.GetInputForAction(_Input::KEYBOARD, _Actions::MOVE_LEFT)));
	Scripting.DefineLuaVariable("KEY_RIGHT", Input.GetKeyName(Actions.GetInputForAction(_Input::KEYBOARD, _Actions::MOVE_RIGHT)));
	Scripting.DefineLuaVariable("KEY_RESET", Input.GetKeyName(Actions.GetInputForAction(_Input::KEYBOARD, _Actions::RESET)));
	Scripting.DefineLuaVariable("KEY_JUMP", Input.GetKeyName(Actions.GetInputForAction(_Input::KEYBOARD, _Actions::JUMP)));

	// Run scripts
	for(uint32_t i = 0; i < Scripts.size(); i++) {

		// Load a level
		Scripting.LoadFile(Scripts[i]);
	}
}
