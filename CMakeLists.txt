cmake_minimum_required(VERSION 3.22)
project(QSynthi)

set(CMAKE_CXX_STANDARD 23)

# setup guide:
# https://www.youtube.com/watch?v=Uq7Hwt18s3s
# https://github.com/JanWilczek/audio-plugin-template

# I like to download the dependencies to the same folder as the project.
# If you want to install them system wide, set CPM_SOURCE_CACHE with the path to the dependencies
# either as an environment variable or pass it to the cmake script with -DCPM_SOURCE_CACHE=<path>.

IF (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    # save release lib builds in cmake-build-release[...] folder
    set(LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/libs)
ELSE()
    set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)
ENDIF()

# Downloads CPM if not already downloaded. CPM is an easy-to-use package manager nicely integrated with CMake.
include(cmake/get_cpm.cmake)

# This commands downloads AND configures JUCE. It sets up some variables, like JUCE_SOURCE_DIR.
CPMAddPackage(
        NAME JUCE
        GIT_TAG 7.0.8
        VERSION 7.0.8
        GITHUB_REPOSITORY juce-framework/JUCE
        SOURCE_DIR ${LIB_DIR}/juce
)

# Adds googletest.
#CPMAddPackage(
#        NAME GOOGLETEST
#        GITHUB_REPOSITORY google/googletest
#        GIT_TAG v1.13.0
#        VERSION 1.13.0
#        SOURCE_DIR ${LIB_DIR}/googletest
#        OPTIONS
#        "INSTALL_GTEST OFF"
#        "gtest_force_shared_crt ON"
#)

# This command allows running tests from the "build" folder (the one where CMake generates the project to).
#enable_testing()

# Enables all warnings and treats warnings as errors.
# This needs to be added AFTER all the third-party dependencies.
if(MSVC)
    #add_compile_options(/Wall /WX)
else()
    #add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Adds all the targets configured in the "plugin" folder.
add_subdirectory(Source)

# Adds all the targets configured in the "test" folder.
#add_subdirectory(test)
