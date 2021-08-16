#pragma once

#include <iostream>
#include <tuple>

using namespace std;

class DataType
{
	/*
	* Usually a data type is a string, but if it is a function type,
	* then it is a triplet of strings
	*	( return type, calling convention & qualifiers, arguments )
	* We create a dedicated class, because we want to use += (aka. __iadd__)
	*/
private:
	tuple<string, string, string> value;
	bool isFunctionType;

public:
	DataType()
	{
		value = {};
		isFunctionType = false;
	}

	DataType(tuple<string, string, string> value, bool isFunctionType)
	{
		this->value = value;
		this->isFunctionType = isFunctionType;
	}

	string GetValue()
	{
		if (isFunctionType)
		{
			string callingConvention = get<1>(value);

			if (callingConvention.length())
			{
				callingConvention = "(" + callingConvention + ")";
			}

			return get<0>(value) + " " + callingConvention + get<2>(value);
		}

		return get<0>(value);
	}

	void SetValue(tuple<string, string, string> value)
	{
		this->value = value;
	}

	void Append(string value)
	{
		if (isFunctionType)
		{
			get<2>(this->value) += value;
		}
		else
		{
			get<0>(this->value) += value;
		}
	}

	void Prepend(string value)
	{
		get<0>(this->value).insert(0, value);
	}

	void Add(string value)
	{
		if (isFunctionType)
		{
			get<1>(this->value) += value;
		}
		else
		{
			get<0>(this->value) += value;
		}
	}
};
