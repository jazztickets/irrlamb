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
#include <objectmanager.h>
#include <replay.h>
#include <level.h>
#include <physics.h>
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

	if(Object != nullptr) {

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
	for(auto &Iterator : Objects) {
		if(Iterator->GetName() == Name)
			return Iterator;
	}

	return nullptr;
}

// Gets an object by type
_Object *_ObjectManager::GetObjectByType(int Type) {

	// Search through object list
	for(auto &Iterator : Objects) {
		if(Iterator->GetType() == Type)
			return Iterator;
	}

	return nullptr;
}

// Deletes all of the objects
void _ObjectManager::ClearObjects() {

	// Delete constraints first
	for(auto Iterator = Objects.begin(); Iterator != Objects.end(); ) {
		_Object *Object = *Iterator;
		if(Object->GetType() == _Object::CONSTRAINT_D6 || Object->GetType() == _Object::CONSTRAINT_HINGE) {
			delete Object;
			Object = nullptr;
			Iterator = Objects.erase(Iterator);
		}
		else
			++Iterator;
	}

	// Delete objects
	for(auto &Iterator : Objects) {
		delete Iterator;
	}

	Objects.clear();
	NextObjectID = 0;
}

// Performs start frame operations on the objects
void _ObjectManager::BeginFrame() {

	for(auto &Iterator : Objects)
		Iterator->BeginFrame();
}

// Performs end frame operations on the objects
void _ObjectManager::EndFrame() {
	bool UpdateReplay = Replay.NeedsPacket();
	uint16_t ReplayMovementCount = 0;

	// Get replay update count
	for(auto &Iterator : Objects) {

		// Perform specific end-of-frame operations
		Iterator->EndFrame();

		// Get a count for all the objects that need replay events recorded
		if(Iterator->ReadyForReplayUpdate()) {
			ReplayMovementCount++;
		}
	}

	// Write a replay movement packet
	if(UpdateReplay && ReplayMovementCount > 0) {

		// Write replay event
		std::fstream &ReplayFile = Replay.GetFile();
		Replay.WriteEvent(_Replay::PACKET_MOVEMENT);
		ReplayFile.write((char *)&ReplayMovementCount, sizeof(ReplayMovementCount));

		// Write the updated objects
		for(auto &Iterator : Objects) {

			// Save the replay
			if(Iterator->ReadyForReplayUpdate()) {
				btVector3 EulerRotation;
				Physics.QuaternionToEuler(Iterator->GetRotation(), EulerRotation);

				// Write object update
				ReplayFile.write((char *)&Iterator->GetID(), sizeof(Iterator->GetID()));
				ReplayFile.write((char *)&Iterator->GetPosition(), sizeof(btScalar) * 3);
				ReplayFile.write((char *)&EulerRotation[0], sizeof(btScalar) * 3);
				Iterator->WroteReplayPacket();
			}
		}
		//printf("ObjectIndex=%d\n", ObjectIndex);
	}
}

// Updates all objects in the scene
void _ObjectManager::Update(float FrameTime) {

	// Update objects
	for(auto Iterator = Objects.begin(); Iterator != Objects.end(); ) {
		_Object *Object = *Iterator;

		// Update the object
		Object->Update(FrameTime);

		// Delete old objects
		if(Object->GetDeleted()) {

			// Write delete events to the replay
			if(Replay.IsRecording()) {
				std::fstream &ReplayFile = Replay.GetFile();
				Replay.WriteEvent(_Replay::PACKET_DELETE);
				ReplayFile.write((char *)&Object->GetID(), sizeof(Object->GetID()));
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
	for(auto &Iterator : Objects)
		Iterator->UpdateReplay(FrameTime);
}

// Updates all objects in the scene from a replay file
void _ObjectManager::UpdateFromReplay() {
	core::vector3df Position, Rotation;

	// Get replay stream and read object count
	std::fstream &ReplayFile = Replay.GetFile();
	int16_t ObjectCount;
	ReplayFile.read((char *)&ObjectCount, sizeof(ObjectCount));

	// Read first object
	int16_t ObjectID;
	ReplayFile.read((char *)&ObjectID, sizeof(ObjectID));
	ReplayFile.read((char *)&Position.X, sizeof(float) * 3);
	ReplayFile.read((char *)&Rotation.X, sizeof(float) * 3);

	// Loop through the rest of the objects
	int UpdatedObjectCount = 0;
	for(auto &Iterator : Objects) {
		if(ObjectID == Iterator->GetID()) {
			Iterator->SetPositionFromReplay(Position);
			Iterator->GetNode()->setRotation(Rotation);

			//printf("ObjectPacket ObjectID=%d Type=%d Position=%f %f %f Rotation=%f %f %f\n", ObjectID, Object->GetType(), Position.X, Position.Y, Position.Z, Rotation.X, Rotation.Y, Rotation.Z);
			if(UpdatedObjectCount < ObjectCount - 1) {
				ReplayFile.read((char *)&ObjectID, sizeof(ObjectID));
				ReplayFile.read((char *)&Position.X, sizeof(float) * 3);
				ReplayFile.read((char *)&Rotation.X, sizeof(float) * 3);
			}

			UpdatedObjectCount++;
		}
	}

	//printf("ObjectIndex=%d\n", ObjectIndex);
}

// Returns an object by an index, nullptr if no such index
_Object *_ObjectManager::GetObjectByID(int ID) {

	for(auto &Iterator : Objects) {
		if(Iterator->GetID() == ID)
			return Iterator;
	}

	return nullptr;
}

// Print all object orientations
void _ObjectManager::PrintObjectOrientations() {
	for(auto &Iterator : Objects) {
		Iterator->PrintOrientation();
	}
}

// Deletes an object by its ID
void _ObjectManager::DeleteObjectByID(int ID) {

	for(auto Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator) {
		if((*Iterator)->GetID() == ID) {
			delete (*Iterator);
			Objects.erase(Iterator);
			return;
		}
	}
}
