cmake_minimum_required(VERSION 3.22)

# Version is needed by JUCE.
project(AudioPlugin VERSION 0.8.1)

# Adding a directory with the library/application name as a subfolder of the
# include folder is a good practice. It helps avoid name clashes later on.
#set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/QSynthi2")

IF (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(PROD_NAME "QSynthi-Debug")
ELSE()
    set(PROD_NAME "QSynthi")
ENDIF()

# Adds a plugin target (that's basically what the Projucer does).
juce_add_plugin(${PROJECT_NAME}
        COMPANY_NAME AJ
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT TRUE
        COPY_PLUGIN_AFTER_BUILD FALSE
        PLUGIN_MANUFACTURER_CODE Ajfm
        PLUGIN_CODE Qsyi
        FORMATS AU VST3 Standalone
        PRODUCT_NAME ${PROD_NAME}
)

# add image folder
add_subdirectory(img)

# Sets the source files of the plugin project.
target_sources(${PROJECT_NAME}
        PRIVATE
        PluginEditor.cpp
        PluginProcessor.cpp
        QSynthi.cpp
        Parameter.cpp
        Wavetable.cpp
        WavetableOscillator.cpp
        WavetablePlot.cpp

        PluginEditor.h
        PluginProcessor.h
        QSynthi.hpp
        Parameter.h
        Wavetable.hpp
        WavetableOscillator.hpp
        WavetablePlot.h
        list.hpp
        pocketfft_hdronly.h
)

# Sets the include directories of the plugin project.
target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Links to all necessary dependencies. The present ones are recommended by JUCE.
# If you use one of the additional modules, like the DSP module, you need to specify it here.
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        BinaryData
        juce::juce_audio_utils
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# These definitions are recommended by JUCE.
target_compile_definitions(${PROJECT_NAME}
        PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

IF (WIN32)
    # set stuff for windows
    # copy plugin file command (template from https://stackoverflow.com/a/49064484)
    #    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    #        COMMAND ${CMAKE_COMMAND} -E -E copy $<TARGET_FILE:${PROJECT_NAME}> ${targetfile}
    #    )
ELSE()
    # set stuff for other systems
ENDIF()

# Silences some deprecation warnings that (hopefully) will be fixed in a future release of JUCE.
#if (MSVC)
#    target_compile_definitions(${PROJECT_NAME}
#            PRIVATE
#            _SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING)
#endif()

# In Visual Studio this command provides a nice grouping of source files in "filters".
#source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/..)
