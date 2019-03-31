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
#include <objects/object.h>
#include <camera.h>
#include <ILightSceneNode.h>

// Classes
class _Player : public _Object {

	public:

		_Player(const _ObjectSpawn &Object);
		~_Player();

		void Update(float FrameTime);
		void HandleInput();
		void HandlePush(irr::core::vector3df &Push);
		void GetPushDirection(irr::core::vector3df &Push);

		void Jump();
		void SetCamera(_Camera *Camera) { this->Camera = Camera; }
		void SetPositionFromReplay(const irr::core::vector3df &Position);

	private:

		// Camera
		_Camera *Camera;

		// Graphics
		irr::scene::ILightSceneNode *Light;
		irr::scene::ISceneNode *InnerNode;

		// Audio
		_AudioSource *Sound;

		// Jumping
		float JumpTimer;
		float JumpCooldown;
		float TorqueFactor;

};
