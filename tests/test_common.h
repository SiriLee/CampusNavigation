// tests/test_common.h — Shared test infrastructure for CampusNavigation test programs.
//
// Each test executable (must_do, explore, custom) includes this header and
// provides its own compareOutputLine() + main().
//
// Usage in CMakeLists.txt:
//   target_include_directories(<target> PRIVATE ${CMAKE_SOURCE_DIR}/include)
//   target_compile_features(<target> PRIVATE cxx_std_17)

#pragma once

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

// ─── types ───────────────────────────────────────────────────────────────────

struct TestCase {
	fs::path directory;
	fs::path relativeName;
};

struct RunResult {
	bool launched = false;
	DWORD exitCode = 0;
	std::string error;
};

inline const DWORD PROCESS_TIMEOUT_MS = 30000;

// ─── file / text utilities ───────────────────────────────────────────────────

inline std::string readFile(const fs::path& path) {
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open()) return {};
	return std::string((std::istreambuf_iterator<char>(file)),
	                   std::istreambuf_iterator<char>());
}

inline std::string normalizeText(std::string text) {
	std::string normalized;
	normalized.reserve(text.size());
	for (size_t i = 0; i < text.size(); ++i) {
		if (text[i] == '\r') {
			if (i + 1 < text.size() && text[i + 1] == '\n') continue;
			normalized.push_back('\n');
		} else {
			normalized.push_back(text[i]);
		}
	}
	return normalized;
}

inline void rtrim_inplace(std::string& s) {
	while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.pop_back();
}

