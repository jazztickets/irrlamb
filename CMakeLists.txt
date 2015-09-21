cmake_minimum_required(VERSION 2.8)

# define constants
add_definitions(-DGAME_VERSION="0.2.1")

# projects
project(irrlamb)
subdirs(tools)

# add extra find modules
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

# set default values
set(EXTRA_LIBS)
set(BULLET_ROOT "$ENV{BULLET_ROOT}")

# check for -DLUA_VERSION=x.x
if(NOT LUA_VERSION)
	set(LUA_VERSION "5.3")
endif()

# windows only options
if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	add_definitions("-D_CRT_SECURE_NO_WARNINGS")

	# set output path
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)
else()
	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Release)
	endif()

	# compiler flags
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

	# x11
	find_package(X11 REQUIRED)
	set(EXTRA_LIBS ${EXTRA_LIBS} ${X11_X11_LIB} ${X11_Xrandr_LIB} ${X11_Xext_LIB} ${X11_Xxf86vm_LIB})
	include_directories(${X11_Xrandr_INCLUDE_PATH})
	include_directories(${X11_X11_INCLUDE_PATH})
	include_directories(${X11_xf86vmode_INCLUDE_PATH})

	if(PANDORA)
		add_definitions("-DPANDORA")
		add_definitions("-DLUA_USE_LINUX")
	endif()

	# set output path
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
endif()

# find libraries
find_package(Irrlicht 1.8 REQUIRED)
if(NOT IRRLICHT_FOUND)
	message(SEND_ERROR "Could not find Irrlicht!")
endif()

find_package(Bullet REQUIRED)
if(NOT BULLET_FOUND)
	message(SEND_ERROR "Could not find Bullet!")
endif()

find_package(Freetype REQUIRED)
if(NOT FREETYPE_FOUND)
	message(SEND_ERROR "Could not find Freetype 2!")
endif()

find_package(OpenAL REQUIRED)
if(NOT OPENAL_FOUND)
	message(SEND_ERROR "Could not find OpenAL!")
endif()

find_package(Vorbis REQUIRED)
if(NOT VORBIS_FOUND)
	message(SEND_ERROR "Could not find libvorbis or libvorbisfile!")
endif()

find_package(Ogg REQUIRED)
if(NOT OGG_FOUND)
	message(SEND_ERROR "Could not find libogg!")
endif()

find_package(SQLite REQUIRED)
if(NOT SQLITE_FOUND)
	message(SEND_ERROR "Could not find SQLite3!")
endif()

find_package(Lua ${LUA_VERSION} REQUIRED)
if(NOT LUA_FOUND)
	message(SEND_ERROR "Could not find Lua " ${LUA_VERSION} "!")
endif()

# include paths
include_directories(${BULLET_INCLUDE_DIRS})
include_directories(${IRRLICHT_INCLUDE_DIRS})
include_directories(${OPENAL_INCLUDE_DIR})
include_directories(${VORBIS_INCLUDE_DIR})
include_directories(${OGG_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_DIR})
include_directories(${SQLITE_INCLUDE_DIR})
include_directories(${FREETYPE_INCLUDE_DIRS})
include_directories("src")

# add source code
file(GLOB SRC_MAIN src/*.cpp src/*.h)
file(GLOB SRC_ENGINE src/engine/*.cpp src/engine/*.h)
file(GLOB SRC_OBJECTS src/objects/*.cpp src/objects/*.h)
file(GLOB SRC_IRRB src/irrb/*.cpp src/irrb/*.h)
file(GLOB SRC_FONT src/font/*.cpp src/font/*.h)
file(GLOB SRC_TINYXML src/tinyxml/*.cpp src/tinyxml/*.h)

file(GLOB SRC_ALL ${SRC_MAIN} ${SRC_ENGINE} ${SRC_OBJECTS} ${SRC_IRRB} ${SRC_TINYXML} ${SRC_FONT})

# add virtual folders
source_group(main FILES ${SRC_MAIN})
source_group(engine FILES ${SRC_ENGINE})
source_group(objects FILES ${SRC_OBJECTS})
source_group(irrb FILES ${SRC_IRRB})
source_group(font FILES ${SRC_FONT})
source_group(tinyxml FILES ${SRC_TINYXML})

# create exe
add_executable(${CMAKE_PROJECT_NAME} ${SRC_ALL} src/resource.rc)

# link libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
	${IRRLICHT_LIBRARIES}
	${BULLET_LIBRARIES}
	${OPENAL_LIBRARY}
	${LUA_LIBRARIES}
	${SQLITE_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${VORBIS_LIBRARIES}
	${OGG_LIBRARIES}
	${EXTRA_LIBS}
	)

if(WIN32)

	# copy libraries to working directory
	add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$ENV{IRRLICHT_ROOT}/bin/Win64-VisualStudio/Irrlicht.dll" ${PROJECT_SOURCE_DIR}/working)
	add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$ENV{IRRLICHT_ROOT}/bin/Win64-VisualStudio/Irrlicht_d.dll" ${PROJECT_SOURCE_DIR}/working)
	add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$ENV{OGG_ROOT}/win32/VS2010/x64/Release/libogg.dll" ${PROJECT_SOURCE_DIR}/working)
	add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$ENV{OGG_ROOT}/win32/VS2010/x64/Debug/libogg_d.dll" ${PROJECT_SOURCE_DIR}/working)
	add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$ENV{VORBIS_ROOT}/win32/VS2010/x64/Release/libvorbis.dll" ${PROJECT_SOURCE_DIR}/working)
	add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$ENV{VORBIS_ROOT}/win32/VS2010/x64/Debug/libvorbis_d.dll" ${PROJECT_SOURCE_DIR}/working)
	add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$ENV{VORBIS_ROOT}/win32/VS2010/x64/Release/libvorbisfile.dll" ${PROJECT_SOURCE_DIR}/working)
	add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$ENV{VORBIS_ROOT}/win32/VS2010/x64/Debug/libvorbisfile_d.dll" ${PROJECT_SOURCE_DIR}/working)
else()

	# linux installation
	install(TARGETS ${CMAKE_PROJECT_NAME} RUNTIME DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/art DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/fonts DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/levels DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/meshes DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/scripts DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/shaders DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/sounds DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/textures DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(FILES ${PROJECT_SOURCE_DIR}/deployment/irrlamb.png DESTINATION share/pixmaps)
	install(FILES ${PROJECT_SOURCE_DIR}/deployment/irrlamb.desktop DESTINATION share/applications)
	install(FILES ${PROJECT_SOURCE_DIR}/CHANGELOG DESTINATION share/doc/${CMAKE_PROJECT_NAME})
	install(FILES ${PROJECT_SOURCE_DIR}/LICENSE DESTINATION share/doc/${CMAKE_PROJECT_NAME})
	install(FILES ${PROJECT_SOURCE_DIR}/README DESTINATION share/doc/${CMAKE_PROJECT_NAME})

	# generate the script to launch the program
	configure_file(${PROJECT_SOURCE_DIR}/deployment/${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles)
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${CMAKE_PROJECT_NAME} DESTINATION bin)
endif()
