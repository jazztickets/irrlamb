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
#include <globals.h>
#include <framework.h>
#include <constants.h>
#include <objects/object.h>
#include <objects/template.h>
#include <ode/odeinit.h>
#include <ode/objects.h>
#include <ode/collision.h>
#include <ode/misc.h>
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
static void ODECallback(void *Data, dGeomID Geometry1, dGeomID Geometry2) {
	dBodyID Body1 = dGeomGetBody(Geometry1);
	dBodyID Body2 = dGeomGetBody(Geometry2);

	// Get objects
	_Object *Object1 = nullptr;
	if(Geometry1)
		Object1 = (_Object *)dGeomGetData(Geometry1);

	_Object *Object2 = nullptr;
	if(Geometry2)
		Object2 = (_Object *)dGeomGetData(Geometry2);

	dContact Contacts[MAX_CONTACTS];
	int Count = dCollide(Geometry1, Geometry2, MAX_CONTACTS, &Contacts[0].geom, sizeof(dContact));
	for(int i = 0; i < Count; i++) {

		// Test for zones
		bool Response = true;
		if(Object1->GetTemplate()->CollisionGroup & _Physics::FILTER_ZONE || Object2->GetTemplate()->CollisionGroup & _Physics::FILTER_ZONE)
			Response = false;

		// Create contact joins
		if(Response) {
			Contacts[i].surface.mode = dContactRolling | dContactApprox1;
			Contacts[i].surface.mu = 1;
			Contacts[i].surface.rho = 0.01;
			Contacts[i].surface.rho2 = 0.01;

			dJointID Joint = dJointCreateContact(Physics.GetWorld(), Physics.GetContactGroup(), &Contacts[i]);
			dJointAttach(Joint, Body1, Body2);
		}

		// Handle object collision callbacks
		if(Object1)
			Object1->HandleCollision(Object2, Contacts[i].geom.normal, 1);

		if(Object2)
			Object2->HandleCollision(Object1, Contacts[i].geom.normal, -1);
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
	//dWorldSetCFM(World, 1e-5);
	dWorldSetCFM(World, 0.0);
	dWorldSetERP(World, 0.2);

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
		dSpaceCollide(Space, 0, &ODECallback);
		dWorldQuickStep(World, FrameTime);
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
			dCopyVector3(&End[0], HitPosition);

			return true;
		}
	}

	return false;
}

// Removes a bit field from a value
void _Physics::RemoveFilter(int &Value, int Filter) {
	Value &= (~Filter);
}

// Converts a quaternion to an euler angle
void _Physics::QuaternionToEuler(const glm::quat &Quaternion, float *Euler) {
	float W = Quaternion[0];
	float X = Quaternion[1];
	float Y = Quaternion[2];
	float Z = Quaternion[3];
	float WSquared = W * W;
	float XSquared = X * X;
	float YSquared = Y * Y;
	float ZSquared = Z * Z;

	Euler[0] = glm::degrees((atan2f(2.0f * (Y * Z + X * W), -XSquared - YSquared + ZSquared + WSquared)));
	Euler[1] = glm::degrees((asinf(-2.0f * (X * Z - Y * W))));
	Euler[2] = glm::degrees((atan2f(2.0f * (X * Y + Z * W), XSquared - YSquared - ZSquared + WSquared)));
}
