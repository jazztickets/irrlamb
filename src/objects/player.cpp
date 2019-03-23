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
#include <objects/player.h>
#include <constants.h>
#include <globals.h>
#include <physics.h>
#include <input.h>
#include <audio.h>
#include <actions.h>
#include <graphics.h>
#include <config.h>
#include <objects/sphere.h>
#include <objects/constraint.h>
#include <objects/template.h>
#include <ode/objects.h>
#include <ode/collision.h>
#include <IAnimatedMesh.h>
#include <IAnimatedMeshSceneNode.h>
#include <ISceneManager.h>
#include <ISceneNode.h>
#include <IMeshSceneNode.h>
#include <IBillboardSceneNode.h>

using namespace irr;

// Constructor
_Player::_Player(const _ObjectSpawn &Object)
:	_Object(Object.Template),
	Camera(nullptr),
	Light(nullptr),
	Sound(nullptr),
	JumpTimer(0.0f),
	JumpCooldown(0.0f),
	TorqueFactor(4.0f) {

	// Graphics
	Node = irrScene->addSphereSceneNode(Object.Template->Radius, 24);
	Node->setMaterialTexture(0, irrDriver->getTexture("textures/player_outer0.png"));
	Node->setMaterialFlag(video::EMF_LIGHTING, false);
	Node->setMaterialType(video::EMT_ONETEXTURE_BLEND);
	Node->getMaterial(0).MaterialTypeParam = pack_textureBlendFunc(video::EBF_ONE, video::EBF_ONE);

	// Emit Light
	if(Object.Template->EmitLight) {
		Light = irrScene->addLightSceneNode(0, core::vector3df(Object.Position[0], Object.Position[1], Object.Position[2]), video::SColorf(1.0f, 1.0f, 1.0f), 15.0f);
		Light->getLightData().Attenuation.set(0.5f, 0.05f, 0.05f);
		Light->getLightData().DiffuseColor.set(0.0f, 0.75f, 0.75f, 1.0f);
	}

	// Add glow
	scene::ISceneNode *InnerNode;
	InnerNode = irrScene->addBillboardSceneNode(Node, core::dimension2df(1.5f, 1.5f));
	InnerNode->setMaterialFlag(video::EMF_LIGHTING, false);
	InnerNode->setMaterialFlag(video::EMF_ZBUFFER, false);
	InnerNode->setMaterialTexture(0, irrDriver->getTexture("textures/player_glow0.png"));
	InnerNode->setMaterialType(video::EMT_ONETEXTURE_BLEND);
	InnerNode->getMaterial(0).MaterialTypeParam = pack_textureBlendFunc(video::EBF_ONE, video::EBF_ONE);

	if(Physics.IsEnabled()) {

		// Create object
		Geometry = dCreateSphere(Physics.GetSpace(), Object.Template->Radius);
		CreateRigidBody(Object, Geometry);

		// Set mass
		dMass Mass;
		dMassSetSphereTotal(&Mass, Template->Mass, Template->Radius);
		dBodySetMass(Body, &Mass);

		// Audio
		Sound = new _AudioSource(Audio.GetBuffer("player.ogg"), true, 0.0, 0.50f);
		Sound->SetPosition(Object.Position[0], Object.Position[1], Object.Position[2]);
		Sound->Play();
	}

	SetProperties(Object);
	if(CollisionCallback == "")
		CollisionCallback = "OnHitPlayer";
}

// Destructor
_Player::~_Player() {

	if(Light)
		Light->remove();

	delete Sound;
}

// Update the player
void _Player::Update(float FrameTime) {
	_Object::Update(FrameTime);

	// Update audio
	const dReal *Position = GetPosition();
	Sound->SetPosition(Position[0], Position[1], Position[2]);
	Sound->SetGain(Config.PlayerSounds);

	// Update light
	if(Light) {
		Light->setPosition(core::vector3df(Position[0], Position[1], Position[2]));
	}

	/*
	// Get pitch for player idle sound
	float MinSpeed = 3.0f;
	float MaxSpeed = 120.0f;
	float Pitch = std::max(MinSpeed, std::min(GetAngularVelocity().length() + GetLinearVelocity().length(), MaxSpeed));
	Pitch -= MinSpeed;
	Pitch /= MaxSpeed / 2;
	Pitch += 0.9f;
	Sound->SetPitch(Pitch);
*/
	// Update jump cooldown
	if(JumpCooldown > 0.0f) {
		JumpCooldown -= FrameTime;
	}

	// Update jump timer
	if(JumpTimer > 0.0f) {
		JumpTimer -= FrameTime;
		if(JumpTimer < 0.0f)
			JumpTimer = 0.0f;

		if(TouchingGround && JumpCooldown <= 0.0f) {
			if(Body) {
				//Body->activate();
				dBodyAddForce(Body, 0, JUMP_POWER * (1.0f / FrameTime), 0);
			}
			JumpTimer = 0.0f;
			JumpCooldown = 0.1f;
		}
	}
}

// Processes input from the keyboard
void _Player::HandleInput() {

	// Get push direction
	core::vector3df Push(0.0f, 0.0f, 0.0f);
	GetPushDirection(Push);

	// Push player
	HandlePush(Push);
}

// Handle push vector
void _Player::HandlePush(core::vector3df &Push) {

	// Push the player
	if(!Push.equals(core::vector3df())) {

		// Get push direction relative to camera
		core::matrix4 DirectionTransform;
		DirectionTransform.makeIdentity();
		DirectionTransform.setRotationDegrees(core::vector3df(0.0f, Camera->GetYaw(), 0.0f));
		DirectionTransform.transformVect(Push);

		// Apply torque
		core::vector3df RotationAxis = Push.crossProduct(core::vector3df(0.0f, -1.0f, 0.0f)) * TorqueFactor;
		if(Body) {
			dBodyAddTorque(Body, RotationAxis.X, RotationAxis.Y, RotationAxis.Z);
		}
	}
}

// Get push direction from inputs
void _Player::GetPushDirection(irr::core::vector3df &Push) {

	// Get input direction
	Push.X += -Actions.GetState(_Actions::MOVE_LEFT);
	Push.X += Actions.GetState(_Actions::MOVE_RIGHT);
	Push.Z += Actions.GetState(_Actions::MOVE_FORWARD);
	Push.Z += -Actions.GetState(_Actions::MOVE_BACK);

	// Normalize
	if(Push.getLength() > 1.0f)
		Push.normalize();
}

// Request a jump
void _Player::Jump() {
	JumpTimer = JUMP_WINDOW;
}

// Update the graphic node position
void _Player::SetPositionFromReplay(const irr::core::vector3df &Position) {
	if(Node)
		Node->setPosition(Position);

	if(Light)
		Light->setPosition(Position);
}
