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
#include <camera.h>
#include <replay.h>
#include <globals.h>
#include <audio.h>
#include <physics.h>
#include <ISceneManager.h>

using namespace irr;

// Constructor
_Camera::_Camera()
:	Node(nullptr),
	Yaw(0.0f),
	Pitch(30.0f),
	MaxDistance(5.0f),
	Distance(5.0f),
	MovementChanged(true) {

	Node = irrScene->addCameraSceneNode();
	Node->setNearValue(0.1f);
	Node->setFarValue(1000.0f);
}

// Destructor
_Camera::~_Camera() {

	Node->remove();
}

// Updates the camera based on the mouse
void _Camera::HandleMouseMotion(float UpdateX, float UpdateY) {

	// Update yaw and pitch
	Yaw += UpdateX;
	Pitch += UpdateY;

	// Check limits
	if(Pitch > 89.0f)
		Pitch = 89.0f;
	else if(Pitch <= -89.0f)
		Pitch = -89.0f;
}

// Updates the camera
void _Camera::Update(const core::vector3df &Target) {

	// Get camera rotation
	Transform.makeIdentity();
	Transform.setRotationDegrees(core::vector3df(Pitch, Yaw, 0.0f));

	// Set distance from object
	Distance = MaxDistance;

	// Get camera offset
	core::vector3df Offset(0.0f, 0.0f, 1.0f);
	Transform.transformVect(Offset);

	// Set listener direction
	Audio.SetDirection(Offset.X, 0, Offset.Z);

	// Position the camera
	Offset *= -Distance;

	// Set camera target
	Node->setTarget(Target);

	// Get ray intervals
	glm::vec3 RayStart(Target.X, Target.Y, Target.Z), RayEnd(RayStart);
	RayEnd[0] += Offset.X;
	RayEnd[1] += Offset.Y;
	RayEnd[2] += Offset.Z;

	// Get point on wall where ray collides
	Physics.RaycastWorld(RayStart, RayEnd);

	// Shorten the offset a little bit
	glm::vec3 NewOffset = RayEnd - RayStart;
	NewOffset *= 0.9f;

	// Set the new position
	RayEnd = RayStart + NewOffset;
	Node->setPosition(core::vector3df(RayEnd[0], RayEnd[1], RayEnd[2]));

	// Note changes
	if(!MovementChanged && (PreviousPosition != Node->getPosition() || PreviousLookAt != Node->getTarget()))
		MovementChanged = true;

	PreviousPosition = Node->getPosition();
	PreviousLookAt = Node->getTarget();
}

// Record the camera
void _Camera::RecordReplay() {

	// Write replay
	if(Replay.NeedsPacket() && MovementChanged) {
		MovementChanged = false;

		// Write replay information
		std::fstream &ReplayFile = Replay.GetFile();
		Replay.WriteEvent(_Replay::PACKET_CAMERA);
		ReplayFile.write((char *)&Node->getPosition(), sizeof(core::vector3df));
		ReplayFile.write((char *)&Node->getTarget(), sizeof(core::vector3df));
	}
}
