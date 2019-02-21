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

_Physics Physics;

// Initializes the physics system
int _Physics::Init() {

	// Set up physics modules
	CollisionConfiguration = new btDefaultCollisionConfiguration();
	//BroadPhase = new btAxisSweep3(btVector3(-1000, -1000, -1000), btVector3(1000, 1000, 1000));
	BroadPhase = new btDbvtBroadphase();
	Dispatcher = new btCollisionDispatcher(CollisionConfiguration);
	Solver = new btSequentialImpulseConstraintSolver();
	World = new btFixedWorld(Dispatcher, BroadPhase, Solver, CollisionConfiguration);
	World->setGravity(btVector3(0.0f, -9.81f, 0.0f));
	btContactSolverInfo &SolverInfo = World->getSolverInfo();
	SolverInfo.m_timeStep = PHYSICS_TIMESTEP;

	Enabled = false;

	return 1;
}

// Closes the physics system
int _Physics::Close() {

	delete World;
	delete Solver;
	delete Dispatcher;
	delete BroadPhase;
	delete CollisionConfiguration;

	return 1;
}

// Updates the physics system
void _Physics::Update(float FrameTime) {

	if(Enabled) {

		World->stepSimulation(FrameTime);

		// Handle collision callbacks
		int ManifoldCount = World->getDispatcher()->getNumManifolds();
		for(int i = 0; i < ManifoldCount; i++) {
			btPersistentManifold *ContactManifold = World->getDispatcher()->getManifoldByIndexInternal(i);
			if(ContactManifold->getNumContacts() > 0) {
				const btCollisionObject *CollisionObject0 = static_cast<const btCollisionObject *>(ContactManifold->getBody0());
				const btCollisionObject *CollisionObject1 = static_cast<const btCollisionObject *>(ContactManifold->getBody1());

				_Object *Object0 = static_cast<_Object *>(CollisionObject0->getUserPointer());
				_Object *Object1 = static_cast<_Object *>(CollisionObject1->getUserPointer());

				Object0->HandleCollision(Object1, ContactManifold, 1);
				Object1->HandleCollision(Object0, ContactManifold, -1);
			}
		}
	}
}

// Resets the physics world
void _Physics::Reset() {
	BroadPhase->resetPool(Dispatcher);
	Solver->reset();
}

// Performs raycasting on the world and returns the point of collision
bool _Physics::RaycastWorld(const btVector3 &Start, btVector3 &End, btVector3 &Normal) {

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

	return false;
}

// Converts a quaternion to an euler angle
void _Physics::QuaternionToEuler(const btQuaternion &Quat, btVector3 &Euler) {
	btScalar W = Quat.getW();
	btScalar X = Quat.getX();
	btScalar Y = Quat.getY();
	btScalar Z = Quat.getZ();
	float WSquared = W * W;
	float XSquared = X * X;
	float YSquared = Y * Y;
	float ZSquared = Z * Z;

	Euler.setX(atan2f(2.0f * (Y * Z + X * W), -XSquared - YSquared + ZSquared + WSquared));
	Euler.setY(asinf(-2.0f * (X * Z - Y * W)));
	Euler.setZ(atan2f(2.0f * (X * Y + Z * W), XSquared - YSquared - ZSquared + WSquared));
	Euler *= irr::core::RADTODEG;
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
