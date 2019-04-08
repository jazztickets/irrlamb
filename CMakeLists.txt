cmake_minimum_required(VERSION 2.8)

# set default build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

# define constants
add_definitions(-DGAME_VERSION="1.0.0")
add_definitions(-D_IRR_STATIC_LIB_)
add_definitions(-DdIDEDOUBLE)
add_definitions(-DCCD_IDEDOUBLE)
add_definitions(-DdTRIMESH_ENABLED)
add_definitions(-DdTRIMESH_OPCODE)
add_definitions(-DdLIBCCD_ENABLED)
add_definitions(-DdLIBCCD_CYL_CYL)

# projects
project(irrlamb)
subdirs(tools)

# add extra find modules
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

# platform
if(WIN32)
	set(EXTRA_LIBS ${EXTRA_LIBS} winmm ws2_32)
	add_definitions(-DPLATFORM=1)
elseif(UNIX)
	add_definitions(-DPLATFORM=0)
endif()

# compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-deprecated-declarations -pedantic -std=gnu++11 -msse -mfpmath=sse")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DdNODEBUG -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DdNODEBUG -s")

# x11
find_package(X11 REQUIRED)
set(EXTRA_LIBS ${EXTRA_LIBS} ${X11_X11_LIB} ${X11_Xrandr_LIB} ${X11_Xext_LIB} ${X11_Xxf86vm_LIB})
include_directories(${X11_Xrandr_INCLUDE_PATH})
include_directories(${X11_X11_INCLUDE_PATH})
include_directories(${X11_xf86vmode_INCLUDE_PATH})

# pandora devices
if(PANDORA)
	add_definitions("-DPANDORA")
	add_definitions("-DLUA_USE_LINUX")
endif()

# set output path
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})

# Set gl preference
if(NOT DEFINED OpenGL_GL_PREFERENCE)
	set(OpenGL_GL_PREFERENCE GLVND)
endif()

# find libraries
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenAL REQUIRED)
find_package(Vorbis REQUIRED)
find_package(Ogg REQUIRED)
find_package(SQLite REQUIRED)
find_package(Lua ${LUA_VERSION} REQUIRED)
find_package(TinyXML2 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(Threads REQUIRED)

# include paths
include_directories("src")
include_directories("src/irrlicht/include")
include_directories("src/OPCODE")
include_directories("src/libccd")
include_directories("src/libccd/custom")
include_directories(${FREETYPE_INCLUDE_DIRS})
include_directories(${OPENAL_INCLUDE_DIR})
include_directories(${VORBIS_INCLUDE_DIR})
include_directories(${OGG_INCLUDE_DIR})
include_directories(${SQLITE_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_DIR})
include_directories(${TINYXML2_INCLUDE_DIR})
include_directories(${ZLIB_INCLUDE_DIRS})
include_directories(${PNG_INCLUDE_DIRS})
include_directories(${JPEG_INCLUDE_DIRS})

# add source code
file(GLOB_RECURSE SRC_MAIN src/*.c src/*.cpp src/*.h)

file(GLOB SRC_ALL
	${SRC_MAIN}
	src/resource.rc
)

# create executable
add_executable(${CMAKE_PROJECT_NAME} ${SRC_ALL})

# link libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
	${OPENGL_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${OPENAL_LIBRARY}
	${VORBIS_LIBRARIES}
	${OGG_LIBRARIES}
	${SQLITE_LIBRARIES}
	${LUA_LIBRARIES}
	${TINYXML2_LIBRARIES}
	${ZLIB_LIBRARIES}
	${PNG_LIBRARIES}
	${JPEG_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${EXTRA_LIBS}
)

if(WIN32)
else()

	# linux installation
	install(TARGETS ${CMAKE_PROJECT_NAME} RUNTIME DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/fonts DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/levels DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/meshes DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/scripts DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/shaders DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/sounds DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/textures DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/ui DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(FILES ${PROJECT_SOURCE_DIR}/deployment/irrlamb.png DESTINATION share/pixmaps)
	install(FILES ${PROJECT_SOURCE_DIR}/deployment/irrlamb.desktop DESTINATION share/applications)
	install(FILES ${PROJECT_SOURCE_DIR}/CHANGELOG DESTINATION share/doc/${CMAKE_PROJECT_NAME})
	install(FILES ${PROJECT_SOURCE_DIR}/LICENSE DESTINATION share/doc/${CMAKE_PROJECT_NAME})
	install(FILES ${PROJECT_SOURCE_DIR}/README DESTINATION share/doc/${CMAKE_PROJECT_NAME})

	# generate the script to launch the program
	configure_file(${PROJECT_SOURCE_DIR}/deployment/${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles)
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${CMAKE_PROJECT_NAME} DESTINATION bin)
endif()
