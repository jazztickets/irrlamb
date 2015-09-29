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
#include <fader.h>
#include <audio.h>
#include <globals.h>
#include <constants.h>

using namespace irr;

_Fader Fader;

// Initializes the fader system
int _Fader::Init() {

	// Set state
	State = STATE_NONE;
	Fade = 0.0f;
	Speed = 0.0f;
	TimeStep = PHYSICS_TIMESTEP;
	TimeStepAccumulator = 0.0f;

	// Load resources
	FadeImage = irrDriver->getTexture("art/fade.png");

	return 1;
}

// Closes the fader system
int _Fader::Close() {

	return 1;
}

// Update fader
void _Fader::Update(float FrameTime) {

	// Update fading
	switch(State) {
		case STATE_NONE:
		break;
		case STATE_WAITFRAME:
			State = STATE_NONE;
		break;
		case STATE_FADING:

			TimeStepAccumulator += FrameTime;
			while(TimeStepAccumulator >= TimeStep) {
				TimeStepAccumulator -= TimeStep;

				// Update fade
				Fade += Speed * TimeStep;
				if(Fade <= 0.0f) {
					Fade = 0.0f;
					State = STATE_WAITFRAME;
				}
				else if(Fade >= 1.0f) {
					Fade = 1.0f;
					State = STATE_WAITFRAME;
				}

				// Change audio
				Audio.SetGain(Fade);
			}

		break;
	}
}

// Draw fader
void _Fader::Draw() {
	irrDriver->draw2DImage(FadeImage, core::position2di(0, 0), core::recti(0, 0, irrDriver->getScreenSize().Width, irrDriver->getScreenSize().Height), 0, video::SColor((uint32_t)((1.0f - Fade) * 255), 255, 255, 255), true);
}

// Starts fading the audio/screen
void _Fader::Start(float Speed) {

	this->Speed = Speed;
	State = STATE_FADING;
}

