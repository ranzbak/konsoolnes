# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# set(EXTRA_COMPONENT_DIRS main/components)

if(DEVICE STREQUAL "")
	set(DEVICE "$ENV{TARGET}")
endif()

if(DEVICE STREQUAL "")
	message(FATAL_ERROR "DEVICE not set")
endif()

if(DEVICE STREQUAL "tanmatsu")
	set(SDKCONFIG_DEFAULTS "sdkconfigs/general;sdkconfigs/tanmatsu")
	set(IDF_TARGET esp32p4)
elseif(DEVICE STREQUAL "mch2022")
	set(SDKCONFIG_DEFAULTS "sdkconfigs/general;sdkconfigs/mch2022")
	set(IDF_TARGET esp32)
else()
	message(FATAL_ERROR "Unknown target device ${DEVICE}")
endif()

# Switch to PAL mode
add_compile_definitions(PAL CONFIG_SOUND_ENABLED)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(konsoolnes)
