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
#include <menu.h>
#include <graphics.h>
#include <input.h>
#include <constants.h>
#include <interface.h>
#include <globals.h>
#include <config.h>
#include <game.h>
#include <save.h>
#include <campaign.h>
#include <level.h>
#include <save.h>
#include <states/viewreplay.h>
#include <states/play.h>
#include <states/null.h>
#include <font/CGUITTFont.h>
#include <IGUIElement.h>
#include <IGUIComboBox.h>
#include <IGUICheckBox.h>
#include <IGUIEditBox.h>
#include <IFileSystem.h>

using namespace irr;

_Menu Menu;

const int REPLAY_LEVELID = 1100;
const int CAMPAIGN_LEVELID = 1000;
const int PLAY_CAMPAIGNID = 900;

const int TITLE_Y = 200;
const int TITLE_SPACING = 120;
const int BUTTON_SPACING = 70;
const int CAMPAIGN_SPACING = 60;
const int BACK_Y = 220;
const int SAVE_X = 60;

const int STATS_MIN_WIDTH = 250;
const int STATS_MIN_HEIGHT = 330;
const int STATS_PADDING = 15;
const int STATS_MOUSE_OFFSETX = 20;
const int STATS_MOUSE_OFFSETY = -105;

const int LOSE_WIDTH = 500;
const int LOSE_HEIGHT = 380;
const int WIN_WIDTH = 560;
const int WIN_HEIGHT = 380;

const int KeyMapOrder[] = {
	_Actions::MOVE_FORWARD,
	_Actions::MOVE_BACK,
	_Actions::MOVE_LEFT,
	_Actions::MOVE_RIGHT,
	_Actions::JUMP,
	_Actions::RESET
};

enum GUIElements {
	MAIN_SINGLEPLAYER, MAIN_REPLAYS, MAIN_OPTIONS, MAIN_QUIT,
	SINGLEPLAYER_BACK,
	LEVELS_GO, LEVELS_BUY, LEVELS_HIGHSCORES, LEVELS_BACK, LEVELS_SELECTEDLEVEL,
	LEVELINFO_DESCRIPTION, LEVELINFO_ATTEMPTS, LEVELINFO_WINS, LEVELINFO_LOSSES, LEVELINFO_PLAYTIME, LEVELINFO_BESTTIME,
	REPLAYS_VIEW, REPLAYS_DELETE, REPLAYS_BACK,
	OPTIONS_VIDEO, OPTIONS_AUDIO, OPTIONS_CONTROLS, OPTIONS_BACK,
	VIDEO_SAVE, VIDEO_CANCEL, VIDEO_VIDEOMODES, VIDEO_FULLSCREEN, VIDEO_SHADOWS, VIDEO_SHADERS, VIDEO_VSYNC, VIDEO_ANISOTROPY, VIDEO_ANTIALIASING,
	AUDIO_ENABLED, AUDIO_SAVE, AUDIO_CANCEL,
	CONTROLS_SAVE, CONTROLS_CANCEL, CONTROLS_INVERTMOUSE, CONTROLS_INVERTGAMEPADY, CONTROLS_KEYMAP,
	PAUSE_RESUME=(CONTROLS_KEYMAP + _Actions::COUNT), PAUSE_SAVEREPLAY, PAUSE_RESTART, PAUSE_OPTIONS, PAUSE_QUITLEVEL,
	SAVEREPLAY_NAME, SAVEREPLAY_SAVE, SAVEREPLAY_CANCEL,
	LOSE_RESTARTLEVEL, LOSE_SAVEREPLAY, LOSE_MAINMENU,
	WIN_RESTARTLEVEL, WIN_NEXTLEVEL, WIN_SAVEREPLAY, WIN_MAINMENU,
};

// Handle action inputs
bool _Menu::HandleAction(int InputType, int Action, float Value) {
	if(Input.HasJoystick())
		Input.DriveMouse(Action, Value);

	// On action press
	if(Value) {
		switch(Action) {
			case _Actions::MENU_PAUSE:
			case _Actions::MENU_BACK:
				switch(State) {
					case STATE_MAIN:
						Game.SetDone(true);
					break;
					case STATE_SINGLEPLAYER:
					case STATE_OPTIONS:
					case STATE_REPLAYS:
						if(Game.GetState() == &PlayState)
							InitPause();
						else
							InitMain();
					break;
					case STATE_LEVELS:
						InitSinglePlayer();
					break;
					case STATE_VIDEO:
					case STATE_AUDIO:
					case STATE_CONTROLS:
						InitOptions();
					break;
					case STATE_PAUSED:
						InitPlay();
					break;
					case STATE_LOSE:
					case STATE_WIN:
						NullState.State = STATE_LEVELS;
						Game.ChangeState(&NullState);
					break;
					case STATE_SAVEREPLAY:
						if(PreviousState == STATE_WIN)
							InitWin();
						else if(PreviousState == STATE_LOSE)
							InitLose();
						else
							InitPause();
					break;
					default:
					break;
				}

				return true;
			break;
			case _Actions::RESET:
				if(!Value)
					return false;

				if(State == STATE_LOSE || State == STATE_WIN) {
					PlayState.StartReset();
					return true;
				}
			break;
			default:
			break;
		}
	}

	return false;
}

// Key presses
bool _Menu::HandleKeyPress(int Key) {

	bool Processed = true;
	switch(State) {
		case STATE_MAIN:
			switch(Key) {
				case KEY_RETURN:
					InitSinglePlayer();
				break;
				default:
					Processed = false;
				break;
			}
		break;
		case STATE_SINGLEPLAYER:
			switch(Key) {
				case KEY_RETURN:
					InitLevels();
				break;
				default:
					Processed = false;
				break;
			}
		break;
		case STATE_CONTROLS:
			if(KeyButton != NULL) {
				core::stringw KeyName = Input.GetKeyName(Key);

				// Assign the key
				if(Key != KEY_ESCAPE && KeyName != "") {
					int ActionType = KeyMapOrder[KeyButton->getID() - CONTROLS_KEYMAP];

					// Swap if the key already exists
					for(int i = 0; i <= _Actions::RESET; i++) {
						if(CurrentKeys[KeyMapOrder[i]] == Key) {

							// Get button
							gui::IGUIButton *SwapButton = static_cast<gui::IGUIButton *>(CurrentLayout->getElementFromId(CONTROLS_KEYMAP + KeyMapOrder[i]));

							// Swap text
							SwapButton->setText(core::stringw(Input.GetKeyName(CurrentKeys[KeyMapOrder[ActionType]])).c_str());
							CurrentKeys[KeyMapOrder[i]] = CurrentKeys[KeyMapOrder[ActionType]];
							break;
						}
					}

					// Update key
					KeyButton->setText(KeyName.c_str());
					CurrentKeys[KeyMapOrder[ActionType]] = Key;
				}
				else
					KeyButton->setText(KeyButtonOldText.c_str());

				KeyButton = NULL;
			}
		break;
		case STATE_SAVEREPLAY:
			switch(Key) {
				case KEY_RETURN:
					Menu.SaveReplay();
				break;
				default:
					Processed = false;
				break;
			}
		break;
		default:
		break;
	}

	return Processed;
}

