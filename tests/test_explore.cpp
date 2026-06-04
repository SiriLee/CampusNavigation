#include <windows.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct TestCase {
	fs::path directory;
	fs::path relativeName;
};

struct RunResult {
	bool launched = false;
	DWORD exitCode = 0;
	std::string error;
};

// Maximum time (ms) to wait for a single test case before declaring timeout
const DWORD PROCESS_TIMEOUT_MS = 30000;

std::string readFile(const fs::path& path) {
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open()) {
		return {};
	}
	return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

std::string normalizeText(std::string text) {
	std::string normalized;
	normalized.reserve(text.size());
	for (size_t i = 0; i < text.size(); ++i) {
		if (text[i] == '\r') {
			if (i + 1 < text.size() && text[i + 1] == '\n') {
				continue;
			}
			normalized.push_back('\n');
		} else {
			normalized.push_back(text[i]);
		}
	}
	return normalized;
}

static void rtrim_inplace(std::string& s) {
	while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) {
		s.pop_back();
	}
}

std::string normalizeLinesTrim(const std::string& text) {
	std::string out;
	size_t start = 0;
	while (start < text.size()) {
		size_t pos = text.find('\n', start);
		std::string line;
		if (pos == std::string::npos) {
			line = text.substr(start);
			start = text.size();
		} else {
			line = text.substr(start, pos - start);
			start = pos + 1;
		}
		rtrim_inplace(line);
		out.append(line);
		out.push_back('\n');
	}
	return out;
}

std::vector<std::string> splitWords(const std::string& line) {
	std::istringstream iss(line);
	std::vector<std::string> tokens;
	std::string token;
	while (iss >> token) {
		tokens.push_back(token);
	}
	return tokens;
}

// ─── Comparison for standard PATH lines (SHORTEST / TIMED_SHORTEST / MUST_PASS) ───

bool compareStandardPathLines(const std::string& actual, const std::string& expected) {
	if (actual == expected) {
		return true;
	}
	const auto actualTokens = splitWords(actual);
	const auto expectedTokens = splitWords(expected);
	if (actualTokens.size() < 5 || expectedTokens.size() < 5) {
		return false;
	}
	if (actualTokens[0] != "PATH" || expectedTokens[0] != "PATH") {
		return false;
	}
	// MODE and TOTAL_COST must match
	if (actualTokens[1] != expectedTokens[1] || actualTokens[2] != expectedTokens[2]) {
		return false;
	}
	if (actualTokens[3] != "NODES" || expectedTokens[3] != "NODES") {
		return false;
	}
	// Start and end nodes must match
	return actualTokens[4] == expectedTokens[4] && actualTokens.back() == expectedTokens.back();
}

// ─── Comparison for SHORTEST_K PATH lines ───
//
// Format: PATH <totalTime> K_USED <usedK> NODES <nodes...> FAST <count> [<edges...>]
//
// Strict fields: totalTime, usedK, first/last node, FAST count
// Relaxed fields: intermediate nodes, fast edge set (order-independent)

