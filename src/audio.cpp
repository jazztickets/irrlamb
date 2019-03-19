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
#include <audio.h>
#include <log.h>
#include <config.h>
#include <vorbis/vorbisfile.h>
#include <vector>

_Audio Audio;

// Initializes the audio system
int _Audio::Init(bool Enabled) {

	// Set audio enabled
	this->Enabled = Enabled;
	if(!Enabled)
		return 1;

	Log.Write("Initializing audio");

	// Create device
	ALCdevice *Device = alcOpenDevice(nullptr);
	if(Device == nullptr) {
		Log.Write("Unable to create audio device");
		Enabled = false;
		return 0;
	}

	// Create context
	ALCcontext *Context = alcCreateContext(Device, nullptr);

	// Set active context
	alcMakeContextCurrent(Context);

	// Clear code
	alGetError();

	return 1;
}

// Closes the audio system
int _Audio::Close() {
	if(!Enabled)
		return 1;

	// Remove playing sounds
	for(auto &Iterator : Sources)
		delete Iterator;
	Sources.clear();

	// Free loaded sounds
	FreeAllBuffers();

	// Get active context
	ALCcontext *Context = alcGetCurrentContext();

	// Get device for active context
	ALCdevice *Device = alcGetContextsDevice(Context);

	// Disable context
	alcMakeContextCurrent(nullptr);

	// Free context
	alcDestroyContext(Context);

	// Close device
	alcCloseDevice(Device);

	Enabled = false;

	return 1;
}

// Update all audio sources
void _Audio::Update() {
	if(!Enabled)
		return;

	// Update sources
	for(auto Iterator = Sources.begin(); Iterator != Sources.end(); ) {
		_AudioSource *Source = *Iterator;
		bool NeedsDelete = false;

		// Check conditions for deletion
		if(!Source->IsPlaying()) {
			NeedsDelete = true;
		}

		// Delete source
		if(NeedsDelete) {
			delete Source;
			Iterator = Sources.erase(Iterator);
		}
		else {
			++Iterator;
		}
	}
}

// Stop all sounds from the audio manager and remove them
void _Audio::StopSounds() {
	for(auto &Iterator : Sources) {
		_AudioSource *Source = Iterator;
		Source->Stop();

		delete Source;
	}

	Sources.clear();
}

// Loads an ogg file into memory
bool _Audio::LoadBuffer(const std::string &File) {
	if(!Enabled)
		return true;

	// Get path
	std::string Path = std::string("sounds/") + File;

	// Find existing buffer in map
	if(Buffers.find(Path) != Buffers.end())
		return true;

	// Open vorbis stream
	OggVorbis_File VorbisStream;
	int ReturnCode = ov_fopen(Path.c_str(), &VorbisStream);
	if(ReturnCode != 0) {
		Log.Write("ov_fopen failed on file %s with code %d", Path.c_str(), ReturnCode);
		return false;
	}

	// Get vorbis file info
	vorbis_info *Info = ov_info(&VorbisStream, -1);

	// Create new buffer
	_AudioBuffer AudioBuffer;
	switch(Info->channels) {
		case 1:
			AudioBuffer.Format = AL_FORMAT_MONO16;
		break;
		case 2:
			AudioBuffer.Format = AL_FORMAT_STEREO16;
		break;
		default:
			Log.Write("Unsupported # of channels %d for %s", Path.c_str());
			return false;
		break;
	}

	// Alloc some memory for the samples
	std::vector<char> Data;

	// Decode vorbis file
	long BytesRead;
	char Buffer[4096];
	int BitStream;
	do {
		BytesRead = ov_read(&VorbisStream, Buffer, 4096, 0, 2, 1, &BitStream);
		Data.insert(Data.end(), Buffer, Buffer + BytesRead);
	} while(BytesRead > 0);

	// Create buffer
	alGenBuffers(1, &AudioBuffer.ID);
	alBufferData(AudioBuffer.ID, AudioBuffer.Format, &Data[0], (ALsizei)Data.size(), Info->rate);

	// Close vorbis file
	ov_clear(&VorbisStream);

	// Add to map
	Buffers[Path] = AudioBuffer;

	return true;
}

