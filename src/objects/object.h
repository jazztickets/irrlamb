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
#include <glm/gtc/quaternion.hpp>

// Forward Declarations
class _AudioSource;
struct _ObjectSpawn;
struct _ConstraintSpawn;
struct _Template;
struct _ObjectCollision;

// Classes
class _Object {

	public:

		enum ObjectType {
			NONE,
			PLAYER,
			ORB,
			COLLISION,
			PLANE,
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
		void InterpolateOrientation(float BlendFactor);

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

		virtual void SetPosition(const glm::vec3 &Position);
		virtual void SetPositionFromReplay(const irr::core::vector3df &Position);
		virtual glm::vec3 GetPosition() const;
		const glm::vec3 &GetDrawPosition() const { return DrawPosition; }

		virtual void SetQuaternion(const glm::quat &Quaternion);
		virtual glm::quat GetQuaternion() const;

		void SetLinearVelocity(const glm::vec3 &Velocity) { dBodyEnable(Body); dBodySetLinearVel(Body, Velocity[0], Velocity[1], Velocity[2]); }
		glm::vec3 GetLinearVelocity() const;

		void SetAngularVelocity(const glm::vec3 &Velocity) { dBodyEnable(Body); dBodySetAngularVel(Body, Velocity[0], Velocity[1], Velocity[2]); }
		glm::vec3 GetAngularVelocity() const;

		irr::scene::ISceneNode *GetNode() { return Node; }
		dBodyID GetBody() { return Body; }

		virtual void HandleCollision(const _ObjectCollision &ObjectCollision);
		bool IsTouchingGround() const { return TouchingGroundTimer > 0.0f; }

	protected:

		// Physics
		irr::scene::ISceneNode *LoadMesh(const std::string &MeshFile);
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
		glm::vec3 LastPosition;
		glm::quat LastRotation;
		glm::vec3 DrawPosition;
		dBodyID Body;
		dGeomID Geometry;

		// Replays
		bool NeedsReplayPacket;

		// Collision
		std::string CollisionCallback;
		float TouchingGroundTimer;
		bool TouchingGround;

};
