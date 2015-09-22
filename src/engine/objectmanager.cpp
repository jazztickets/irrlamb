/*************************************************************************************
*	irrlamb - https://github.com/jazztickets/irrlamb
*	Copyright (C) 2015  Alan Witkowski
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************************/
#include <engine/objectmanager.h>
#include <engine/replay.h>
#include <engine/level.h>
#include <engine/physics.h>
#include <objects/object.h>

using namespace irr;

_ObjectManager ObjectManager;

// Constructor
_ObjectManager::_ObjectManager()
:	NextObjectID(0) {

}

// Initializes the level manager
int _ObjectManager::Init() {

	NextObjectID = 0;

	return 1;
}

// Closes the graphics system
int _ObjectManager::Close() {

	ClearObjects();

	return 1;
}

// Adds an object to the manager
_Object *_ObjectManager::AddObject(_Object *Object) {

	if(Object != NULL) {

		// Set replay ID
		Object->SetID(NextObjectID);
		NextObjectID++;

		Objects.push_back(Object);
	}

	return Object;
}

// Deletes an object
void _ObjectManager::DeleteObject(_Object *Object) {

	Object->SetDeleted(true);
}

// Gets an object by name
_Object *_ObjectManager::GetObjectByName(const std::string &Name) {

	// Search through object list
	for(std::list<_Object *>::iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator) {
		if((*Iterator)->GetName() == Name)
			return *Iterator;
	}

	return NULL;
}

// Gets an object by type
_Object *_ObjectManager::GetObjectByType(int Type) {

	// Search through object list
	for(std::list<_Object *>::iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator) {
		if((*Iterator)->GetType() == Type)
			return *Iterator;
	}

	return NULL;
}

// Deletes all of the objects
void _ObjectManager::ClearObjects() {

	// Delete constraints first
	for(std::list<_Object *>::iterator Iterator = Objects.begin(); Iterator != Objects.end(); ) {
		_Object *Object = *Iterator;
		if(Object->GetType() == _Object::CONSTRAINT_D6 || Object->GetType() == _Object::CONSTRAINT_HINGE) {
			delete Object;
			Object = NULL;
			Iterator = Objects.erase(Iterator);
		}
		else
			++Iterator;
	}

	// Delete objects
	for(std::list<_Object *>::iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator) {
		delete (*Iterator);
	}

	Objects.clear();
	NextObjectID = 0;
}

// Performs start frame operations on the objects
void _ObjectManager::BeginFrame() {

	for(std::list<_Object *>::iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator) {
		_Object *Object = *Iterator;

		// Perform specific start-of-frame operations
		Object->BeginFrame();
	}
}

// Performs end frame operations on the objects
void _ObjectManager::EndFrame() {
	bool UpdateReplay = Replay.NeedsPacket();
	u16 ReplayMovementCount = 0;

	// Get replay update count
	for(std::list<_Object *>::iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator) {
		_Object *Object = *Iterator;

		// Perform specific end-of-frame operations
		Object->EndFrame();

		// Get a count for all the objects that need replay events recorded
		if(Object->ReadyForReplayUpdate()) {
			ReplayMovementCount++;
		}
	}

	// Write a replay movement packet
	if(UpdateReplay && ReplayMovementCount > 0) {

		// Write replay event
		_File &ReplayStream = Replay.GetReplayStream();
		Replay.WriteEvent(_Replay::PACKET_MOVEMENT);
		ReplayStream.WriteShortInt(ReplayMovementCount);

		// Write the updated objects
		for(std::list<_Object *>::iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator) {
			_Object *Object = *Iterator;

			// Save the replay
			if(Object->ReadyForReplayUpdate()) {
				btVector3 EulerRotation;
				Physics.QuaternionToEuler(Object->GetRotation(), EulerRotation);

				// Write object update
				ReplayStream.WriteShortInt(Object->GetID());
				ReplayStream.WriteData((void *)&Object->GetPosition(), sizeof(btScalar) * 3);
				ReplayStream.WriteData((void *)&EulerRotation[0], sizeof(btScalar) * 3);
				Object->WroteReplayPacket();
			}
		}
		//printf("ObjectIndex=%d\n", ObjectIndex);
	}
}

// Updates all objects in the scene
void _ObjectManager::Update(float FrameTime) {

	// Update objects
	for(std::list<_Object *>::iterator Iterator = Objects.begin(); Iterator != Objects.end(); ) {
		_Object *Object = *Iterator;

		// Update the object
		Object->Update(FrameTime);

		// Delete old objects
		if(Object->GetDeleted()) {

			// Write delete events to the replay
			if(Replay.IsRecording()) {
				_File &ReplayStream = Replay.GetReplayStream();
				Replay.WriteEvent(_Replay::PACKET_DELETE);
				ReplayStream.WriteShortInt(Object->GetID());
			}

			delete Object;
			Iterator = Objects.erase(Iterator);
		}
		else {

			++Iterator;
		}
	}
}

// Update special replays function for each object
void _ObjectManager::UpdateReplay(float FrameTime) {

	// Update objects
	for(std::list<_Object *>::iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator)
		(*Iterator)->UpdateReplay(FrameTime);
}

// Updates all objects in the scene from a replay file
void _ObjectManager::UpdateFromReplay() {
	core::vector3df Position, Rotation;

	// Get replay stream and read object count
	_File &ReplayStream = Replay.GetReplayStream();
	int ObjectCount = ReplayStream.ReadShortInt();

	// Read first object
	int ObjectID = ReplayStream.ReadShortInt();
	ReplayStream.ReadData(&Position.X, sizeof(float) * 3);
	ReplayStream.ReadData(&Rotation.X, sizeof(float) * 3);

	// Loop through the rest of the objects
	int UpdatedObjectCount = 0;
	for(std::list<_Object *>::iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator) {
		_Object *Object = *Iterator;
		if(ObjectID == Object->GetID()) {
			Object->SetPositionFromReplay(Position);
			Object->GetNode()->setRotation(Rotation);

			//printf("ObjectPacket ObjectID=%d Type=%d Position=%f %f %f Rotation=%f %f %f\n", ObjectID, Object->GetType(), Position.X, Position.Y, Position.Z, Rotation.X, Rotation.Y, Rotation.Z);
			if(UpdatedObjectCount < ObjectCount - 1) {
				ObjectID = ReplayStream.ReadShortInt();
				ReplayStream.ReadData(&Position.X, sizeof(float) * 3);
				ReplayStream.ReadData(&Rotation.X, sizeof(float) * 3);
			}

			UpdatedObjectCount++;
		}
	}

	//printf("ObjectIndex=%d\n", ObjectIndex);
}

// Returns an object by an index, NULL if no such index
_Object *_ObjectManager::GetObjectByID(int ID) {

	for(std::list<_Object *>::iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator) {
		if((*Iterator)->GetID() == ID)
			return *Iterator;
	}

	return NULL;
}

// Deletes an object by its ID
void _ObjectManager::DeleteObjectByID(int ID) {

	for(std::list<_Object *>::iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator) {
		if((*Iterator)->GetID() == ID) {
			delete (*Iterator);
			Objects.erase(Iterator);
			return;
		}
	}
}
