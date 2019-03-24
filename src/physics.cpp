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
			HitPosition[0] = Contacts[i].geom.pos[0];
			HitPosition[1] = Contacts[i].geom.pos[1];
			HitPosition[2] = Contacts[i].geom.pos[2];
			HitPosition[3] = Contacts[i].geom.depth;
		}
	}
}

// Near collision callback
static void ODECallback(void *Data, dGeomID Geometry1, dGeomID Geometry2) {
	dBodyID Body1 = dGeomGetBody(Geometry1);
	dBodyID Body2 = dGeomGetBody(Geometry2);

	dContact Contacts[MAX_CONTACTS];
	int Count = dCollide(Geometry1, Geometry2, MAX_CONTACTS, &Contacts[0].geom, sizeof(dContact));
	for(int i = 0; i < Count; i++) {

		// Get objects
		_Object *Object1 = nullptr;
		if(Body1)
			Object1 = (_Object *)dBodyGetData(Body1);
		else if(Geometry1)
			Object1 = (_Object *)dGeomGetData(Geometry1);

		_Object *Object2 = nullptr;
		if(Body2)
			Object2 = (_Object *)dBodyGetData(Body2);
		else if(Geometry2)
			Object2 = (_Object *)dGeomGetData(Geometry2);

		// Test for zones
		bool Response = true;
		if(Object1->GetTemplate()->CollisionGroup & _Physics::FILTER_ZONE || Object2->GetTemplate()->CollisionGroup & _Physics::FILTER_ZONE)
			Response = false;

		// Create contact joins
		if(Response) {
			Contacts[i].surface.mode = 0;//dContactBounce | dContactApprox1 | dContactSoftCFM;
			Contacts[i].surface.mu = dInfinity;
			Contacts[i].surface.bounce = 0.0;
			Contacts[i].surface.bounce_vel = 0.0;
			Contacts[i].surface.soft_cfm = 0.000;

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

	// Initialize
	dInitODE();
	dRandSetSeed(0);

	// Create world
	World = dWorldCreate();
	Space = dHashSpaceCreate(0);
	dWorldSetGravity(World, 0, -9.81, 0);
	//dWorldSetGravity(World, 0, 0, 0);
	//dWorldSetCFM(World, 1e-5);
	dWorldSetCFM(World, 0);
	ContactGroup = dJointGroupCreate(0);

	Enabled = false;

	return 1;
}

// Close the physics system
int _Physics::Close() {

	dJointGroupDestroy(ContactGroup);
	dSpaceDestroy(Space);
	dWorldDestroy(World);
	dCloseODE();

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

	Euler[0] = irr::core::RADTODEG * (atan2f(2.0f * (Y * Z + X * W), -XSquared - YSquared + ZSquared + WSquared));
	Euler[1] = irr::core::RADTODEG * (asinf(-2.0f * (X * Z - Y * W)));
	Euler[2] = irr::core::RADTODEG * (atan2f(2.0f * (X * Y + Z * W), XSquared - YSquared - ZSquared + WSquared));
}
