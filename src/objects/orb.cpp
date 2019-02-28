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
#include <objects/orb.h>
#include <globals.h>
#include <graphics.h>
#include <physics.h>
#include <scripting.h>
#include <replay.h>
#include <constants.h>
#include <config.h>
#include <audio.h>
#include <objects/template.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <ISceneManager.h>
#include <IMeshSceneNode.h>

using namespace irr;

// Constructor
_Orb::_Orb(const _ObjectSpawn &Object)
:	_Object(),
	Light(nullptr),
	Sound(nullptr),
	DeactivationCallback(""),
	State(ORBSTATE_NORMAL),
	OrbTime(0.0f),
	DeactivateLength(ORB_DEACTIVATETIME) {

	_Template *Template = Object.Template;

	// Graphics
	Node = irrScene->addSphereSceneNode(Template->Radius, 24);
	if(Template->Textures[0] != "")
		Node->setMaterialTexture(0, irrDriver->getTexture(Template->Textures[0].c_str()));
	else
		Node->setMaterialTexture(0, irrDriver->getTexture("textures/orb_outer0.png"));
	Node->setMaterialType(video::EMT_ONETEXTURE_BLEND);
	Node->getMaterial(0).MaterialTypeParam = pack_textureBlendFunc(video::EBF_ONE, video::EBF_ONE);

	// Create glow
	GlowColor = video::SColor(255, 255, 255, 255);
	InnerNode = irrScene->addBillboardSceneNode(Node, core::dimension2df(ORB_GLOWSIZE, ORB_GLOWSIZE));
	InnerNode->setColor(GlowColor);
	InnerNode->setMaterialFlag(video::EMF_LIGHTING, false);
	InnerNode->setMaterialFlag(video::EMF_ZBUFFER, false);
	InnerNode->setMaterialType(video::EMT_ONETEXTURE_BLEND);
	InnerNode->getMaterial(0).MaterialTypeParam = pack_textureBlendFunc(video::EBF_ONE, video::EBF_ONE);
	if(Template->Textures[1] != "")
		InnerNode->setMaterialTexture(0, irrDriver->getTexture(Template->Textures[1].c_str()));
	else
		InnerNode->setMaterialTexture(0, irrDriver->getTexture("textures/orb_glow0.png"));

	// Emit Light
	if(Config.MultipleLights && Object.Template->EmitLight) {
		Light = irrScene->addLightSceneNode(0, core::vector3df(Object.Position[0], Object.Position[1], Object.Position[2]), video::SColorf(1.0f, 1.0f, 1.0f), 15.0f);

		video::SLight LightData;
		LightData.Attenuation.set(0.5f, 0.05f, 0.05f);
		LightData.CastShadows = false;
		Light->setLightData(LightData);
	}

	// Set up physics
	if(Physics.IsEnabled()) {

		// Create shape
		btSphereShape *Shape = new btSphereShape(Template->Radius);

		// Set up physics
		CreateRigidBody(Object, Shape);

		// Audio
		Sound = new _AudioSource(Audio.GetBuffer("orb.ogg"), true, 0.0f, 0.40f, 8.0f, 16.0f);
		Sound->SetPitch(ORB_PITCH);
		Sound->SetPosition(Object.Position[0], Object.Position[1], Object.Position[2]);
		Sound->Play();
	}

	SetProperties(Object);
	if(CollisionCallback == "")
		CollisionCallback = "OnHitOrb";
}

// Destructor
_Orb::~_Orb() {
	if(Light) {
		Light->remove();
		Graphics.SetLightCount();
	}

	delete Sound;
}

// Deactivates the object
void _Orb::StartDeactivation(const std::string &Callback, float Length) {

	if(State == ORBSTATE_NORMAL) {
		State = ORBSTATE_DEACTIVATING;
		DeactivationCallback = Callback;
		DeactivateLength = Length;

		// Save the event on the replay
		if(Replay.IsRecording()) {
			std::fstream &ReplayFile = Replay.GetFile();
			Replay.WriteEvent(_Replay::PACKET_ORBDEACTIVATE);
			ReplayFile.write((char *)&ID, sizeof(ID));
			ReplayFile.write((char *)&DeactivateLength, sizeof(DeactivateLength));
		}
	}
}

// Updates the orb
void _Orb::Update(float FrameTime) {

	// Update object
	_Object::Update(FrameTime);

	// Get object position
	const btVector3 &Position = GetPosition();

	// Update light
	if(Light) {
		Light->setPosition(core::vector3df(Position[0], Position[1], Position[2]));
	}

	// Update audio
	Sound->SetPosition(Position[0], Position[1], Position[2]);

	// Update orb
	UpdateDeactivation(FrameTime);
}

// Updates the replay
void _Orb::UpdateReplay(float FrameTime) {
	_Object::UpdateReplay(FrameTime);
	UpdateDeactivation(FrameTime);
}

// Updates the deactivation
void _Orb::UpdateDeactivation(float FrameTime) {

	switch(State) {
		case ORBSTATE_NORMAL:
		break;
		case ORBSTATE_DEACTIVATING: {
			OrbTime += FrameTime;

			// Set glow size
			float PercentLeft = 1.0f - OrbTime / DeactivateLength;
			InnerNode->setSize(core::dimension2df(ORB_GLOWSIZE * PercentLeft, ORB_GLOWSIZE * PercentLeft));
			if(Sound)
				Sound->SetPitch(ORB_PITCH * PercentLeft);

			if(Light) {
				Light->getLightData().DiffuseColor.set(1.0f, PercentLeft, PercentLeft, PercentLeft);
			}

			// Change states
			if(OrbTime >= DeactivateLength) {
				Scripting.CallFunction(DeactivationCallback);
				InnerNode->setVisible(false);
				State = ORBSTATE_DEACTIVATED;

				if(Sound)
					Sound->SetGain(0.0f);
			}
		} break;
		case ORBSTATE_DEACTIVATED:
			if(Light) {
				Light->remove();
				Light = nullptr;
				Graphics.SetLightCount();
			}
		break;
	}
}

// Update the graphic node position
void _Orb::SetPositionFromReplay(const irr::core::vector3df &Position) {
	if(Node)
		Node->setPosition(Position);

	if(Light)
		Light->setPosition(Position);
}
