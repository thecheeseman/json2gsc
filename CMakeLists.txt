# SPDX-FileCopyrightText: 2024 thecheeseman
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.19 FATAL_ERROR)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")
project(json2gsc LANGUAGES C CXX)

# disallow in-source builds
include(AssureOutOfSourceBuilds)
AssureOutOfSourceBuilds()

# includes
include(CheckBuildType)
include(EnableCompileCommands)
include(EnableCompilerDiagnostics)
include(EnableSanitizers)
include(Optimization)
include(SetProjectWarnings)
include(SetStandards)
#

#
# options
#
add_library(json2gscOptions INTERFACE)
EnableInterproceduralOptimization(json2gscOptions)
EnableSanitizers(json2gscOptions)

if (ENABLE_UNITY_BUILD)
    set_target_properties(json2gscOptions PROPERTIES UNITY_BUILD ON)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_options(json2gscOptions INTERFACE
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:RelWithDebInfo>:-g -O2>
        $<$<CONFIG:MinSizeRel>:-Os>

        -fno-omit-frame-pointer
        -ffast-math
        -fvisibility=hidden
        -fvisibility-inlines-hidden
        -fno-rtti
    )

    target_link_options(json2gscOptions INTERFACE
        -fno-omit-frame-pointer
        -ffast-math
        -fvisibility=hidden
        -fvisibility-inlines-hidden
    )

    target_compile_definitions(json2gscOptions INTERFACE
        -D_GLIBCXX_ASSERTIONS
        -D_FORTIFY_SOURCE=3
    )
elseif (MSVC)
    target_compile_options(json2gscOptions INTERFACE
        $<$<CONFIG:Debug>:/Od /Zi>
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:RelWithDebInfo>:/O2 /Zi>
        $<$<CONFIG:MinSizeRel>:/Os>

        /utf-8                                  # use UTF-8 for source and execution

        /EHsc                                   # enable C++ exceptions (ISO C++ EH)

        /sdl                                    # enable additional security checks
        /guard:cf                               # enable control flow guard

        /Zc:__cplusplus                         # report the correct C++ standard
        /Zc:__STDC__                            # report the correct C standard
        /Zc:char8_t                             # enable char8_t, the new C++20 char type
    )

    target_link_options(json2gscOptions INTERFACE
        $<$<CONFIG:Debug>:/DEBUG:FASTLINK>

        /DYNAMICBASE                            # enable ASLR
        /CETCOMPAT                              # enable CET
        /NXCOMPAT                               # enable DEP
    )

    target_compile_options(json2gscOptions INTERFACE
        -D_CRT_SECURE_NO_WARNINGS
    )
endif()

#
# warnings
#
add_library(json2gscWarnings INTERFACE)
SetProjectWarnings(json2gscWarnings)

#
# includes
#
add_library(json2gscIncludes INTERFACE)
target_include_directories(json2gscIncludes INTERFACE
    "${CMAKE_SOURCE_DIR}/ThirdParty/json/include"
    "${CMAKE_SOURCE_DIR}/ThirdParty/argh"
)
target_precompile_headers(json2gscIncludes INTERFACE
    <cstddef>
    <cstdint>
    <algorithm>
    <array>
    <bit>
    <concepts>
    <ranges>
    <type_traits>
)

#
# staged install
#
set(STAGED_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/Staged")
message(STATUS "Staged install prefix: ${STAGED_INSTALL_PREFIX}")

#
# projects
#

# probably shouldn't have this here but it's convenient for everything to have fmt
add_subdirectory(ThirdParty)
add_subdirectory(Source)
