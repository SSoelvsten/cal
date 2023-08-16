# ============================================================================ #
# BDD Reordering Tests
add_executable (test_reorder calBddReorderTest.c)
target_link_libraries(test_reorder cal)

# ============================================================================ #
# Performance Tests
add_executable (test_performance calPerformanceTest.c)
target_link_libraries(test_performance cal)

# ============================================================================ #
# Cache Table Two Test
add_executable (test_cachetabletwo calCacheTableTwo.c)
target_link_libraries(test_cachetabletwo cal)

# ============================================================================ #
# Util Test
add_executable (test_util calUtil.c)
target_link_libraries(test_util cal)

# ============================================================================ #
# Unit Tests
add_executable (test_unit calTest.c)
target_link_libraries(test_unit cal)

target_compile_definitions(test_unit PRIVATE TEST)
