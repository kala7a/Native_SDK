cmake_minimum_required(VERSION 3.10)
project(VulkanHybridReflections)
if(ANDROID)
	message("Skipping VulkanHybridReflections : Not supported on Android")
	return()
endif()
if(IOS)
	message("Skipping VulkanHybridReflections : Not supported on iOS")
	return()
endif()
if(APPLE AND NOT IOS)
	message("Skipping VulkanHybridReflections : Not supported on macOS")
	return()
endif()
if(IOS)
	message ("Skipping VulkanHybridReflections : Vulkan is not supported on iOS.")
	return()
endif()

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsVk REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanHybridReflections)

set(SRC_FILES VulkanHybridReflections.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_VulkanHybridReflections)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(VulkanHybridReflections ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(VulkanHybridReflections
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/PBR
	FILE_LIST 
		brdfLUT.pvr)

add_assets_to_target(VulkanHybridReflections
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Environments
	FILE_LIST 
		LancellottiChapel.pvr
		LancellottiChapel_astc.pvr
		LancellottiChapel_Irradiance.pvr
		LancellottiChapel_Prefiltered.pvr)

add_assets_to_target(VulkanHybridReflections
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Satyr
	FILE_LIST 
		Marble.pvr
		Marble_astc.pvr)

add_assets_to_target(VulkanHybridReflections
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/RayTracingReflections
	FILE_LIST 
		Reflections.POD
		Alabaster.pvr
		Alabaster_astc.pvr
		Orange.pvr
		Orange_astc.pvr
		RoundTable.pvr
		RoundTable_astc.pvr
		RustGreen.pvr
		RustGreen_astc.pvr)

### Shaders that need to be compiled to SpirV (Vulkan) ###
# During build time, glslangvalidator will be called to create a "<shadername.extension>.spv" file
add_spirv_shaders_to_target(VulkanHybridReflections 
	SOURCE_GROUP "shaders_source" 
	SPIRV_SOURCE_GROUP "shaders_generated"
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	VULKAN_TARGET_ENV spirv1.4
	FILE_LIST
		DeferredShadingFragmentShader.fsh
		ForwardShadingFragmentShader.fsh
		FullscreenQuadVertexShader.vsh
		GBufferFragmentShader.fsh
		GBufferVertexShader.vsh
		RayTraceReflections.rchit
		RayTraceReflections.rgen
		RayTraceReflections.rmiss
		RayTraceShadows.rchit
		RayTraceShadows.rmiss
		SkyboxFragmentShader.fsh
		SkyboxVertexShader.vsh)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(VulkanHybridReflections)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(VulkanHybridReflections)

target_link_libraries(VulkanHybridReflections PUBLIC
	PVRShell
	PVRUtilsVk
)