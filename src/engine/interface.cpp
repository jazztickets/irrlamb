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
#include <engine/interface.h>
#include <engine/globals.h>
#include <engine/config.h>
#include <engine/log.h>
#include <engine/audio.h>
#include <engine/level.h>
#include <font/CGUITTFont.h>
#include <menu.h>

using namespace irr;

_Interface Interface;

const int MESSAGE_WIDTH = 475;
const int MESSAGE_HEIGHT = 120;
const int MESSAGE_PADDING = 20;
const int INTERFACE_LEVEL_DISPLAY_TIME = 5.0f;

struct _Font {
	const char *Path;
	int Type;
	int Size;
};

// List of fonts
const _Font DefaultFonts[] = {
	"fonts/Arimo-Regular.ttf", _Interface::FONT_SMALL, 16,
	"fonts/Arimo-Regular.ttf", _Interface::FONT_MEDIUM, 24,
	"fonts/RobotoCondensed-Regular.ttf", _Interface::FONT_LARGE, 48,
	"fonts/FjallaOne-Regular.ttf", _Interface::FONT_BUTTON, 24,
};

// Constructor for empty element
CGUIEmptyElement::CGUIEmptyElement(irr::gui::IGUIEnvironment *Environment, irr::gui::IGUIElement *Parent)
	:	irr::gui::IGUIElement((gui::EGUI_ELEMENT_TYPE)MGUIET_EMPTY, Environment, Parent, -1, core::recti(0, 0, irrDriver->getScreenSize().Width, irrDriver->getScreenSize().Height)) { }

// Initializes the graphics system
int _Interface::Init() {
	DrawHUD = true;

	// Get skin
	gui::IGUISkin *Skin = irrGUI->getSkin();

	// Load all fonts
	for(int i = 0; i < FONT_COUNT; i++) {

		// Load font
		Fonts[DefaultFonts[i].Type] = gui::CGUITTFont::createTTFont(irrGUI, DefaultFonts[i].Path, DefaultFonts[i].Size);
		if(!Fonts[DefaultFonts[i].Type]) {
			Log.Write("_Interface::Init - Unable to load font %s", DefaultFonts[i].Path);
			return 0;
		}
	}
	Skin->setFont(Fonts[FONT_MEDIUM]);

	// Load images
	Images[IMAGE_MOUSECURSOR] = irrDriver->getTexture("art/cursor.png");
	Images[IMAGE_FADE] = irrDriver->getTexture("art/fade.png");
	Images[IMAGE_BUTTON_SMALL] = irrDriver->getTexture("art/button_small.png");
	Images[IMAGE_BUTTON_MEDIUM] = irrDriver->getTexture("art/button_medium.png");
	Images[IMAGE_BUTTON_BIG] = irrDriver->getTexture("art/button_big.png");
	Images[IMAGE_TEXTBOXSHEET0] = irrDriver->getTexture("art/sheet_textbox0.png");
	Images[IMAGE_TEXTBOXSHEET1] = irrDriver->getTexture("art/sheet_textbox1.png");
	Images[IMAGE_TEXTBOXSHEET2] = irrDriver->getTexture("art/sheet_textbox2.png");
	Images[IMAGE_PAUSE] = irrDriver->getTexture("art/button_pause.png");
	Images[IMAGE_REWIND] = irrDriver->getTexture("art/button_rewind.png");
	Images[IMAGE_FASTFORWARD] = irrDriver->getTexture("art/button_ff.png");
	Images[IMAGE_FASTREVERSE] = irrDriver->getTexture("art/button_fr.png");
	Images[IMAGE_INCREASE] = irrDriver->getTexture("art/button_inc.png");
	Images[IMAGE_DECREASE] = irrDriver->getTexture("art/button_dec.png");

	// Set sounds to NULL
	for(int i = 0; i < SOUND_COUNT; i++)
		Sounds[i] = NULL;

	// Set up skins
	ChangeSkin(SKIN_MENU);
	Timer = 0.0f;

	CenterX = irrDriver->getScreenSize().Width / 2;
	CenterY = irrDriver->getScreenSize().Height / 2;

	TutorialText.MessageX = CenterX;
	TutorialText.MessageY = irrDriver->getScreenSize().Height - MESSAGE_HEIGHT / 2 - 30;

	Menu.ClearCurrentLayout();

	return 1;
}