bool compareShortestKPathLines(const std::string& actual, const std::string& expected) {
	if (actual == expected) {
		return true;
	}

	// Helper: find keyword position, return npos if missing
	auto findKeyword = [](const std::vector<std::string>& tokens, const std::string& kw) -> size_t {
		for (size_t i = 0; i < tokens.size(); ++i) {
			if (tokens[i] == kw) return i;
		}
		return std::string::npos;
	};

	const auto at = splitWords(actual);
	const auto et = splitWords(expected);

	// Minimum: PATH <time> K_USED <k> NODES <from> <to> FAST <count>
	if (at.size() < 8 || et.size() < 8) return false;
	if (at[0] != "PATH" || et[0] != "PATH") return false;

	size_t aKU = findKeyword(at, "K_USED");
	size_t eKU = findKeyword(et, "K_USED");
	size_t aND = findKeyword(at, "NODES");
	size_t eND = findKeyword(et, "NODES");
	size_t aFS = findKeyword(at, "FAST");
	size_t eFS = findKeyword(et, "FAST");

	if (aKU == std::string::npos || eKU == std::string::npos) return false;
	if (aND == std::string::npos || eND == std::string::npos) return false;
	if (aFS == std::string::npos || eFS == std::string::npos) return false;

	// 1. totalTime must match (token immediately after "PATH")
	if (at[1] != et[1]) return false;

	// 2. usedK must match (token immediately after "K_USED")
	if (at[aKU + 1] != et[aKU + 1]) return false;

	// 3. First node (after "NODES") and last node (before "FAST") must match
	if (at[aND + 1] != et[eND + 1]) return false;
	if (at[aFS - 1] != et[eFS - 1]) return false;

	// 4. FAST count must match and equal usedK
	int aFastCount = std::stoi(at[aFS + 1]);
	int eFastCount = std::stoi(et[eFS + 1]);
	if (aFastCount != eFastCount) return false;
	int aUsedK = std::stoi(at[aKU + 1]);
	if (aUsedK != aFastCount) return false;

	// 5. FAST edges: set comparison (edges are normalized to from_id <= to_id
	//    and sorted lexicographically by the algorithm)
	if (aFastCount > 0) {
		std::set<std::string> aEdges, eEdges;
		for (size_t i = aFS + 2; i < at.size(); ++i) aEdges.insert(at[i]);
		for (size_t i = eFS + 2; i < et.size(); ++i) eEdges.insert(et[i]);
		if (aEdges != eEdges) return false;
	}

	return true;
}

// ─── Comparison for MST lines ───

bool compareMstLines(const std::string& actual, const std::string& expected) {
	if (actual == expected) {
		return true;
	}
	const auto actualTokens = splitWords(actual);
	const auto expectedTokens = splitWords(expected);
	if (actualTokens.size() < 3 || expectedTokens.size() < 3) {
		return false;
	}
	if (actualTokens[0] != "MST" || expectedTokens[0] != "MST") {
		return false;
	}
	if (actualTokens[1] != expectedTokens[1]) {
		return false;
	}
	if (actualTokens[2] != "EDGES" || expectedTokens[2] != "EDGES") {
		return false;
	}
	if (actualTokens.size() != expectedTokens.size()) {
		return false;
	}
	return true;
}

// ─── Comparison for CRITICAL lines ───

bool compareCriticalLines(const std::string& actual, const std::string& expected) {
	if (actual == expected) {
		return true;
	}
	const auto at = splitWords(actual);
	const auto et = splitWords(expected);
	if (at.size() < 4 || et.size() < 4) return false;
	if (at[0] != "CRITICAL" || et[0] != "CRITICAL") return false;
	if (at[1] != "NODES" || et[1] != "NODES") return false;

	auto findEdgesPos = [](const std::vector<std::string>& tokens) -> size_t {
		for (size_t i = 2; i < tokens.size(); ++i) {
			if (tokens[i] == "EDGES") return i;
		}
		return std::string::npos;
	};

	size_t aEP = findEdgesPos(at);
	size_t eEP = findEdgesPos(et);
	if (aEP == std::string::npos || eEP == std::string::npos) return false;

	if (std::stoi(at[2]) != std::stoi(et[2])) return false;

	std::set<std::string> aNodes, eNodes;
	for (size_t i = 3; i < aEP; ++i) aNodes.insert(at[i]);
	for (size_t i = 3; i < eEP; ++i) eNodes.insert(et[i]);
	if (aNodes != eNodes) return false;

	if (std::stoi(at[aEP + 1]) != std::stoi(et[eEP + 1])) return false;

	std::set<std::string> aEdges, eEdges;
	for (size_t i = aEP + 2; i < at.size(); ++i) aEdges.insert(at[i]);
	for (size_t i = eEP + 2; i < et.size(); ++i) eEdges.insert(et[i]);
	return aEdges == eEdges;
}

// ─── Per-line comparison dispatcher ───

