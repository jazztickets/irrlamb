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
#include <ode/collision.h>
#include <ode/objects.h>

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
	Body(nullptr),
	Geometry(nullptr),
	NeedsReplayPacket(false),
	TouchingGround(false),
	TouchingWall(false) {
}

// Destructor
_Object::~_Object() {

	// Remove graphics node
	if(Node)
		Node->remove();

	// Delete rigid body
	if(Body) {
		dBodyDestroy(Body);

		//Physics.GetWorld();
		/*
		Physics.GetWorld()->removeRigidBody(RigidBody);
		if(Importer) {
			Importer->deleteAllData();
			delete Importer;
		}
		else
			delete RigidBody->getCollisionShape();

		delete RigidBody;
		*/
	}

	if(Geometry)
		dGeomDestroy(Geometry);
}

// Print object position and rotation
void _Object::PrintOrientation() {
	if(!Body)
		return;

	// Get rigid body state
	const dReal *Quaternion = GetRotation();
	const dReal *Position = GetPosition();
	const dReal *LinearVelocity = GetLinearVelocity();
	const dReal *AngularVelocity = GetAngularVelocity();

	// Write information
	Log.Write("<object name=\"%s\" template=\"%s\">", Name.c_str(), Template->Name.c_str());
	Log.Write("\t<!-- %f, %f, %f -->", Position[0], Position[1], Position[2]);
	Log.Write("\t<position x=\"%f\" y=\"%f\" z=\"%f\" />", Position[0], Position[1], Position[2]);
	Log.Write("\t<quaternion w=\"%f\" x=\"%f\" y=\"%f\" z=\"%f\" />", Quaternion[0], Quaternion[1], Quaternion[2], Quaternion[3]);
	Log.Write("\t<linear_velocity x=\"%f\" y=\"%f\" z=\"%f\" />", LinearVelocity[0], LinearVelocity[1], LinearVelocity[2]);
	Log.Write("\t<angular_velocity x=\"%f\" y=\"%f\" z=\"%f\" />", AngularVelocity[0], AngularVelocity[1], AngularVelocity[2]);
	Log.Write("</object>");
}

// Creates a rigid body object and adds it to the world
void _Object::CreateRigidBody(const _ObjectSpawn &Object, dGeomID Geometry, bool SetTransform) {
	_Template *Template = Object.Template;

	// Create body
	Body = dBodyCreate(Physics.GetWorld());
	dGeomSetBody(Geometry, Body);

	// Set initial velocities
	SetLinearVelocity(Object.LinearVelocity);
	SetAngularVelocity(Object.AngularVelocity);

	// Disable body
	if(Template->Sleep)
		dBodyDisable(Body);

	/*

	// Create body
	RigidBody = new btRigidBody(Template->Mass, this, Shape, LocalInertia);
	RigidBody->setFriction(Template->Friction);
	RigidBody->setRestitution(Template->Restitution);
	RigidBody->setDamping(Template->LinearDamping, Template->AngularDamping);
	RigidBody->setSleepingThresholds(0.2f, 0.2f);
	*/
}

// Updates the object
void _Object::Update(float FrameTime) {
	Timer += FrameTime;

	// Check for expiration
	if(Lifetime > 0.0f && Timer > Lifetime)
		Deleted = true;

	// Update transform
	if(Node && Body) {

		// Set position
		const dReal *Position = GetPosition();
		Node->setPosition(core::vector3df(Position[0], Position[1], Position[2]));

		// Rotation
		const dReal *Quaternion = GetRotation();
		dReal EulerRotation[3];
		Physics.QuaternionToEuler(Quaternion, EulerRotation);
		Node->setRotation(core::vector3df(EulerRotation[0], EulerRotation[1], EulerRotation[2]));
	}
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

	// Set transform
	if(SetTransform) {

		// Get rotation
		glm::quat QuaternionRotation = Object.Quaternion;
		if(!Object.HasQuaternion)
			QuaternionRotation = glm::quat(glm::vec3(Object.Rotation[2], Object.Rotation[1], Object.Rotation[0]) * core::DEGTORAD);
		SetQuaternion(&QuaternionRotation[0]);
		SetPosition(Object.Position);
	}

	// Graphics
	if(Node) {
		if(SetTransform) {

			// Use quaternion if available
			glm::vec3 Rotation;
			if(Object.HasQuaternion)
				Physics.QuaternionToEuler(&Object.Quaternion[0], &Rotation[0]);
			else
				Rotation = Object.Rotation;

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
	if(Body)
		dBodySetData(Body, this);

	if(Geometry)
		dGeomSetData(Geometry, this);

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

// Stops the body's movement
void _Object::Stop() {
	if(Body) {
		dBodySetLinearVel(Body, 0.0f, 0.0f, 0.0f);
		dBodySetAngularVel(Body, 0.0f, 0.0f, 0.0f);
	}
}

// Sets the position of the object
void _Object::SetPosition(const glm::vec3 &Position) {
	if(Body)
		dBodySetPosition(Body, Position[0], Position[1], Position[2]);
	else if(Geometry)
		dGeomSetPosition(Geometry, Position[0], Position[1], Position[2]);

	//LastOrientation.setOrigin(Position);
}

// Set rotation from quaternion
void _Object::SetQuaternion(const dQuaternion Quaternion) {
	if(Body)
		dBodySetQuaternion(Body, Quaternion);
	else if(Geometry)
		dGeomSetQuaternion(Geometry, Quaternion);
}

// Collision callback
void _Object::HandleCollision(_Object *OtherObject, const dReal *Normal, float NormalScale) {
	//if(!OtherObject)
	//	return;

	// Get touching states
	//if(OtherObject->GetType() != ZONE) {
		float NormalY = Normal[1] * NormalScale;
		if(NormalY > 0.6f)
			TouchingGround = true;
		if(NormalY < 0.7f && NormalY > -0.7f)
			TouchingWall = true;
	//}

	//if(CollisionCallback.size())
	//	Scripting.CallCollisionHandler(CollisionCallback, this, OtherObject);
}

// Resets the object state before the frame begins
void _Object::BeginFrame() {
	TouchingGround = TouchingWall = false;
	//if(RigidBody) {
		//LastOrientation = RigidBody->getWorldTransform();
	//}
}

// Determines if the object moved
void _Object::EndFrame() {

	// Note changes for replays
	if(Node && Body && !NeedsReplayPacket /* && !(LastOrientation == RigidBody->getWorldTransform())*/) {
		NeedsReplayPacket = true;
	}
}

// Update the graphic node position
void _Object::SetPositionFromReplay(const irr::core::vector3df &Position) {
	if(Node) {
		Node->setPosition(Position);
	}
}
