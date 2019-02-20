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

// Libraries
#include <string>
#include <list>
#include <irrTypes.h>

// Forward Declarations
class _Object;

// Classes
class _ObjectManager {

	public:

		_ObjectManager();

		int Init();
		int Close();

		void Update(float FrameTime);
		void UpdateReplay(float FrameTime);
		void UpdateFromReplay();
		void InterpolatePositions(float BlendFactor);
		void BeginFrame();
		void EndFrame();

		_Object *AddObject(_Object *Object);
		void DeleteObject(_Object *Object);
		void DeleteObjectByID(int ID);
		_Object *GetObjectByName(const std::string &Name);
		_Object *GetObjectByType(int Type);
		_Object *GetObjectByID(int ID);

		void ClearObjects();
		size_t GetObjectCount() const { return Objects.size(); }
		const std::list<_Object *> &GetObjects() const { return Objects; }

	private:

		std::list<_Object *> Objects;
		uint16_t NextObjectID;

};

// Singletons
extern _ObjectManager ObjectManager;