// Handles GUI events
void _Menu::HandleGUI(irr::gui::EGUI_EVENT_TYPE EventType, gui::IGUIElement *Element) {

	switch(EventType) {
		case gui::EGET_BUTTON_CLICKED:
			switch(Element->getID()) {
				case MAIN_SINGLEPLAYER:
					InitSinglePlayer();
				break;
				case MAIN_REPLAYS:
					InitReplays();
				break;
				case MAIN_OPTIONS:
					InitOptions();
				break;
				case MAIN_QUIT:
					Game.SetDone(true);
				break;
				case SINGLEPLAYER_BACK:
					InitMain();
				break;
				case LEVELS_GO:
					LaunchLevel();
				break;
				case LEVELS_BACK:
					InitSinglePlayer();
				break;
				case REPLAYS_VIEW:
					LaunchReplay();
				break;
				case REPLAYS_DELETE: {

					// Get list
					if(SelectedLevel != -1) {

						// Get replay file name
						std::string FileName = ReplayFiles[SelectedLevel].Filename;

						// Remove file
						std::string FilePath = Save.GetReplayPath() + FileName;
						remove(FilePath.c_str());

						// Refresh screen
						InitReplays();
					}
				}
				break;
				case REPLAYS_BACK:
					InitMain();
				break;
				case OPTIONS_VIDEO:
					InitVideo();
				break;
				case OPTIONS_AUDIO:
					InitAudio();
				break;
				case OPTIONS_CONTROLS:
					InitControls();
				break;
				case OPTIONS_BACK:
					if(Game.GetState() == &PlayState)
						InitPause();
					else
						InitMain();
				break;
				case VIDEO_SAVE: {

					// Save the video mode
					gui::IGUIComboBox *VideoModes = static_cast<gui::IGUIComboBox *>(CurrentLayout->getElementFromId(VIDEO_VIDEOMODES));
					if(VideoModes != NULL) {
						VideoModeStruct Mode = Graphics.GetVideoModes()[VideoModes->getSelected()];
						Config.ScreenWidth = Mode.Width;
						Config.ScreenHeight = Mode.Height;
					}

					// Save full screen
					gui::IGUICheckBox *Fullscreen = static_cast<gui::IGUICheckBox *>(CurrentLayout->getElementFromId(VIDEO_FULLSCREEN));
					Config.Fullscreen = Fullscreen->isChecked();

					// Save shadows
					gui::IGUICheckBox *Shadows = static_cast<gui::IGUICheckBox *>(CurrentLayout->getElementFromId(VIDEO_SHADOWS));
					Config.Shadows = Shadows->isChecked();

					// Save the anisotropy
					gui::IGUIComboBox *Anisotropy = static_cast<gui::IGUIComboBox *>(CurrentLayout->getElementFromId(VIDEO_ANISOTROPY));
					if(Anisotropy != NULL) {
						if(Anisotropy->getSelected() == 0)
							Config.AnisotropicFiltering = 0;
						else
							Config.AnisotropicFiltering = 1 << (Anisotropy->getSelected() - 1);
					}

					// Save the antialiasing
					gui::IGUIComboBox *Antialiasing = static_cast<gui::IGUIComboBox *>(CurrentLayout->getElementFromId(VIDEO_ANTIALIASING));
					if(Antialiasing != NULL) {
						if(Antialiasing->getSelected() == 0)
							Config.AntiAliasing = 0;
						else
							Config.AntiAliasing = 1 << (Antialiasing->getSelected());
					}

					// Save shaders
					gui::IGUICheckBox *Shaders = static_cast<gui::IGUICheckBox *>(CurrentLayout->getElementFromId(VIDEO_SHADERS));
					Config.Shaders = Shaders->isChecked();
					if(Config.Shaders)
						Graphics.LoadShaders();

					// Save vsync
					gui::IGUICheckBox *Vsync = static_cast<gui::IGUICheckBox *>(CurrentLayout->getElementFromId(VIDEO_VSYNC));
					Config.Vsync = Vsync->isChecked();

					// Write config
					Config.WriteConfig();

					InitOptions();
				}
				break;
				case VIDEO_CANCEL:
					InitOptions();
				break;
				case AUDIO_SAVE: {
					bool OldAudioEnabled = Config.AudioEnabled;

					// Get settings
					gui::IGUICheckBox *AudioEnabled = static_cast<gui::IGUICheckBox *>(CurrentLayout->getElementFromId(AUDIO_ENABLED));
					bool Enabled = AudioEnabled->isChecked();

					// Save
					Config.AudioEnabled = Enabled;
					Config.WriteConfig();

					// Init or disable audio system
					if(OldAudioEnabled != Enabled) {
						if(Enabled) {
							Game.EnableAudio();
						}
						else
							Game.DisableAudio();
					}

					InitOptions();
				}
				break;
				case AUDIO_CANCEL:
					InitOptions();
				break;
				case CONTROLS_SAVE: {

					// Write config

					for(int i = 0; i <= _Actions::RESET; i++) {
						Actions.ClearMappingsForAction(_Input::KEYBOARD, KeyMapOrder[i]);
						Actions.AddInputMap(_Input::KEYBOARD, CurrentKeys[KeyMapOrder[i]], KeyMapOrder[i], 1.0f, -1.0f, false);
					}

					// Save invert mouse
					gui::IGUICheckBox *InvertMouse = static_cast<gui::IGUICheckBox *>(CurrentLayout->getElementFromId(CONTROLS_INVERTMOUSE));
					Config.InvertMouse = InvertMouse->isChecked();

					// Save invert gamepad Y
					gui::IGUICheckBox *InvertGamepadY = static_cast<gui::IGUICheckBox *>(CurrentLayout->getElementFromId(CONTROLS_INVERTGAMEPADY));
					Config.InvertGamepadY = InvertGamepadY->isChecked();

					Config.WriteConfig();

					InitOptions();
				}
				break;
				case CONTROLS_CANCEL:
					InitOptions();
				break;
				default: {

					if(Element->getID() >= CONTROLS_KEYMAP && Element->getID() < CONTROLS_KEYMAP + _Actions::COUNT) {
						if(KeyButton)
							CancelKeyBind();

						KeyButton = static_cast<gui::IGUIButton *>(Element);
						KeyButtonOldText = KeyButton->getText();
						KeyButton->setText(L"");
					}
					else if(Element->getID() >= REPLAY_LEVELID) {
						SelectedLevel = Element->getID() - REPLAY_LEVELID;
						if(SelectedElement == Element)
							LaunchReplay();

						SelectedElement = Element;
					}
					else if(Element->getID() >= CAMPAIGN_LEVELID) {
						SelectedLevel = Element->getID() - CAMPAIGN_LEVELID;
						LaunchLevel();
					}
					else if(Element->getID() >= PLAY_CAMPAIGNID) {
						CampaignIndex = Element->getID() - PLAY_CAMPAIGNID;
						InitLevels();
					}
				}
				break;
				case PAUSE_RESUME:
					InitPlay();
				break;
				case PAUSE_SAVEREPLAY:
					InitSaveReplay();
				break;
				case PAUSE_OPTIONS:
					InitOptions();
				break;
				case PAUSE_RESTART:
					PlayState.StartReset();
				break;
				case PAUSE_QUITLEVEL:
					NullState.State = STATE_LEVELS;
					Game.ChangeState(&NullState);
				break;
				case SAVEREPLAY_SAVE:
					SaveReplay();
				break;
				case SAVEREPLAY_CANCEL:
					if(PreviousState == STATE_WIN)
						InitWin();
					else if(PreviousState == STATE_LOSE)
						InitLose();
					else
						InitPause();
				break;
				case LOSE_RESTARTLEVEL:
				case WIN_RESTARTLEVEL:
					PlayState.StartReset();
				break;
				case WIN_NEXTLEVEL:
					Campaign.GetNextLevel(PlayState.CurrentCampaign, PlayState.CampaignLevel, true);
					Game.ChangeState(&PlayState);
					CampaignIndex = PlayState.CurrentCampaign;
				break;
				case LOSE_SAVEREPLAY:
				case WIN_SAVEREPLAY:
					InitSaveReplay();
				break;
				case LOSE_MAINMENU:
				case WIN_MAINMENU:
					NullState.State = STATE_LEVELS;
					Game.ChangeState(&NullState);
				break;
			}
		break;
		case gui::EGET_ELEMENT_HOVERED:
			if(Element->getID() >= REPLAY_LEVELID)
				HighlightedLevel = Element->getID() - REPLAY_LEVELID;
			else if(Element->getID() >= CAMPAIGN_LEVELID)
				HighlightedLevel = Element->getID() - CAMPAIGN_LEVELID;
		break;
		case gui::EGET_ELEMENT_LEFT:
			if(State == STATE_LEVELS || State == STATE_REPLAYS) {
				HighlightedLevel = -1;
			}
		break;
		default:
		break;
	}
}

