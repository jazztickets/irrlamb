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
#include <objects/plane.h>
#include <globals.h>
#include <physics.h>
#include <objects/template.h>
#include <IAnimatedMesh.h>
#include <IAnimatedMeshSceneNode.h>
#include <ISceneManager.h>
#include <ode/objects.h>
#include <ode/collision.h>
#include <glm/gtx/rotate_vector.hpp>

using namespace irr;

// Constructor
_Plane::_Plane(const _ObjectSpawn &Object) :
	_Object(Object.Template),
	Plane(Object.Plane) {

	// Check for mesh file
	if(Template->Mesh != "") {

		// Load mesh
		Node = LoadMesh(Template->Mesh);
		if(Node) {
			Node->setScale(core::vector3df(Template->Scale[0], Template->Scale[1], Template->Scale[2]));
			if(Template->Textures[0] != "")
				Node->setMaterialTexture(0, irrDriver->getTexture(Template->Textures[0].c_str()));
			if(Template->CustomMaterial != -1)
				Node->setMaterialType((video::E_MATERIAL_TYPE)Template->CustomMaterial);

			Node->getMaterial(0).getTextureMatrix(0).setScale(core::vector3df(Template->TextureScale[0], Template->TextureScale[0], 0));
		}
	}

	// Set up physics
	if(Physics.IsEnabled()) {

		// Create geometry
		Geometry = dCreatePlane(Physics.GetSpace(), Plane[0], Plane[1], Plane[2], Plane[3]);
	}

	// Set common properties
	SetProperties(Object, false);
	UpdateTransform();
}

// Update node transform from plane equation
void _Plane::UpdateTransform() {
	if(!Node)
		return;

	// Get normal
	glm::vec3 Normal = Plane;

	// Set position
	Node->setPosition(core::vector3df(Normal[0], Normal[1], Normal[2]) * Plane[3]);

	// Set rotation
	if(Normal != glm::vec3(0, 1, 0)) {
		glm::mat4 RotationMatrix = glm::orientation(Normal, glm::vec3(0, 1, 0));
		glm::vec3 Rotation = Physics.QuaternionToEuler(glm::quat(RotationMatrix));
		Node->setRotation(core::vector3df(Rotation[0], Rotation[1], Rotation[2]));
	}
}
