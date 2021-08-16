#include "DemangleData.h"

DemangleData::DemangleData(string value, bool verbose)
{
	/*
	* The main data is the input string.
	* But we also store, for backreferences, the list of name fragments
	* and the list of arguments (non-primitive types only).
	*/

	this->value = value;
	this->verbose = verbose;
}

string DemangleData::GetValue()
{
	return value;
}

void DemangleData::SetValue(string value)
{
	this->value = value;
}

int DemangleData::GetArgumentsSize()
{
	return static_cast<int>(arguments.size());
}

string DemangleData::GetArgument(int index)
{
	return arguments[index];
}

string DemangleData::GetFragment(int index)
{
	return fragments[index];
}

void DemangleData::Advance(int count)
{
	/*
	* This is linear in the size of self.value, therefore the complexity
	* of symbol_demangle_reentrant is quadratic, which makes it vulnerable
	* to DoS attacks. That's why DemangleData's interface can support an
	* implementation such that self.value is constant and the current
	* position in self.value is stored in e.g. self.pos; I chose to
	* modify self.value, it seems more understandable.
	*/

	value = value.substr(count, value.length() - count);
}

int DemangleData::Index(char pos)
{
	return static_cast<int>(value.find(pos));
}

void DemangleData::AddFragment(string fragment)
{
	fragments.push_back(fragment);
}

void DemangleData::AddArgument(string argument)
{
	arguments.push_back(argument);
}

void DemangleData::EnterTemplate()
{
	map<vector<string>, vector<string>> map;

	map.insert(make_pair(fragments, arguments));
	history.push_back(map);

	fragments.clear();
	arguments.clear();
}

void DemangleData::ExitTemplate()
{
	map<vector<string>, vector<string>> map = history.back();

	fragments = map.begin()->first;
	arguments = map.begin()->second;

	history.pop_back();
}

bool DemangleData::IsInTemplate()
{
	//'?' data type depends on whether we are in a template

	return history.size() > 0;
}

string DemangleData::Join(const vector<string>& vector, string delimiter)
{
	string result;

	for (auto it = vector.begin(); it != vector.end(); it++)
	{
		result += *it;

		if (it != vector.end() - 1)
		{
			result += delimiter;
		}
	}

	return result;
}