// Create the main menu
void _Menu::InitMain() {
	Interface.ChangeSkin(_Interface::SKIN_MENU);
	Input.SetMouseLocked(false);
	ClearCurrentLayout();

	// Logo
	irrGUI->addImage(irrDriver->getTexture("art/logo.jpg"), core::position2di(Interface.GetCenterX() - 256, Interface.GetCenterY() - 270), true, CurrentLayout);
	AddMenuText(core::position2di(40, irrDriver->getScreenSize().Height - 20), core::stringw(GAME_VERSION).c_str(), _Interface::FONT_SMALL);

	// Button
	int Y = Interface.GetCenterY() - TITLE_Y + TITLE_SPACING;
	AddMenuButton(Interface.GetCenteredRect(Interface.GetCenterX(), Y + BUTTON_SPACING * 0, 194, 52), MAIN_SINGLEPLAYER, L"Single Player", _Interface::IMAGE_BUTTON_BIG);
	AddMenuButton(Interface.GetCenteredRect(Interface.GetCenterX(), Y + BUTTON_SPACING * 1, 194, 52), MAIN_REPLAYS, L"Replays", _Interface::IMAGE_BUTTON_BIG);
	AddMenuButton(Interface.GetCenteredRect(Interface.GetCenterX(), Y + BUTTON_SPACING * 2, 194, 52), MAIN_OPTIONS, L"Options", _Interface::IMAGE_BUTTON_BIG);
	AddMenuButton(Interface.GetCenteredRect(Interface.GetCenterX(), Y + BUTTON_SPACING * 3, 194, 52), MAIN_QUIT, L"Quit", _Interface::IMAGE_BUTTON_BIG);

	// Play sound
	if(!FirstStateLoad)
		Interface.PlaySound(_Interface::SOUND_CONFIRM);
	FirstStateLoad = false;

	PreviousState = State;
	State = STATE_MAIN;
}

// Create the single player menu
void _Menu::InitSinglePlayer() {
	ClearCurrentLayout();

	// Reset menu variables
	CampaignIndex = 0;
	SelectedLevel = -1;
	HighlightedLevel = -1;

	// Text
	int X = Interface.GetCenterX(), Y = Interface.GetCenterY() - TITLE_Y;
	AddMenuText(core::position2di(X, Y), L"Level Sets");

	// Campaigns
	Y += TITLE_SPACING - 30;
	const std::vector<CampaignStruct> &Campaigns = Campaign.GetCampaigns();
	for(uint32_t i = 0; i < Campaigns.size(); i++) {
		irr::core::stringw Name(Campaigns[i].Name.c_str());
		AddMenuButton(Interface.GetCenteredRect(X, Y, 194, 52), PLAY_CAMPAIGNID + i, Name.c_str());

		Y += CAMPAIGN_SPACING;
	}

	Y = Interface.GetCenterY() + BACK_Y;
	AddMenuButton(Interface.GetCenteredRect(X, Y, 108, 44), SINGLEPLAYER_BACK, L"Back", _Interface::IMAGE_BUTTON_SMALL);

	// Play sound
	Interface.PlaySound(_Interface::SOUND_CONFIRM);

	PreviousState = State;
	State = STATE_SINGLEPLAYER;
}

// Create the levels menu
void _Menu::InitLevels() {
	ClearCurrentLayout();
	LevelStats.clear();
	SelectedLevel = -1;
	HighlightedLevel = -1;

	const CampaignStruct &CampaignData = Campaign.GetCampaign(CampaignIndex);
	int X = Interface.GetCenterX(), Y = Interface.GetCenterY() - TITLE_Y;

	// Text
	AddMenuText(core::position2di(X, Y), core::stringw(CampaignData.Name.c_str()).c_str());

	// Add level list
	X = Interface.GetCenterX() - 160;
	Y += TITLE_SPACING;
	int Column = 0, Row = 0;
	for(uint32_t i = 0; i < CampaignData.Levels.size(); i++) {
		bool Unlocked = true;

		// Get level stats
		const SaveLevelStruct *Stats = Save.GetLevelStats(CampaignData.Levels[i].File);
		LevelStats.push_back(Stats);

		// Set unlocked status
		if(Stats->Unlocked == 0) {
			Unlocked = false;

			// Unlock the level if it's always unlocked in the campaign
			if((Input.GetKeyState(KEY_F1) && Input.GetKeyState(KEY_F10)) || CampaignData.Levels[i].Unlocked) {
				Save.UnlockLevel(CampaignData.Levels[i].File);
				Unlocked = true;
			}
		}

		// Add button
		gui::IGUIButton *Level = irrGUI->addButton(Interface.GetCenteredRect(X + Column * 80, Y + Row * 80, 64, 64), CurrentLayout, CAMPAIGN_LEVELID + i);

		// Set thumbnail
		if(Unlocked)
			Level->setImage(irrDriver->getTexture((CampaignData.Levels[i].DataPath + "icon.jpg").c_str()));
		else
			Level->setImage(irrDriver->getTexture("art/locked.png"));

		Column++;
		if(Column >= 5) {
			Column = 0;
			Row++;
		}
	}

	// Buttons
	X = Interface.GetCenterX();
	Y = Interface.GetCenterY() + BACK_Y;
	AddMenuButton(Interface.GetCenteredRect(X, Y, 108, 44), LEVELS_BACK, L"Back", _Interface::IMAGE_BUTTON_SMALL);

	// Play sound
	if(!FirstStateLoad)
		Interface.PlaySound(_Interface::SOUND_CONFIRM);
	FirstStateLoad = false;

	PreviousState = State;
	State = STATE_LEVELS;
}

