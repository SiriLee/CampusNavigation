add_executable(CampusNavigationTests
	test_core.cpp
)

target_compile_features(CampusNavigationTests PRIVATE cxx_std_17)
target_include_directories(CampusNavigationTests PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_test(
	NAME must_do
	COMMAND CampusNavigationTests
		$<TARGET_FILE:CampusNavigation>
		${CMAKE_SOURCE_DIR}/test_data/must_do
		${CMAKE_BINARY_DIR}/test_outputs
)

# ─── Extended feature tests (SHORTEST_K / shared bike coupons) ───

add_executable(CampusNavigationExploreTests
	test_explore.cpp
)

target_compile_features(CampusNavigationExploreTests PRIVATE cxx_std_17)
target_include_directories(CampusNavigationExploreTests PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_test(
	NAME explore
	COMMAND CampusNavigationExploreTests
		$<TARGET_FILE:CampusNavigation>
		${CMAKE_SOURCE_DIR}/test_data/explore
		${CMAKE_BINARY_DIR}/test_outputs_explore
)

# ─── Custom scenario tests ───

add_executable(CampusNavigationCustomTests
	test_custom.cpp
)

target_compile_features(CampusNavigationCustomTests PRIVATE cxx_std_17)
target_include_directories(CampusNavigationCustomTests PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_test(
	NAME custom
	COMMAND CampusNavigationCustomTests
		$<TARGET_FILE:CampusNavigation>
		${CMAKE_SOURCE_DIR}/test_data/custom
		${CMAKE_BINARY_DIR}/test_outputs_custom
)

# ─── Adversarial / edge-case tests ───

add_executable(CampusNavigationContrastTests
	test_contrast.cpp
)

target_compile_features(CampusNavigationContrastTests PRIVATE cxx_std_17)
target_include_directories(CampusNavigationContrastTests PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_test(
	NAME contrast
	COMMAND CampusNavigationContrastTests
		$<TARGET_FILE:CampusNavigation>
		${CMAKE_SOURCE_DIR}/test_data/contrast
		${CMAKE_BINARY_DIR}/test_outputs_contrast
)