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
#include <state.h>
#include <actions.h>
#include <interface.h>
#include <IGUIButton.h>
#include <vector>

// Forward Declarations
struct _LevelStat;

// Replay info struct
struct _ReplayInfo {
	std::string Filename;
	std::string LevelName;
	std::string LevelNiceName;
	std::string Description;
	std::string FinishTime;
	std::string Date;
	int Timestamp;
	char Platform;
	bool Autosave;
	bool Won;
};

// Classes
class _Menu {

	friend class _PlayState;
	friend class _NullState;

	public:

		enum MenuType {
			STATE_NONE,
			STATE_MAIN,
			STATE_CAMPAIGNS,
			STATE_LEVELS,
			STATE_REPLAYS,
			STATE_OPTIONS,
			STATE_OPTIONS_GAMEPLAY,
			STATE_OPTIONS_VIDEO,
			STATE_OPTIONS_AUDIO,
			STATE_OPTIONS_CONTROLS,
			STATE_PAUSED,
			STATE_SAVEREPLAY,
			STATE_LOSE,
			STATE_WIN
		};

		enum ReplaySortType {
			SORT_TIMESTAMP,
			SORT_LEVELNAME,
			SORT_MAX,
		};

		_Menu() {
			State = PreviousState = STATE_NONE;
			FirstStateLoad = true;
			CurrentLayout = 0;
			SelectedElement = nullptr;
			StartOffset = 0;
			ReplaySort = SORT_TIMESTAMP;
		}

		void InitMain();
		void InitCampaigns();
		void InitLevels();
		void InitReplays(bool LoadReplays);
		void InitOptions();
		void InitGameplayOptions();
		void InitVideoOptions();
		void InitAudioOptions();
		void InitControlOptions();

		void InitPlay();
		void InitLose();
		void InitWin();
		void InitPause();
		void InitSaveReplay();
		void SaveReplay();

		bool HandleKeyPress(int Key);
		bool HandleAction(int InputType, int Action, float Value);
		void HandleGUI(irr::gui::EGUI_EVENT_TYPE EventType, irr::gui::IGUIElement *Element);

		void Update(float FrameTime);
		void Draw();

		void DrawLoseScreen();
		void DrawWinScreen();
		void ClearCurrentLayout();
		void SetLoseMessage(const std::string &Message) { LoseMessage = Message; }

	private:

		irr::gui::IGUIButton *AddMenuButton(const irr::core::recti &Rectangle, int ID, const wchar_t *Text, _Interface::ImageType ButtonImage=_Interface::IMAGE_BUTTON_BIG);
		irr::gui::IGUIStaticText *AddMenuText(const irr::core::position2di &CenterPosition, const wchar_t *Text, _Interface::FontType Type=_Interface::FONT_LARGE, int ID=-1, irr::gui::EGUI_ALIGNMENT HorizontalAlign=irr::gui::EGUIA_CENTER);

		void CancelKeyBind();
		void LaunchReplay();
		void ValidateReplay();
		void LaunchLevel();
		void ReplayScrollUp();
		void ReplayScrollDown();

		MenuType State, PreviousState;
		bool FirstStateLoad;
		irr::gui::IGUIElement *CurrentLayout;

		// Replays
		std::vector<_ReplayInfo> ReplayFiles;
		irr::gui::IGUIElement *SelectedElement;
		int ReplaySort;
		uint32_t StartOffset;

		// Campaigns
		int CampaignIndex;
		int SelectedLevel;
		int HighlightedLevel;
		float DoubleClickTimer;

		// Level info
		std::vector<const _LevelStat *> LevelStats;
		const _LevelStat *WinStats;
		std::string LoseMessage;

		// Key mapping
		int CurrentKeys[_Actions::COUNT];
		irr::gui::IGUIButton *KeyButton;
		std::wstring KeyButtonOldText;

};

extern _Menu Menu;
