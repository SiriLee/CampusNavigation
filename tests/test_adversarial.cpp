// tests/test_contrast.cpp — Adversarial / edge-case test suite
#include "test_common.h"

static bool compareOutputLine(const std::string& actual, const std::string& expected) {
	if (actual == expected) return true;

	bool aIsK = (actual.find("K_USED") != std::string::npos);
	bool eIsK = (expected.find("K_USED") != std::string::npos);
	if (aIsK && eIsK)
		return compareShortestKPathLines(actual, expected);

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
		"usage: CampusNavigationContrastTests <app_exe> <contrast_root> <result_root>",
		compareOutputLine);
}
