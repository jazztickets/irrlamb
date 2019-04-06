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
#include <IBillboardSceneNode.h>
#include <ILightSceneNode.h>

// Classes
class _Orb : public _Object {

	public:

		enum OrbStateType {
			ORBSTATE_NORMAL,
			ORBSTATE_DEACTIVATING,
			ORBSTATE_DEACTIVATED,
		};

		_Orb(const _ObjectSpawn &Object);
		~_Orb();

		void Update(float FrameTime);
		void UpdateReplay(float FrameTime);
		void SetPositionFromReplay(const irr::core::vector3df &Position);

		void StartDeactivation(const std::string &TCallback, float Length);
		bool IsStillActive() const { return State == ORBSTATE_NORMAL; }
		int GetState() const { return State; }

		void SetShape(const glm::vec3 &Shape) override;

	private:

		void UpdateDeactivation(float FrameTime);

		// Graphics
		irr::video::SColor GlowColor;
		irr::scene::IBillboardSceneNode *InnerNode;
		irr::scene::ILightSceneNode *Light;

		// Audio
		_AudioSource *Sound;

		// Deactivation
		std::string DeactivationCallback;
		int State;
		float OrbTime;
		float DeactivateLength;

};
