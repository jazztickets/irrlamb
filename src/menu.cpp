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
#include <menu.h>
#include <graphics.h>
#include <input.h>
#include <constants.h>
#include <interface.h>
#include <globals.h>
#include <config.h>
#include <framework.h>
#include <save.h>
#include <campaign.h>
#include <level.h>
#include <save.h>
#include <audio.h>
#include <states/viewreplay.h>
#include <states/play.h>
#include <states/null.h>
#include <font/CGUITTFont.h>
#include <IGUIImage.h>
#include <IGUIElement.h>
#include <IGUIComboBox.h>
#include <IGUICheckBox.h>
#include <IGUIEditBox.h>
#include <IFileSystem.h>
#include <algorithm>

using namespace irr;

_Menu Menu;

const int REPLAY_LEVELID = 1100;
const int CAMPAIGN_LEVELID = 1000;
const int PLAY_CAMPAIGNID = 900;

const int STATS_MIN_WIDTH = 440;
const int STATS_MIN_HEIGHT = 580;
const int STATS_PADDING = 30;
const int STATS_MOUSE_OFFSETX = 20;
const int STATS_MOUSE_OFFSETY = -105;

const int LOSE_WIDTH = 800;
const int LOSE_HEIGHT = 380;
const int WIN_WIDTH = 1100;
const int WIN_HEIGHT = 650;

const int CAMPAIGN_COLUMNS = 8;

const int REPLAY_COLUMNS = 8;
const int REPLAY_SCROLL_AMOUNT = REPLAY_COLUMNS;
const int REPLAY_DISPLAY_COUNT = REPLAY_COLUMNS * 4;

const int KeyMapOrder[] = {
	_Actions::MOVE_FORWARD,
	_Actions::MOVE_BACK,
	_Actions::MOVE_LEFT,
	_Actions::MOVE_RIGHT,
	_Actions::JUMP,
	_Actions::RESET
};

enum GUIElements {
	MAIN_CAMPAIGNS, MAIN_REPLAYS, MAIN_OPTIONS, MAIN_QUIT,
	CAMPAIGNS_BACK,
	LEVELS_GO, LEVELS_BUY, LEVELS_HIGHSCORES, LEVELS_BACK, LEVELS_SELECTEDLEVEL,
	LEVELINFO_DESCRIPTION, LEVELINFO_ATTEMPTS, LEVELINFO_WINS, LEVELINFO_LOSSES, LEVELINFO_PLAYTIME, LEVELINFO_BESTTIME,
	REPLAYS_SORT, REPLAYS_DELETE, REPLAYS_VIEW, REPLAYS_VALIDATE, REPLAYS_BACK, REPLAYS_UP, REPLAYS_DOWN,
	OPTIONS_GAMEPLAY, OPTIONS_VIDEO, OPTIONS_AUDIO, OPTIONS_CONTROLS, OPTIONS_BACK,
	GAMEPLAY_SHOWFPS, GAMEPLAY_SHOWTUTORIAL, GAMEPLAY_SAVE, GAMEPLAY_CANCEL,
	VIDEO_SAVE, VIDEO_CANCEL, VIDEO_VIDEOMODES, VIDEO_FULLSCREEN, VIDEO_SHADOWS, VIDEO_SHADERS, VIDEO_MULTIPLELIGHTS, VIDEO_VSYNC, VIDEO_ANISOTROPY, VIDEO_ANTIALIASING,
	AUDIO_ENABLED, AUDIO_PLAYERSOUNDS, AUDIO_SAVE, AUDIO_CANCEL,
	CONTROLS_SAVE, CONTROLS_CANCEL, CONTROLS_DEADZONE, CONTROLS_SENSITIVITY, CONTROLS_INVERTMOUSE, CONTROLS_INVERTGAMEPADY, CONTROLS_KEYMAP,
	PAUSE_RESUME=(CONTROLS_KEYMAP + _Actions::COUNT), PAUSE_SAVEREPLAY, PAUSE_RESTART, PAUSE_OPTIONS, PAUSE_QUITLEVEL,
	SAVEREPLAY_NAME, SAVEREPLAY_SAVE, SAVEREPLAY_CANCEL,
	LOSE_RESTARTLEVEL, LOSE_SAVEREPLAY, LOSE_MAINMENU,
	WIN_RESTARTLEVEL, WIN_NEXTLEVEL, WIN_SAVEREPLAY, WIN_MAINMENU,
};

// Compare function for sorting replays by timestamp
bool CompareReplaysTimestamp(const _ReplayInfo &Left, const _ReplayInfo &Right) {
	return Left.Timestamp > Right.Timestamp;
}

