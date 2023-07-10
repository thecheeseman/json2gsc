# SPDX-FileCopyrightText: 2023 thecheeseman
#
# SPDX-License-Identifier: CC0-1.0

set(SOURCES
    Main.cpp
)

add_executable(json2gsc ${SOURCES})
target_link_libraries(json2gsc PRIVATE json2gscOptions json2gscWarnings json2gscIncludes)

set_target_properties(json2gsc PROPERTIES
    PREFIX ""
    ARCHIVE_OUTPUT_DIRECTORY "${STAGED_INSTALL_PREFIX}"
    LIBRARY_OUTPUT_DIRECTORY "${STAGED_INSTALL_PREFIX}"
    RUNTIME_OUTPUT_DIRECTORY "${STAGED_INSTALL_PREFIX}"
)

add_custom_command(
    TARGET json2gsc
    POST_BUILD
    COMMENT "Generate sha512 checksum for json2gsc"
    WORKING_DIRECTORY $<TARGET_FILE_DIR:json2gsc>
    COMMAND ${CMAKE_COMMAND} -E sha512sum $<TARGET_FILE_NAME:json2gsc> > $<TARGET_FILE_NAME:json2gsc>.sha512
    VERBATIM
)
