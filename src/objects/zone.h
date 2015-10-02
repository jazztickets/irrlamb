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
#pragma once

// Libraries
#include <objects/object.h>
#include <list>

// Keeps track of touched objects
struct ObjectTouchState {
	ObjectTouchState() { }
	ObjectTouchState(_Object *Object, int TouchCount) : Object(Object), TouchCount(TouchCount) { }

	_Object *Object;
	int TouchCount;
};

// Classes
class _Zone : public _Object {

	public:

		_Zone(const _ObjectSpawn &Object);

		void EndFrame();
		virtual void HandleCollision(_Object *OtherObject, const btPersistentManifold *ContactManifold, float NormalScale);

		void SetActive(bool Value);

	private:

		// Attributes
		bool Active;

		// Touch state
		std::list<ObjectTouchState> TouchState;

};
