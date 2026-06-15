#pragma once

#include <vector>
#include <string>


class Process
{
public:
	// TODO: Add the attached core
	Process();
	Process(const int processID, const std::string& processName) : id(processID), name(processName) {}
	~Process() = default;

	void Initialize(const int processID, const std::string& processName);
	void Run();
	void Pause();
	void Resume();
	void Terminate();

	void PrintInfo() const;

	int GetID() const;
	std::string GetName() const;
	std::string GetStatus() const;
	std::string GetStartTime() const;
	std::string GetEndTime() const;

private:
	int id;
	std::string name;
};