bool compareOutputLine(const std::string& actual, const std::string& expected) {
	if (actual == expected) {
		return true;
	}

	// Detect SHORTEST_K PATH lines by the presence of "K_USED"
	bool aIsK = (actual.find("K_USED") != std::string::npos);
	bool eIsK = (expected.find("K_USED") != std::string::npos);

	if (aIsK && eIsK) {
		return compareShortestKPathLines(actual, expected);
	}

	// Standard PATH lines (SHORTEST, TIMED_SHORTEST, MUST_PASS)
	if (actual.rfind("PATH ", 0) == 0 && expected.rfind("PATH ", 0) == 0) {
		return compareStandardPathLines(actual, expected);
	}
	if (actual.rfind("MST ", 0) == 0 && expected.rfind("MST ", 0) == 0) {
		return compareMstLines(actual, expected);
	}
	if (actual.rfind("CRITICAL ", 0) == 0 && expected.rfind("CRITICAL ", 0) == 0) {
		return compareCriticalLines(actual, expected);
	}
	return false;
}

// ─── Test case discovery ───

std::vector<TestCase> discoverCases(const fs::path& root) {
	std::vector<TestCase> cases;
	if (!fs::exists(root)) {
		return cases;
	}

	for (const auto& entry : fs::recursive_directory_iterator(root)) {
		if (!entry.is_directory()) {
			continue;
		}
		const fs::path directory = entry.path();
		if (fs::exists(directory / "command.txt") && fs::exists(directory / "answer.txt")) {
			cases.push_back(TestCase{directory, fs::relative(directory, root)});
		}
	}

	std::sort(cases.begin(), cases.end(), [](const TestCase& lhs, const TestCase& rhs) {
		return lhs.relativeName.generic_string() < rhs.relativeName.generic_string();
	});
	return cases;
}

// ─── Helpers ───

bool ensureParentDirectory(const fs::path& path) {
	std::error_code ec;
	fs::create_directories(path.parent_path(), ec);
	return !ec;
}

std::string makeSafeName(const fs::path& relativeName) {
	std::string name = relativeName.generic_string();
	std::replace(name.begin(), name.end(), '/', '_');
	return name;
}

// ─── Process launcher ───

