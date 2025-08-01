cmake_minimum_required(VERSION 3.10)
# Copyright (c) Imagination Technologies Limited.

project(PVRAssets)
include(GNUInstallDirs)
include(CMakeFindDependencyMacro)
include(../FrameworkCommon.cmake)

# Prevent PVRAssets being added multiple times
if(TARGET PVRAssets)
	return()
endif()

message(STATUS "Adding PVRAssets")

# PVRAssets include files
set(PVRAssets_HEADERS
	BoundingBox.h
	Geometry.h
	Helper.h
	IndexedArray.h
	Model.h
	PVRAssets.h
	ShadowVolume.h
	Volume.h
	fileio/GltfReader.h
	fileio/PODDefines.h
	fileio/PODReader.h
	model/Animation.h
	model/Camera.h
	model/Light.h
	model/Mesh.h
	model/Animation.h
	model/Camera.h
	model/Light.h
	model/Mesh.h
	model/FormattedUserData.h)

# PVRAssets sources
set(PVRAssets_SRC
	fileio/GltfReader.cpp
	fileio/PODReader.cpp
	Helper.cpp
	model/Animation.cpp
	model/Camera.cpp
	model/Light.cpp
	model/Mesh.cpp
	model/Model.cpp
	ShadowVolume.cpp
	Volume.cpp)

# Create the library
add_library(PVRAssets STATIC ${PVRAssets_SRC} ${PVRAssets_HEADERS})

if(PVR_PREBUILT_DEPENDENCIES)
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../cmake/modules")
	find_dependency(PVRCore MODULE REQUIRED)
	find_dependency(tinygltf MODULE REQUIRED)
endif()

# Apply the common framework compile definitions to tinygltf
# This sets certain warnings on/off and adds appropriate
# debug and release flags in the compiler and linker
apply_framework_compile_options_to_target(PVRAssets)

# Link PVRAssets to PVRCore and tinygltf
target_link_libraries(PVRAssets
	PUBLIC
		PVRCore
	PRIVATE
		tinygltf)

# Set the include directories of PVRAssets to allow it to see the rest of the framework
# As well as tinygltf
target_include_directories(PVRAssets 
	PUBLIC 
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if(PVR_ENABLE_INSTALL)
	install(TARGETS PVRAssets EXPORT PVRAssetsTargets
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
		OPTIONAL)
	foreach(file ${PVRAssets_HEADERS})
		get_filename_component(dir ${file} DIRECTORY)
		install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PVRAssets/${dir})
	endforeach()
	install(EXPORT PVRAssetsTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRAssets OPTIONAL)

	export(EXPORT PVRAssetsTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/PVRAssets/PVRAssetsTargets.cmake")
	# build tree package config
	file(COPY cmake/PVRAssetsConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRAssets/" NO_SOURCE_PERMISSIONS)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRAssets/PVRAssetsConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/PVRAssets/PVRAssetsConfig.cmake" COPYONLY)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PVRAssets/PVRAssetsTargets.cmake" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRAssets OPTIONAL)
endif()