// Closes the graphics system
int _Interface::Close() {

	for(int i = 0; i < FONT_COUNT; i++)
		Fonts[i]->drop();

	return 1;
}

// Changes irrlicht skins
void _Interface::ChangeSkin(SkinType Type) {

	gui::IGUISkin *Skin = irrGUI->getSkin();
	Skin->setColor(gui::EGDC_BUTTON_TEXT, video::SColor(255, 255, 255, 255));
	Skin->setColor(gui::EGDC_WINDOW, video::SColor(255, 0, 0, 20));
	Skin->setColor(gui::EGDC_WINDOW_SYMBOL, video::SColor(255, 255, 255, 255));
	Skin->setColor(gui::EGDC_GRAY_WINDOW_SYMBOL, video::SColor(255, 128, 128, 128));
	Skin->setColor(gui::EGDC_GRAY_EDITABLE, video::SColor(255, 0, 0, 0));
	Skin->setColor(gui::EGDC_FOCUSED_EDITABLE, video::SColor(255, 0, 0, 0));
	Skin->setColor(gui::EGDC_EDITABLE, video::SColor(255, 0, 0, 0));

	switch(Type) {
		case SKIN_MENU:
			Skin->setColor(gui::EGDC_3D_FACE, video::SColor(255, 32, 32, 32));
			Skin->setColor(gui::EGDC_3D_SHADOW, video::SColor(255, 0, 0, 16));

			Skin->setColor(gui::EGDC_3D_HIGH_LIGHT, video::SColor(255, 16, 16, 16));
			Skin->setColor(gui::EGDC_3D_DARK_SHADOW, video::SColor(255, 0, 0, 16));
		break;
		case SKIN_GAME:
			Skin->setColor(gui::EGDC_3D_FACE, video::SColor(0, 0, 0, 0));
			Skin->setColor(gui::EGDC_3D_SHADOW, video::SColor(0, 0, 0, 0));

			Skin->setColor(gui::EGDC_3D_HIGH_LIGHT, video::SColor(0, 0, 0, 0));
			Skin->setColor(gui::EGDC_3D_DARK_SHADOW, video::SColor(0, 0, 0, 0));
		break;
	}
}

// Clear all the GUI elements
void _Interface::Clear() {
	if(TutorialText.Text) {
		TutorialText.Text->remove();
		TutorialText.Text = NULL;
	}
	Timer = 0.0f;
}

// Adds tutorial text to the screen
void _Interface::SetTutorialText(const std::string &Text, float Length) {
	if(TutorialText.Text) {
		TutorialText.Text->remove();
		TutorialText.Text = NULL;
	}

	TutorialText.Text = irrGUI->addStaticText(core::stringw(Text.c_str()).c_str(), GetCenteredRect(TutorialText.MessageX, TutorialText.MessageY, MESSAGE_WIDTH - MESSAGE_PADDING, MESSAGE_HEIGHT), false, true);
	TutorialText.Text->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_CENTER);
	TutorialText.DeleteTime = Length + Timer;
}

// Maintains all of the timed elements
void _Interface::Update(float FrameTime) {

	Timer += FrameTime;
	if(TutorialText.Text) {
		if(Timer >= TutorialText.DeleteTime) {
			TutorialText.Text->remove();
			TutorialText.Text = NULL;
		}
	}
}

