#Stb
create_virtual_module(Stb "" "./stb/")
#Glfw
if (WIN32)
	create_virtual_module(Glfw "${INTERMEDIATES_DIR}/Dependencies/glfw3" "./glfw/include")
else()
	create_virtual_module(Glfw "${INTERMEDIATES_DIR}/Dependencies/libglfw3" "./glfw/include")
endif()

#Gl3w
add_library(Gl3w STATIC gl3w/src/gl3w.c gl3w/include/GL/gl3w.h gl3w/include/GL/glcorearb.h gl3w/include/KHR/khrplatform.h)
target_include_directories(Gl3w PUBLIC gl3w/include)
if (UNIX)
	target_compile_options(Gl3w PUBLIC -fPIC)
endif()
set_target_properties(Gl3w PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY "${BINARIES_DIR}/Lib"
	RUNTIME_OUTPUT_DIRECTORY "${BINARIES_DIR}/Bin"
	VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_ROOT}")
set_target_properties(Gl3w PROPERTIES FOLDER ThirdParty)

#Imgui
file(GLOB SOURCES  imgui/*.cpp imgui/*.h imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_glfw.h imgui/backends/imgui_impl_opengl3.cpp imgui/backends/imgui_impl_opengl3.h)
add_library(Imgui STATIC ${SOURCES})
target_include_directories(Imgui PUBLIC imgui)
target_link_libraries(Imgui PUBLIC Gl3w)
target_link_libraries(Imgui PUBLIC Glfw)
set_target_properties(Imgui PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY "${BINARIES_DIR}/Lib"
	RUNTIME_OUTPUT_DIRECTORY "${BINARIES_DIR}/Bin"
	VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_ROOT}")
set_target_properties(Imgui PROPERTIES FOLDER ThirdParty)
target_link_libraries(Imgui PUBLIC Glfw)