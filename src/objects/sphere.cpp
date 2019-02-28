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
#include <objects/sphere.h>
#include <globals.h>
#include <physics.h>
#include <config.h>
#include <objects/template.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <ISceneManager.h>
#include <IMeshSceneNode.h>

using namespace irr;

// Constructor
_Sphere::_Sphere(const _ObjectSpawn &Object)
:	_Object() {
	_Template *Template = Object.Template;

	// Get file path
	std::string MeshPath = std::string("meshes/") + Template->Mesh;

	// Add mesh
	Node = irrScene->addSphereSceneNode(Template->Radius, Template->Detail);
	if(Node) {
		if(Template->Textures[0] != "")
			Node->setMaterialTexture(0, irrDriver->getTexture(Template->Textures[0].c_str()));
		if(Template->CustomMaterial != -1)
			Node->setMaterialType((video::E_MATERIAL_TYPE)Template->CustomMaterial);
	}

	// Set up physics
	if(Physics.IsEnabled()) {

		// Create shape
		btSphereShape *Shape = new btSphereShape(Template->Radius);

		// Set up physics
		CreateRigidBody(Object, Shape);
	}

	// Set common properties
	SetProperties(Object);
}