// Draw interface elements
void _Interface::Draw(float Time, bool FirstLoad) {

	// Draw timer
	char TimeString[32];
	ConvertSecondsToString(Time, TimeString);
	if(DrawHUD)
		RenderText(TimeString, 10, 10, _Interface::ALIGN_LEFT, _Interface::FONT_MEDIUM);

	// Draw level name and highscore
	if(FirstLoad && Time < INTERFACE_LEVEL_DISPLAY_TIME) {
		video::SColor LevelNameColor(255, 255, 255, 255);
		if(Time >= INTERFACE_LEVEL_DISPLAY_TIME - 1.0f)
			LevelNameColor.setAlpha((uint32_t)(255 * (INTERFACE_LEVEL_DISPLAY_TIME - Time)));

		RenderText(Level.GetLevelNiceName().c_str(), irrDriver->getScreenSize().Width - 25, 10, _Interface::ALIGN_RIGHT, _Interface::FONT_LARGE, LevelNameColor);
		if(Level.GetFastestTime() > 0.0f) {
			ConvertSecondsToString(Level.GetFastestTime(), TimeString, "Record: ");
			RenderText(TimeString, irrDriver->getScreenSize().Width - 25, 70, _Interface::ALIGN_RIGHT, _Interface::FONT_MEDIUM, LevelNameColor);
		}
	}

	// Draw tutorial text
	if(TutorialText.Text) {
		TutorialText.Text->setVisible(DrawHUD);

		// Get tutorial text alpha
		video::SColor TextColor(255, 255, 255, 255), BoxColor(160, 255, 255, 255);
		float TimeLeft = TutorialText.DeleteTime - Timer;
		if(TimeLeft < 2.0f) {
			TextColor.setAlpha((uint32_t)(255 * TimeLeft / 2.0));
			BoxColor.setAlpha((uint32_t)(160 * TimeLeft / 2.0));
		}

		// Update tutorial text color
		TutorialText.Text->setOverrideColor(TextColor);

		// Draw tutorial text
		if(DrawHUD)
			DrawTextBox(TutorialText.MessageX, TutorialText.MessageY, MESSAGE_WIDTH, MESSAGE_HEIGHT, BoxColor);
	}
}

// Converts milliseconds to a time string
void _Interface::ConvertSecondsToString(float Time, char *String, const char *Prefix) {
	uint32_t Minutes = (uint32_t)(Time) / 60;
	uint32_t Seconds = (uint32_t)(Time - Minutes * 60);
	uint32_t Centiseconds = (uint32_t)((Time - (uint32_t)(Time)) * 100);
	sprintf(String, "%s%.2d:%.2d:%.2d", Prefix, Minutes, Seconds, Centiseconds);
}

// Gets a rectangle centered around a point
core::recti _Interface::GetCenteredRect(int PositionX, int PositionY, int Width, int Height) {

	return core::recti(PositionX - (Width >> 1), PositionY - (Height >> 1), PositionX + (Width >> 1), PositionY + (Height >> 1));
}

// Gets a rectangle aligned right
core::recti _Interface::GetRightRect(int PositionX, int PositionY, int Width, int Height) {

	return core::recti(PositionX - Width, PositionY - (Height >> 1), PositionX, PositionY + (Height >> 1));
}

// Gets a rectangle
core::recti _Interface::GetRect(int PositionX, int PositionY, int Width, int Height) {

	return core::recti(PositionX, PositionY, PositionX + Width, PositionY + Height);
}

// Fades the screen
void _Interface::FadeScreen(float Amount) {
	irrDriver->draw2DImage(Images[IMAGE_FADE], core::position2di(0, 0), core::recti(0, 0, irrDriver->getScreenSize().Width, irrDriver->getScreenSize().Height), 0, video::SColor((uint32_t)(Amount * 255), 255, 255, 255), true);
	if(TutorialText.Text) {
		video::SColor TextColor = TutorialText.Text->getOverrideColor();
		TextColor.setAlpha((uint32_t)(TextColor.getAlpha() * (1.0f - Amount)));
		TutorialText.Text->setOverrideColor(TextColor);
	}
}

