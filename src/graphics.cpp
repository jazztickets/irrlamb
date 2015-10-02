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
#include <graphics.h>
#include <save.h>
#include <globals.h>
#include <interface.h>
#include <input.h>
#include <log.h>
#include <fader.h>
#include <config.h>
#include <irrlicht.h>
#include <irrb/CIrrBMeshFileLoader.h>
#include <string>
#include <sstream>

using namespace irr;

_Graphics Graphics;

// Initializes the graphics system
int _Graphics::Init(int Width, int Height, bool Fullscreen, video::E_DRIVER_TYPE DriverType, IEventReceiver *EventReceiver) {
	ShowCursor = true;
	ShadersSupported = false;
	CustomMaterial[0] = -1;
	CustomMaterial[1] = -1;
	LightCount = 0;

	// irrlicht parameters
	SIrrlichtCreationParameters Parameters;
	Parameters.DriverType = DriverType;
	Parameters.Fullscreen = Fullscreen;
	Parameters.Bits = 32;
	Parameters.Vsync = Config.Vsync;
	Parameters.Stencilbuffer = Config.Shadows;
	Parameters.AntiAlias = Config.AntiAliasing;
	Parameters.WindowSize.set(Width, Height);

	// Create the irrlicht device
	irrDevice = createDeviceEx(Parameters);
	if(irrDevice == nullptr)
		return 0;

	irrDevice->setWindowCaption(L"irrlamb");
	irrDevice->setEventReceiver(EventReceiver);
	irrDevice->getCursorControl()->setVisible(false);
	irrDevice->getLogger()->setLogLevel(ELL_ERROR);
	irrDevice->setResizable(false);

	// Save off global pointers
	irrDriver = irrDevice->getVideoDriver();
	irrScene = irrDevice->getSceneManager();
	irrGUI = irrDevice->getGUIEnvironment();
	irrFile = irrDevice->getFileSystem();
	irrTimer = irrDevice->getTimer();

	VideoModes.clear();

	// Generate a list of video modes
	video::IVideoModeList *VideoModeList = irrDevice->getVideoModeList();
	_VideoMode VideoMode;
	for(int i = 0; i < VideoModeList->getVideoModeCount(); i++) {
		VideoMode.Width = VideoModeList->getVideoModeResolution(i).Width;
		VideoMode.Height = VideoModeList->getVideoModeResolution(i).Height;
		VideoMode.BPP = VideoModeList->getVideoModeDepth(i);

		// Add the video mode
		if((VideoMode.BPP == 32 || VideoMode.BPP == 24) && VideoMode.Width >= 800) {
			std::wstringstream Stream;
			Stream << VideoMode.Width << " x " << VideoMode.Height;
			VideoMode.String = Stream.str();
			VideoModes.push_back(VideoMode);
		}
	}

	//irrScene->setShadowColor(video::SColor(150, 0, 0, 0));
	DrawScene = true;
	ScreenshotRequested = false;

	// Load custom loader
	scene::CIrrBMeshFileLoader *Loader = new scene::CIrrBMeshFileLoader(irrScene, irrFile);
	irrScene->addExternalMeshLoader(Loader);
	Loader->drop();

	// Check for shader support
	if(irrDriver->queryFeature(video::EVDF_PIXEL_SHADER_1_1)
	&& irrDriver->queryFeature(video::EVDF_ARB_FRAGMENT_PROGRAM_1)
	&& irrDriver->queryFeature(video::EVDF_VERTEX_SHADER_1_1)
	&& irrDriver->queryFeature(video::EVDF_ARB_VERTEX_PROGRAM_1)) {
		ShadersSupported = true;

		if(Config.Shaders)
			LoadShaders();
	}
	else {
		Log.Write("Shaders not supported.");
		Config.Shaders = false;
	}

	Input.SetMouseX(Width / 2.0f);
	Input.SetMouseY(Height / 2.0f);

	irrDevice->getCursorControl()->setPosition(Width / 2, Height / 2);

	return 1;
}

// Closes the graphics system
int _Graphics::Close() {

	// Close irrlicht
	irrDevice->drop();

	return 1;
}

// Load all shaders
void _Graphics::LoadShaders() {

	// Create shader materials
	if(ShadersSupported) {

		ShaderCallback *Shader = new ShaderCallback();
		if(CustomMaterial[0] == -1) {
			CustomMaterial[0] = irrDriver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
				"shaders/lighting.vert", "main", video::EVST_VS_1_1,
				"shaders/lighting.frag", "main", video::EPST_PS_1_1,
				Shader,	video::EMT_SOLID);
		}
		if(CustomMaterial[1] == -1) {
			CustomMaterial[1] = irrDriver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
				"shaders/lighting.vert", "main", video::EVST_VS_1_1,
				"shaders/lighting.frag", "main", video::EPST_PS_1_1,
				Shader,	video::EMT_TRANSPARENT_ALPHA_CHANNEL);
		}
		Shader->drop();
	}
}

// Erases the buffer and sets irrlicht up for the next frame
void _Graphics::BeginFrame() {
	irrDriver->beginScene(true, true, ClearColor);

	if(DrawScene)
		irrScene->drawAll();
}

// Draws the buffer to the screen
void _Graphics::EndFrame() {

	// Draw cursor
	if(ShowCursor)
		Interface.DrawImage(_Interface::IMAGE_MOUSECURSOR, (int)Input.GetMouseX(), (int)Input.GetMouseY(), 16, 16);

	Fader.Draw();
	irrDriver->endScene();

	// Handle screenshots
	if(ScreenshotRequested)
		CreateScreenshot();
}

// Returns the index of the current video mode
std::size_t _Graphics::GetCurrentVideoModeIndex() {

	// Find the video mode
	for(std::size_t i = 0; i < VideoModes.size(); i++) {
		if(Config.ScreenWidth == VideoModes[i].Width && Config.ScreenHeight == VideoModes[i].Height)
			return i;
	}

	return 0;
}

// Request screenshot
void _Graphics::SaveScreenshot() {
	ScreenshotRequested = 1;
}

// Create the screenshot
void _Graphics::CreateScreenshot() {

	// Get time
	time_t Now;
	time(&Now);

	// Get filename
	char Filename[32];
	strftime(Filename, 32, "%Y%m%d-%H%M%S.jpg", localtime(&Now));

	// Create image
	video::IImage *Image = irrDriver->createScreenShot();
	std::string FilePath = Save.ScreenshotsPath + Filename;
	irrDriver->writeImageToFile(Image, FilePath.c_str());
	Image->drop();

	// Drop request
	ScreenshotRequested = 0;
}

// Update the internal light count variable
void _Graphics::SetLightCount() {
	core::array<scene::ISceneNode *> LightNodes;
	irrScene->getSceneNodesFromType(scene::ESNT_LIGHT, LightNodes);

	LightCount = LightNodes.size();
}

// Shader callback
void ShaderCallback::OnSetConstants(irr::video::IMaterialRendererServices *Services, irr::s32 UserData) {
	Services->setPixelShaderConstant("light_count", &Graphics.LightCount, 1);
}
