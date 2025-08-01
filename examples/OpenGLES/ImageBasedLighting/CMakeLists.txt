cmake_minimum_required(VERSION 3.10)
project(OpenGLESImageBasedLighting)
if(IOS)
	message("Skipping OpenGLESImageBasedLighting : Not supported on iOS")
	return()
endif()

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsGles REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESImageBasedLighting)

set(SRC_FILES OpenGLESImageBasedLighting.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenGLESImageBasedLighting)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(OpenGLESImageBasedLighting ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(OpenGLESImageBasedLighting
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/damagedHelmet
	FILE_LIST 
		damagedHelmet.bin
		damagedHelmet.gltf
		Default_albedo.pvr
		Default_albedo_astc.pvr
		Default_emissive.pvr
		Default_emissive_astc.pvr
		Default_occlusionRoughnessMetallic.pvr
		Default_occlusionRoughnessMetallic_astc.pvr
		Default_normal.pvr
		Default_normal_astc.pvr)

add_assets_to_target(OpenGLESImageBasedLighting
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Environments
	FILE_LIST 
		misty_pines_rgb9e5.pvr
		misty_pines_rgb9e5_Irradiance.pvr
		misty_pines_rgb9e5_Prefiltered.pvr
		satara_night_scale_0.305_rgb9e5.pvr
		satara_night_scale_0.305_rgb9e5_Irradiance.pvr
		satara_night_scale_0.305_rgb9e5_Prefiltered.pvr)

add_assets_to_target(OpenGLESImageBasedLighting
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/GeneralPurposeMesh
	FILE_LIST 
		sphere.pod)

add_assets_to_target(OpenGLESImageBasedLighting
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/PBR
	FILE_LIST 
		brdfLUT.pvr)

### Textural shaders (OpenGL ES) ###
add_assets_to_target(OpenGLESImageBasedLighting 
	SOURCE_GROUP "shaders" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	FILE_LIST 
		PBRFragShader.fsh
		PBRVertShader.vsh
		SkyboxFragShader.fsh
		SkyboxVertShader.vsh)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(OpenGLESImageBasedLighting)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(OpenGLESImageBasedLighting)

target_link_libraries(OpenGLESImageBasedLighting PUBLIC
	PVRShell
	PVRUtilsGles
)