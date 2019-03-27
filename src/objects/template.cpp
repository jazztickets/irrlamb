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
#include <objects/template.h>
#include <objects/object.h>
#include <physics.h>

_Template::_Template() {

	// Generic properties
	TemplateID = -1;
	Name = "";
	Type = _Object::NONE;
	Lifetime = 0.0f;

	// Collisions
	CollisionCallback = "";
	CollisionGroup = _Physics::FILTER_RIGIDBODY | _Physics::FILTER_CAMERA;
	CollisionMask = _Physics::FILTER_RIGIDBODY | _Physics::FILTER_STATIC;

	// Physical properties
	Sleep = 0;
	CollisionFile = "";
	Plane = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	Shape = glm::vec3(1.0f, 1.0f, 1.0f);
	Radius = 0.5f;
	Mass = 1.0f;
	Friction = 1.0f;
	RollingFriction = 0.0f;
	Restitution = 0.0f;
	LinearDamping = 0.0001f;
	AngularDamping = 0.0003f;

	// Constraints
	ConstraintAxis = glm::vec3(0.0f, 0.0f, 0.0f);

	// Textures
	for(int i = 0; i < 4; i++)
		TextureScale[i] = 1.0f;

	// Graphics
	for(int i = 0; i < 3; i++)
		Scale[i] = 1.0f;
	Mesh = "";
	Fog = false;
	Shadows = true;
	Detail = 24;
	CustomMaterial = -1;
	EmitLight = false;

	// Zones
	Active = true;

	// Terrain
	Smooth = 5;
}

_ObjectSpawn::_ObjectSpawn() {
	Name = "";
	Position = glm::vec3(0.0f, 0.0f, 0.0f);
	Rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	Quaternion = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	LinearVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
	AngularVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
	Template = nullptr;
	HasQuaternion = false;
}

// Get rotation from Quaternion if it exists
void _ObjectSpawn::CalculateRotation() {
	if(HasQuaternion)
		Rotation = glm::degrees(glm::eulerAngles(Quaternion));
}

_ConstraintSpawn::_ConstraintSpawn() {
	Name = "";
	BodyA = nullptr;
	BodyB = nullptr;
	Template = nullptr;
}
