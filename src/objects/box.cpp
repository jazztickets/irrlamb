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
#include <objects/box.h>
#include <globals.h>
#include <physics.h>
#include <config.h>
#include <objects/template.h>
#include <IAnimatedMesh.h>
#include <IAnimatedMeshSceneNode.h>
#include <ISceneManager.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <ode/objects.h>
#include <ode/collision.h>

using namespace irr;

// Constructor
_Box::_Box(const _ObjectSpawn &Object)
:	_Object(Object.Template) {

	// Check for mesh file
	if(Template->Mesh != "") {

		// Get file path
		std::string MeshPath = std::string("meshes/") + Template->Mesh;

		// Add mesh
		scene::IAnimatedMesh *AnimatedMesh = irrScene->getMesh(MeshPath.c_str());
		Node = irrScene->addAnimatedMeshSceneNode(AnimatedMesh);
		if(Node) {
			Node->setScale(core::vector3df(Template->Scale[0], Template->Scale[1], Template->Scale[2]));
			if(Template->Textures[0] != "")
				Node->setMaterialTexture(0, irrDriver->getTexture(Template->Textures[0].c_str()));
			if(Template->CustomMaterial != -1)
				Node->setMaterialType((video::E_MATERIAL_TYPE)Template->CustomMaterial);

			// Add shadows
			if(Config.Shadows && Template->Shadows) {
				((scene::IAnimatedMeshSceneNode *)Node)->addShadowVolumeSceneNode();
			}
		}
	}

	// Set up physics
	if(Physics.IsEnabled()) {

		// Create Geometry
		Geometry = dCreateBox(Physics.GetSpace(), Template->Shape[0], Template->Shape[1], Template->Shape[2]);

		// Create body
		if(Template->Mass > 0) {
			CreateRigidBody(Object, Geometry);

			// Set mass
			dMass Mass;
			dMassSetBoxTotal(&Mass, Template->Mass, Template->Shape[0], Template->Shape[1], Template->Shape[2]);
			dBodySetMass(Body, &Mass);
		}
		else {
			dGeomSetPosition(Geometry, Object.Position[0], Object.Position[1], Object.Position[2]);
		}
	}

	// Set common properties
	SetProperties(Object);
}
