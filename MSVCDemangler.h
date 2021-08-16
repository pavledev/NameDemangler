#pragma once

#include <iostream>
#include <map>
#include "DemangleData.h"
#include "DataType.h"

using namespace std;

class MSVCDemangler
{
public:
	MSVCDemangler();
	string DemangleSymbol(string symbol, string& rest, bool verbose = false);

private:
	string quoteB = "`";
	string quoteE = "'";

	map<string, string> callingConventions;
	map<string, multimap<string, vector<string>>> dataTypes;
	map<string, string> enumTypes;
	map<string, string> specialFragments;
	map<string, multimap<string, string>> thunkAccesses;

	string DemangleReentrantSymbol(DemangleData* data);
	tuple<DataType, string, string> DemangleFunctionPrototypeSymbol(DemangleData* data);
	string DemangleLocalStaticGuardSymbol(vector<string> names, DemangleData* data);
	string DemangleVariableSymbol(vector<string> names, DemangleData* data);
	string DemangleFunctionSymbol(vector<string> names, DemangleData* data);
	string ExtractTemplate(DemangleData* data);
	string ExtractNameString(DemangleData* data);
	string ExtractSpecialName(DemangleData* data);
	vector<string> ExtractNamesList(DemangleData* data);
	string ExtractNameFragment(DemangleData* data);
	vector<string> ArgList(DemangleData* data, string stop = "");
	DataType GetDataType(DemangleData* data, int depth = 0);
	void FinalizeName(vector<string>& names, DataType& returnType);

	template <typename T>
	T ParseValue(DemangleData* data, map<string, T> table, string logMessage = "");

	template <>
	string ParseValue<string>(DemangleData* data, map<string, string> table, string logMessage)
	{
		//Function for accessing the tables below

		for (auto it = table.begin(); it != table.end(); it++)
		{
			string name = data->GetValue();

			if (name.substr(0, it->first.length()) == it->first)
			{
				data->Advance(static_cast<int>(it->first.length()));

				if (logMessage.length())
				{
					data->Log("%s%s", logMessage, it->first);
				}

				return it->second;
			}
		}

		if (logMessage.length())
		{
			data->Log("%NONE", logMessage);
		}

		return {};
	}

	template <>
	multimap<string, vector<string>> ParseValue<multimap<string, vector<string>>>(DemangleData* data, map<string, multimap<string, vector<string>>> table, string logMessage)
	{
		//Function for accessing the tables below

		for (auto it = table.begin(); it != table.end(); it++)
		{
			string name = data->GetValue();

			if (name.substr(0, it->first.length()) == it->first)
			{
				data->Advance(static_cast<int>(it->first.length()));

				if (logMessage.length())
				{
					data->Log("%s%s", logMessage, it->first);
				}

				return it->second;
			}
		}

		if (logMessage.length())
		{
			data->Log("%NONE", logMessage);
		}

		return {};
	}

	template <>
	multimap<string, string> ParseValue<multimap<string, string>>(DemangleData* data, map<string, multimap<string, string>> table, string logMessage)
	{
		//Function for accessing the tables below

		for (auto it = table.begin(); it != table.end(); it++)
		{
			string name = data->GetValue();

			if (name.substr(0, it->first.length()) == it->first)
			{
				data->Advance(static_cast<int>(it->first.length()));

				if (logMessage.length())
				{
					data->Log("%s%s", logMessage, it->first);
				}

				return it->second;
			}
		}

		if (logMessage.length())
		{
			data->Log("%NONE", logMessage);
		}

		return {};
	}

	int DecodeNumber(DemangleData* data);
	vector<string> GetClassModifiers(DemangleData* data);
	string Join(const vector<string>& vector, string delimiter);

	template <typename T, typename... Args>
	vector<T> GetVector(const Args&... args)
	{
		return { args... };
	}
};
