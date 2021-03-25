cmake_minimum_required(VERSION 3.8)

#Global cmake features
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

#Include extended cmake library
add_subdirectory(Tools)

#Set startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Visualizer)

#set project name
project(ImageVisualizer)

# Pure3D only support x64 architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
	message(FATAL_ERROR "32-bits is not supported")
endif()

# Options
option(PHT_DEBUG "Enable debug mode for PureReflectionTool" OFF) # PHT debug mode (OFF)

# Set compiler options
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# MP build for MSVC
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /EHsc")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	# Suppress invalid offset warning on GCC
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof")
endif()
	
# Set project constants
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}) # Project dir
set(BINARIES_DIR ${PROJECT_ROOT}/Binaries) # Binaries dir
set(INTERMEDIATES_DIR ${PROJECT_ROOT}/Intermediates) # Intermediates dir
set(REFLECTION_DIR ${PROJECT_ROOT}/Intermediates/Reflection) # Reflection dir



function(create_custom_target_library TARGET)
	add_library(${TARGET} STATIC ${ARGN})
	_module_definition(${TARGET} ${ARGN})
	set_target_properties(${TARGET} PROPERTIES FOLDER "ThirdParty")
endfunction()

# Create a lib module
function(create_engine_library)	
	log_module("Add module")
	get_module_name(TARGET)
	file(GLOB_RECURSE SOURCES *.cpp *.hpp *.h *.c)
	create_custom_target_library(${TARGET} ${SOURCES})
	set_target_properties(${TARGET} PROPERTIES FOLDER "Modules")
endfunction()

# Create an executable module
function(create_executable)
	get_module_name(TARGET)
	log_module("Add executable")
	
	file(GLOB_RECURSE SOURCES *.cpp *.hpp *.h *.c)
	add_executable(${TARGET} ${SOURCES} ${ARGN})
	_module_definition(${TARGET} ${ARGN})	
endfunction()

# Include sources
add_subdirectory(Sources)
