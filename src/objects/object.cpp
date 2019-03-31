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

const float TOUCHING_GROUND_WINDOW = 0.13f;

using namespace irr;

// Constructor
_Object::_Object(const _Template *Template) :
	Name(""),
	Template(Template),
	Type(NONE),
	ID(-1),
	Deleted(false),
	Timer(0.0f),
	Lifetime(0.0f),
	Node(nullptr),
	LastPosition(0.0f, 0.0f, 0.0f),
	LastRotation(1.0f, 0.0f, 0.0f, 0.0f),
	DrawPosition(0.0f, 0.0f, 0.0f),
	Body(nullptr),
	Geometry(nullptr),
	NeedsReplayPacket(false),
	TouchingGroundTimer(0.0f),
	TouchingGround(false) {
}

// Destructor
_Object::~_Object() {

	// Remove graphics node
	if(Node)
		Node->remove();

	// Delete body
	if(Body)
		dBodyDestroy(Body);

	// Delete geometry
	if(Geometry)
		dGeomDestroy(Geometry);
}

// Print object position and rotation
void _Object::PrintOrientation() {
	if(!Body)
		return;

	// Get rigid body state
	glm::quat Quaternion = GetQuaternion();
	glm::vec3 Position = GetPosition();
	glm::vec3 LinearVelocity = GetLinearVelocity();
	glm::vec3 AngularVelocity = GetAngularVelocity();

	// Write information
	Log.Write("<object name=\"%s\" template=\"%s\">", Name.c_str(), Template->Name.c_str());
	Log.Write("\t<!-- %f, %f, %f -->", Position.x, Position.y, Position.z);
	Log.Write("\t<position x=\"%f\" y=\"%f\" z=\"%f\" />", Position.x, Position.y, Position.z);
	Log.Write("\t<quaternion w=\"%f\" x=\"%f\" y=\"%f\" z=\"%f\" />", Quaternion.w, Quaternion.x, Quaternion.y, Quaternion.z);
	Log.Write("\t<linear_velocity x=\"%f\" y=\"%f\" z=\"%f\" />", LinearVelocity.x, LinearVelocity.y, LinearVelocity.z);
	Log.Write("\t<angular_velocity x=\"%f\" y=\"%f\" z=\"%f\" />", AngularVelocity.x, AngularVelocity.y, AngularVelocity.z);
	Log.Write("</object>");
}

// Creates a rigid body object and adds it to the world
void _Object::CreateRigidBody(const _ObjectSpawn &Object, dGeomID Geometry, bool SetTransform) {
	_Template *Template = Object.Template;

	// Create body
	Body = dBodyCreate(Physics.GetWorld());
	dBodySetAutoDisableDefaults(Body);
	dBodySetAutoDisableFlag(Body, true);
	dBodySetDampingDefaults(Body);
	dBodySetAngularDampingThreshold(Body, 0);
	dBodySetLinearDampingThreshold(Body, 0);
	dBodySetDamping(Body, Template->LinearDamping, Template->AngularDamping);
	dGeomSetBody(Geometry, Body);

	// Set initial velocities
	SetLinearVelocity(Object.LinearVelocity);
	SetAngularVelocity(Object.AngularVelocity);

	// Disable body
	if(Template->Sleep)
		dBodyDisable(Body);
}