RunResult runProgram(const fs::path& executable, const fs::path& workingDirectory,
                     const fs::path& inputFile, const fs::path& outputFile) {
	RunResult result;

	SECURITY_ATTRIBUTES attributes{};
	attributes.nLength = sizeof(attributes);
	attributes.bInheritHandle = TRUE;

	HANDLE inputHandle = CreateFileA(
		inputFile.string().c_str(), GENERIC_READ, FILE_SHARE_READ,
		&attributes, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (inputHandle == INVALID_HANDLE_VALUE) {
		result.error = "cannot_open_command_file";
		return result;
	}

	HANDLE outputHandle = CreateFileA(
		outputFile.string().c_str(), GENERIC_WRITE, 0,
		&attributes, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (outputHandle == INVALID_HANDLE_VALUE) {
		CloseHandle(inputHandle);
		result.error = "cannot_create_output_file";
		return result;
	}

	STARTUPINFOA startupInfo{};
	startupInfo.cb = sizeof(startupInfo);
	startupInfo.dwFlags = STARTF_USESTDHANDLES;
	startupInfo.hStdInput = inputHandle;
	startupInfo.hStdOutput = outputHandle;
	startupInfo.hStdError = outputHandle;

	PROCESS_INFORMATION processInfo{};
	std::string commandLine = '"' + executable.string() + '"';
	std::vector<char> mutableCommandLine(commandLine.begin(), commandLine.end());
	mutableCommandLine.push_back('\0');

	BOOL created = CreateProcessA(
		nullptr, mutableCommandLine.data(), nullptr, nullptr, TRUE, 0,
		nullptr, workingDirectory.string().c_str(), &startupInfo, &processInfo);

	CloseHandle(inputHandle);
	CloseHandle(outputHandle);

	if (!created) {
		result.error = "cannot_launch_program";
		return result;
	}

	result.launched = true;
	DWORD waitResult = WaitForSingleObject(processInfo.hProcess, PROCESS_TIMEOUT_MS);
	if (waitResult == WAIT_TIMEOUT) {
		TerminateProcess(processInfo.hProcess, 1);
		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);
		result.launched = false;
		result.error = "process_timeout";
		return result;
	}
	DWORD exitCode = 0;
	if (!GetExitCodeProcess(processInfo.hProcess, &exitCode)) {
		exitCode = static_cast<DWORD>(-1);
	}
	result.exitCode = exitCode;

	CloseHandle(processInfo.hThread);
	CloseHandle(processInfo.hProcess);
	return result;
}

// ─── Main ───

int main(int argc, char* argv[]) {
	if (argc < 4) {
		std::cerr << "usage: CampusNavigationExploreTests <app_exe> <explore_root> <result_root>\n";
		return 2;
	}

	const fs::path executable = fs::absolute(argv[1]);
	const fs::path root = fs::absolute(argv[2]);
	const fs::path resultRoot = fs::absolute(argv[3]);

	const auto cases = discoverCases(root);
	if (cases.empty()) {
		std::cerr << "no test cases found under " << root.string() << '\n';
		return 2;
	}

	std::vector<std::string> failedCases;
	size_t passed = 0;

	for (const auto& testCase : cases) {
		const fs::path outputFile = resultRoot / (makeSafeName(testCase.relativeName) + "_output.txt");
		if (!ensureParentDirectory(outputFile)) {
			failedCases.push_back(testCase.relativeName.generic_string() + ": cannot_prepare_output_directory");
			std::cout << "FAIL " << testCase.relativeName.generic_string() << " cannot_prepare_output_directory\n";
			continue;
		}

		const RunResult runResult = runProgram(executable, testCase.directory,
		                                       testCase.directory / "command.txt", outputFile);
		if (!runResult.launched) {
			failedCases.push_back(testCase.relativeName.generic_string() + ": " + runResult.error);
			std::cout << "FAIL " << testCase.relativeName.generic_string() << ' ' << runResult.error << '\n';
			continue;
		}

		if (runResult.exitCode != 0) {
			failedCases.push_back(testCase.relativeName.generic_string() + ": process_exit_" + std::to_string(runResult.exitCode));
			std::cout << "FAIL " << testCase.relativeName.generic_string() << " process_exit_" << runResult.exitCode << '\n';
			continue;
		}

		const std::string actual = normalizeLinesTrim(normalizeText(readFile(outputFile)));
		const std::string expected = normalizeLinesTrim(normalizeText(readFile(testCase.directory / "answer.txt")));

		if (actual == expected) {
			++passed;
			std::cout << "PASS " << testCase.relativeName.generic_string() << '\n';
			continue;
		}

		bool same = true;
		size_t actualLineStart = 0;
		size_t expectedLineStart = 0;
		while (actualLineStart < actual.size() || expectedLineStart < expected.size()) {
			size_t actualLineEnd = actual.find('\n', actualLineStart);
			if (actualLineEnd == std::string::npos) actualLineEnd = actual.size();
			size_t expectedLineEnd = expected.find('\n', expectedLineStart);
			if (expectedLineEnd == std::string::npos) expectedLineEnd = expected.size();
			std::string actualLine = actual.substr(actualLineStart, actualLineEnd - actualLineStart);
			std::string expectedLine = expected.substr(expectedLineStart, expectedLineEnd - expectedLineStart);
			if (!compareOutputLine(actualLine, expectedLine)) {
				same = false;
				break;
			}
			actualLineStart = (actualLineEnd == actual.size()) ? actual.size() : actualLineEnd + 1;
			expectedLineStart = (expectedLineEnd == expected.size()) ? expected.size() : expectedLineEnd + 1;
		}

		if (!same) {
			failedCases.push_back(testCase.relativeName.generic_string() + ": output_mismatch");
			std::cout << "FAIL " << testCase.relativeName.generic_string() << " output_mismatch\n";
			continue;
		}

		++passed;
		std::cout << "PASS " << testCase.relativeName.generic_string() << '\n';
	}

	const size_t total = cases.size();
	const size_t failed = total - passed;
	std::cout << "TOTAL " << total << " PASS " << passed << " FAIL " << failed << '\n';

	if (!failedCases.empty()) {
		std::cout << "FAILED_CASES\n";
		for (const auto& item : failedCases) {
			std::cout << item << '\n';
		}
		return 1;
	}

	return 0;
}
