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
#include <physics.h>
#include <objects/object.h>
#include <objects/template.h>
#include <ode/odeinit.h>
#include <ode/objects.h>
#include <ode/collision.h>
#include <ode/misc.h>
#include <ode/export-dif.h>
#include <ode/odemath.h>
#include <glm/geometric.hpp>

const int MAX_CONTACTS = 32;

_Physics Physics;

// Check ray collision against a space
static void RayCallback(void *Data, dGeomID Geometry1, dGeomID Geometry2) {
	dReal *HitPosition = (dReal *)Data;

	// Check collisions
	dContact Contacts[MAX_CONTACTS];
	int Count = dCollide(Geometry1, Geometry2, MAX_CONTACTS, &Contacts[0].geom, sizeof(dContact));
	for(int i = 0; i < Count; i++) {

		// Check depth against current closest hit
		if(Contacts[i].geom.depth < HitPosition[3]) {
			dCopyVector3(HitPosition, Contacts[i].geom.pos);
			HitPosition[3] = Contacts[i].geom.depth;
		}
	}
}

// Near collision callback
static void ODECallback(void *Data, dGeomID Geometry, dGeomID OtherGeometry) {
	std::vector<_ObjectCollision> *ObjectCollisions = (std::vector<_ObjectCollision> *)Data;
	dBodyID Body = dGeomGetBody(Geometry);
	dBodyID OtherBody = dGeomGetBody(OtherGeometry);

	// Get objects
	_Object *Object =(_Object *)dGeomGetData(Geometry);
	_Object *OtherObject = (_Object *)dGeomGetData(OtherGeometry);

	// Get contacts
	dContact Contacts[MAX_CONTACTS];
	int Count = dCollide(Geometry, OtherGeometry, MAX_CONTACTS, &Contacts[0].geom, sizeof(dContact));
	for(int i = 0; i < Count; i++) {

		// Test for zones
		bool Response = true;
		if(Object->GetTemplate()->CollisionGroup & _Physics::FILTER_ZONE || OtherObject->GetTemplate()->CollisionGroup & _Physics::FILTER_ZONE)
			Response = false;

		// Collision response
		if(Response) {
			Contacts[i].surface.mode = dContactApprox1 | dContactSoftERP | dContactSoftCFM;
			Contacts[i].surface.mu = std::min(Object->GetTemplate()->Friction, OtherObject->GetTemplate()->Friction);

			// Handle ERP and CFM
			Contacts[i].surface.soft_erp = std::min(Object->GetTemplate()->ERP, OtherObject->GetTemplate()->ERP);
			Contacts[i].surface.soft_cfm = std::max(Object->GetTemplate()->CFM, OtherObject->GetTemplate()->CFM);

			// Handle rolling friction
			float RollingFriction = std::max(Object->GetTemplate()->RollingFriction, OtherObject->GetTemplate()->RollingFriction);
			if(RollingFriction > 0) {
				Contacts[i].surface.mode |= dContactRolling;
				Contacts[i].surface.rho = RollingFriction;
				Contacts[i].surface.rho2 = RollingFriction;
			}

			// Handle restitution
			float Restitution = std::max(Object->GetTemplate()->Restitution, OtherObject->GetTemplate()->Restitution);
			if(Restitution > 0) {
				Contacts[i].surface.mode |= dContactBounce;
				Contacts[i].surface.bounce = Restitution;
				Contacts[i].surface.bounce_vel = 0;
			}

			// Create contact joint
			dJointID Joint = dJointCreateContact(Physics.GetWorld(), Physics.GetContactGroup(), &Contacts[i]);
			dJointAttach(Joint, Body, OtherBody);
		}

		// Get normal
		glm::vec3 Normal(Contacts[i].geom.normal[0], Contacts[i].geom.normal[1], Contacts[i].geom.normal[2]);

		// Handle collision callback
		ObjectCollisions->push_back(_ObjectCollision(Object, OtherObject, Normal, 1));
		ObjectCollisions->push_back(_ObjectCollision(OtherObject, Object, Normal, -1));
	}
}

// Initialize the physics system
int _Physics::Init() {

	// Enable physics
	Enabled = true;

	// Initialize
	dInitODE();
	dRandSetSeed(0);

	// Create world
	World = dWorldCreate();
	dWorldSetGravity(World, 0, -9.81, 0);
	dWorldSetCFM(World, 0.0);

	// Create space
	Space = dHashSpaceCreate(0);

	// Create contact group
	ContactGroup = dJointGroupCreate(0);

	return 1;
}

// Close the physics system
int _Physics::Close() {
	if(!Enabled)
		return 0;

	// Free contact group
	if(ContactGroup)
		dJointGroupDestroy(ContactGroup);

	// Free space
	if(Space)
		dSpaceDestroy(Space);

	// Free world
	if(World)
		dWorldDestroy(World);

	// Close ODE
	dCloseODE();

	// Disable physics
	Enabled = false;

	return 1;
}

// Updates the physics system
void _Physics::Update(float FrameTime) {
	if(Enabled) {

		// Handle collisions
		dSpaceCollide(Space, &ObjectCollisions, &ODECallback);

		// Handle callbacks
		for(auto ObjectCollision : ObjectCollisions)
			ObjectCollision.Object->HandleCollision(ObjectCollision);
		ObjectCollisions.clear();

		// Run timestep
		dWorldQuickStep(World, FrameTime);

		// Remove contact joints
		dJointGroupEmpty(ContactGroup);
	}
}

// Resets the physics world
void _Physics::Reset() {
	Physics.Close();
	Physics.Init();
	Physics.SetEnabled(true);
}

// Performs raycasting on the world and returns the point of collision
bool _Physics::RaycastWorld(const glm::vec3 &Start, glm::vec3 &End) {

	if(Enabled) {

		// Get ray length and direction
		float Length = glm::length(End - Start);
		glm::vec3 Direction = (End - Start) / Length;

		// Create ray
		dGeomID Ray = dCreateRay(0, Length);
		dGeomRaySet(Ray, Start[0], Start[1], Start[2], Direction[0], Direction[1], Direction[2]);
		dGeomSetCategoryBits(Ray, 0);
		dGeomSetCollideBits(Ray, _Physics::FILTER_CAMERA);

		// Check collisions
		dVector4 HitPosition = { 0, 0, 0, dInfinity };
		dSpaceCollide2(Ray, (dGeomID)Space, HitPosition, &RayCallback);

		// Cleanup
		dGeomDestroy(Ray);

		// Check for hit
		if(HitPosition[3] != dInfinity) {
			End[0] = HitPosition[0];
			End[1] = HitPosition[1];
			End[2] = HitPosition[2];

			return true;
		}
	}

	return false;
}

// Removes a bit field from a value
void _Physics::RemoveFilter(int &Value, int Filter) {
	Value &= (~Filter);
}

// Dump physics state to stdout
void _Physics::Dump() {
	dWorldExportDIF(World, stdout, "");
}

// Converts a quaternion to an euler angle
glm::vec3 _Physics::QuaternionToEuler(const glm::quat &Quaternion) {
	irr::core::vector3df EulerAngles;
	irr::core::quaternion Quat(Quaternion.x, Quaternion.y, Quaternion.z, Quaternion.w);
	Quat.toEuler(EulerAngles);

	return glm::degrees(glm::vec3(EulerAngles.X, EulerAngles.Y, EulerAngles.Z));
}
