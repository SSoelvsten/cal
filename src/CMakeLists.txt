set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ============================================================================ #
# List of all files

set(CAL_C_HEADERS
  cal.h
  calInt.h
  calMem.h
)

set(CAL_C_SOURCES
  cal.c calBddOp.c
  calBddManager.c
  calMemoryManagement.c
	calHashTable.c
  calUtil.c
  calGC.c
  calTerminal.c
  calAssociation.c
  calBddSubstitute.c
  calReduce.c
  calQuant.c
  calBddSwapVars.c
  calBddSatisfy.c
  calBddSize.c
  calBddSupport.c
  calPrint.c
  calPrintProfile.c
  calDump.c
	calHashTableOne.c
  calPipeline.c
  calHashTableThree.c
  calBddITE.c
  calBddCompose.c
	calCacheTableTwo.c
  calApplyReduce.c
  calBlk.c
	calReorderBF.c
  calReorderDF.c
  calInteract.c
	calBddVarSubstitute.c
  calReorderUtil.c
  calMem.c
)

# ============================================================================ #
# Setup as library
if (CAL_SHARED)
  add_library(cal SHARED ${CAL_C_HEADERS} ${CAL_C_SOURCES})
else()
  add_library(cal STATIC ${CAL_C_HEADERS} ${CAL_C_SOURCES})
endif()

include(GenerateExportHeader)
generate_export_header(cal
  EXPORT_MACRO_NAME CAL_API
)

if (UNIX)
  execute_process(COMMAND "getconf" "PAGESIZE"
    RESULT_VARIABLE UNIX_PS_CODE
    OUTPUT_VARIABLE UNIX_PS_STDOUT)
  if (UNIX_PS_CODE EQUAL 0)
    set (CAL_PAGE_SIZE "${UNIX_PS_STDOUT}")
  endif ()
endif (UNIX)
if (NOT DEFINED CAL_PAGE_SIZE)
  set (CAL_PAGE_SIZE "4096")
endif()

target_compile_definitions(cal PRIVATE PAGE_SIZE=${CAL_PAGE_SIZE})

target_include_directories(cal PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
                                      ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(cal PROPERTIES
  # Language settings
  LINKER_LANGUAGE C
#  C_STANDARD 17
#  C_STANDARD_REQUIRED ON
#  CXX_STANDARD 17
#  CXX_STANDARD_REQUIRED ON
#  CXX_VISIBILITY_PRESET hidden

  # Linkage?
#  FOLDER cal

  # Other
  VERSION ${PROJECT_VERSION}
  POSITION_INDEPENDENT_CODE ON
)

install(TARGETS cal
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${LIBLEGACY_INCLUDE_DIRS}
)
