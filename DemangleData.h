#pragma once

#include <iostream>
#include <vector>
#include <map>

using namespace std;

class DemangleData
{
private:
	vector<string> arguments;
	vector<string> fragments;
	vector<map<vector<string>, vector<string>>> history;
	string value;
	bool verbose;

public:
	DemangleData(string value, bool verbose);
	string GetValue();
	void SetValue(string value);
	int GetArgumentsSize();
	string GetArgument(int index);
	string GetFragment(int index);
	void Advance(int count);
	int Index(char pos);
	void AddFragment(string fragment);
	void AddArgument(string argument);
	void EnterTemplate();
	void ExitTemplate();
	bool IsInTemplate();
	string Join(const vector<string>& vector, string delimiter);

	template <typename... Args>
	void Log(char const* const format, Args const&... args) noexcept
	{
		if (verbose)
		{
			printf_s(format, args...);

			printf_s(" REST=%s", value.c_str());
			printf_s(" ARG=%s", Join(arguments, ",").c_str());
			printf_s(" FRAG=%s\n", Join(fragments, ",").c_str());
		}
	}
};