inline std::string normalizeLinesTrim(const std::string& text) {
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

inline std::vector<std::string> splitWords(const std::string& line) {
	std::istringstream iss(line);
	std::vector<std::string> tokens;
	std::string token;
	while (iss >> token) tokens.push_back(token);
	return tokens;
}

// ─── comparison helpers ──────────────────────────────────────────────────────
// Each test executable provides its own compareOutputLine() that calls these.

inline bool compareStandardPathLines(const std::string& actual, const std::string& expected) {
	if (actual == expected) return true;
	const auto at = splitWords(actual);
	const auto et = splitWords(expected);
	if (at.size() < 5 || et.size() < 5) return false;
	if (at[0] != "PATH" || et[0] != "PATH") return false;
	if (at[1] != et[1] || at[2] != et[2]) return false;       // MODE, TOTAL_COST
	if (at[3] != "NODES" || et[3] != "NODES") return false;
	return at[4] == et[4] && at.back() == et.back();           // first & last node
}

inline bool compareShortestKPathLines(const std::string& actual, const std::string& expected) {
	if (actual == expected) return true;

	auto findKw = [](const std::vector<std::string>& t, const std::string& kw) -> size_t {
		for (size_t i = 0; i < t.size(); ++i) if (t[i] == kw) return i;
		return std::string::npos;
	};

	const auto at = splitWords(actual);
	const auto et = splitWords(expected);
	if (at.size() < 8 || et.size() < 8) return false;
	if (at[0] != "PATH" || et[0] != "PATH") return false;

	size_t aKU = findKw(at, "K_USED"), eKU = findKw(et, "K_USED");
	size_t aND = findKw(at, "NODES"),  eND = findKw(et, "NODES");
	size_t aFS = findKw(at, "FAST"),   eFS = findKw(et, "FAST");
	if (aKU == std::string::npos || eKU == std::string::npos) return false;
	if (aND == std::string::npos || eND == std::string::npos) return false;
	if (aFS == std::string::npos || eFS == std::string::npos) return false;

	if (at[1] != et[1]) return false;               // totalTime
	if (at[aKU + 1] != et[eKU + 1]) return false;   // usedK
	if (at[aND + 1] != et[eND + 1]) return false;   // first node
	if (at[aFS - 1] != et[eFS - 1]) return false;   // last node

	int aCnt = std::stoi(at[aFS + 1]), eCnt = std::stoi(et[eFS + 1]);
	if (aCnt != eCnt) return false;                  // FAST count
	if (std::stoi(at[aKU + 1]) != aCnt) return false; // usedK == FAST count

	if (aCnt > 0) {
		std::set<std::string> aEdges, eEdges;
		for (size_t i = aFS + 2; i < at.size(); ++i) aEdges.insert(at[i]);
		for (size_t i = eFS + 2; i < et.size(); ++i) eEdges.insert(et[i]);
		if (aEdges != eEdges) return false;
	}
	return true;
}

inline bool compareMstLines(const std::string& actual, const std::string& expected) {
	if (actual == expected) return true;
	const auto at = splitWords(actual);
	const auto et = splitWords(expected);
	if (at.size() < 3 || et.size() < 3) return false;
	if (at[0] != "MST" || et[0] != "MST") return false;
	if (at[1] != et[1]) return false;
	if (at[2] != "EDGES" || et[2] != "EDGES") return false;
	if (at.size() != et.size()) return false;
	return true;
}

inline bool compareCriticalLines(const std::string& actual, const std::string& expected) {
	if (actual == expected) return true;
	const auto at = splitWords(actual);
	const auto et = splitWords(expected);
	if (at.size() < 4 || et.size() < 4) return false;
	if (at[0] != "CRITICAL" || et[0] != "CRITICAL") return false;
	if (at[1] != "NODES" || et[1] != "NODES") return false;

	auto findEP = [](const std::vector<std::string>& t) -> size_t {
		for (size_t i = 2; i < t.size(); ++i) if (t[i] == "EDGES") return i;
		return std::string::npos;
	};
	size_t aEP = findEP(at), eEP = findEP(et);
	if (aEP == std::string::npos || eEP == std::string::npos) return false;

	if (std::stoi(at[2]) != std::stoi(et[2])) return false;
	std::set<std::string> aN, eN;
	for (size_t i = 3; i < aEP; ++i) aN.insert(at[i]);
	for (size_t i = 3; i < eEP; ++i) eN.insert(et[i]);
	if (aN != eN) return false;
	if (std::stoi(at[aEP + 1]) != std::stoi(et[eEP + 1])) return false;
	std::set<std::string> aE, eE;
	for (size_t i = aEP + 2; i < at.size(); ++i) aE.insert(at[i]);
	for (size_t i = eEP + 2; i < et.size(); ++i) eE.insert(et[i]);
	return aE == eE;
}

// ─── discovery / filesystem ──────────────────────────────────────────────────

inline std::vector<TestCase> discoverCases(const fs::path& root) {
	std::vector<TestCase> cases;
	if (!fs::exists(root)) return cases;

	// Check root directory itself (for single-directory test data)
	if (fs::exists(root / "command.txt") && fs::exists(root / "answer.txt"))
		cases.push_back({root, "."});

	// Scan subdirectories
	for (const auto& entry : fs::recursive_directory_iterator(root)) {
		if (!entry.is_directory()) continue;
		const fs::path d = entry.path();
		if (d == root) continue; // already checked above
		if (fs::exists(d / "command.txt") && fs::exists(d / "answer.txt"))
			cases.push_back({d, fs::relative(d, root)});
	}
	std::sort(cases.begin(), cases.end(), [](const TestCase& a, const TestCase& b) {
		return a.relativeName.generic_string() < b.relativeName.generic_string();
	});
	return cases;
}

inline bool ensureParentDirectory(const fs::path& path) {
	std::error_code ec;
	fs::create_directories(path.parent_path(), ec);
	return !ec;
}

inline std::string makeSafeName(const fs::path& relativeName) {
	std::string name = relativeName.generic_string();
	std::replace(name.begin(), name.end(), '/', '_');
	return name;
}

// ─── process launcher ────────────────────────────────────────────────────────

inline RunResult runProgram(const fs::path& executable, const fs::path& workingDir,
                            const fs::path& inputFile, const fs::path& outputFile) {
	RunResult result;
	SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};

	HANDLE hIn = CreateFileA(inputFile.string().c_str(), GENERIC_READ,
	                         FILE_SHARE_READ, &sa, OPEN_EXISTING,
	                         FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hIn == INVALID_HANDLE_VALUE) { result.error = "cannot_open_command_file"; return result; }

	HANDLE hOut = CreateFileA(outputFile.string().c_str(), GENERIC_WRITE, 0,
	                          &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hOut == INVALID_HANDLE_VALUE) { CloseHandle(hIn);
		result.error = "cannot_create_output_file"; return result; }

	STARTUPINFOA si{sizeof(si)};
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = hIn;  si.hStdOutput = hOut;  si.hStdError = hOut;

	std::string cmdLine = '"' + executable.string() + '"';
	std::vector<char> mutCmd(cmdLine.begin(), cmdLine.end());
	mutCmd.push_back('\0');

	PROCESS_INFORMATION pi{};
	BOOL ok = CreateProcessA(nullptr, mutCmd.data(), nullptr, nullptr, TRUE,
	                         0, nullptr, workingDir.string().c_str(), &si, &pi);
	CloseHandle(hIn); CloseHandle(hOut);
	if (!ok) { result.error = "cannot_launch_program"; return result; }

	result.launched = true;
	if (WaitForSingleObject(pi.hProcess, PROCESS_TIMEOUT_MS) == WAIT_TIMEOUT) {
		TerminateProcess(pi.hProcess, 1);
		CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
		result.launched = false;
		result.error = "process_timeout";
		return result;
	}
	DWORD ec = 0;
	if (!GetExitCodeProcess(pi.hProcess, &ec)) ec = static_cast<DWORD>(-1);
	result.exitCode = ec;
	CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
	return result;
}

