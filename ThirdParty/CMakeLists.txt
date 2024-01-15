# SPDX-FileCopyrightText: 2023 thecheeseman
#
# SPDX-License-Identifier: MIT

message(STATUS "Configuring External Projects")
list(APPEND CMAKE_MESSAGE_INDENT "  ")

message(STATUS "Configuring json")
list(APPEND CMAKE_MESSAGE_INDENT "  ")
add_subdirectory(json)
list(POP_BACK CMAKE_MESSAGE_INDENT)

list(POP_BACK CMAKE_MESSAGE_INDENT)
