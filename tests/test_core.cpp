// tests/test_core.cpp — Must-do test suite for basic features
#include "test_common.h"

static bool compareOutputLine(const std::string& actual, const std::string& expected) {
	if (actual == expected) return true;
	if (actual.rfind("PATH ", 0) == 0 && expected.rfind("PATH ", 0) == 0)
		return compareStandardPathLines(actual, expected);
	if (actual.rfind("MST ", 0) == 0 && expected.rfind("MST ", 0) == 0)
		return compareMstLines(actual, expected);
	if (actual.rfind("CRITICAL ", 0) == 0 && expected.rfind("CRITICAL ", 0) == 0)
		return compareCriticalLines(actual, expected);
	return false;
}

int main(int argc, char* argv[]) {
	return runTestSuite(argc, argv,
		"usage: CampusNavigationTests <app_exe> <must_do_root> <result_root>",
		compareOutputLine);
}
