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
#include <objects/template.h>
#include <IAnimatedMesh.h>
#include <IAnimatedMeshSceneNode.h>
#include <ISceneManager.h>
#include <IMeshSceneNode.h>
#include <ode/objects.h>
#include <ode/collision.h>

using namespace irr;

// Constructor
_Sphere::_Sphere(const _ObjectSpawn &Object) :
	_Object(Object.Template) {

	// Check for mesh file
	if(Template->Mesh != "") {

		// Get file path
		std::string MeshPath = std::string("meshes/") + Template->Mesh;

		// Load mesh
		scene::IAnimatedMesh *AnimatedMesh = irrScene->getMesh(MeshPath.c_str());
		Node = irrScene->addAnimatedMeshSceneNode(AnimatedMesh);
		Node->setScale(core::vector3df(Template->Scale[0], Template->Scale[1], Template->Scale[2]));
	}
	else
		Node = irrScene->addSphereSceneNode(Template->Radius, Template->Detail);

	if(Node) {
		if(Template->Textures[0] != "")
			Node->setMaterialTexture(0, irrDriver->getTexture(Template->Textures[0].c_str()));
		if(Template->CustomMaterial != -1)
			Node->setMaterialType((video::E_MATERIAL_TYPE)Template->CustomMaterial);
	}

	// Set up physics
	if(Physics.IsEnabled()) {

		// Create geometry
		Geometry = dCreateSphere(Physics.GetSpace(), Object.Template->Radius);

		// Create body
		if(Template->Mass > 0) {
			CreateRigidBody(Object, Geometry);

			// Set mass
			dMass Mass;
			dMassSetSphereTotal(&Mass, Template->Mass, Template->Radius);
			dBodySetMass(Body, &Mass);
		}
	}

	// Set common properties
	SetProperties(Object);
}
