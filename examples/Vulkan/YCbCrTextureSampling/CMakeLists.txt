cmake_minimum_required(VERSION 3.3)
project(VulkanYCbCrTextureSampling)
if(IOS)
	message ("Skipping VulkanYCbCrTextureSampling : Vulkan is not supported on iOS.")
	return()
endif()

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsVk REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanYCbCrTextureSampling)

set(SRC_FILES VulkanYCbCrTextureSampling.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_VulkanYCbCrTextureSampling)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(VulkanYCbCrTextureSampling ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(VulkanYCbCrTextureSampling
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/YCbCrTextureSampling
	FILE_LIST 
		Plane.pod
		ColourTest2_YUV_2-Plane_420_UNorm_sRGB_NV12.pvr
		ColourTest2_YUV_2-Plane_422_UNorm_sRGB_P208.pvr
		ColourTest2_YUV_3-Plane_420_UNorm_sRGB.pvr
		ColourTest2_YUV_3-Plane_422_UNorm_sRGB.pvr
		ColourTest2_YUV_3-Plane_444_UNorm_sRGB.pvr)

### Shaders that need to be compiled to SpirV (Vulkan) ###
# During build time, glslangvalidator will be called to create a "<shadername.extension>.spv" file
add_spirv_shaders_to_target(VulkanYCbCrTextureSampling 
	SOURCE_GROUP "shaders_source" 
	SPIRV_SOURCE_GROUP "shaders_generated"
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	VULKAN_TARGET_ENV vulkan1.0
	FILE_LIST
		FragShader.fsh
		VertShader.vsh)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(VulkanYCbCrTextureSampling)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(VulkanYCbCrTextureSampling)

target_link_libraries(VulkanYCbCrTextureSampling PUBLIC
	PVRShell
	PVRUtilsVk
)