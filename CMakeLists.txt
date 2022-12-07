cmake_minimum_required(VERSION 3.13)

#set project name
project(SuperPacker)

# Set project constants
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}) # Project dir
set(BINARIES_DIR ${PROJECT_ROOT}/bin) # Binaries dir
set(INTERMEDIATES_DIR ${CMAKE_BINARY_DIR}) # Intermediates dir

#Global cmake features
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#Set startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SuperPacker)

# Pure3D only support x64 architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
	message(FATAL_ERROR "32-bits is not supported")
endif()

# Set compiler options
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

	# Enable Hot Reload
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MDd /ZI /Ob0 /Od /RTC1")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /ZI /Ob0 /Od /RTC1")
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /debug /INCREMENTAL /LTCG:OFF")
	set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} /debug /INCREMENTAL /LTCG:OFF")
	set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /debug /INCREMENTAL /LTCG:OFF")
	
	# MP build for MSVC
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /EHsc")
endif()
	
# Set project constants
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${BINARIES_DIR}/Lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BINARIES_DIR}/Bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${BINARIES_DIR}/Bin")
set(CMAKE_VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_ROOT}")


function(configure_project PROJECT_NAME SOURCES)
	set_target_properties(${PROJECT_NAME} PROPERTIES 
		ARCHIVE_OUTPUT_DIRECTORY "${BINARIES_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY "${BINARIES_DIR}/Bin"
		LIBRARY_OUTPUT_DIRECTORY "${BINARIES_DIR}/Bin"
		VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_ROOT}")
	target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
	set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)
	source_group(TREE ${PROJECT_ROOT} FILES ${SOURCES})
endfunction()

# Ensure dependencies are available
find_package(glfw3 CONFIG REQUIRED)
find_package(gl3w CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(freeimage CONFIG REQUIRED)

set(APP_ICON_RESOURCE_WINDOWS "${PROJECT_ROOT}/resources/appicon.rc")

add_subdirectory(third_party/nativefiledialog-extended)

file(GLOB_RECURSE SOURCES src/*.cpp src/*.h resources/** include/*.h)
add_executable(SuperPacker WIN32 ${SOURCES} ${APP_ICON_RESOURCE_WINDOWS})
configure_project(SuperPacker ${SOURCES})
target_include_directories(SuperPacker PRIVATE resources)
target_include_directories(SuperPacker PRIVATE src)
target_include_directories(SuperPacker PUBLIC include)
target_include_directories(SuperPacker PRIVATE ${STB_INCLUDE_DIRS})
target_link_libraries(SuperPacker PRIVATE glfw)
target_link_libraries(SuperPacker PRIVATE unofficial::gl3w::gl3w)
target_link_libraries(SuperPacker PRIVATE imgui::imgui)
target_link_libraries(SuperPacker PRIVATE nlohmann_json::nlohmann_json)  
target_link_libraries(SuperPacker PRIVATE freeimage::FreeImage freeimage::FreeImagePlus)
target_link_libraries(SuperPacker PRIVATE nfd)
source_group(TREE ${PROJECT_ROOT} FILES  ${SOURCES})