// Create the replay menu
void _Menu::InitReplays() {
	Interface.ChangeSkin(_Interface::SKIN_MENU);
	ClearCurrentLayout();

	SelectedLevel = -1;
	HighlightedLevel = -1;

	// Text
	int X = Interface.GetCenterX(), Y = Interface.GetCenterY() - TITLE_Y;
	AddMenuText(core::position2di(X, Y), L"Replays");

	// Change directories
	std::string OldWorkingDirectory(irrFile->getWorkingDirectory().c_str());
	irrFile->changeWorkingDirectoryTo(Save.GetReplayPath().c_str());

	// Clear list
	ReplayFiles.clear();

	X = Interface.GetCenterX() - 160;
	Y += TITLE_SPACING;
	int Column = 0, Row = 0;

	// Get a list of replays
	io::IFileList *FileList = irrFile->createFileList();
	uint32_t FileCount = FileList->getFileCount();
	uint32_t ReplayIndex = 0;
	for(uint32_t i = 0; i < FileCount; i++) {
		if(!FileList->isDirectory(i) && FileList->getFileName(i).find(".replay") != -1) {

			// Load header
			bool Loaded = Replay.LoadReplay(FileList->getFileName(i).c_str(), true);
			if(Loaded && Replay.GetVersion() == REPLAY_VERSION) {
				char Buffer[256];

				// Get level info
				Level.Init(Replay.GetLevelName(), true);

				// Get replay info
				_ReplayInfo ReplayInfo;
				ReplayInfo.Filename = FileList->getFileName(i).c_str();
				ReplayInfo.Description = Replay.GetDescription();
				ReplayInfo.LevelNiceName = Level.GetLevelNiceName();
				ReplayInfo.Autosave = Replay.GetAutosave();

				// Date
				strftime(Buffer, 32, "%Y-%m-%d", localtime(&Replay.GetTimeStamp()));
				ReplayInfo.Date = Buffer;

				// Get time string
				Interface.ConvertSecondsToString(Replay.GetFinishTime(), Buffer);
				ReplayInfo.FinishTime = Buffer;

				ReplayFiles.push_back(ReplayInfo);

				// Add button
				gui::IGUIButton *Level = irrGUI->addButton(Interface.GetCenteredRect(X + Column * 80, Y + Row * 80, 64, 64), CurrentLayout, REPLAY_LEVELID + ReplayIndex);

				// Set thumbnail
				Level->setImage(irrDriver->getTexture((Game.GetWorkingPath() + "levels/" + Replay.GetLevelName() + "/icon.jpg").c_str()));

				Column++;
				if(Column >= 5) {
					Column = 0;
					Row++;
				}

				ReplayIndex++;
			}
		}
	}
	FileList->drop();
	irrFile->changeWorkingDirectoryTo(OldWorkingDirectory.c_str());

	// Confirmations
	X = Interface.GetCenterX();
	Y = Interface.GetCenterY() + BACK_Y;
	AddMenuButton(Interface.GetCenteredRect(X - 123, Y, 108, 44), REPLAYS_VIEW, L"View", _Interface::IMAGE_BUTTON_SMALL);
	AddMenuButton(Interface.GetCenteredRect(X, Y, 108, 44), REPLAYS_DELETE, L"Delete", _Interface::IMAGE_BUTTON_SMALL);
	AddMenuButton(Interface.GetCenteredRect(X + 123, Y, 108, 44), REPLAYS_BACK, L"Back", _Interface::IMAGE_BUTTON_SMALL);

	// Play sound
	if(!FirstStateLoad)
		Interface.PlaySound(_Interface::SOUND_CONFIRM);
	FirstStateLoad = false;

	PreviousState = State;
	State = STATE_REPLAYS;
}

// Create the options menu
void _Menu::InitOptions() {
	Interface.ChangeSkin(_Interface::SKIN_MENU);
	ClearCurrentLayout();

	// Text
	int X = Interface.GetCenterX(), Y = Interface.GetCenterY() - TITLE_Y;
	AddMenuText(core::position2di(X, Y), L"Options");

	Y += TITLE_SPACING;

	// Buttons
	AddMenuButton(Interface.GetCenteredRect(Interface.GetCenterX(), Y + 0 * BUTTON_SPACING, 194, 52), OPTIONS_VIDEO, L"Video");
	AddMenuButton(Interface.GetCenteredRect(Interface.GetCenterX(), Y + 1 * BUTTON_SPACING, 194, 52), OPTIONS_AUDIO, L"Audio");
	AddMenuButton(Interface.GetCenteredRect(Interface.GetCenterX(), Y + 2 * BUTTON_SPACING, 194, 52), OPTIONS_CONTROLS, L"Controls");
	AddMenuButton(Interface.GetCenteredRect(Interface.GetCenterX(), Interface.GetCenterY() + BACK_Y, 108, 44), OPTIONS_BACK, L"Back", _Interface::IMAGE_BUTTON_SMALL);

	// Play sound
	Interface.PlaySound(_Interface::SOUND_CONFIRM);

	PreviousState = State;
	State = STATE_OPTIONS;
}

