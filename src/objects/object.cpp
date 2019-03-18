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
#include <objects/object.h>
#include <objects/template.h>
#include <config.h>
#include <scripting.h>
#include <physics.h>
#include <log.h>
#include <BulletWorldImporter/btBulletWorldImporter.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>

using namespace irr;

// Constructor
_Object::_Object(const _Template *Template)
:	Name(""),
	Template(Template),
	Type(NONE),
	ID(-1),
	Deleted(false),
	Timer(0.0f),
	Lifetime(0.0f),
	Node(nullptr),
	RigidBody(nullptr),
	NeedsReplayPacket(false),
	TouchingGround(false),
	TouchingWall(false),
	Importer(nullptr) {

	LastOrientation.setIdentity();
}

// Destructor
_Object::~_Object() {

	// Remove graphics node
	if(Node)
		Node->remove();

	// Delete rigid body
	if(RigidBody) {
		Physics.GetWorld()->removeRigidBody(RigidBody);
		if(Importer) {
			Importer->deleteAllData();
			delete Importer;
		}
		else
			delete RigidBody->getCollisionShape();

		delete RigidBody;
	}
}

// Print object position and rotation
void _Object::PrintOrientation() {

	// Get rigid body state
	btQuaternion Quaternion = GetRotation();
	const btVector3 &Position = GetPosition();
	const btVector3 &LinearVelocity = GetLinearVelocity();
	const btVector3 &AngularVelocity = GetAngularVelocity();

	// Write information
	Log.Write("<object name=\"%s\" template=\"%s\">", Name.c_str(), Template->Name.c_str());
	Log.Write("\t<!-- %f, %f, %f -->", Position[0], Position[1], Position[2]);
	Log.Write("\t<position x=\"%f\" y=\"%f\" z=\"%f\" />", Position[0], Position[1], Position[2]);
	Log.Write("\t<quaternion x=\"%f\" y=\"%f\" z=\"%f\" w=\"%f\" />", Quaternion[0], Quaternion[1], Quaternion[2], Quaternion[3]);
	Log.Write("\t<linear_velocity x=\"%f\" y=\"%f\" z=\"%f\" />", LinearVelocity[0], LinearVelocity[1], LinearVelocity[2]);
	Log.Write("\t<angular_velocity x=\"%f\" y=\"%f\" z=\"%f\" />", AngularVelocity[0], AngularVelocity[1], AngularVelocity[2]);
	Log.Write("</object>");
}

// Creates a rigid body object and adds it to the world
void _Object::CreateRigidBody(const _ObjectSpawn &Object, btCollisionShape *Shape, bool SetTransform) {
	_Template *Template = Object.Template;

	// Rotation
	btQuaternion QuaternionRotation = Object.Quaternion;
	if(!Object.HasQuaternion)
		QuaternionRotation.setEuler(Object.Rotation[1] * core::DEGTORAD, Object.Rotation[0] * core::DEGTORAD, Object.Rotation[2] * core::DEGTORAD);

	// Transform
	CenterOfMassTransform.setIdentity();
	if(SetTransform) {
		CenterOfMassTransform.setOrigin(btVector3(Object.Position[0], Object.Position[1], Object.Position[2]));
		CenterOfMassTransform.setRotation(QuaternionRotation);
	}

	// Local inertia
	btVector3 LocalInertia(0.0f, 0.0f, 0.0f);
	if(Template->Mass != 0.0f)
		Shape->calculateLocalInertia(Template->Mass, LocalInertia);

	// Create body
	RigidBody = new btRigidBody(Template->Mass, this, Shape, LocalInertia);
	RigidBody->setUserPointer((void *)this);
	RigidBody->setFriction(Template->Friction);
	RigidBody->setRestitution(Template->Restitution);
	RigidBody->setDamping(Template->LinearDamping, Template->AngularDamping);
	RigidBody->setSleepingThresholds(0.2f, 0.2f);
	RigidBody->setLinearVelocity(Object.LinearVelocity);
	RigidBody->setAngularVelocity(Object.AngularVelocity);
	if(Template->Sleep)
		RigidBody->setActivationState(ISLAND_SLEEPING);

	// Add body
	Physics.GetWorld()->addRigidBody(RigidBody, Template->CollisionGroup, Template->CollisionMask);
}

// Updates the object
void _Object::Update(float FrameTime) {
	Timer += FrameTime;

	// Check for expiration
	if(Lifetime > 0.0f && Timer > Lifetime)
		Deleted = true;
}

