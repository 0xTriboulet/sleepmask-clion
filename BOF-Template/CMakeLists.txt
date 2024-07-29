cmake_minimum_required(VERSION 3.28)
project(BOF_Template)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CXX_COMPILER_FLAGS "/std:c++20" "/Wall" "/Os")
set(ASM_FLAGS "/c")

# List of .cpp source files common to debug and release
set(CPP_SOURCE_FILES_COMMON
        ${CMAKE_SOURCE_DIR}/library/gate.cpp
        ${CMAKE_SOURCE_DIR}/library/masking.cpp
        ${CMAKE_SOURCE_DIR}/library/pivot.cpp
        ${CMAKE_SOURCE_DIR}/library/sleep.cpp
        ${CMAKE_SOURCE_DIR}/library/utils.cpp
        ${CMAKE_SOURCE_DIR}/library/stdlib.cpp
        ${CMAKE_SOURCE_DIR}/sleepmask.cpp
)

# List of debug only .cpp source files
set(CPP_SOURCE_FILES_DEBUG
        ${CMAKE_SOURCE_DIR}/base/mock.cpp
        ${CMAKE_SOURCE_DIR}/library/debug.cpp
)

if(CMAKE_BUILD_TYPE MATCHES Release)
    set(CPP_SOURCE_FILES ${CPP_SOURCE_FILES_COMMON})
else() # Debug
    set(CPP_SOURCE_FILES ${CPP_SOURCE_FILES_COMMON} ${CPP_SOURCE_FILES_DEBUG})
endif()

if (CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$ENV{OUTDIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$ENV{OUTDIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$ENV{OUTDIR})
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR})  # To ensure all paths are included
include_directories(.)  # To ensure all paths are included


# Check target architecture using CMAKE_SYSTEM_PROCESSOR
if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "amd64")
    set(TARGET_ARCH "64-bit")
    set(ASSEMBLER ml64.exe)
    set(OUTEXT .x64.o)
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "x86" OR CMAKE_SYSTEM_PROCESSOR MATCHES "i386" OR CMAKE_SYSTEM_PROCESSOR MATCHES "i686")
    set(TARGET_ARCH "32-bit")
    set(ASSEMBLER ml.exe)  # Assuming 32-bit assembler
    set(OUTEXT .x86.o)
else()
    message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

message(STATUS "Target architecture is: ${TARGET_ARCH}")

# Directory containing your assembly source files
set(ASM_DIR assembly)

# Find all .asm files in ASM_DIR
file(GLOB ASM_SOURCE_FILES ${ASM_DIR}/*.asm)

# Initialize list to hold all .obj filenames
set(ASM_OBJECT_FILES "")

# Loop over all .asm files and generate corresponding .obj files
foreach(ASM_SOURCE_FILE IN LISTS ASM_SOURCE_FILES)
    string(REPLACE ".asm" ".obj" ASM_OBJECT_FILE ${ASM_SOURCE_FILE})
    string(REPLACE "${CMAKE_SOURCE_DIR}/" "" ASM_OBJECT_FILE ${ASM_OBJECT_FILE}) # Remove full path

    # Custom command to compile each assembly file to an object file using ml64.exe
    add_custom_command(
            OUTPUT ${ASM_OBJECT_FILE}
            COMMAND ${ASSEMBLER} /Fo ${ASM_OBJECT_FILE} ${ASM_FLAGS} ${ASM_SOURCE_FILE}
            DEPENDS ${ASM_SOURCE_FILE}
            COMMENT "Generating object file ${ASM_OBJECT_FILE} from assembly source ${ASM_SOURCE_FILE}"
            VERBATIM
    )

    # Add object files to list
    list(APPEND ASM_OBJECT_FILES "${ASM_OBJECT_FILE}")
endforeach()

# List to hold all .obj filenames generated from .cpp files
set(CPP_OBJECT_FILES "")

# Loop over all .cpp files and generate corresponding .obj files
foreach(CPP_SOURCE_FILE IN LISTS CPP_SOURCE_FILES)
    get_filename_component(CPP_FILENAME ${CPP_SOURCE_FILE} NAME_WE)
    set(CPP_OBJECT_FILE "${CMAKE_BINARY_DIR}/${CPP_FILENAME}.obj")

    add_custom_command(
            OUTPUT ${CPP_OBJECT_FILE}
            COMMAND ${CMAKE_CXX_COMPILER} ${CXX_COMPILER_FLAGS} /Fo${CPP_OBJECT_FILE} /std:c++20 /I${CMAKE_SOURCE_DIR}/include /c ${CPP_SOURCE_FILE}
            DEPENDS ${CPP_SOURCE_FILE}
            COMMENT "Generating object file ${CPP_OBJECT_FILE} from C++ source ${CPP_SOURCE_FILE}"
            VERBATIM
    )

    # Add object files to list
    list(APPEND CPP_OBJECT_FILES "${CPP_OBJECT_FILE}")
endforeach()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_executable(BOF_Template
            ${CPP_SOURCE_FILES}
            ${ASM_OBJECT_FILES}
    )
endif()

if (CMAKE_BUILD_TYPE MATCHES Release)
    # Combine all object files into a single object file
    add_custom_target(BOF_Template ALL
            DEPENDS ${ASM_OBJECT_FILES} ${CPP_OBJECT_FILES}
            COMMAND lib /OUT:${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/final.obj ${ASM_OBJECT_FILES} ${CPP_OBJECT_FILES}
    )

    # Rename the final object file to have the custom extension
    add_custom_command(
            TARGET BOF_Template
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rename "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/final.obj" "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/sleepmask${OUTEXT}"
            VERBATIM
    )
endif()

# Fixing warnings in my dev env
add_definitions(-D_WIN32_WINNT_WIN10_RS5=0x0A00)
add_definitions(-D_WIN32_WINNT_WIN10_TH2=0x0A00)
add_definitions(-D_WIN32_WINNT_WIN10_RS1=0x0A00)
add_definitions(-D_WIN32_WINNT_WIN10_RS2=0x0A00)
add_definitions(-D_WIN32_WINNT_WIN10_RS3=0x0A00)
add_definitions(-D_WIN32_WINNT_WIN10_RS4=0x0A00)