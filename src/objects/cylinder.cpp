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
#include <objects/cylinder.h>
#include <globals.h>
#include <physics.h>
#include <config.h>
#include <objects/template.h>
#include <BulletCollision/CollisionShapes/btCylinderShape.h>
#include <IAnimatedMesh.h>
#include <IAnimatedMeshSceneNode.h>
#include <ISceneManager.h>

using namespace irr;

// Constructor
_Cylinder::_Cylinder(const SpawnStruct &Object)
:	_Object() {
	TemplateStruct *Template = Object.Template;

	// Check for mesh file
	if(Template->Mesh != "") {

		// Get file path
		std::string MeshPath = std::string("meshes/") + Template->Mesh;

		// Add mesh
		scene::IAnimatedMesh *AnimatedMesh = irrScene->getMesh(MeshPath.c_str());
		Node = irrScene->addAnimatedMeshSceneNode(AnimatedMesh);
		if(Node) {
			Node->setScale(core::vector3df(Template->Scale, Template->Scale, Template->Scale));
			if(Template->Textures[0] != "")
				Node->setMaterialTexture(0, irrDriver->getTexture(Template->Textures[0].c_str()));
			if(Template->CustomMaterial != -1)
				Node->setMaterialType((video::E_MATERIAL_TYPE)Template->CustomMaterial);

			// Add shadows
			if(Config.Shadows) {
				//((scene::IAnimatedMeshSceneNode *)Node)->addShadowVolumeSceneNode();
			}
		}
	}

	// Set up physics
	if(Physics.IsEnabled()) {

		// Create shape
		btVector3 HalfExtents = Template->Shape * 0.5f;
		btCylinderShape *Shape = new btCylinderShape(HalfExtents);

		// Set up physics
		CreateRigidBody(Object, Shape);
	}

	// Set common properties
	SetProperties(Object);
}