// ─── generic test runner (template-method pattern) ───────────────────────────
//
// Each test program calls runTestSuite() with:
//   - executable path, data root, result root
//   - a lambda `compareOutputLine(const std::string&, const std::string&) -> bool`
//
// Returns 0 on all-pass, 1 on any failure, 2 on usage error.

template <typename CompareFn>
int runTestSuite(int argc, char* argv[], const char* usageLine, CompareFn compareOutputLine) {
	if (argc < 4) {
		std::cerr << usageLine << '\n';
		return 2;
	}
	const fs::path executable = fs::absolute(argv[1]);
	const fs::path root       = fs::absolute(argv[2]);
	const fs::path resultRoot = fs::absolute(argv[3]);

	const auto cases = discoverCases(root);
	if (cases.empty()) {
		std::cerr << "no test cases found under " << root.string() << '\n';
		return 2;
	}

	std::vector<std::string> failedCases;
	size_t passed = 0;

	for (const auto& tc : cases) {
		const fs::path outFile = resultRoot / (makeSafeName(tc.relativeName) + "_output.txt");
		if (!ensureParentDirectory(outFile)) {
			failedCases.push_back(tc.relativeName.generic_string() + ": cannot_prepare_output_directory");
			std::cout << "FAIL " << tc.relativeName.generic_string() << " cannot_prepare_output_directory\n";
			continue;
		}

		const RunResult rr = runProgram(executable, tc.directory, tc.directory / "command.txt", outFile);
		if (!rr.launched) {
			failedCases.push_back(tc.relativeName.generic_string() + ": " + rr.error);
			std::cout << "FAIL " << tc.relativeName.generic_string() << ' ' << rr.error << '\n';
			continue;
		}
		if (rr.exitCode != 0) {
			failedCases.push_back(tc.relativeName.generic_string() + ": process_exit_" + std::to_string(rr.exitCode));
			std::cout << "FAIL " << tc.relativeName.generic_string() << " process_exit_" << rr.exitCode << '\n';
			continue;
		}

		const std::string actual   = normalizeLinesTrim(normalizeText(readFile(outFile)));
		const std::string expected = normalizeLinesTrim(normalizeText(readFile(tc.directory / "answer.txt")));

		if (actual == expected) { ++passed;
			std::cout << "PASS " << tc.relativeName.generic_string() << '\n'; continue; }

		bool same = true;
		size_t al = 0, el = 0;
		while (al < actual.size() || el < expected.size()) {
			size_t aEnd = actual.find('\n', al);
			if (aEnd == std::string::npos) aEnd = actual.size();
			size_t eEnd = expected.find('\n', el);
			if (eEnd == std::string::npos) eEnd = expected.size();

			if (!compareOutputLine(actual.substr(al, aEnd - al),
			                       expected.substr(el, eEnd - el))) {
				same = false; break;
			}
			al = (aEnd == actual.size())   ? actual.size()   : aEnd + 1;
			el = (eEnd == expected.size()) ? expected.size() : eEnd + 1;
		}

		if (!same) {
			failedCases.push_back(tc.relativeName.generic_string() + ": output_mismatch");
			std::cout << "FAIL " << tc.relativeName.generic_string() << " output_mismatch\n";
		} else {
			++passed;
			std::cout << "PASS " << tc.relativeName.generic_string() << '\n';
		}
	}

	const size_t total = cases.size();
	std::cout << "TOTAL " << total << " PASS " << passed << " FAIL " << (total - passed) << '\n';
	if (!failedCases.empty()) {
		std::cout << "FAILED_CASES\n";
		for (const auto& item : failedCases) std::cout << item << '\n';
		return 1;
	}
	return 0;
}
