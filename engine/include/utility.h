#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <windows.h>
#include <iostream>

inline void PrintMemoryStatus()
{
	MEMORYSTATUSEX statex; statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);
	std::cout << "There is " << statex.ullAvailPhys / (1024 * 1024) << " MB of physical memory available.\n";
	std::cout << "There is " << statex.ullAvailVirtual / (1024 * 1024) << " MB of virtual memory available.\n";
}

inline bool HasEnoughMemory(u64 pRequiredBytes)
{
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);

	std::cout << "There is " << statex.ullAvailPhys / (1024 * 1024) << " MB of physical memory available.\n";
	std::cout << "There is " << statex.ullAvailVirtual / (1024 * 1024) << " MB of virtual memory available.\n";

	if (statex.ullAvailPhys >= pRequiredBytes && statex.ullAvailVirtual >= pRequiredBytes)
	{
		return true;
	}

	return false;
}

inline void ReadFile(const std::string &path, std::vector<char> &outBuffer)
{
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file.");
	}

	size_t fileSize = file.tellg();

	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	outBuffer = buffer;
}

inline void ReadFile(const std::string &path, std::vector<u32> &outBuffer)
{
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file.");
	}

	size_t fileSize = file.tellg();

	std::vector<u32> buffer(fileSize / sizeof(u32));

	file.seekg(0);
	file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
	file.close();

	outBuffer = buffer;
}

// NOTE(Sergei): Why doesn't C++ standard library have a function for trimming a string? I'm spoiled by C#. Thanks, stackoverflow.
inline void rtrim(std::string &s)
{
	auto pred = [](unsigned char ch)
	{
		return !std::isspace(ch);
	};

	s.erase(std::find_if(s.rbegin(), s.rend(), pred).base(), s.end());
}