// Compare function for sorting replays by level, then timestamp
bool CompareReplaysLevel(const _ReplayInfo &Left, const _ReplayInfo &Right) {
	return Left.LevelName < Right.LevelName || (Left.LevelName == Right.LevelName && Left.Timestamp > Right.Timestamp);
}

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
						Framework.SetDone(true);
					break;
					case STATE_CAMPAIGNS:
					case STATE_OPTIONS:
					case STATE_REPLAYS:
						if(Framework.GetState() == &PlayState)
							InitPause();
						else
							InitMain();
					break;
					case STATE_LEVELS:
						InitCampaigns();
					break;
					case STATE_OPTIONS_GAMEPLAY:
					case STATE_OPTIONS_VIDEO:
					case STATE_OPTIONS_AUDIO:
					case STATE_OPTIONS_CONTROLS:
						InitOptions();
					break;
					case STATE_PAUSED:
						if(PlayState.ReplayInputs && PlayState.InputReplay->ReplayStopped()) {
							NullState.State = STATE_REPLAYS;
							Framework.ChangeState(&NullState);
						}
						else
							InitPlay();
					break;
					case STATE_LOSE:
					case STATE_WIN:
						if(PlayState.ReplayInputs)
							NullState.State = STATE_REPLAYS;
						else
							NullState.State = STATE_LEVELS;
						Framework.ChangeState(&NullState);
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
			case _Actions::MENU_PAGEUP:
				switch(State) {
					case STATE_REPLAYS:
						ReplayScrollUp();
						return true;
					break;
					default:
					break;
				}
			break;
			case _Actions::MENU_PAGEDOWN:
				switch(State) {
					case STATE_REPLAYS:
						ReplayScrollDown();
						return true;
					break;
					default:
					break;
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
					InitCampaigns();
				break;
				default:
					Processed = false;
				break;
			}
		break;
		case STATE_CAMPAIGNS:
			switch(Key) {
				case KEY_RETURN:
					InitLevels();
				break;
				default:
					Processed = false;
				break;
			}
		break;
		case STATE_REPLAYS:
			if(Key == KEY_KEY_V) {
				ValidateReplay();
			}
		break;
		case STATE_OPTIONS_CONTROLS:
			if(KeyButton != nullptr) {
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

				KeyButton = nullptr;
			}
			else
				Processed = false;
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
				case MAIN_CAMPAIGNS:
					InitCampaigns();
				break;
				case MAIN_REPLAYS:
					StartOffset = 0;
					InitReplays(true);
				break;
				case MAIN_OPTIONS:
					InitOptions();
				break;
				case MAIN_QUIT:
					Framework.SetDone(true);
				break;
				case CAMPAIGNS_BACK:
					InitMain();
				break;
				case LEVELS_GO:
					LaunchLevel();
				break;
				case LEVELS_BACK:
					InitCampaigns();
				break;
				case REPLAYS_SORT:
					ReplaySort++;
					if(ReplaySort >= SORT_MAX)
						ReplaySort = 0;
					InitReplays(true);
				break;
				case REPLAYS_DELETE: {

					// Get list
					if(SelectedLevel != -1) {

						// Get replay file name
						std::string FileName = ReplayFiles[SelectedLevel].Filename;

						// Remove file
						std::string FilePath = Save.ReplayPath + FileName;
						remove(FilePath.c_str());

						// Refresh screen
						InitReplays(true);
					}
				}
				break;
				case REPLAYS_VIEW:
					LaunchReplay();
				break;
				case REPLAYS_VALIDATE:
					ValidateReplay();
				break;
				case REPLAYS_BACK:
					InitMain();
				break;
				case REPLAYS_UP:
					ReplayScrollUp();
				break;
				case REPLAYS_DOWN:
					ReplayScrollDown();
				break;
				case OPTIONS_GAMEPLAY:
					InitGameplayOptions();
				break;
				case OPTIONS_VIDEO:
					InitVideoOptions();
				break;
				case OPTIONS_AUDIO:
					InitAudioOptions();
				break;
				case OPTIONS_CONTROLS:
					InitControlOptions();
				break;
				case OPTIONS_BACK:
					if(Framework.GetState() == &PlayState)
						InitPause();
					else
						InitMain();
				break;
				case GAMEPLAY_SAVE: {

					// Get settings
					gui::IGUICheckBox *ShowFPS = static_cast<gui::IGUICheckBox *>(CurrentLayout->getElementFromId(GAMEPLAY_SHOWFPS));
					gui::IGUICheckBox *ShowTutorial = static_cast<gui::IGUICheckBox *>(CurrentLayout->getElementFromId(GAMEPLAY_SHOWTUTORIAL));
					Config.ShowFPS = ShowFPS->isChecked();
					Config.ShowTutorial = ShowTutorial->isChecked();
					Config.WriteConfig();
					InitOptions();
				}
				break;
				case GAMEPLAY_CANCEL:
					InitOptions();
				break;
				case VIDEO_SAVE: {

					// Save the video mode
					gui::IGUIComboBox *VideoModes = static_cast<gui::IGUIComboBox *>(CurrentLayout->getElementFromId(VIDEO_VIDEOMODES));
					if(VideoModes != nullptr) {
						_VideoMode Mode = Graphics.GetVideoModes()[VideoModes->getSelected()];
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
					if(Anisotropy != nullptr) {
						if(Anisotropy->getSelected() == 0)
							Config.AnisotropicFiltering = 0;
						else
							Config.AnisotropicFiltering = 1 << (Anisotropy->getSelected() - 1);
					}

					// Save the antialiasing
					gui::IGUIComboBox *Antialiasing = static_cast<gui::IGUIComboBox *>(CurrentLayout->getElementFromId(VIDEO_ANTIALIASING));
					if(Antialiasing != nullptr) {
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

					// Save multiple lights
					gui::IGUICheckBox *MultipleLights = static_cast<gui::IGUICheckBox *>(CurrentLayout->getElementFromId(VIDEO_MULTIPLELIGHTS));
					Config.MultipleLights = MultipleLights->isChecked();

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

					// Get settings
					gui::IGUICheckBox *AudioEnabled = static_cast<gui::IGUICheckBox *>(CurrentLayout->getElementFromId(AUDIO_ENABLED));
					gui::IGUICheckBox *PlayerSounds = static_cast<gui::IGUICheckBox *>(CurrentLayout->getElementFromId(AUDIO_PLAYERSOUNDS));

					// Save
					Config.SoundVolume = AudioEnabled->isChecked() ? 1.0 : 0.0;
					Config.PlayerSounds = PlayerSounds->isChecked();
					Config.WriteConfig();
					Audio.SetGain(Config.SoundVolume);

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

					// Save deadzone
					gui::IGUIEditBox *DeadZone = static_cast<gui::IGUIEditBox *>(CurrentLayout->getElementFromId(CONTROLS_DEADZONE));
					Config.DeadZone = wcstof(DeadZone->getText(), nullptr);

					// Save sensitivity
					gui::IGUIEditBox *MouseSensitivity = static_cast<gui::IGUIEditBox *>(CurrentLayout->getElementFromId(CONTROLS_SENSITIVITY));
					Config.MouseSensitivity = wcstof(MouseSensitivity->getText(), nullptr);

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
					Framework.ChangeState(&NullState);
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
					Framework.ChangeState(&PlayState);
					CampaignIndex = PlayState.CurrentCampaign;
				break;
				case LOSE_SAVEREPLAY:
				case WIN_SAVEREPLAY:
					InitSaveReplay();
				break;
				case LOSE_MAINMENU:
				case WIN_MAINMENU:
					if(PlayState.ReplayInputs)
						NullState.State = STATE_REPLAYS;
					else
						NullState.State = STATE_LEVELS;
					Framework.ChangeState(&NullState);
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

	// Title
	irr::video::ITexture *Texture = irrDriver->getTexture("ui/title.png");
	irr::gui::IGUIImage *Image = irrGUI->addImage(
			Interface.GetCenteredRectPercent(
				0.5,
				0.15,
				Texture->getSize().Width,
				Texture->getSize().Height
			),
		CurrentLayout
	);
	Image->setScaleImage(true);
	Image->setImage(Texture);

	// Add version
	AddMenuText(Interface.GetPositionPercent(0.03, 0.97), core::stringw(GAME_VERSION).c_str(), _Interface::FONT_SMALL);

	// Buttons
	AddMenuButton(Interface.GetCenteredRectPercent(0.5, 0.36, BUTTON_SIZE_X, BUTTON_SIZE_Y), MAIN_CAMPAIGNS, L"Play", _Interface::IMAGE_BUTTON_BIG);
	AddMenuButton(Interface.GetCenteredRectPercent(0.5, 0.51, BUTTON_SIZE_X, BUTTON_SIZE_Y), MAIN_REPLAYS, L"Replays", _Interface::IMAGE_BUTTON_BIG);
	AddMenuButton(Interface.GetCenteredRectPercent(0.5, 0.66, BUTTON_SIZE_X, BUTTON_SIZE_Y), MAIN_OPTIONS, L"Options", _Interface::IMAGE_BUTTON_BIG);
	AddMenuButton(Interface.GetCenteredRectPercent(0.5, 0.81, BUTTON_SIZE_X, BUTTON_SIZE_Y), MAIN_QUIT, L"Quit", _Interface::IMAGE_BUTTON_BIG);

	// Play sound
	if(!FirstStateLoad)
		Interface.PlaySound(_Interface::SOUND_CONFIRM);
	FirstStateLoad = false;

	PreviousState = State;
	State = STATE_MAIN;
}

// Create the single player menu
void _Menu::InitCampaigns() {
	ClearCurrentLayout();

	// Reset menu variables
	CampaignIndex = 0;
	SelectedLevel = -1;
	HighlightedLevel = -1;

	// Text
	AddMenuText(Interface.GetPositionPercent(0.5, 0.1), L"Categories");

	// Campaigns
	float X = 0.5;
	float Y = 0.3;
	const float CAMPAIGN_SPACING_Y = (BUTTON_SIZE_Y * Interface.GetUIScale() / irrDriver->getScreenSize().Height) + 0.01f;
	const std::vector<_CampaignInfo> &Campaigns = Campaign.GetCampaigns();
	for(uint32_t i = 0; i < Campaigns.size(); i++) {
		if(!Campaigns[i].Show)
			continue;

		// Add campaign button
		irr::core::stringw Name(Campaigns[i].Name.c_str());
		AddMenuButton(Interface.GetCenteredRectPercent(X, Y, BUTTON_SIZE_X, BUTTON_SIZE_Y), PLAY_CAMPAIGNID + i, Name.c_str());

		// Update position
		Y += CAMPAIGN_SPACING_Y;
	}

	// Back button
	AddMenuButton(Interface.GetCenteredRectPercent(0.5, 0.9, BUTTON_SMALL_SIZE_X, BUTTON_SMALL_SIZE_Y), CAMPAIGNS_BACK, L"Back", _Interface::IMAGE_BUTTON_SMALL);

	// Play sound
	Interface.PlaySound(_Interface::SOUND_CONFIRM);

	PreviousState = State;
	State = STATE_CAMPAIGNS;
}

// Create the levels menu
void _Menu::InitLevels() {
	ClearCurrentLayout();
	LevelStats.clear();
	SelectedLevel = -1;
	HighlightedLevel = -1;
	const _CampaignInfo &CampaignData = Campaign.GetCampaign(CampaignIndex);

	// Title
	AddMenuText(Interface.GetPositionPercent(0.5, 0.1), core::stringw(CampaignData.Name.c_str()).c_str());

	// Calculate layout
	int ColumnsPerRow = std::min((std::size_t)CAMPAIGN_COLUMNS, CampaignData.Levels.size());
	int Column = 0, Row = 0;
	float SpacingX = (BUTTON_LEVEL_SIZE * Interface.GetUIScale()) / irrDriver->getScreenSize().Width + 0.01f * irrDriver->getScreenSize().Height / irrDriver->getScreenSize().Width;
	float SpacingY = (BUTTON_LEVEL_SIZE * Interface.GetUIScale()) / irrDriver->getScreenSize().Height + 0.01f;
	float StartX = 0.5f - (SpacingX * (ColumnsPerRow - 1) / 2.0f);
	float StartY = 0.3f;

	// Add level list
	for(uint32_t i = 0; i < CampaignData.Levels.size(); i++) {
		bool Unlocked = true;

		// Get level stats
		const _LevelStat *Stats = &Save.LevelStats[CampaignData.Levels[i].File];
		LevelStats.push_back(Stats);

		// Set unlocked status
		if(Stats->Unlocked == 0) {

			// Unlock level if previous level has any highscores
			if(i > 0 && Save.LevelStats[CampaignData.Levels[i-1].File].HighScores.size() > 0) {
				Save.UnlockLevel(CampaignData.Levels[i].File);
			}
			else {
				Unlocked = false;
			}

			// Unlock the level if it's always unlocked in the campaign
			if((Input.GetKeyState(KEY_F1) && Input.GetKeyState(KEY_F10)) || CampaignData.Levels[i].Unlocked) {
				Save.UnlockLevel(CampaignData.Levels[i].File);
				Unlocked = true;
			}
		}

		// Add button
		gui::IGUIButton *Level = irrGUI->addButton(
			Interface.GetCenteredRectPercent(
				StartX + Column * SpacingX,
				StartY + Row * SpacingY,
				BUTTON_LEVEL_SIZE,
				BUTTON_LEVEL_SIZE
			),
			CurrentLayout,
			CAMPAIGN_LEVELID + i
		);

		Level->setScaleImage(true);

		// Set thumbnail
		if(Unlocked)
			Level->setImage(irrDriver->getTexture((CampaignData.Levels[i].DataPath + "icon.jpg").c_str()));
		else
			Level->setImage(irrDriver->getTexture("ui/locked.png"));

		// Update columns and rows
		Column++;
		if(Column >= ColumnsPerRow) {
			Column = 0;
			Row++;
		}
	}

	// Back button
	AddMenuButton(Interface.GetCenteredRectPercent(0.5, 0.9, BUTTON_SMALL_SIZE_X, BUTTON_SMALL_SIZE_Y), LEVELS_BACK, L"Back", _Interface::IMAGE_BUTTON_SMALL);

	// Play sound
	if(!FirstStateLoad)
		Interface.PlaySound(_Interface::SOUND_CONFIRM);
	FirstStateLoad = false;

	PreviousState = State;
	State = STATE_LEVELS;
}

// Create the replay menu
void _Menu::InitReplays(bool LoadReplays) {
	Interface.ChangeSkin(_Interface::SKIN_MENU);
	ClearCurrentLayout();
	Input.SetMouseLocked(false);

	SelectedLevel = -1;
	HighlightedLevel = -1;

	// Text
	AddMenuText(Interface.GetPositionPercent(0.5, 0.1), L"Replays");

	// Load replay files
	if(LoadReplays) {

		// Change directories
		std::string OldWorkingDirectory(irrFile->getWorkingDirectory().c_str());
		irrFile->changeWorkingDirectoryTo(Save.ReplayPath.c_str());

		// Clear list
		ReplayFiles.clear();

		// Get a list of replays
		io::IFileList *FileList = irrFile->createFileList();
		uint32_t FileCount = FileList->getFileCount();
		for(uint32_t i = 0; i < FileCount; i++) {
			if(!FileList->isDirectory(i) && FileList->getFileName(i).find(".replay") != -1) {

				// Load header
				bool Loaded = Replay.LoadReplay(FileList->getFileName(i).c_str(), true);
				if(Loaded && Replay.GetVersion() == REPLAY_VERSION && Replay.GetTimeStep() == PHYSICS_TIMESTEP) {
					char Buffer[256];

					// Get level info
					Level.Init(Replay.GetLevelName(), true);
					if(Level.LevelVersion > Replay.GetLevelVersion())
						continue;

					// Get replay info
					_ReplayInfo ReplayInfo;
					ReplayInfo.Filename = FileList->getFileName(i).c_str();
					ReplayInfo.Description = Replay.GetDescription();
					ReplayInfo.LevelName = Level.LevelName;
					ReplayInfo.LevelNiceName = Level.LevelNiceName;
					ReplayInfo.Autosave = Replay.GetAutosave();
					ReplayInfo.Won = Replay.GetWon();
					ReplayInfo.Timestamp = Replay.GetTimestamp();
					ReplayInfo.Platform = Replay.GetPlatform();

					// Date
					strftime(Buffer, 32, "%Y-%m-%d %H:%M:%S", localtime(&Replay.GetTimestamp()));
					ReplayInfo.Date = Buffer;

					// Get time string
					Interface.ConvertSecondsToString(Replay.GetFinishTime(), Buffer);
					ReplayInfo.FinishTime = Buffer;
					ReplayFiles.push_back(ReplayInfo);
				}
			}
		}
		FileList->drop();
		irrFile->changeWorkingDirectoryTo(OldWorkingDirectory.c_str());

		// Sort replays
		if(ReplaySort == SORT_TIMESTAMP)
			std::sort(ReplayFiles.begin(), ReplayFiles.end(), CompareReplaysTimestamp);
		else if(ReplaySort == SORT_LEVELNAME)
			std::sort(ReplayFiles.begin(), ReplayFiles.end(), CompareReplaysLevel);
	}

	// Calculate layout
	int ColumnsPerRow = REPLAY_COLUMNS;
	int Column = 0, Row = 0;
	float SpacingX = (BUTTON_LEVEL_SIZE * Interface.GetUIScale()) / irrDriver->getScreenSize().Width + 0.01f * irrDriver->getScreenSize().Height / irrDriver->getScreenSize().Width;
	float SpacingY = (BUTTON_LEVEL_SIZE * Interface.GetUIScale()) / irrDriver->getScreenSize().Height + 0.01f;
	float StartX = 0.5f - (SpacingX * (ColumnsPerRow - 1) / 2.0f);
	float StartY = 0.3f;
	float EndX = 0.5f + (SpacingX * (ColumnsPerRow - 1) / 2.0f) + SpacingX;

	// Add replay buttons
	uint32_t ReplayIndex = 0;
	uint32_t ReplayCount = 0;
	for(const auto &ReplayInfo : ReplayFiles) {
		if(ReplayCount >= StartOffset) {

			// Add button
			gui::IGUIButton *LevelButton = irrGUI->addButton(
				Interface.GetCenteredRectPercent(
					StartX + Column * SpacingX,
					StartY + Row * SpacingY,
					BUTTON_LEVEL_SIZE,
					BUTTON_LEVEL_SIZE
				),
				CurrentLayout,
				REPLAY_LEVELID + ReplayIndex + StartOffset
			);
			LevelButton->setImage(irrDriver->getTexture((Framework.GetWorkingPath() + "levels/" + ReplayInfo.LevelName + "/icon.jpg").c_str()));
			LevelButton->setScaleImage(true);

			// Update columns and rows
			Column++;
			if(Column >= REPLAY_COLUMNS) {
				Column = 0;
				Row++;
			}

			// Cap display count
			ReplayIndex++;
			if(ReplayIndex >= REPLAY_DISPLAY_COUNT)
				break;
		}

		ReplayCount++;
	}

	// Page buttons
	AddMenuButton(Interface.GetCenteredRectPercent(EndX, 0.46, BUTTON_ICON_SIZE, BUTTON_ICON_SIZE), REPLAYS_UP, L"", _Interface::IMAGE_BUTTON_UP);
	AddMenuButton(Interface.GetCenteredRectPercent(EndX, 0.54, BUTTON_ICON_SIZE, BUTTON_ICON_SIZE), REPLAYS_DOWN, L"", _Interface::IMAGE_BUTTON_DOWN);

	// Controls
	AddMenuButton(Interface.GetCenteredRectPercent(EndX, 0.3, BUTTON_ICON_SIZE, BUTTON_ICON_SIZE), REPLAYS_SORT, L"", _Interface::IMAGE_BUTTON_SORT);
	AddMenuButton(Interface.GetCenteredRectPercent(EndX, 0.7, BUTTON_ICON_SIZE, BUTTON_ICON_SIZE), REPLAYS_DELETE, L"", _Interface::IMAGE_BUTTON_DELETE);

	// View and back buttons
	AddMenuButton(Interface.GetRightRectPercent(0.5 - 0.01, 0.9, BUTTON_SMALL_SIZE_X, BUTTON_SMALL_SIZE_Y), REPLAYS_VIEW, L"View", _Interface::IMAGE_BUTTON_SMALL);
	AddMenuButton(Interface.GetRectPercent(0.5 + 0.01, 0.9, BUTTON_SMALL_SIZE_X, BUTTON_SMALL_SIZE_Y), REPLAYS_BACK, L"Back", _Interface::IMAGE_BUTTON_SMALL);

	// Play sound
	if(!FirstStateLoad && LoadReplays)
		Interface.PlaySound(_Interface::SOUND_CONFIRM);
	FirstStateLoad = false;

	PreviousState = State;
	State = STATE_REPLAYS;
}

// Create the options menu
void _Menu::InitOptions() {
	Interface.ChangeSkin(_Interface::SKIN_MENU);
	ClearCurrentLayout();

	// Title
	AddMenuText(Interface.GetPositionPercent(0.5, 0.1), L"Options");

	// Buttons
	AddMenuButton(Interface.GetCenteredRectPercent(0.5, 0.35, BUTTON_SIZE_X, BUTTON_SIZE_Y), OPTIONS_GAMEPLAY, L"Gameplay");
	AddMenuButton(Interface.GetCenteredRectPercent(0.5, 0.45, BUTTON_SIZE_X, BUTTON_SIZE_Y), OPTIONS_VIDEO, L"Video");
	AddMenuButton(Interface.GetCenteredRectPercent(0.5, 0.55, BUTTON_SIZE_X, BUTTON_SIZE_Y), OPTIONS_AUDIO, L"Audio");
	AddMenuButton(Interface.GetCenteredRectPercent(0.5, 0.65, BUTTON_SIZE_X, BUTTON_SIZE_Y), OPTIONS_CONTROLS, L"Controls");

	// Back button
	AddMenuButton(Interface.GetCenteredRectPercent(0.5, 0.9, BUTTON_SMALL_SIZE_X, BUTTON_SMALL_SIZE_Y), OPTIONS_BACK, L"Back", _Interface::IMAGE_BUTTON_SMALL);

	// Play sound
	Interface.PlaySound(_Interface::SOUND_CONFIRM);

	PreviousState = State;
	State = STATE_OPTIONS;
}

// Gameplay options
void _Menu::InitGameplayOptions() {
	ClearCurrentLayout();
	float X = 0.5;
	float Y = 0.4;
	float SpacingY = 0.05;
	float SidePadding = 0.01;

	// Title
	AddMenuText(Interface.GetPositionPercent(0.5, 0.1), L"Gameplay");

	// Show FPS
	AddMenuText(Interface.GetPositionPercent(X - SidePadding, Y), L"Show FPS", _Interface::FONT_SMALL, -1, gui::EGUIA_LOWERRIGHT);
	irrGUI->addCheckBox(Config.ShowFPS, Interface.GetRectPercent(X + SidePadding, Y, 32, 32), CurrentLayout, GAMEPLAY_SHOWFPS);

	// Show tutorial
	Y += SpacingY;
	AddMenuText(Interface.GetPositionPercent(X - SidePadding, Y), L"Show Tutorial Messages", _Interface::FONT_SMALL, -1, gui::EGUIA_LOWERRIGHT);
	irrGUI->addCheckBox(Config.ShowTutorial, Interface.GetRectPercent(X + SidePadding, Y, 32, 32), CurrentLayout, GAMEPLAY_SHOWTUTORIAL);

	// Save and cancel buttons
	AddMenuButton(Interface.GetRightRectPercent(0.5 - SidePadding, 0.9, BUTTON_SMALL_SIZE_X, BUTTON_SMALL_SIZE_Y), GAMEPLAY_SAVE, L"Save", _Interface::IMAGE_BUTTON_SMALL);
	AddMenuButton(Interface.GetRectPercent(0.5 + SidePadding, 0.9, BUTTON_SMALL_SIZE_X, BUTTON_SMALL_SIZE_Y), GAMEPLAY_CANCEL, L"Cancel", _Interface::IMAGE_BUTTON_SMALL);

	// Play sound
	Interface.PlaySound(_Interface::SOUND_CONFIRM);

	PreviousState = State;
	State = STATE_OPTIONS_GAMEPLAY;
}

// Create the video options menu
void _Menu::InitVideoOptions() {
	ClearCurrentLayout();
	float X = 0.5;
	float Y = 0.3;
	float SpacingY = 0.05;
	float SidePadding = 0.01;

	// Title
	AddMenuText(Interface.GetPositionPercent(0.5, 0.1), L"Video");

	// Screen resolution
	const std::vector<_VideoMode> &ModeList = Graphics.GetVideoModes();
	if(ModeList.size() > 0) {
		AddMenuText(Interface.GetPositionPercent(X - SidePadding, Y), L"Screen Resolution", _Interface::FONT_SMALL, -1, gui::EGUIA_LOWERRIGHT);
		gui::IGUIComboBox *ListScreenResolution = irrGUI->addComboBox(Interface.GetRectPercent(X + SidePadding, Y, 210, 45), CurrentLayout, VIDEO_VIDEOMODES);

		// Populate mode list
		for(uint32_t i = 0; i < ModeList.size(); i++)
			ListScreenResolution->addItem(ModeList[i].String.c_str());
		ListScreenResolution->setSelected(Graphics.GetCurrentVideoModeIndex());
	}

	// Full Screen
	Y += SpacingY;
	AddMenuText(Interface.GetPositionPercent(X - SidePadding, Y), L"Fullscreen", _Interface::FONT_SMALL, -1, gui::EGUIA_LOWERRIGHT);
	irrGUI->addCheckBox(Config.Fullscreen, Interface.GetRectPercent(X + SidePadding, Y, 32, 32), CurrentLayout, VIDEO_FULLSCREEN);

	// Shadows
	Y += SpacingY;
	AddMenuText(Interface.GetPositionPercent(X - SidePadding, Y), L"Shadows", _Interface::FONT_SMALL, -1, gui::EGUIA_LOWERRIGHT);
	irrGUI->addCheckBox(Config.Shadows, Interface.GetRectPercent(X + SidePadding, Y, 32, 32), CurrentLayout, VIDEO_SHADOWS);

	// Shaders
	Y += SpacingY;
	AddMenuText(Interface.GetPositionPercent(X - SidePadding, Y), L"Shaders", _Interface::FONT_SMALL, -1, gui::EGUIA_LOWERRIGHT);
	gui::IGUICheckBox *CheckBoxShaders = irrGUI->addCheckBox(Config.Shaders, Interface.GetRectPercent(X + SidePadding, Y, 32, 32), CurrentLayout, VIDEO_SHADERS);
	if(!Graphics.GetShadersSupported())
		CheckBoxShaders->setEnabled(false);

	// Multiple Lights
	Y += SpacingY;
	AddMenuText(Interface.GetPositionPercent(X - SidePadding, Y), L"Multiple Lights", _Interface::FONT_SMALL, -1, gui::EGUIA_LOWERRIGHT);
	irrGUI->addCheckBox(Config.MultipleLights, Interface.GetRectPercent(X + SidePadding, Y, 32, 32), CurrentLayout, VIDEO_MULTIPLELIGHTS);

	// Vsync
	Y += SpacingY;
	AddMenuText(Interface.GetPositionPercent(X - SidePadding, Y), L"V-sync", _Interface::FONT_SMALL, -1, gui::EGUIA_LOWERRIGHT);
	irrGUI->addCheckBox(Config.Vsync, Interface.GetRectPercent(X + SidePadding, Y, 32, 32), CurrentLayout, VIDEO_VSYNC);

	// Anisotropic Filtering
	Y += SpacingY;
	int MaxAnisotropy = irrDriver->getDriverAttributes().getAttributeAsInt("MaxAnisotropy");
	AddMenuText(Interface.GetPositionPercent(X - SidePadding, Y), L"Anisotropic Filtering", _Interface::FONT_SMALL, -1, gui::EGUIA_LOWERRIGHT);
	gui::IGUIComboBox *Anisotropy = irrGUI->addComboBox(Interface.GetRectPercent(X + SidePadding, Y, 210, 45), CurrentLayout, VIDEO_ANISOTROPY);

	// Populate anisotropy list
	Anisotropy->addItem(core::stringw(0).c_str());
	for(int i = 0, Level = 1; Level <= MaxAnisotropy; i++, Level <<= 1) {
		Anisotropy->addItem(core::stringw(Level).c_str());
		if(Config.AnisotropicFiltering == Level)
			Anisotropy->setSelected(i+1);
	}

	// Anti-aliasing
	Y += SpacingY;
	AddMenuText(Interface.GetPositionPercent(X - SidePadding, Y), L"MSAA", _Interface::FONT_SMALL, -1, gui::EGUIA_LOWERRIGHT);
	gui::IGUIComboBox *Antialiasing = irrGUI->addComboBox(Interface.GetRectPercent(X + SidePadding, Y, 210, 45), CurrentLayout, VIDEO_ANTIALIASING);

	// Populate anti-aliasing list
	Antialiasing->addItem(core::stringw(0).c_str());
	for(int i = 0, Level = 2; Level <= 8; i++, Level <<= 1) {
		Antialiasing->addItem(core::stringw(Level).c_str());
		if(Config.AntiAliasing == Level)
			Antialiasing->setSelected(i+1);
	}

	// Warning
	AddMenuText(Interface.GetPositionPercent(0.5, 0.8), L"Changes are applied after restart", _Interface::FONT_SMALL, -1);

	// Save and cancel buttons
	AddMenuButton(Interface.GetRightRectPercent(0.5 - SidePadding, 0.9, BUTTON_SMALL_SIZE_X, BUTTON_SMALL_SIZE_Y), VIDEO_SAVE, L"Save", _Interface::IMAGE_BUTTON_SMALL);
	AddMenuButton(Interface.GetRectPercent(0.5 + SidePadding, 0.9, BUTTON_SMALL_SIZE_X, BUTTON_SMALL_SIZE_Y), VIDEO_CANCEL, L"Cancel", _Interface::IMAGE_BUTTON_SMALL);

	// Play sound
	Interface.PlaySound(_Interface::SOUND_CONFIRM);

	PreviousState = State;
	State = STATE_OPTIONS_VIDEO;
}

// Create the audio options menu
void _Menu::InitAudioOptions() {
	ClearCurrentLayout();
	float X = 0.5;
	float Y = 0.45;
	float SidePadding = 0.01;
	float SpacingY = 0.05;

	// Title
	AddMenuText(Interface.GetPositionPercent(0.5, 0.1), L"Audio");

	// Enabled
	AddMenuText(Interface.GetPositionPercent(X - SidePadding, Y), L"Audio Enabled", _Interface::FONT_SMALL, -1, gui::EGUIA_LOWERRIGHT);
	irrGUI->addCheckBox(Config.SoundVolume == 1.0f, Interface.GetRectPercent(X + SidePadding, Y, 32, 32), CurrentLayout, AUDIO_ENABLED);
	Y += SpacingY;

	// Player sounds
	AddMenuText(Interface.GetPositionPercent(X - SidePadding, Y), L"Player Sounds", _Interface::FONT_SMALL, -1, gui::EGUIA_LOWERRIGHT);
	irrGUI->addCheckBox(Config.PlayerSounds, Interface.GetRectPercent(X + SidePadding, Y, 32, 32), CurrentLayout, AUDIO_PLAYERSOUNDS);

	// Save and cancel buttons
	AddMenuButton(Interface.GetRightRectPercent(0.5 - SidePadding, 0.9, BUTTON_SMALL_SIZE_X, BUTTON_SMALL_SIZE_Y), AUDIO_SAVE, L"Save", _Interface::IMAGE_BUTTON_SMALL);
	AddMenuButton(Interface.GetRectPercent(0.5 + SidePadding, 0.9, BUTTON_SMALL_SIZE_X, BUTTON_SMALL_SIZE_Y), AUDIO_CANCEL, L"Cancel", _Interface::IMAGE_BUTTON_SMALL);

	// Play sound
	Interface.PlaySound(_Interface::SOUND_CONFIRM);

	PreviousState = State;
	State = STATE_OPTIONS_AUDIO;
}

// Create the control options menu
void _Menu::InitControlOptions() {
	ClearCurrentLayout();
	float X = 0.5;
	float Y = 0.21;
	float KeySpacingY = 0.07;
	float SpacingY = 0.05;
	float SidePadding = 0.01;
	wchar_t Buffer[32];

	// Title
	AddMenuText(Interface.GetPositionPercent(0.5, 0.1), L"Controls");

	// Create the key buttons
	KeyButton = nullptr;
	for(int i = 0; i <= _Actions::RESET; i++) {
		CurrentKeys[KeyMapOrder[i]] = Actions.GetInputForAction(_Input::KEYBOARD, KeyMapOrder[i]);
		AddMenuText(Interface.GetPositionPercent(X - SidePadding, Y), core::stringw(Actions.GetName(KeyMapOrder[i]).c_str()).c_str(), _Interface::FONT_SMALL, -1, gui::EGUIA_LOWERRIGHT);
		gui::IGUIButton *Button = AddMenuButton(Interface.GetRectPercent(X + SidePadding, Y, 190, 64), CONTROLS_KEYMAP + KeyMapOrder[i], core::stringw(Input.GetKeyName(CurrentKeys[KeyMapOrder[i]])).c_str(), _Interface::IMAGE_BUTTON_KEY);
		Button->setOverrideFont(Interface.Fonts[_Interface::FONT_SMALL]);

		Y += KeySpacingY;
	}

	// Invert mouse
	Y += SpacingY / 4;
	AddMenuText(Interface.GetPositionPercent(X - SidePadding, Y), L"Invert Mouse Y", _Interface::FONT_SMALL, -1, gui::EGUIA_LOWERRIGHT);
	irrGUI->addCheckBox(Config.InvertMouse, Interface.GetRectPercent(X + SidePadding, Y, 32, 32), CurrentLayout, CONTROLS_INVERTMOUSE);

	// Invert Gamepad Y
	Y += SpacingY;
	AddMenuText(Interface.GetPositionPercent(X - SidePadding, Y), L"Invert Gamepad Y", _Interface::FONT_SMALL, -1, gui::EGUIA_LOWERRIGHT);
	irrGUI->addCheckBox(Config.InvertGamepadY, Interface.GetRectPercent(X + SidePadding, Y, 32, 32), CurrentLayout, CONTROLS_INVERTGAMEPADY);

	// Add mouse sensitivity
	Y += SpacingY;
	AddMenuText(Interface.GetPositionPercent(X - SidePadding, Y), L"Mouse Sensitivity", _Interface::FONT_SMALL, -1, gui::EGUIA_LOWERRIGHT);
	swprintf(Buffer, 32, L"%.3f", Config.MouseSensitivity);
	gui::IGUIEditBox *EditMouseSensitivity = irrGUI->addEditBox(Buffer, Interface.GetRectPercent(X + SidePadding, Y, 150, 48), true, CurrentLayout, CONTROLS_SENSITIVITY);
	EditMouseSensitivity->setMax(32);

	// Add deadzone
	Y += SpacingY;
	AddMenuText(Interface.GetPositionPercent(X - SidePadding, Y), L"Gamepad Deadzone", _Interface::FONT_SMALL, -1, gui::EGUIA_LOWERRIGHT);
	swprintf(Buffer, 32, L"%.3f", Config.DeadZone);
	gui::IGUIEditBox *EditDeadZone = irrGUI->addEditBox(Buffer, Interface.GetRectPercent(X + SidePadding, Y, 150, 48), true, CurrentLayout, CONTROLS_DEADZONE);
	EditDeadZone->setMax(32);

	// Save and cancel buttons
	AddMenuButton(Interface.GetRightRectPercent(0.5 - SidePadding, 0.9, BUTTON_SMALL_SIZE_X, BUTTON_SMALL_SIZE_Y), CONTROLS_SAVE, L"Save", _Interface::IMAGE_BUTTON_SMALL);
	AddMenuButton(Interface.GetRectPercent(0.5 + SidePadding, 0.9, BUTTON_SMALL_SIZE_X, BUTTON_SMALL_SIZE_Y), CONTROLS_CANCEL, L"Cancel", _Interface::IMAGE_BUTTON_SMALL);

	// Play sound
	Interface.PlaySound(_Interface::SOUND_CONFIRM);

	PreviousState = State;
	State = STATE_OPTIONS_CONTROLS;
}

// Init play GUI
void _Menu::InitPlay() {
	Interface.ChangeSkin(_Interface::SKIN_GAME);
	ClearCurrentLayout();

	Graphics.SetClearColor(Level.ClearColor);
	Input.SetMouseLocked(true);
	LoseMessage = "You died!";

	PreviousState = State;
	State = STATE_NONE;
}

// Create the pause menu
void _Menu::InitPause() {
	ClearCurrentLayout();

	gui::IGUIButton *ButtonResume = AddMenuButton(Interface.GetCenteredRectPercent(0.5, 0.3, BUTTON_SIZE_X, BUTTON_SIZE_Y), PAUSE_RESUME, L"Resume");
	gui::IGUIButton *ButtonSaveReplay = AddMenuButton(Interface.GetCenteredRectPercent(0.5, 0.4, BUTTON_SIZE_X, BUTTON_SIZE_Y), PAUSE_SAVEREPLAY, L"Save Replay");
	AddMenuButton(Interface.GetCenteredRectPercent(0.5, 0.5, BUTTON_SIZE_X, BUTTON_SIZE_Y), PAUSE_RESTART, L"Restart Level");
	AddMenuButton(Interface.GetCenteredRectPercent(0.5, 0.6, BUTTON_SIZE_X, BUTTON_SIZE_Y), PAUSE_OPTIONS, L"Options");
	AddMenuButton(Interface.GetCenteredRectPercent(0.5, 0.7, BUTTON_SIZE_X, BUTTON_SIZE_Y), PAUSE_QUITLEVEL, L"Quit Level");

	// Disable buttons for replay validation
	if(PlayState.ReplayInputs) {
		ButtonSaveReplay->setEnabled(false);
		if(PlayState.InputReplay->ReplayStopped())
			ButtonResume->setEnabled(false);
	}

	Input.SetMouseLocked(false);

	PreviousState = State;
	State = STATE_PAUSED;
}

// Create the save replay GUI
void _Menu::InitSaveReplay() {
	Interface.ChangeSkin(_Interface::SKIN_MENU);
	ClearCurrentLayout();

	// Text box
	AddMenuText(Interface.GetPositionPercent(0.5, 0.35), L"Enter a name", _Interface::FONT_MEDIUM);
	gui::IGUIEditBox *EditName = irrGUI->addEditBox(L"", Interface.GetCenteredRectPercent(0.5, 0.43, 520, 64), true, CurrentLayout, SAVEREPLAY_NAME);

	// Save and cancel buttons
	float SidePadding = 0.01;
	AddMenuButton(Interface.GetRightRectPercent(0.5 - SidePadding, 0.55, BUTTON_SMALL_SIZE_X, BUTTON_SMALL_SIZE_Y), SAVEREPLAY_SAVE, L"Save", _Interface::IMAGE_BUTTON_SMALL);
	AddMenuButton(Interface.GetRectPercent(0.5 + SidePadding, 0.55, BUTTON_SMALL_SIZE_X, BUTTON_SMALL_SIZE_Y), SAVEREPLAY_CANCEL, L"Cancel", _Interface::IMAGE_BUTTON_SMALL);

	// Set focus
	irrGUI->setFocus(EditName);
	EditName->setMax(32);

	PreviousState = State;
	State = STATE_SAVEREPLAY;
}

// Create the lose screen
void _Menu::InitLose() {
	Interface.Clear();
	ClearCurrentLayout();

	// Add buttons
	float Spacing = (BUTTON_MEDIUM_SIZE_X + 20) * Interface.GetUIScale() / irrDriver->getScreenSize().Width;
	gui::IGUIButton *Button = AddMenuButton(Interface.GetCenteredRectPercent(0.5 - Spacing, 0.7, BUTTON_MEDIUM_SIZE_X, BUTTON_MEDIUM_SIZE_Y), LOSE_RESTARTLEVEL, L"Retry Level", _Interface::IMAGE_BUTTON_MEDIUM);
	gui::IGUIButton *ButtonSaveReplay = AddMenuButton(Interface.GetCenteredRectPercent(0.5, 0.7, BUTTON_MEDIUM_SIZE_X, BUTTON_MEDIUM_SIZE_Y), LOSE_SAVEREPLAY, L"Save Replay", _Interface::IMAGE_BUTTON_MEDIUM);
	AddMenuButton(Interface.GetCenteredRectPercent(0.5 + Spacing, 0.7, BUTTON_MEDIUM_SIZE_X, BUTTON_MEDIUM_SIZE_Y), LOSE_MAINMENU, L"Main Menu", _Interface::IMAGE_BUTTON_MEDIUM);

	// Disable save replay if playing from replay
	if(PlayState.ReplayInputs)
		ButtonSaveReplay->setEnabled(false);

	// Set cursor position to retry level
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
	WinStats = &Save.LevelStats[Level.LevelName];

	// Clear interface
	ClearCurrentLayout();

	// Calculate layout
	float X = 0.5;
	float Y = 0.88;
	float Spacing = (BUTTON_MEDIUM_SIZE_X + 20) * Interface.GetUIScale() / irrDriver->getScreenSize().Width;
	float Padding = 10 * Interface.GetUIScale() / irrDriver->getScreenSize().Width;

	// Add buttons
	AddMenuButton(Interface.GetRightRectPercent(X - Padding - Spacing, Y, BUTTON_MEDIUM_SIZE_X, BUTTON_MEDIUM_SIZE_Y), WIN_RESTARTLEVEL, L"Retry Level", _Interface::IMAGE_BUTTON_MEDIUM);
	gui::IGUIButton *ButtonNextLevel = AddMenuButton(Interface.GetRightRectPercent(X - Padding, Y, BUTTON_MEDIUM_SIZE_X, BUTTON_MEDIUM_SIZE_Y), WIN_NEXTLEVEL, L"Next Level", _Interface::IMAGE_BUTTON_MEDIUM);
	gui::IGUIButton *ButtonSaveReplay = AddMenuButton(Interface.GetRectPercent(X + Padding, Y, BUTTON_MEDIUM_SIZE_X, BUTTON_MEDIUM_SIZE_Y), WIN_SAVEREPLAY, L"Save Replay", _Interface::IMAGE_BUTTON_MEDIUM);
	AddMenuButton(Interface.GetRectPercent(X + Padding + Spacing, Y, BUTTON_MEDIUM_SIZE_X, BUTTON_MEDIUM_SIZE_Y), WIN_MAINMENU, L"Main Menu", _Interface::IMAGE_BUTTON_MEDIUM);

	// Set state of next level button
	if(!Campaign.GetNextLevel(PlayState.CurrentCampaign, PlayState.CampaignLevel, false))
		ButtonNextLevel->setEnabled(false);

	// Disable save replay if playing from replay
	if(PlayState.ReplayInputs)
		ButtonSaveReplay->setEnabled(false);

	// Set cursor on next level button
	Input.SetMouseLocked(false);
	core::vector2di Position = ButtonNextLevel->getAbsolutePosition().getCenter();
	irrDevice->getCursorControl()->setPosition(Position.X, Position.Y);

	PreviousState = State;
	State = STATE_WIN;
}

// Saves a replay
void _Menu::SaveReplay() {

	gui::IGUIEditBox *EditName = static_cast<gui::IGUIEditBox *>(CurrentLayout->getElementFromId(SAVEREPLAY_NAME));
	if(EditName != nullptr) {
		irr::core::stringc ReplayTitle(EditName->getText());
		Replay.SaveReplay(ReplayTitle.c_str(), false, PreviousState == STATE_WIN);
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

	video::SColor Gray(255, 128, 128, 128);
	video::SColor White(255, 255, 255, 255);

	// Draw level tooltip
	switch(State) {
		case STATE_LEVELS:
			if(HighlightedLevel != -1) {
				char Buffer[256];
				const _LevelStat *Stats = LevelStats[HighlightedLevel];
				const std::string &NiceName = Campaign.GetLevelNiceName(CampaignIndex, HighlightedLevel);

				// Get text dimensions
				core::dimension2du NiceNameSize = Interface.Fonts[_Interface::FONT_MEDIUM]->getDimension(core::stringw(NiceName.c_str()).c_str());

				// Get box position
				int Width = NiceNameSize.Width + STATS_PADDING * 2 * Interface.GetUIScale();
				int Height = (STATS_MIN_HEIGHT + STATS_PADDING) * Interface.GetUIScale(), X, Y;
				int Left = (int)Input.GetMouseX() + STATS_MOUSE_OFFSETX;
				int Top = (int)Input.GetMouseY() + STATS_MOUSE_OFFSETY;

				if(Width < STATS_MIN_WIDTH * Interface.GetUIScale())
					Width = STATS_MIN_WIDTH * Interface.GetUIScale();

				// Cap limits
				if(Top < STATS_PADDING)
					Top = STATS_PADDING;
				if(Left + Width > (int)irrDriver->getScreenSize().Width - 10)
					Left -= Width + 35 * Interface.GetUIScale();

				// Draw box
				Interface.DrawTextBox(Left + Width/2, Top + Height/2, Width, Height);
				X = Left + Width/2;
				Y = Top + STATS_PADDING * 0.75 * Interface.GetUIScale();
				int SidePadding = 10 * Interface.GetUIScale();
				if(Stats->Unlocked) {

					// Level nice name
					Interface.RenderText(NiceName.c_str(), X, Y, _Interface::ALIGN_CENTER, _Interface::FONT_MEDIUM);
					Y += 65 * Interface.GetUIScale();

					// Play time
					Interface.RenderText("Play time", X - SidePadding, Y, _Interface::ALIGN_RIGHT);
					Interface.ConvertSecondsToString(Stats->PlayTime, Buffer);
					Interface.RenderText(Buffer, X + SidePadding, Y, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, White);

					// Load count
					Y += 32 * Interface.GetUIScale();
					Interface.RenderText("Plays", X - SidePadding, Y, _Interface::ALIGN_RIGHT);
					sprintf(Buffer, "%d", Stats->LoadCount);
					Interface.RenderText(Buffer, X + SidePadding, Y, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, White);

					// Win count
					Y += 32 * Interface.GetUIScale();
					Interface.RenderText("Wins", X - SidePadding, Y, _Interface::ALIGN_RIGHT);
					sprintf(Buffer, "%d", Stats->WinCount);
					Interface.RenderText(Buffer, X + SidePadding, Y, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, White);

					// Lose count
					Y += 32 * Interface.GetUIScale();
					Interface.RenderText("Deaths", X - SidePadding, Y, _Interface::ALIGN_RIGHT);
					sprintf(Buffer, "%d", Stats->LoseCount);
					Interface.RenderText(Buffer, X + SidePadding, Y, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, White);

					// Scores
					if(Stats->HighScores.size() > 0) {

						// High scores
						int HighX = Left + Width/2 - 190 * Interface.GetUIScale();
						int HighY = Y + 50 * Interface.GetUIScale();
						int ScorePadding = 60 * Interface.GetUIScale();
						int DatePadding = 210 * Interface.GetUIScale();

						// Draw header
						Interface.RenderText("#", HighX, HighY, _Interface::ALIGN_LEFT);
						Interface.RenderText("Time", HighX + ScorePadding, HighY, _Interface::ALIGN_LEFT);
						Interface.RenderText("Date", HighX + DatePadding, HighY, _Interface::ALIGN_LEFT);
						HighY += 32 * Interface.GetUIScale();

						for(size_t i = 0; i < Stats->HighScores.size(); i++) {
							video::SColor TextColor = Gray;
							if(i == 0)
								TextColor = White;

							// Number
							char SmallBuffer[32];
							sprintf(SmallBuffer, "%d", (int)i+1);
							Interface.RenderText(SmallBuffer, HighX, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, TextColor);

							// Time
							Interface.ConvertSecondsToString(Stats->HighScores[i].Time, Buffer);
							Interface.RenderText(Buffer, HighX + ScorePadding, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, TextColor);

							// Date
							char DateString[32];
							strftime(DateString, 32, "%Y-%m-%d", localtime(&Stats->HighScores[i].DateStamp));
							Interface.RenderText(DateString, HighX + DatePadding, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, TextColor);

							HighY += 32 * Interface.GetUIScale();
						}
					}
				}
				else {

					// Locked
					Interface.RenderText("Level Locked", X, Y, _Interface::ALIGN_CENTER, _Interface::FONT_MEDIUM);
				}
			}
		break;
		case STATE_REPLAYS:

			// Show selected replay
			if(SelectedElement) {
				core::recti SelectedRect = SelectedElement->getRelativePosition();
				irrDriver->draw2DImage(Interface.Images[_Interface::IMAGE_SELECTED], SelectedRect, core::recti(0, 0, 128, 128), 0, 0, true);
			}

			// Show replay stats
			if(HighlightedLevel != -1) {
				const _ReplayInfo &ReplayInfo = ReplayFiles[HighlightedLevel];

				// Get text dimensions
				core::dimension2du DescriptionSize = Interface.Fonts[_Interface::FONT_MEDIUM]->getDimension(core::stringw(ReplayInfo.Description.c_str()).c_str());
				core::dimension2du NiceNameSize = Interface.Fonts[_Interface::FONT_SMALL]->getDimension(core::stringw(ReplayInfo.LevelNiceName.c_str()).c_str());

				// Get box position
				int Width = std::max(DescriptionSize.Width, NiceNameSize.Width) + STATS_PADDING * 2 * Interface.GetUIScale();
				int Height = (240 + STATS_PADDING) * Interface.GetUIScale(), X, Y;
				int Left = (int)Input.GetMouseX() - Width / 2;
				int Top = (int)Input.GetMouseY() - Height - 35 * Interface.GetUIScale();

				if(Width < STATS_MIN_WIDTH * Interface.GetUIScale())
					Width = STATS_MIN_WIDTH * Interface.GetUIScale();

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
				Y = Top + STATS_PADDING * 0.75 * Interface.GetUIScale();

				// Replay description
				Interface.RenderText(ReplayInfo.Description.c_str(), X, Y, _Interface::ALIGN_CENTER, _Interface::FONT_MEDIUM, White);
				Y += 60 * Interface.GetUIScale();

				// Replay description
				Interface.RenderText(ReplayInfo.LevelNiceName.c_str(), X, Y, _Interface::ALIGN_CENTER, _Interface::FONT_SMALL, White);
				Y += 40 * Interface.GetUIScale();

				// Date recorded
				Interface.RenderText(ReplayInfo.Date.c_str(), X, Y, _Interface::ALIGN_CENTER, _Interface::FONT_SMALL, Gray);
				Y += 60 * Interface.GetUIScale();

				// Finish time
				video::SColor Color = Gray;
				if(ReplayInfo.Won || ReplayInfo.Autosave)
					Color = White;
				Interface.RenderText(ReplayInfo.FinishTime.c_str(), X, Y, _Interface::ALIGN_CENTER, _Interface::FONT_MEDIUM, Color);
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

	// Calculate layout
	int X = Interface.CenterX;
	int Y = Interface.CenterY;
	float SidePadding = 10 * Interface.GetUIScale();

	// Draw box
	Interface.DrawTextBox(Interface.CenterX, Interface.CenterY, WIN_WIDTH * Interface.GetUIScale(), WIN_HEIGHT * Interface.GetUIScale());

	// Draw header
	Y -= (WIN_HEIGHT / 2 - 10) * Interface.GetUIScale();
	Interface.RenderText("Level Completed!", X, Y, _Interface::ALIGN_CENTER, _Interface::FONT_LARGE);

	// Draw time
	Y += 130 * Interface.GetUIScale();
	Interface.RenderText("Your Time", X - SidePadding, Y, _Interface::ALIGN_RIGHT, _Interface::FONT_MEDIUM);
	Interface.RenderText(TimeString, X + SidePadding, Y, _Interface::ALIGN_LEFT, _Interface::FONT_MEDIUM);

	// Best time
	Y += 50 * Interface.GetUIScale();
	if(WinStats->HighScores.size() > 0) {
		Interface.RenderText("Best Time", X - SidePadding, Y, _Interface::ALIGN_RIGHT, _Interface::FONT_MEDIUM);
		Interface.ConvertSecondsToString(WinStats->HighScores[0].Time, Buffer);
		Interface.RenderText(Buffer, X + SidePadding, Y, _Interface::ALIGN_LEFT, _Interface::FONT_MEDIUM);
	}

	// High scores
	int HighX = Interface.CenterX - 190 * Interface.GetUIScale();
	int HighY = Y + 70 * Interface.GetUIScale();

	// Draw header
	video::SColor Gray(255, 128, 128, 128);
	video::SColor White(255, 255, 255, 255);
	int ScorePadding = 60 * Interface.GetUIScale();
	int DatePadding = 210 * Interface.GetUIScale();
	Interface.RenderText("#", HighX, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));
	Interface.RenderText("Time", HighX + ScorePadding, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));
	Interface.RenderText("Date", HighX + DatePadding, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, video::SColor(255, 255, 255, 255));
	HighY += 32 * Interface.GetUIScale();
	for(uint32_t i = 0; i < WinStats->HighScores.size(); i++) {

		// Set color
		video::SColor Color = Gray;
		if(i == (uint32_t)PlayState.HighScoreIndex)
			Color = White;

		// Number
		char SmallBuffer[32];
		sprintf(SmallBuffer, "%d", i+1);
		Interface.RenderText(SmallBuffer, HighX, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, Color);

		// Time
		Interface.ConvertSecondsToString(WinStats->HighScores[i].Time, Buffer);
		Interface.RenderText(Buffer, HighX + ScorePadding, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, Color);

		// Date
		char DateString[32];
		strftime(DateString, 32, "%Y-%m-%d", localtime(&WinStats->HighScores[i].DateStamp));
		Interface.RenderText(DateString, HighX + DatePadding, HighY, _Interface::ALIGN_LEFT, _Interface::FONT_SMALL, Color);

		HighY += 32 * Interface.GetUIScale();
	}
}

// Draw the lose screen
void _Menu::DrawLoseScreen() {

	// Draw header
	int X = Interface.CenterX;
	int Y = Interface.CenterY / 3;
	Interface.RenderText(LoseMessage.c_str(), X, Y, _Interface::ALIGN_CENTER, _Interface::FONT_LARGE);
}

// Cancels the key bind state
void _Menu::CancelKeyBind() {
	KeyButton->setText(KeyButtonOldText.c_str());
	KeyButton = nullptr;
}

// Launchs a level
void _Menu::LaunchLevel() {

	_LevelStat &Stats = Save.LevelStats[Campaign.GetCampaign(CampaignIndex).Levels[SelectedLevel].File];
	if(Stats.Unlocked == 0)
		return;

	PlayState.SetTestLevel("");
	PlayState.SetValidateReplay("");
	PlayState.SetCampaign(CampaignIndex);
	PlayState.SetCampaignLevel(SelectedLevel);
	Framework.ChangeState(&PlayState);
}

// Scroll the replay list up
void _Menu::ReplayScrollUp() {
	if(StartOffset >= REPLAY_SCROLL_AMOUNT)
		StartOffset -= REPLAY_SCROLL_AMOUNT;

	InitReplays(false);
}

// Scroll the replay list down
void _Menu::ReplayScrollDown() {
	if(StartOffset < ReplayFiles.size() - REPLAY_SCROLL_AMOUNT)
		StartOffset += REPLAY_SCROLL_AMOUNT;

	InitReplays(false);
}

// Launchs a replay from a list item
void _Menu::LaunchReplay() {

	// Get replay file
	if(SelectedLevel >= 0) {

		// Load replay
		ViewReplayState.SetCurrentReplay(ReplayFiles[SelectedLevel].Filename);
		Framework.ChangeState(&ViewReplayState);
	}
}

// Launch replay in play mode
void _Menu::ValidateReplay() {

	// Get replay file
	if(SelectedLevel >= 0 && ReplayFiles[SelectedLevel].Platform == PLATFORM) {

		// Load replay
		PlayState.SetValidateReplay(ReplayFiles[SelectedLevel].Filename);
		Framework.ChangeState(&PlayState);
	}
}

// Add a regular menu button
gui::IGUIButton *_Menu::AddMenuButton(const irr::core::recti &Rectangle, int ID, const wchar_t *Text, _Interface::ImageType ButtonImage) {
	gui::IGUIButton *Button = irrGUI->addButton(Rectangle, CurrentLayout, ID, Text);
	Button->setImage(Interface.Images[ButtonImage]);
	Button->setUseAlphaChannel(true);
	Button->setDrawBorder(false);
	Button->setScaleImage(true);
	Button->setOverrideFont(Interface.Fonts[_Interface::FONT_BUTTON]);

	return Button;
}

// Add menu text label
gui::IGUIStaticText *_Menu::AddMenuText(const core::position2di &CenterPosition, const wchar_t *Text, _Interface::FontType Font, int ID, gui::EGUI_ALIGNMENT HorizontalAlign) {

	// Get text dimensions
	core::dimension2du Size = Interface.Fonts[Font]->getDimension(Text);
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
	NewText->setOverrideFont(Interface.Fonts[Font]);
	//NewText->setOverrideColor(video::SColor(255, 255, 255, 255));

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
