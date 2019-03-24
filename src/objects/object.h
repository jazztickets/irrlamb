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
#pragma once
#include <ode/common.h>
#include <ode/objects.h>
#include <irrTypes.h>
#include <vector3d.h>
#include <ISceneNode.h>
#include <string>
#include <glm/vec3.hpp>

// Forward Declarations
struct _ObjectSpawn;
struct _ConstraintSpawn;
struct _Template;
class _AudioSource;

// Classes
class _Object {

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

		_Object(const _Template *Template);
		virtual ~_Object();

		void PrintOrientation();

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
		void SetDeleted(bool Value) { Deleted = Value; }
		void SetLifetime(float Value) { Lifetime = Timer + Value; }

		std::string GetName() const { return Name; }
		bool GetDeleted() const { return Deleted; }
		float GetLifetime() const { return Lifetime; }
		int GetType() const { return Type; }
		const uint16_t &GetID() const { return ID; }
		const _Template *GetTemplate() const { return Template; }

		// Rigid body
		void Stop();

		void SetPosition(const glm::vec3 &Position);
		virtual void SetPositionFromReplay(const irr::core::vector3df &Position);
		const dReal *GetPosition() const;
		const dReal *GetGraphicsPosition() const { return dBodyGetPosition(Body); }

		void SetRotation(const dMatrix3 Rotation) { dBodySetRotation(Body, Rotation); }
		void SetQuaternion(const dQuaternion Quaternion);
		const dReal *GetRotation() const { return dBodyGetQuaternion(Body); }

		void SetLinearVelocity(const glm::vec3 &Velocity) { dBodySetLinearVel(Body, Velocity[0], Velocity[1], Velocity[2]); }
		const dReal *GetLinearVelocity() { return dBodyGetLinearVel(Body); }

		void SetAngularVelocity(const glm::vec3 &Velocity) { dBodySetAngularVel(Body, Velocity[0], Velocity[1], Velocity[2]); }
		const dReal *GetAngularVelocity() { return dBodyGetAngularVel(Body); }

		irr::scene::ISceneNode *GetNode() { return Node; }
		dBodyID GetBody() { return Body; }

		virtual void HandleCollision(_Object *OtherObject, const dReal *Normal, float NormalScale);
		bool IsTouchingGround() const { return TouchingGround; }

	protected:

		// Physics
		void CreateRigidBody(const _ObjectSpawn &Object, dGeomID Geometry, bool SetTransform=true);
		void SetProperties(const _ObjectSpawn &Object, bool SetTransform=true);
		void SetProperties(const _ConstraintSpawn &Object);

		// Attributes
		std::string Name;
		const _Template *Template;
		int Type;
		uint16_t ID;

		// State
		bool Deleted;

		// Life
		float Timer, Lifetime;

		// Physics and graphics
		irr::scene::ISceneNode *Node;
		dBodyID Body;
		dGeomID Geometry;
		//btTransform LastOrientation;

		// Replays
		bool NeedsReplayPacket;

		// Collision
		std::string CollisionCallback;
		bool TouchingGround, TouchingWall;

};
