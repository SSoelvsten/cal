cmake_minimum_required(VERSION 3.10)
project (cal
  VERSION 3.0.0
  DESCRIPTION "CAL: University of California BDD Package"
  HOMEPAGE_URL "https://ssoelvsten.github.io/cal/"
  LANGUAGES CXX
)

enable_language(C)

# ============================================================================ #
# Settings
# ============================================================================ #

message(STATUS "CMake build configuration for CAL ${PROJECT_VERSION}")

message(STATUS "  OS: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "    C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "    CXX Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "    EXE Linker Flags: ${CMAKE_EXE_LINKER_FLAGS}")

option(CAL_SHARED "Build CAL as a shared library" OFF)
message(STATUS "  Shared: ${CAL_SHARED}")

message(STATUS "  Constants:")

set(CAL_PAGE_SIZE "4096")
message(STATUS "    Page Size: ${CAL_PAGE_SIZE}")

message(STATUS "")

# ============================================================================ #
# src/
# ============================================================================ #
add_subdirectory (src)

# ============================================================================ #
# test/
# ============================================================================ #
add_subdirectory (test)