// Create the video options menu
void _Menu::InitVideo() {
	ClearCurrentLayout();

	// Text
	int X = Interface.GetCenterX(), Y = Interface.GetCenterY() - TITLE_Y;
	AddMenuText(core::position2di(X, Y), L"Video");

	// Video modes
	Y += TITLE_SPACING - 40;
	const std::vector<VideoModeStruct> &ModeList = Graphics.GetVideoModes();
	if(ModeList.size() > 0) {
		AddMenuText(core::position2di(X, Y), L"Screen Resolution", _Interface::FONT_MEDIUM, -1, gui::EGUIA_LOWERRIGHT);
		gui::IGUIComboBox *ListScreenResolution = irrGUI->addComboBox(Interface.GetCenteredRect(X + 111, Y, 200, 30), CurrentLayout, VIDEO_VIDEOMODES);

		// Populate mode list
		for(uint32_t i = 0; i < ModeList.size(); i++)
			ListScreenResolution->addItem(ModeList[i].String.c_str());
		ListScreenResolution->setSelected(Graphics.GetCurrentVideoModeIndex());
	}

	// Full Screen
	Y += 40;
	AddMenuText(core::position2di(X, Y), L"Fullscreen", _Interface::FONT_MEDIUM, -1, gui::EGUIA_LOWERRIGHT);
	irrGUI->addCheckBox(Config.Fullscreen, Interface.GetCenteredRect(X + 20, Y, 18, 18), CurrentLayout, VIDEO_FULLSCREEN);

	// Shadows
	Y += 40;
	AddMenuText(core::position2di(X, Y), L"Shadows", _Interface::FONT_MEDIUM, -1, gui::EGUIA_LOWERRIGHT);
	irrGUI->addCheckBox(Config.Shadows, Interface.GetCenteredRect(X + 20, Y, 18, 18), CurrentLayout, VIDEO_SHADOWS);

	// Shaders
	Y += 40;
	AddMenuText(core::position2di(X, Y), L"Shaders", _Interface::FONT_MEDIUM, -1, gui::EGUIA_LOWERRIGHT);
	gui::IGUICheckBox *CheckBoxShaders = irrGUI->addCheckBox(Config.Shaders, Interface.GetCenteredRect(X + 20, Y, 18, 18), CurrentLayout, VIDEO_SHADERS);
	if(!Graphics.GetShadersSupported())
		CheckBoxShaders->setEnabled(false);

	// Vsync
	Y += 40;
	AddMenuText(core::position2di(X, Y), L"V-sync", _Interface::FONT_MEDIUM, -1, gui::EGUIA_LOWERRIGHT);
	irrGUI->addCheckBox(Config.Vsync, Interface.GetCenteredRect(X + 20, Y, 18, 18), CurrentLayout, VIDEO_VSYNC);

	// Anisotropic Filtering
	Y += 40;
	int MaxAnisotropy = irrDriver->getDriverAttributes().getAttributeAsInt("MaxAnisotropy");
	AddMenuText(core::position2di(X, Y), L"Anisotropic Filtering", _Interface::FONT_MEDIUM, -1, gui::EGUIA_LOWERRIGHT);
	gui::IGUIComboBox *Anisotropy = irrGUI->addComboBox(Interface.GetCenteredRect(X + 61, Y, 100, 30), CurrentLayout, VIDEO_ANISOTROPY);

	// Populate anisotropy list
	Anisotropy->addItem(core::stringw(0).c_str());
	for(int i = 0, Level = 1; Level <= MaxAnisotropy; i++, Level <<= 1) {
		Anisotropy->addItem(core::stringw(Level).c_str());
		if(Config.AnisotropicFiltering == Level)
			Anisotropy->setSelected(i+1);
	}

	// Anti-aliasing
	Y += 40;
	AddMenuText(core::position2di(X, Y), L"MSAA", _Interface::FONT_MEDIUM, -1, gui::EGUIA_LOWERRIGHT);
	gui::IGUIComboBox *Antialiasing = irrGUI->addComboBox(Interface.GetCenteredRect(X + 61, Y, 100, 30), CurrentLayout, VIDEO_ANTIALIASING);

	// Populate anti-aliasing list
	Antialiasing->addItem(core::stringw(0).c_str());
	for(int i = 0, Level = 2; Level <= 8; i++, Level <<= 1) {
		Antialiasing->addItem(core::stringw(Level).c_str());
		if(Config.AntiAliasing == Level)
			Antialiasing->setSelected(i+1);
	}

	// Save
	Y = Interface.GetCenterY() + BACK_Y;
	AddMenuButton(Interface.GetCenteredRect(X - SAVE_X, Y, 108, 44), VIDEO_SAVE, L"Save", _Interface::IMAGE_BUTTON_SMALL);
	AddMenuButton(Interface.GetCenteredRect(X + SAVE_X, Y, 108, 44), VIDEO_CANCEL, L"Cancel", _Interface::IMAGE_BUTTON_SMALL);

	// Warning
	Y = Interface.GetCenterY() + BACK_Y - 50;
	AddMenuText(core::position2di(X, Y), L"Changes are applied after restart", _Interface::FONT_SMALL, -1);

	// Play sound
	Interface.PlaySound(_Interface::SOUND_CONFIRM);

	PreviousState = State;
	State = STATE_VIDEO;
}

// Create the audio options menu
void _Menu::InitAudio() {
	ClearCurrentLayout();

	// Text
	int X = Interface.GetCenterX(), Y = Interface.GetCenterY() - TITLE_Y;
	AddMenuText(core::position2di(X, Y), L"Audio");

	// Sound enabled
	Y += TITLE_SPACING;
	AddMenuText(core::position2di(X, Y), L"Audio Enabled", _Interface::FONT_MEDIUM, -1, gui::EGUIA_LOWERRIGHT);
	irrGUI->addCheckBox(Config.AudioEnabled, Interface.GetCenteredRect(X + 20, Y, 18, 18), CurrentLayout, AUDIO_ENABLED);

	// Save
	Y += 90;
	AddMenuButton(Interface.GetCenteredRect(X - SAVE_X, Interface.GetCenterY() + BACK_Y, 108, 44), AUDIO_SAVE, L"Save", _Interface::IMAGE_BUTTON_SMALL);
	AddMenuButton(Interface.GetCenteredRect(X + SAVE_X, Interface.GetCenterY() + BACK_Y, 108, 44), AUDIO_CANCEL, L"Cancel", _Interface::IMAGE_BUTTON_SMALL);

	// Play sound
	Interface.PlaySound(_Interface::SOUND_CONFIRM);

	PreviousState = State;
	State = STATE_AUDIO;
}

// Create the control options menu
void _Menu::InitControls() {
	ClearCurrentLayout();

	int X = Interface.GetCenterX();
	int Y = Interface.GetCenterY() - TITLE_Y;

	// Text
	AddMenuText(core::position2di(X, Y), L"Controls");

	// Create the key buttons
	Y = Interface.GetCenterY() - TITLE_Y + TITLE_SPACING - 40;
	KeyButton = NULL;
	for(int i = 0; i <= _Actions::RESET; i++) {

		CurrentKeys[KeyMapOrder[i]] = Actions.GetInputForAction(_Input::KEYBOARD, KeyMapOrder[i]);
		AddMenuText(core::position2di(X - 25, Y), core::stringw(Actions.GetName(KeyMapOrder[i]).c_str()).c_str(), _Interface::FONT_MEDIUM, -1, gui::EGUIA_LOWERRIGHT);
		AddMenuButton(Interface.GetCenteredRect(X + 50, Y, 130, 44), CONTROLS_KEYMAP + KeyMapOrder[i], core::stringw(Input.GetKeyName(CurrentKeys[KeyMapOrder[i]])).c_str(), _Interface::IMAGE_BUTTON_MEDIUM);

		Y += 45;
	}

	// Invert mouse
	Y += 15;
	AddMenuText(core::position2di(X - 15, Y), L"Invert Mouse Y", _Interface::FONT_MEDIUM, -1, gui::EGUIA_LOWERRIGHT);
	irrGUI->addCheckBox(Config.InvertMouse, Interface.GetCenteredRect(X + 15, Y, 100, 25), CurrentLayout, CONTROLS_INVERTMOUSE);

	// Invert Gamepad Y
	AddMenuText(core::position2di(X - 15 + 145, Y), L"Gamepad Y", _Interface::FONT_MEDIUM, -1, gui::EGUIA_LOWERRIGHT);
	irrGUI->addCheckBox(Config.InvertGamepadY, Interface.GetCenteredRect(X + 15 + 170, Y, 100, 25), CurrentLayout, CONTROLS_INVERTGAMEPADY);

	// Save
	AddMenuButton(Interface.GetCenteredRect(Interface.GetCenterX() - SAVE_X, Interface.GetCenterY() + BACK_Y, 108, 44), CONTROLS_SAVE, L"Save", _Interface::IMAGE_BUTTON_SMALL);
	AddMenuButton(Interface.GetCenteredRect(Interface.GetCenterX() + SAVE_X, Interface.GetCenterY() + BACK_Y, 108, 44), CONTROLS_CANCEL, L"Cancel", _Interface::IMAGE_BUTTON_SMALL);

	// Play sound
	Interface.PlaySound(_Interface::SOUND_CONFIRM);

	PreviousState = State;
	State = STATE_CONTROLS;
}

