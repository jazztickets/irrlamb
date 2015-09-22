/******************************************************************************
* irrlamb - https://github.com/jazztickets/irrlamb
* Copyright (C) 2015  Alan Witkowski
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
#pragma once
#include <LinearMath/btMotionState.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/NarrowPhaseCollision/btPersistentManifold.h>
#include <irrTypes.h>
#include <vector3d.h>
#include <ISceneNode.h>
#include <string>

// Forward Declarations
struct SpawnStruct;
struct ConstraintStruct;
class _AudioSource;

// Classes
class _Object : public btMotionState {

	public:

		enum ObjectType {
			NONE,
			PLAYER,
			ORB,
			COLLISION,
			SPHERE,
			BOX,
			CYLINDER,
			TERRAIN,
			CONSTRAINT_HINGE,
			CONSTRAINT_D6,
			ZONE,
		};

		_Object();
		virtual ~_Object();

		// Updates
		virtual void Update(float FrameTime);
		void BeginFrame();
		virtual void EndFrame();

		// Replays
		virtual void UpdateReplay(float FrameTime);
		bool ReadyForReplayUpdate() const { return NeedsReplayPacket; }
		void WroteReplayPacket() { NeedsReplayPacket = false; }

		// Object properties
		void SetID(int Value) { ID = Value; }
		void SetTemplateID(int Value) { TemplateID = Value; }
		void SetDeleted(bool Value) { Deleted = Value; }
		void SetLifetime(float Value) { Lifetime = Timer + Value; }

		std::string GetName() const { return Name; }
		bool GetDeleted() const { return Deleted; }
		float GetLifetime() const { return Lifetime; }
		int GetType() const { return Type; }
		irr::u16 GetID() const { return ID; }
		irr::u16 GetTemplateID() const { return TemplateID; }

		// Rigid body
		bool IsHookable() const { return Hookable; }
		void Stop();

		void CalculateInterpolatedPosition(float BlendFactor);
		void SetPosition(const btVector3 &Position);
		virtual void SetPositionFromReplay(const irr::core::vector3df &Position);
		const btVector3 &GetPosition() const { return RigidBody->getWorldTransform().getOrigin(); }
		const btVector3 &GetGraphicsPosition() const { return CenterOfMassTransform.getOrigin(); }

		void SetRotation(const btQuaternion &Rotation) { RigidBody->getWorldTransform().setRotation(Rotation); }
		btQuaternion GetRotation() { return RigidBody->getWorldTransform().getRotation(); }

		void SetLinearVelocity(const btVector3 &Velocity) { RigidBody->setLinearVelocity(Velocity); }
		const btVector3 &GetLinearVelocity() { return RigidBody->getLinearVelocity(); }

		void SetAngularVelocity(const btVector3 &Velocity) { RigidBody->setAngularVelocity(Velocity); }
		const btVector3 &GetAngularVelocity() { return RigidBody->getAngularVelocity(); }

		btRigidBody *GetBody() { return RigidBody; }
		irr::scene::ISceneNode *GetNode() { return Node; }

		virtual void HandleCollision(_Object *OtherObject, const btPersistentManifold *ContactManifold, float NormalScale);
		bool IsTouchingGround() const { return TouchingGround; }

		void getWorldTransform(btTransform &Transform) const;
		void setWorldTransform(const btTransform &Transform);

	protected:

		// Physics
		void CreateRigidBody(const SpawnStruct &Object, btCollisionShape *Shape);
		void SetProperties(const SpawnStruct &Object);
		void SetProperties(const ConstraintStruct &Object);

		// Attributes
		std::string Name;
		int Type;
		irr::u16 ID, TemplateID;

		// State
		bool Deleted;

		// Life
		float Timer, Lifetime;

		// Physics and graphics
		irr::scene::ISceneNode *Node;
		btRigidBody *RigidBody;
		btTransform LastOrientation;
		btTransform CenterOfMassTransform;

		// Replays
		bool NeedsReplayPacket;

		// Collision
		std::string CollisionCallback;
		bool TouchingGround, TouchingWall, Hookable;
};