// Draws text to the screen
void _Interface::RenderText(const char *Text, int PositionX, int PositionY, AlignType AlignType, FontType FontType, const video::SColor &Color) {

	// Convert string
	core::stringw String(Text);

	// Get dimensions
	core::dimension2d<uint32_t> TextArea = Fonts[FontType]->getDimension(String.c_str());

	switch(AlignType) {
		case ALIGN_LEFT:
		break;
		case ALIGN_CENTER:
			PositionX -= TextArea.Width >> 1;
		break;
		case ALIGN_RIGHT:
			PositionX -= TextArea.Width;
		break;
	}

	// Draw text
	Fonts[FontType]->draw(String.c_str(), core::recti(PositionX, PositionY, PositionX + TextArea.Width, PositionY + TextArea.Height), Color);
}

// Draws an interface image centered around a position
void _Interface::DrawImage(ImageType Type, int PositionX, int PositionY, int Width, int Height, const video::SColor &Color) {

	irrDriver->draw2DImage(Images[Type], core::position2di(PositionX - (Width >> 1), PositionY - (Height >> 1)), core::recti(0, 0, Width, Height), 0, Color, true);
}

// Draws a text box
void _Interface::DrawTextBox(int PositionX, int PositionY, int Width, int Height, const video::SColor &Color) {
	PositionX -= Width >> 1;
	PositionY -= Height >> 1;

	// Draw corners
	irrDriver->draw2DImage(Images[IMAGE_TEXTBOXSHEET0], core::position2di(PositionX, PositionY), core::recti(0, 0, 10, 10), 0, Color, true);
	irrDriver->draw2DImage(Images[IMAGE_TEXTBOXSHEET0], core::position2di(PositionX + Width - 10, PositionY), core::recti(10, 0, 20, 10), 0, Color, true);
	irrDriver->draw2DImage(Images[IMAGE_TEXTBOXSHEET0], core::position2di(PositionX, PositionY + Height - 10), core::recti(0, 10, 10, 20), 0, Color, true);
	irrDriver->draw2DImage(Images[IMAGE_TEXTBOXSHEET0], core::position2di(PositionX + Width - 10, PositionY + Height - 10), core::recti(10, 10, 20, 20), 0, Color, true);

	// Draw middle
	irrDriver->draw2DImage(Images[IMAGE_FADE], core::position2di(PositionX + 10, PositionY + 10), core::recti(0, 0, Width - 20, Height - 20), 0, Color, true);

	// Draw edges
	irrDriver->draw2DImage(Images[IMAGE_TEXTBOXSHEET1], core::position2di(PositionX + 10, PositionY), core::recti(0, 0, Width - 20, 10), 0, Color, true);
	irrDriver->draw2DImage(Images[IMAGE_TEXTBOXSHEET1], core::position2di(PositionX + 10, PositionY + Height - 10), core::recti(0, 6, Width - 20, 16), 0, Color, true);
	irrDriver->draw2DImage(Images[IMAGE_TEXTBOXSHEET2], core::position2di(PositionX, PositionY + 10), core::recti(0, 0, 10, Height - 20), 0, Color, true);
	irrDriver->draw2DImage(Images[IMAGE_TEXTBOXSHEET2], core::position2di(PositionX + Width - 10, PositionY + 10), core::recti(6, 0, 16, Height - 20), 0, Color, true);
}

// Load GUI sounds
void _Interface::LoadSounds() {
	Sounds[SOUND_CONFIRM] = new _AudioSource(Audio.GetBuffer("confirm.ogg"), false, 0.0f, 0.70f);
}

// Unload GUI sounds
void _Interface::UnloadSounds() {

	for(int i = 0; i < SOUND_COUNT; i++) {
		delete Sounds[i];
		Sounds[i] = NULL;
	}
}

// Play a GUI sound
void _Interface::PlaySound(SoundType Sound) {
	if(Sounds[Sound])
		Sounds[Sound]->Play();
}
