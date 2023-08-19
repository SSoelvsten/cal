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

option(CAL_DOCS "Build Documentation for CAL" ON)
message(STATUS "  | Documentation (Doxygen): ${CAL_DOCS}")

option(CAL_TEST "Build Documentation for CAL" ON)
message(STATUS "  | Test Executables: ${CAL_TEST}")

message(STATUS "")

# ============================================================================ #
# docs/
# ============================================================================ #
if (CAL_DOCS)
  add_subdirectory(docs)
endif (CAL_DOCS)

# ============================================================================ #
# src/
# ============================================================================ #
add_subdirectory (src)

# ============================================================================ #
# test/
# ============================================================================ #
if (CAL_TEST)
  add_subdirectory (test)
endif (CAL_TEST)
