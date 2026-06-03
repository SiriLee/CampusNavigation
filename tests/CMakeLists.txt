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