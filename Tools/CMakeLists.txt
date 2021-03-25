#Extended cmake function library


# Set current project target category
function(set_project_category CATEGORY)
	get_module_name(TARGET)
	set_target_properties(${TARGET} PROPERTIES FOLDER ${CATEGORY})
endfunction()

# Get module name based on parent directory name
function(get_module_name MODULE_NAME)
	get_filename_component(TARGET ${CMAKE_CURRENT_SOURCE_DIR} NAME)
	set (${MODULE_NAME} ${TARGET} PARENT_SCOPE)
endfunction()

# Enable PHT in current module
function(use_reflection)
	log_module("-- Use reflection")
	
	get_module_name(TARGET)
	
	# Link reflection module
	add_public_dependencies(PureHeaderLib)	
	
	# Add reflection custom target
	set(REFLECTION_TARGET ${TARGET}_ReflectionGenerator)	
	add_custom_target(${REFLECTION_TARGET} ALL
					COMMAND "$<TARGET_FILE:PureHeaderGenerator>"
					-ModuleName="${TARGET}"
					-ModulePath="${CMAKE_CURRENT_SOURCE_DIR}"
					-Debug="${PHT_DEBUG}"
					-OutputPath="${REFLECTION_DIR}/${TARGET}"
					-CmakeRebuild="${PROJECT_ROOT}")
					
	set_target_properties(${REFLECTION_TARGET} PROPERTIES FOLDER "PureHeaderTool/ReflectionData")
	
	add_dependencies(${REFLECTION_TARGET} PureHeaderGenerator)
	add_dependencies(${TARGET} ${REFLECTION_TARGET})
	
	# Include reflection directory
	target_include_directories(${TARGET} PUBLIC ${REFLECTION_DIR}/${TARGET}/Public)
	
	# Build reflection data
	file(GLOB_RECURSE SOURCES ${REFLECTION_DIR}/${TARGET}/Private/*.refl.cpp)		
	target_sources(${TARGET} PRIVATE ${SOURCES})
		
endfunction()

# Global module definition
function(_module_definition TARGET)
	#Set compiller features
	target_compile_features(${TARGET} PUBLIC cxx_std_20)
	set_target_properties(${TARGET} PROPERTIES CXX_EXTENSIONS OFF)
	if (UNIX)
		target_link_options(${TARGET} PUBLIC -ldl -lpthread)
	endif()

	#Set solution directories
	set_target_properties(${TARGET} PROPERTIES 
		ARCHIVE_OUTPUT_DIRECTORY "${BINARIES_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY "${BINARIES_DIR}/Bin"
		LIBRARY_OUTPUT_DIRECTORY "${BINARIES_DIR}/Bin"
		VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_ROOT}")
	
	# Display real hierarchy
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
	
	#Export include directories
	target_include_directories(${TARGET}
		PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}/Public	
		PRIVATE
			${CMAKE_CURRENT_SOURCE_DIR}/Private)
endfunction()


function(log_module MESSAGE)
	get_module_name(TARGET)
	message("-- [${TARGET}] ${MESSAGE}")
endfunction()

#link public dependencies
function(add_public_dependencies)	
	get_module_name(TARGET)
	log_module("-- Public dependencies : ${ARGN}")
	target_link_libraries(${TARGET} PUBLIC ${ARGN})
endfunction()


#link private dependencies
function(add_private_dependencies)	
	get_module_name(TARGET)	
	log_module("-- Private dependencies : ${ARGN}")
	target_link_libraries(${TARGET} PRIVATE ${ARGN})
endfunction()

# Create interface to external compilled libraries
function(create_virtual_module TARGET BINARIES)
	log_module("External : ${TARGET} [${BINARIES}] => ${ARGN}")
	add_library(${TARGET} INTERFACE)
	if("${BINARIES}" STREQUAL "")
	else()
		if (WIN32)
			target_link_libraries(${TARGET} INTERFACE ${BINARIES}.lib)
		else()
			target_link_libraries(${TARGET} INTERFACE ${BINARIES}.a)
		endif()
	endif()
	target_include_directories(${TARGET} INTERFACE ${ARGN})
endfunction()