// Init play GUI
void _Menu::InitPlay() {
	Interface.ChangeSkin(_Interface::SKIN_GAME);
	ClearCurrentLayout();

	Graphics.SetClearColor(Level.GetClearColor());
	Input.SetMouseLocked(true);
	LoseMessage = "You died!";

	PreviousState = State;
	State = STATE_NONE;
}

// Create the pause menu
void _Menu::InitPause() {
	ClearCurrentLayout();

	int X = Interface.GetCenterX();
	int Y = Interface.GetCenterY() - 125;

	AddMenuButton(Interface.GetCenteredRect(X, Y + 0 * BUTTON_SPACING, 194, 52), PAUSE_RESUME, L"Resume");
	AddMenuButton(Interface.GetCenteredRect(X, Y + 1 * BUTTON_SPACING, 194, 52), PAUSE_SAVEREPLAY, L"Save Replay");
	AddMenuButton(Interface.GetCenteredRect(X, Y + 2 * BUTTON_SPACING, 194, 52), PAUSE_RESTART, L"Restart Level");
	AddMenuButton(Interface.GetCenteredRect(X, Y + 3 * BUTTON_SPACING, 194, 52), PAUSE_OPTIONS, L"Options");
	AddMenuButton(Interface.GetCenteredRect(X, Y + 4 * BUTTON_SPACING, 194, 52), PAUSE_QUITLEVEL, L"Quit Level");

	Input.SetMouseLocked(false);

	PreviousState = State;
	State = STATE_PAUSED;
}

// Create the save replay GUI
void _Menu::InitSaveReplay() {
	Interface.ChangeSkin(_Interface::SKIN_MENU);
	ClearCurrentLayout();

	// Draw interface
	int X = Interface.GetCenterX();
	int Y = Interface.GetCenterY() - 50;

	AddMenuText(core::position2di(X, Y), L"Enter a name", _Interface::FONT_MEDIUM);
	Y += 40;
	gui::IGUIEditBox *EditName = irrGUI->addEditBox(L"", Interface.GetCenteredRect(X, Y, 230, 32), true, CurrentLayout, SAVEREPLAY_NAME);
	Y += 80;
	AddMenuButton(Interface.GetCenteredRect(X - SAVE_X, Y, 108, 44), SAVEREPLAY_SAVE, L"Save", _Interface::IMAGE_BUTTON_SMALL);
	AddMenuButton(Interface.GetCenteredRect(X + SAVE_X, Y, 108, 44), SAVEREPLAY_CANCEL, L"Cancel", _Interface::IMAGE_BUTTON_SMALL);

	irrGUI->setFocus(EditName);
	EditName->setMax(32);

	PreviousState = State;
	State = STATE_SAVEREPLAY;
}

// Create the lose screen
void _Menu::InitLose() {
	Interface.Clear();

	// Clear interface
	ClearCurrentLayout();

	int X = Interface.GetCenterX() - LOSE_WIDTH / 2 + 128/2;
	int Y = Interface.GetCenterY() + LOSE_HEIGHT / 2 + 40;

	int Spacing = LOSE_WIDTH/3 + 3;
	gui::IGUIButton *Button = AddMenuButton(Interface.GetCenteredRect(X + Spacing * 0, Y, 130, 44), LOSE_RESTARTLEVEL, L"Retry Level", _Interface::IMAGE_BUTTON_MEDIUM);
	AddMenuButton(Interface.GetCenteredRect(X + Spacing * 1, Y, 130, 44), LOSE_SAVEREPLAY, L"Save Replay", _Interface::IMAGE_BUTTON_MEDIUM);
	AddMenuButton(Interface.GetCenteredRect(X + Spacing * 2, Y, 130, 44), LOSE_MAINMENU, L"Main Menu", _Interface::IMAGE_BUTTON_MEDIUM);

	Input.SetMouseLocked(false);
	core::vector2di Position = Button->getAbsolutePosition().getCenter();
	irrDevice->getCursorControl()->setPosition(Position.X, Position.Y);

	PreviousState = State;
	State = STATE_LOSE;
}

// Create the win screen
void _Menu::InitWin() {
	Interface.Clear();

	// Get level stats
	WinStats = Save.GetLevelStats(Level.GetLevelName());

	// Clear interface
	ClearCurrentLayout();

	int X = Interface.GetCenterX() - WIN_WIDTH / 2 + 128/2;
	int Y = Interface.GetCenterY() + WIN_HEIGHT / 2 + 40;

	int Spacing = WIN_WIDTH/4 + 3;
	AddMenuButton(Interface.GetCenteredRect(X + Spacing * 0, Y, 130, 44), WIN_RESTARTLEVEL, L"Retry Level", _Interface::IMAGE_BUTTON_MEDIUM);
	gui::IGUIButton *ButtonNextLevel = AddMenuButton(Interface.GetCenteredRect(X + Spacing * 1, Y, 130, 44), WIN_NEXTLEVEL, L"Next Level", _Interface::IMAGE_BUTTON_MEDIUM);
	AddMenuButton(Interface.GetCenteredRect(X + Spacing * 2, Y, 130, 44), WIN_SAVEREPLAY, L"Save Replay", _Interface::IMAGE_BUTTON_MEDIUM);
	AddMenuButton(Interface.GetCenteredRect(X + Spacing * 3, Y, 130, 44), WIN_MAINMENU, L"Main Menu", _Interface::IMAGE_BUTTON_MEDIUM);

	if(!Campaign.GetNextLevel(PlayState.CurrentCampaign, PlayState.CampaignLevel, false))
		ButtonNextLevel->setEnabled(false);

	Input.SetMouseLocked(false);
	core::vector2di Position = ButtonNextLevel->getAbsolutePosition().getCenter();
	irrDevice->getCursorControl()->setPosition(Position.X, Position.Y);

	PreviousState = State;
	State = STATE_WIN;
}

