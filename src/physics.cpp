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
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <ode/odeinit.h>
#include <ode/objects.h>
#include <ode/collision.h>
#include <ode/misc.h>

const int MAX_CONTACTS = 32;

_Physics Physics;

static void ODECallback(void *Data, dGeomID Geometry1, dGeomID Geometry2) {
	dBodyID Body1 = dGeomGetBody(Geometry1);
	dBodyID Body2 = dGeomGetBody(Geometry2);

	dContact Contacts[MAX_CONTACTS];
	int Count = dCollide(Geometry1, Geometry2, MAX_CONTACTS, &Contacts[0].geom, sizeof(dContact));
	for(int i = 0; i < Count; i++) {
		Contacts[i].surface.mode = dContactBounce | dContactSoftCFM;
		Contacts[i].surface.mu = dInfinity;
		Contacts[i].surface.bounce = 0.9;
		Contacts[i].surface.bounce_vel = 0.1;
		Contacts[i].surface.soft_cfm = 0.001;

		dJointID Joint = dJointCreateContact(Physics.GetWorld(), Physics.GetContactGroup(), &Contacts[i]);
		dJointAttach(Joint, Body1, Body2);

		_Object *ObjectA = nullptr;
		_Object *ObjectB = nullptr;
		if(Body1)
			ObjectA = static_cast<_Object *>(dBodyGetData(Body1));
		if(Body2)
			ObjectB = static_cast<_Object *>(dBodyGetData(Body2));

		if(ObjectA)
			ObjectA->HandleCollision(ObjectB, Contacts[i].geom.normal, 1);

		if(ObjectB)
			ObjectB->HandleCollision(ObjectA, Contacts[i].geom.normal, -1);
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
	dWorldSetCFM(World, 1e-5);
	dCreatePlane(Space, 0, 1, 0, 0);
	ContactGroup = dJointGroupCreate(0);

	// Set up physics modules
	/*
	CollisionConfiguration = new btDefaultCollisionConfiguration();
	BroadPhase = new btDbvtBroadphase();
	Dispatcher = new btCollisionDispatcher(CollisionConfiguration);
	Solver = new btSequentialImpulseConstraintSolver();
	World = new btFixedWorld(Dispatcher, BroadPhase, Solver, CollisionConfiguration);
	World->setGravity(btVector3(0.0f, -9.81f, 0.0f));
	btContactSolverInfo &SolverInfo = World->getSolverInfo();
	SolverInfo.m_timeStep = PHYSICS_TIMESTEP;
	*/
	Enabled = false;

	return 1;
}

// Close the physics system
int _Physics::Close() {

	/*
	delete World;
	delete Solver;
	delete Dispatcher;
	delete BroadPhase;
	delete CollisionConfiguration;
	*/

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
	//BroadPhase->resetPool(Dispatcher);
	//Solver->reset();
}

// Performs raycasting on the world and returns the point of collision
bool _Physics::RaycastWorld(const btVector3 &Start, btVector3 &End, btVector3 &Normal) {
/*
	if(Enabled) {
		btCollisionWorld::ClosestRayResultCallback RayCallback(Start, End);
		RayCallback.m_collisionFilterMask = FILTER_CAMERA;

		// Perform raycast
		World->rayTest(Start, End, RayCallback);
		if(RayCallback.hasHit()) {

			End = RayCallback.m_hitPointWorld;
			Normal = RayCallback.m_hitNormalWorld;
			return true;
		}
	}
*/
	return false;
}

// Removes a bit field from a value
void _Physics::RemoveFilter(int &Value, int Filter) {

	Value &= (~Filter);
}

// Sets default, static, or kinematic on a rigid body
void _Physics::SetBodyType(int &Value, int Filter) {

	Value &= (~FILTER_BASICBODIES);
	Value |= Filter;
}

// Converts a quaternion to an euler angle
void _Physics::QuaternionToEuler(const float *Quat, float *Euler) {
	float W = Quat[0];
	float X = Quat[1];
	float Y = Quat[2];
	float Z = Quat[3];
	float WSquared = W * W;
	float XSquared = X * X;
	float YSquared = Y * Y;
	float ZSquared = Z * Z;

	Euler[0] = irr::core::RADTODEG * (atan2f(2.0f * (Y * Z + X * W), -XSquared - YSquared + ZSquared + WSquared));
	Euler[1] = irr::core::RADTODEG * (asinf(-2.0f * (X * Z - Y * W)));
	Euler[2] = irr::core::RADTODEG * (atan2f(2.0f * (X * Y + Z * W), XSquared - YSquared - ZSquared + WSquared));
}
