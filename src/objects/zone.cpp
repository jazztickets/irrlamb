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
#include <objects/zone.h>
#include <globals.h>
#include <physics.h>
#include <scripting.h>
#include <objects/template.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>

// Constructor
_Zone::_Zone(const _ObjectSpawn &Object)
:	_Object() {

	_Template *Template = Object.Template;
	Active = Template->Active;

	// Set up physics
	if(Physics.IsEnabled()) {

		// Create shape
		btVector3 HalfExtents = Template->Shape * 0.5f;
		btBoxShape *Shape = new btBoxShape(HalfExtents);

		// Set up physics
		CreateRigidBody(Object, Shape);

		// Set collision flags
		RigidBody->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
	}

	// Set common properties
	SetProperties(Object);
	if(CollisionCallback == "")
		CollisionCallback = "OnHitZone";
}

// Collision callback
void _Zone::HandleCollision(_Object *OtherObject, const btPersistentManifold *ContactManifold, float NormalScale) {

	if(Active) {

		// Search for existing objects in the touch list
		for(auto &Iterator : TouchState) {
			if(Iterator.Object == OtherObject) {

				// Update touch count
				Iterator.TouchCount = 2;
				return;
			}
		}

		// A new object has collided with the zone
		TouchState.push_back(ObjectTouchState(OtherObject, 2));

		// Call Lua function
		if(CollisionCallback.size())
			Scripting.CallZoneHandler(CollisionCallback, 0, this, OtherObject);
	}
}

// Removes old objects from the touch list
void _Zone::EndFrame() {

	if(Active) {

		// Search for old objects
		for(auto Iterator = TouchState.begin(); Iterator != TouchState.end(); ) {
			Iterator->TouchCount--;
			if(Iterator->TouchCount <= 0) {

				// Call Lua function
				if(CollisionCallback.size())
					Scripting.CallZoneHandler(CollisionCallback, 1, this, Iterator->Object);

				Iterator = TouchState.erase(Iterator);
			}
			else
				++Iterator;
		}
	}
}

// Sets the active state of the zone
void _Zone::SetActive(bool Value) {
	Active = Value;

	TouchState.clear();
}