// Updates while replaying
void _Object::UpdateReplay(float FrameTime) {
	Timer += FrameTime;
}

// Sets object properties
void _Object::SetProperties(const _ObjectSpawn &Object, bool SetTransform) {
	Template = Object.Template;

	// Basic properties
	Name = Object.Name;
	Type = Template->Type;
	Lifetime = Template->Lifetime;

	// Graphics
	if(Node) {
		if(SetTransform) {

			// Use quaternion if available
			btVector3 Rotation;
			if(Object.HasQuaternion) {
				Object.Quaternion.getEulerZYX(Rotation[2], Rotation[1], Rotation[0]);
				Rotation *= core::RADTODEG;
			}
			else {
				Rotation = Object.Rotation;
			}

			Node->setPosition(core::vector3df(Object.Position[0], Object.Position[1], Object.Position[2]));
			Node->setRotation(core::vector3df(Rotation[0], Rotation[1], Rotation[2]));
		}
		//Node->setVisible(Template->Visible);
		Node->setMaterialFlag(video::EMF_FOG_ENABLE, Template->Fog);
		Node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
		Node->setMaterialFlag(video::EMF_TRILINEAR_FILTER, Config.TrilinearFiltering);
		Node->getMaterial(0).TextureLayer[0].AnisotropicFilter = Config.AnisotropicFiltering;
	}

	// Physics
	if(RigidBody) {
		RigidBody->setFriction(Template->Friction);
		RigidBody->setDamping(Template->LinearDamping, Template->AngularDamping);
		LastOrientation.setOrigin(GetPosition());
	}

	// Collision
	CollisionCallback = Template->CollisionCallback;
}

// Sets object properties
void _Object::SetProperties(const _ConstraintSpawn &Object) {
	_Template *Template = Object.Template;

	// Basic properties
	Name = Object.Name;
	Type = Template->Type;
	Lifetime = Template->Lifetime;
}

// Get the center of mass transform for the object
void _Object::getWorldTransform(btTransform &Transform) const {
	Transform = CenterOfMassTransform;
}

// Set the center of mass transform for the object
void _Object::setWorldTransform(const btTransform &Transform) {
	CenterOfMassTransform = Transform;

	if(Node) {

		// Set position
		const btVector3 &Position = CenterOfMassTransform.getOrigin();
		Node->setPosition(core::vector3df((f32)Position.x(), (f32)Position.y(), (f32)Position.z()));

		// Rotation
		btVector3 EulerRotation;
		btQuaternion RigidRotation = CenterOfMassTransform.getRotation();
		RigidRotation.getEulerZYX(EulerRotation[2], EulerRotation[1], EulerRotation[0]);
		EulerRotation *= core::RADTODEG;
		Node->setRotation(core::vector3df(EulerRotation[0], EulerRotation[1], EulerRotation[2]));
	}
}

// Stops the body's movement
void _Object::Stop() {
	RigidBody->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
	RigidBody->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
}

// Sets the position of the body
void _Object::SetPosition(const btVector3 &Position) {
	LastOrientation.setOrigin(Position);
	RigidBody->getWorldTransform().setOrigin(Position);
}

// Collision callback
void _Object::HandleCollision(_Object *OtherObject, const btPersistentManifold *ContactManifold, float NormalScale) {

	// Get touching states
	if(OtherObject->GetType() != ZONE) {
		for(int i = 0; i < ContactManifold->getNumContacts(); i++) {
			float NormalY = ContactManifold->getContactPoint(i).m_normalWorldOnB[1] * NormalScale;
			if(NormalY > 0.6f)
				TouchingGround = true;
			if(NormalY < 0.7f && NormalY > -0.7f)
				TouchingWall = true;
		}
	}

	if(CollisionCallback.size())
		Scripting.CallCollisionHandler(CollisionCallback, this, OtherObject);
}

// Resets the object state before the frame begins
void _Object::BeginFrame() {
	TouchingGround = TouchingWall = false;
	if(RigidBody) {
		LastOrientation = RigidBody->getWorldTransform();
	}
}

// Determines if the object moved
void _Object::EndFrame() {

	// Note changes for replays
	if(Node && RigidBody && !NeedsReplayPacket && !(LastOrientation == RigidBody->getWorldTransform())) {
		NeedsReplayPacket = true;
	}
}

// Update the graphic node position
void _Object::SetPositionFromReplay(const irr::core::vector3df &Position) {
	if(Node) {
		Node->setPosition(Position);
	}
}
