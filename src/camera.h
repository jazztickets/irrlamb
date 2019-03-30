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
#include <vector3d.h>
#include <ICameraSceneNode.h>

// Classes
class _Camera {

	public:

		_Camera();
		~_Camera();

		void Update(const irr::core::vector3df &Target);
		void RecordReplay();
		void HandleMouseMotion(float UpdateX, float UpdateY);

		void SetRotation(float Yaw, float Pitch) { this->Yaw = Yaw, this->Pitch = Pitch; }
		void SetDistance(float Distance) { MaxDistance = Distance; }
		void SetFOV(float FOV);

		float GetYaw() const { return Yaw; }
		float GetPitch() const { return Pitch; }

		void SetYaw(float Value) { Yaw = Value; }
		void SetPitch(float Value) { Pitch = Value; }
		const irr::core::vector3df &GetDirection() const { return Direction; }

		irr::scene::ICameraSceneNode *GetNode() { return Node; }

	private:

		irr::scene::ICameraSceneNode *Node;
		irr::core::matrix4 Transform;
		irr::core::vector3df Direction;
		float Yaw, Pitch, MaxDistance, Distance;

		// Replays
		irr::core::vector3df PreviousPosition, PreviousLookAt;
		bool MovementChanged;

};
