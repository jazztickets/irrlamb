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
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

// Forward Declarations
class _Object;

// Structures
struct _Template {
	_Template();

	// Generic properties
	int16_t TemplateID;
	std::string Name;
	int Type;
	float Lifetime;

	// Collision
	std::string CollisionCallback;
	int CollisionGroup;
	int CollisionMask;

	// Physical properties
	std::string CollisionFile;
	glm::vec3 Shape;
	int Kinematic;
	int Sleep;
	float Radius;
	float Mass;
	float Friction;
	float RollingFriction;
	float Restitution;
	float LinearDamping;
	float AngularDamping;
	float ERP;
	float CFM;

	// Constraints
	glm::vec3 ConstraintAxis;

	// Graphics
	std::string Mesh;
	float Scale[3];
	std::string Textures[4];
	float TextureScale[4];
	bool Fog;
	bool EmitLight;
	int Detail;
	int CustomMaterial;

	// Zones
	bool Active;

	// Terrain
	std::string HeightMap;
	int Smooth;
};

struct _ObjectSpawn {
	_ObjectSpawn();

	void CalculateRotation();

	std::string Name;
	glm::vec3 Position;
	glm::vec3 Rotation;
	glm::vec4 Plane;
	glm::quat Quaternion;
	glm::vec3 LinearVelocity;
	glm::vec3 AngularVelocity;
	_Template *Template;

	bool HasQuaternion;
};

struct _ConstraintSpawn {
	_ConstraintSpawn();

	std::string Name;
	std::string MainObjectName;
	std::string OtherObjectName;
	_Object *MainObject;
	_Object *OtherObject;
	_Template *Template;
	glm::vec3 AnchorPosition;
	bool HasAnchorPosition;
};