// Play an audio source and add it to the audio manager
void _Audio::Play(_AudioSource *AudioSource, float X, float Y, float Z) {
	if(!Enabled)
		return;

	AudioSource->SetPosition(X, Y, Z);
	Sources.push_back(AudioSource);

	AudioSource->Play();
}

// Get a loaded buffer
const _AudioBuffer *_Audio::GetBuffer(const std::string &File) {
	if(!Enabled)
		return nullptr;

	// Get path
	std::string Path = std::string("sounds/") + File;

	// Find buffer in map
	auto BuffersIterator = Buffers.find(Path);
	if(BuffersIterator == Buffers.end())
		return nullptr;

	return &BuffersIterator->second;
}

// Free buffer
void _Audio::CloseBuffer(const std::string &File) {
	if(!Enabled)
		return;

	// Get path
	std::string Path = std::string("sounds/") + File;

	// Find buffer in map
	auto BuffersIterator = Buffers.find(Path);
	if(BuffersIterator == Buffers.end())
		return;

	_AudioBuffer &Buffer = BuffersIterator->second;
	alDeleteBuffers(1, &Buffer.ID);
	Buffers.erase(BuffersIterator);
}

// Free all loaded buffers
void _Audio::FreeAllBuffers() {
	if(!Enabled)
		return;

	// Iterate over map
	for(auto BuffersIterator : Buffers) {
		_AudioBuffer &Buffer = BuffersIterator.second;

		alDeleteBuffers(1, &Buffer.ID);
	}

	Buffers.clear();
}

// Set position of listener
void _Audio::SetPosition(float X, float Y, float Z) {
	if(!Enabled)
		return;

	alListener3f(AL_POSITION, X, Y, -Z);
}

// Sets the listener direction
void _Audio::SetDirection(float X, float Y, float Z) {
	if(!Enabled)
		return;

	float Orientation[6] = { X, Y, -Z, 0.0f, 1.0f, 0.0f };
	alListenerfv(AL_ORIENTATION, Orientation);
}

// Set gain
void _Audio::SetGain(float Value) {
	if(!Enabled)
		return;

	alListenerf(AL_GAIN, Value * Config.SoundVolume);
}

// Create an audio source
_AudioSource::_AudioSource(const _AudioBuffer *Buffer, bool Loop, float MinGain, float MaxGain, float ReferenceDistance, float RollOff) {
	Loaded = false;
	if(Buffer) {

		// Create source
		alGenSources(1, &ID);

		// Assign buffer to source
		alSourcei(ID, AL_BUFFER, Buffer->ID);
		alSourcef(ID, AL_MIN_GAIN, MinGain);
		alSourcef(ID, AL_MAX_GAIN, MaxGain);
		alSourcef(ID, AL_REFERENCE_DISTANCE, ReferenceDistance);
		alSourcef(ID, AL_ROLLOFF_FACTOR, RollOff);
		alSourcei(ID, AL_LOOPING, Loop);
		Loaded = true;
	}
}

// Free audio source
_AudioSource::~_AudioSource() {
	if(Loaded) {

		// Create source
		alDeleteSources(1, &ID);
		Loaded = false;
	}
}

// Play
void _AudioSource::Play() {
	if(Loaded) {

		// Get state
		ALint State;
		alGetSourcei(ID, AL_SOURCE_STATE, &State);

		// If already playing, stop
		if(State == AL_PLAYING)
			alSourceStop(ID);

		// Play sound
		alSourcePlay(ID);
	}
}

// Stop
void _AudioSource::Stop() {
	if(Loaded) {
		alSourceStop(ID);
	}
}

// Returns true if the source is playing
bool _AudioSource::IsPlaying() {
	ALenum State;

	alGetSourcei(ID, AL_SOURCE_STATE, &State);

	return State == AL_PLAYING;
}

// Set pitch
void _AudioSource::SetPitch(float Value) {
	if(Loaded) {
		alSourcef(ID, AL_PITCH, Value);
	}
}

// Set gain
void _AudioSource::SetGain(float Value) {
	if(Loaded) {
		alSourcef(ID, AL_GAIN, Value);
	}
}

// Set position
void _AudioSource::SetPosition(float X, float Y, float Z) {
	if(Loaded) {
		alSource3f(ID, AL_POSITION, X, Y, -Z);
	}
}