// Saves a replay
void _Menu::SaveReplay() {

	gui::IGUIEditBox *EditName = static_cast<gui::IGUIEditBox *>(CurrentLayout->getElementFromId(SAVEREPLAY_NAME));
	if(EditName != NULL) {
		irr::core::stringc ReplayTitle(EditName->getText());
		Replay.SaveReplay(ReplayTitle.c_str());
	}

	switch(PreviousState) {
		case STATE_WIN: {
			InitWin();

			gui::IGUIButton *ButtonSaveReplay = static_cast<gui::IGUIButton *>(CurrentLayout->getElementFromId(WIN_SAVEREPLAY));
			ButtonSaveReplay->setEnabled(false);
		}
		break;
		case STATE_LOSE: {
			InitLose();

			gui::IGUIButton *ButtonSaveReplay = static_cast<gui::IGUIButton *>(CurrentLayout->getElementFromId(LOSE_SAVEREPLAY));
			ButtonSaveReplay->setEnabled(false);
		}
		break;
		default:
			InitPause();
		break;
	}
}

// Updates the current state
void _Menu::Update(float FrameTime) {
}

// Draws the current state
void _Menu::Draw() {
	CurrentLayout->draw();
	irrGUI->drawAll();

	// Draw level tooltip
	switch(State) {
		case STATE_LEVELS:
			if(HighlightedLevel != -1) {
				char Buffer[256];
				const SaveLevelStruct *Stats = LevelStats[HighlightedLevel];
				const std::string &NiceName = Campaign.GetLevelNiceName(CampaignIndex, HighlightedLevel);

				// Get text dimensions
				core::dimension2du NiceNameSize = Interface.GetFont(_Interface::FONT_MEDIUM)->getDimension(core::stringw(NiceName.c_str()).c_str());

				// Get box position
				int Width = NiceNameSize.Width + STATS_PADDING * 2, Height = STATS_MIN_HEIGHT + STATS_PADDING, X, Y;
				int Left = (int)Input.GetMouseX() + STATS_MOUSE_OFFSETX;
				int Top = (int)Input.GetMouseY() + STATS_MOUSE_OFFSETY;

				if(Width < STATS_MIN_WIDTH)
					Width = STATS_MIN_WIDTH;

				// Cap limits
				if(Top < STATS_PADDING)
					Top = STATS_PADDING;
				if(Left + Width > (int)irrDriver->getScreenSize().Width - 10)
					Left -= Width + 35;

				// Draw box
				Interface.DrawTextBox(Left + Width/2, Top + Height/2, Width, Height);
				X = Left + Width/2;
				Y = Top + STATS_PADDING;

				if(Stats->Unlocked) {

					// Level nice name
					Interface.RenderText(NiceName.c_str(), X, Y, _Interface::ALIGN_CENTER, _Interface::FONT_MEDIUM, video::SColor(255, 255, 255, 255));
					Y += 35;

					// Play time
					Interface.RenderText("Play time", X - 10, Y, _Interface::ALIGN_RIGHT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));
					Interface.ConvertSecondsToString(Stats->PlayTime, Buffer);
					Interface.RenderText(Buffer, X + 10, Y, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));

					// Load count
					Y += 17;
					Interface.RenderText("Plays", X - 10, Y, _Interface::ALIGN_RIGHT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));
					sprintf(Buffer, "%d", Stats->LoadCount);
					Interface.RenderText(Buffer, X + 10, Y, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));

					// Win count
					Y += 17;
					Interface.RenderText("Wins", X - 10, Y, _Interface::ALIGN_RIGHT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));
					sprintf(Buffer, "%d", Stats->WinCount);
					Interface.RenderText(Buffer, X + 10, Y, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));

					// Lose count
					Y += 17;
					Interface.RenderText("Deaths", X - 10, Y, _Interface::ALIGN_RIGHT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));
					sprintf(Buffer, "%d", Stats->LoseCount);
					Interface.RenderText(Buffer, X + 10, Y, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));

					// Scores
					if(Stats->HighScores.size() > 0) {

						// High scores
						int HighX = Left + Width/2 - 100, HighY = Y + 28;

						// Draw header
						Interface.RenderText("#", HighX, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));
						Interface.RenderText("Time", HighX + 30, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));
						Interface.RenderText("Date", HighX + 110, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));
						HighY += 19;

						for(size_t i = 0; i < Stats->HighScores.size(); i++) {

							// Number
							char SmallBuffer[32];
							sprintf(SmallBuffer, "%d", (int)i+1);
							Interface.RenderText(SmallBuffer, HighX, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));

							// Time
							Interface.ConvertSecondsToString(Stats->HighScores[i].Time, Buffer);
							Interface.RenderText(Buffer, HighX + 30, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));

							// Date
							char DateString[32];
							strftime(DateString, 32, "%Y-%m-%d", localtime(&Stats->HighScores[i].DateStamp));
							Interface.RenderText(DateString, HighX + 110, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));

							HighY += 18;
						}
					}
				}
				else {

					// Locked
					Interface.RenderText("Level Locked", X, Y, _Interface::ALIGN_CENTER, _Interface::FONT_MEDIUM, video::SColor(255, 255, 255, 255));
				}
			}
		break;
		case STATE_REPLAYS:

			// Show selected replay
			if(SelectedElement) {
				core::recti SelectedRect = SelectedElement->getRelativePosition();
				Interface.DrawImage(_Interface::IMAGE_SELECTED, SelectedRect.UpperLeftCorner.X + 32, SelectedRect.UpperLeftCorner.Y + 32, 64, 64);
			}

			// Show replay stats
			if(HighlightedLevel != -1) {
				const _ReplayInfo &ReplayInfo = ReplayFiles[HighlightedLevel];

				// Get text dimensions
				core::dimension2du DescriptionSize = Interface.GetFont(_Interface::FONT_MEDIUM)->getDimension(core::stringw(ReplayInfo.Description.c_str()).c_str());
				core::dimension2du NiceNameSize = Interface.GetFont(_Interface::FONT_SMALL)->getDimension(core::stringw(ReplayInfo.LevelNiceName.c_str()).c_str());

				// Get box position
				int Width = std::max(DescriptionSize.Width, NiceNameSize.Width) + STATS_PADDING * 2;
				int Height = 125 + STATS_PADDING, X, Y;
				int Left = (int)Input.GetMouseX() - Width/2;
				int Top = (int)Input.GetMouseY() - Height - 15;

				if(Width < STATS_MIN_WIDTH)
					Width = STATS_MIN_WIDTH;

				// Cap limits
				if(Top < STATS_PADDING)
					Top = STATS_PADDING;
				if(Left + Width > (int)irrDriver->getScreenSize().Width - 10)
					Left = (int)irrDriver->getScreenSize().Width - 10 - Width;
				if(Left < 10)
					Left = 10;

				// Draw box
				Interface.DrawTextBox(Left + Width/2, Top + Height/2, Width, Height);
				X = Left + Width/2;
				Y = Top + STATS_PADDING;

				// Replay description
				Interface.RenderText(ReplayInfo.Description.c_str(), X, Y, _Interface::ALIGN_CENTER, _Interface::FONT_MEDIUM, video::SColor(255, 255, 255, 255));
				Y += 30;

				// Replay description
				Interface.RenderText(ReplayInfo.LevelNiceName.c_str(), X, Y, _Interface::ALIGN_CENTER, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));
				Y += 35;

				// Date recorded
				Interface.RenderText("Date", X - 10, Y, _Interface::ALIGN_RIGHT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));
				Interface.RenderText(ReplayInfo.Date.c_str(), X + 10, Y, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));

				// Finish time
				Y += 20;
				Interface.RenderText("Finish time", X - 10, Y, _Interface::ALIGN_RIGHT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));
				Interface.RenderText(ReplayInfo.FinishTime.c_str(), X + 10, Y, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));
			}
		break;
		case STATE_LOSE:
			Menu.DrawLoseScreen();
		break;
		case STATE_WIN:
			Menu.DrawWinScreen();
		break;
		default:
		break;
	}
}

