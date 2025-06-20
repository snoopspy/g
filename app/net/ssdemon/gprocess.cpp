#include "gprocess.h"
#include <errno.h>
#include <unistd.h>

// ----------------------------------------------------------------------------
// GProcess
// ----------------------------------------------------------------------------
bool GProcess::execute(std::string command, std::string& error) {
	int res = ::system(command.data());
	if (res != EXIT_SUCCESS) {
		error = "execute(\"" + command + "\") return " + std::to_string(res);
		return false;
	}
	return true;
}

#include <unistd.h> // for fork() and execvp()
pid_t GProcess::start(std::string command, std::string& error) {
	std::vector<std::string> arguments = splitCommand(command);
	size_t count = arguments.size();
	if (count == 0) {
		error = "splitCommand(\"" + command + "\") return nothing";
		return 0;
	}

	std::vector<char*> argv(count + 1);
	for (size_t i = 0; i < count; i++)
		argv[i] = (char*)(arguments[i].data());
	argv[count] = nullptr;

	pid_t pid = fork();
	if (pid == 0) { // child
		int res = execvp(argv[0], &argv[0]);
		fprintf(stderr, "not reachable execvp(%s) return %d", command.data(), res);
		exit(EXIT_FAILURE);
	}

	if (pid < 0) { // fail
		error = "fork() return " + std::to_string(pid) + " errno=" + std::to_string(errno).data();
		return 0;
	}

	return pid;
}

#include <wait.h> // for waitpid
bool GProcess::stop(pid_t pid, std::string& error) {
	pid_t _pid = pid;
	int killRes = kill(_pid, SIGTERM);

	int state;
	pid_t waitRes = waitpid(_pid, &state, 0);

	if (killRes != 0) {
		error = "kill(" + std::to_string(pid) + ") return " + std::to_string(killRes) +
			" waitpid return " + std::to_string(waitRes) + " state=" + std::to_string(state) +
			" errno=" + std::to_string(errno);
		return false;
	}
	return true;
}

bool GProcess::startDetached(std::string command, std::string& error) {
	command += " &";
	int res = ::system(command.data());
	if (res != EXIT_SUCCESS) {
		error = "execute(\"" + command + "\") return " + std::to_string(res);
		return false;
	}
	return true;
}

std::vector<std::string> GProcess::splitString(std::string s, char ch) {
	std::vector<std::string> res;

	size_t pos = s.find(ch);
	size_t initialPos = 0;
	res.clear();

	while(pos != std::string::npos) {
		res.push_back(s.substr(initialPos, pos - initialPos));
		initialPos = pos + 1;
		pos = s.find(ch, initialPos);
	}
	res.push_back(s.substr(initialPos, std::min(pos, s.size()) - initialPos + 1));

	return res;
}

std::vector<std::string> GProcess::splitCommand(std::string command) {
	std::vector<std::string> split = splitString(command, ' ');
	int index = 1;
	while (true) {
		int count = split.size();
		if (index >= count) break;

		std::string argument = split.at(index);
		char quotation = '\0';
		if (argument.at(0) == '\'') quotation = '\'';
		else if (argument.at(0) == '\"') quotation = '\"';
		if (quotation == '\0') {
			index++;
			continue;
		}

		int j = index + 1;
		bool merged = false;
		while (j < count) {
			argument = split.at(j);
			if (argument.at(argument.length() - 1) == quotation) {
				std::string mergeArgument;
				for (int k = index; k <= j; k++) {
					mergeArgument += split.at(k);
					if (k != j)
						mergeArgument += " ";
				}
				mergeArgument = mergeArgument.substr(1, mergeArgument.length() - 2);
				int mergeCount = j - index + 1;
				std::vector<std::string>::iterator eraseIndex = split.begin();
				for (int i = 0; i < index; i++) eraseIndex++;
				for (int k = 0; k < mergeCount; k++) split.erase(eraseIndex);
				split.insert(eraseIndex, mergeArgument);
				index++;
				merged = true;
				break;
			} else
				j++;
		}
		if (!merged) {
			return std::vector<std::string>(); // can not find quotation
		}
	}

	if (split.size() == 0) {
		return std::vector<std::string>(); // split count is zero
	}

	return split;
}

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

TEST(GProcess, executeTest) {
	std::string error;

	EXPECT_TRUE(GProcess::execute("ping -c 1 8.8.8.8", error));

	EXPECT_TRUE(GProcess::execute("su -c 'ping -c 1 8.8.8.8'", error));

	EXPECT_TRUE(GProcess::execute("su -c \"ping -c 1 8.8.8.8\"", error));
}

TEST(GProcess, startStopTest) {
	std::string error;

	pid_t pid = GProcess::start("ping -c 10 8.8.8.8", error);
	EXPECT_NE(pid, 0);
	EXPECT_TRUE(GProcess::stop(pid, error));

	pid = GProcess::start("su -c 'ping -c 10 8.8.8.8'", error);
	EXPECT_NE(pid, 0);
	EXPECT_TRUE(GProcess::stop(pid, error));

	pid = GProcess::start("su -c \"ping -c 10 8.8.8.8\"", error);
	EXPECT_NE(pid, 0);
	EXPECT_TRUE(GProcess::stop(pid, error));
}

TEST(GProcess, startDetachecTest) {
	std::string error;

	EXPECT_TRUE(GProcess::startDetached("ping -c 1 8.8.8.8", error));

	EXPECT_TRUE(GProcess::startDetached("su -c 'ping -c 1 8.8.8.8'", error));

	EXPECT_TRUE(GProcess::startDetached("su -c \"ping -c 1 8.8.8.8\"", error));
}

#endif // GTEST