// Updates the object
void _Object::Update(float FrameTime) {
	Timer += FrameTime;

	// Check for expiration
	if(Lifetime > 0.0f && Timer > Lifetime)
		Deleted = true;

	// Set touch timer
	if(TouchingGround)
		TouchingGroundTimer = TOUCHING_GROUND_WINDOW;

	// Update touch timer
	TouchingGroundTimer -= FrameTime;
	if(TouchingGroundTimer < 0.0f)
		TouchingGroundTimer = 0.0f;
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
			QuaternionRotation = glm::quat(glm::vec3(Object.Rotation[0], Object.Rotation[1], Object.Rotation[2]) * core::DEGTORAD);
		SetQuaternion(QuaternionRotation);
		SetPosition(Object.Position);
	}

	// Graphics
	if(Node) {
		if(SetTransform) {

			// Use quaternion if available
			glm::vec3 Rotation;
			if(Object.HasQuaternion)
				Rotation = glm::degrees(glm::eulerAngles(Object.Quaternion));
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

	// Set geometry data
	if(Geometry) {
		dGeomSetData(Geometry, this);
		dGeomSetCategoryBits(Geometry, Template->CollisionGroup);
		dGeomSetCollideBits(Geometry, Template->CollisionMask);
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

// Interpolate between last and current orientation
void _Object::InterpolateOrientation(float BlendFactor) {
	if(!Node || !Body)
		return;

	// Get current orientation
	glm::vec3 CurrentPosition = GetPosition();
	glm::quat CurrentRotation = GetQuaternion();

	// Set node position
	DrawPosition = CurrentPosition * BlendFactor + LastPosition * (1.0f - BlendFactor);
	Node->setPosition(core::vector3df(DrawPosition[0], DrawPosition[1], DrawPosition[2]));

	// Set node rotation
	glm::quat DrawRotation = glm::mix(LastRotation, CurrentRotation, BlendFactor);
	glm::vec3 EulerRotation = glm::degrees(glm::eulerAngles(DrawRotation));
	Node->setRotation(core::vector3df(EulerRotation.x, EulerRotation.y, EulerRotation.z));
}

// Stops the body's movement
void _Object::Stop() {
	if(Body) {
		dBodySetLinearVel(Body, 0.0f, 0.0f, 0.0f);
		dBodySetAngularVel(Body, 0.0f, 0.0f, 0.0f);
	}
}

// Get object position
glm::vec3 _Object::GetPosition() const {
	const dReal *Position = nullptr;
	if(Geometry)
		Position = dGeomGetPosition(Geometry);

	if(!Position)
		return glm::vec3(0, 0, 0);

	return glm::vec3(Position[0], Position[1], Position[2]);
}

// Sets the position of the object
void _Object::SetPosition(const glm::vec3 &Position) {
	if(Geometry)
		dGeomSetPosition(Geometry, Position[0], Position[1], Position[2]);

	LastPosition = Position;
}

// Set rotation from quaternion
void _Object::SetQuaternion(const glm::quat &Quaternion) {
	dQuaternion Rotation = { Quaternion.w, Quaternion.x, Quaternion.y, Quaternion.z };
	if(Geometry)
		dGeomSetQuaternion(Geometry, Rotation);

	LastRotation = Quaternion;
}

// Get rotation
glm::quat _Object::GetQuaternion() const {
	dQuaternion Quaternion;
	if(Geometry)
		dGeomGetQuaternion(Geometry, Quaternion);

	return glm::quat(Quaternion[0], Quaternion[1], Quaternion[2], Quaternion[3]);
}

// Get linear velocity
glm::vec3 _Object::GetLinearVelocity() const {
	const dReal *Velocity = dBodyGetLinearVel(Body);

	return glm::vec3(Velocity[0], Velocity[1], Velocity[2]);
}

// Get angular velocity
glm::vec3 _Object::GetAngularVelocity() const {
	const dReal *Velocity = dBodyGetAngularVel(Body);

	return glm::vec3(Velocity[0], Velocity[1], Velocity[2]);
}

// Collision callback
void _Object::HandleCollision(const _ObjectCollision &ObjectCollision) {
	if(!ObjectCollision.OtherObject)
		return;

	// Get touching states
	if(ObjectCollision.OtherObject->GetType() != ZONE) {
		float NormalY = ObjectCollision.Normal[1] * ObjectCollision.NormalScale;
		if(NormalY > 0.6f)
			TouchingGround = true;
	}

	// Call collision handler
	if(CollisionCallback.size())
		Scripting.CallCollisionHandler(CollisionCallback, this, ObjectCollision.OtherObject);
}

// Resets the object state before the frame begins
void _Object::BeginFrame() {
	TouchingGround = false;
	if(Body || Geometry) {
		LastPosition = GetPosition();
		LastRotation = GetQuaternion();
	}
}

// Determines if the object moved
void _Object::EndFrame() {

	// Note changes for replays
	if(Node && Body && !NeedsReplayPacket && !(LastPosition == GetPosition() && LastRotation == GetQuaternion())) {
		NeedsReplayPacket = true;
	}
}

// Update the graphic node position
void _Object::SetPositionFromReplay(const irr::core::vector3df &Position) {
	if(Node) {
		Node->setPosition(Position);
	}
}