// Draw the win screen
void _Menu::DrawWinScreen() {
	char Buffer[256];

	char TimeString[32];
	Interface.ConvertSecondsToString(PlayState.Timer, TimeString);

	// Draw header
	int X = Interface.GetCenterX();
	int Y = Interface.GetCenterY() - WIN_HEIGHT / 2 + 15;
	Interface.DrawTextBox(Interface.GetCenterX(), Interface.GetCenterY(), WIN_WIDTH, WIN_HEIGHT);
	Interface.RenderText("Level Completed!", X, Y, _Interface::ALIGN_CENTER, _Interface::FONT_LARGE);

	// Draw time
	Y += 75;
	Interface.RenderText("Your Time", X - 10, Y, _Interface::ALIGN_RIGHT, _Interface::FONT_MEDIUM);
	Interface.RenderText(TimeString, X + 10, Y, _Interface::ALIGN_LEFT, _Interface::FONT_MEDIUM);

	// Best time
	Y += 30;
	if(WinStats->HighScores.size() > 0) {
		Interface.RenderText("Best Time", X - 10, Y, _Interface::ALIGN_RIGHT, _Interface::FONT_MEDIUM);
		Interface.ConvertSecondsToString(WinStats->HighScores[0].Time, Buffer);
		Interface.RenderText(Buffer, X + 10, Y, _Interface::ALIGN_LEFT, _Interface::FONT_MEDIUM);
	}

	// High scores
	int HighX = Interface.GetCenterX() - 100, HighY = Y + 48;

	// Draw header
	Interface.RenderText("#", HighX, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));
	Interface.RenderText("Time", HighX + 30, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));
	Interface.RenderText("Date", HighX + 110, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));
	HighY += 17;
	for(uint32_t i = 0; i < WinStats->HighScores.size(); i++) {

		// Number
		char SmallBuffer[32];
		sprintf(SmallBuffer, "%d", i+1);
		Interface.RenderText(SmallBuffer, HighX, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(200, 255, 255, 255));

		// Time
		Interface.ConvertSecondsToString(WinStats->HighScores[i].Time, Buffer);
		Interface.RenderText(Buffer, HighX + 30, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(200, 255, 255, 255));

		// Date
		char DateString[32];
		strftime(DateString, 32, "%Y-%m-%d", localtime(&WinStats->HighScores[i].DateStamp));
		Interface.RenderText(DateString, HighX + 110, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(200, 255, 255, 255));

		HighY += 17;
	}
}

// Draw the lose screen
void _Menu::DrawLoseScreen() {

	// Draw header
	int X = Interface.GetCenterX();
	int Y = Interface.GetCenterY() - WIN_HEIGHT / 2 - 40;
	Interface.RenderText(LoseMessage.c_str(), X, Y, _Interface::ALIGN_CENTER, _Interface::FONT_LARGE);
}

// Cancels the key bind state
void _Menu::CancelKeyBind() {
	KeyButton->setText(KeyButtonOldText.c_str());
	KeyButton = NULL;
}

// Launchs a level
void _Menu::LaunchLevel() {

	SaveLevelStruct Stats;
	Save.GetLevelStats(Campaign.GetCampaign(CampaignIndex).Levels[SelectedLevel].File, Stats);
	if(Stats.Unlocked == 0)
		return;

	PlayState.SetTestLevel("");
	PlayState.SetCampaign(CampaignIndex);
	PlayState.SetCampaignLevel(SelectedLevel);
	Game.ChangeState(&PlayState);
}

// Launchs a replay from a list item
void _Menu::LaunchReplay() {

	// Get replay file
	if(SelectedLevel >= 0) {

		// Load replay
		ViewReplayState.SetCurrentReplay(ReplayFiles[SelectedLevel].Filename);
		Game.ChangeState(&ViewReplayState);
	}
}

// Add a regular menu button
gui::IGUIButton *_Menu::AddMenuButton(const irr::core::recti &Rectangle, int ID, const wchar_t *Text, _Interface::ImageType ButtonImage) {
	gui::IGUIButton *Button = irrGUI->addButton(Rectangle, CurrentLayout, ID, Text);
	Button->setImage(Interface.GetImage(ButtonImage));
	Button->setUseAlphaChannel(true);
	Button->setDrawBorder(false);
	Button->setOverrideFont(Interface.GetFont(_Interface::FONT_BUTTON));

	return Button;
}

// Add menu text label
gui::IGUIStaticText *_Menu::AddMenuText(const core::position2di &CenterPosition, const wchar_t *Text, _Interface::FontType Font, int ID, gui::EGUI_ALIGNMENT HorizontalAlign) {

	// Get text dimensions
	core::dimension2du Size = Interface.GetFont(Font)->getDimension(Text);
	Size.Width++;

	core::recti Rectangle;
	switch(HorizontalAlign) {
		case gui::EGUIA_UPPERLEFT:
			Rectangle = Interface.GetRect(CenterPosition.X, CenterPosition.Y, Size.Width, Size.Height);
		break;
		case gui::EGUIA_CENTER:
			Rectangle = Interface.GetCenteredRect(CenterPosition.X, CenterPosition.Y, Size.Width, Size.Height);
		break;
		case gui::EGUIA_LOWERRIGHT:
			Rectangle = Interface.GetRightRect(CenterPosition.X, CenterPosition.Y, Size.Width, Size.Height);
		break;
		default:
		break;
	}

	// Add text
	gui::IGUIStaticText *NewText = irrGUI->addStaticText(Text, Rectangle, false, false, CurrentLayout);
	NewText->setOverrideFont(Interface.GetFont(Font));

	return NewText;
}

// Clear out the current menu layout
void _Menu::ClearCurrentLayout() {
	if(CurrentLayout) {
		irrGUI->setFocus(0);
		CurrentLayout->remove();
		CurrentLayout = nullptr;
	}
	CurrentLayout = new CGUIEmptyElement(irrGUI, irrGUI->getRootGUIElement());
	CurrentLayout->drop();

	SelectedElement = nullptr;
}