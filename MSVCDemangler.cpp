#include <assert.h>
#include <string>
#include <format>
#include <sstream>
#include "MSVCDemangler.h"
#include "DataType.h"

MSVCDemangler::MSVCDemangler()
{
	callingConventions.insert(make_pair("A", "__cdecl "));
	callingConventions.insert(make_pair("B", "__cdecl __dll_export "));
	callingConventions.insert(make_pair("C", "__pascal "));
	callingConventions.insert(make_pair("D", "__pascal __dll_export "));
	callingConventions.insert(make_pair("E", "__thiscall "));
	callingConventions.insert(make_pair("F", "__thiscall __dll_export "));
	callingConventions.insert(make_pair("G", "__stdcall "));
	callingConventions.insert(make_pair("H", "__stdcall __dll_export "));
	callingConventions.insert(make_pair("I", "__fastcall "));
	callingConventions.insert(make_pair("J", "__fastcall __dll_export "));
	callingConventions.insert(make_pair("K", ""));
	callingConventions.insert(make_pair("L", "__dll_export "));
	callingConventions.insert(make_pair("M", "__clrcall "));
	callingConventions.insert(make_pair("N", "__clrcall __dll_export "));
	callingConventions.insert(make_pair("O", "__eabi "));
	callingConventions.insert(make_pair("P", "__eabi __dll_export "));
	callingConventions.insert(make_pair("Q", "__vectorcall "));

	//We should set '@' to 'void' if we want the same output as wine's undname
	dataTypes["@"].insert(make_pair("SIMPLE", GetVector<string>("")));
	dataTypes["?"].insert(make_pair("MODIFIER", GetVector<string>("")));
	dataTypes["A"].insert(make_pair("MODIFIER", GetVector<string>("&")));
	dataTypes["B"].insert(make_pair("MODIFIER", GetVector<string>("& volatile")));
	dataTypes["C"].insert(make_pair("SIMPLE", GetVector<string>("signed char")));
	dataTypes["D"].insert(make_pair("SIMPLE", GetVector<string>("char")));
	dataTypes["E"].insert(make_pair("SIMPLE", GetVector<string>("unsigned char")));
	dataTypes["F"].insert(make_pair("SIMPLE", GetVector<string>("short")));
	dataTypes["G"].insert(make_pair("SIMPLE", GetVector<string>("unsigned short")));
	dataTypes["H"].insert(make_pair("SIMPLE", GetVector<string>("int")));
	dataTypes["I"].insert(make_pair("SIMPLE", GetVector<string>("unsigned int")));
	dataTypes["J"].insert(make_pair("SIMPLE", GetVector<string>("long")));
	dataTypes["K"].insert(make_pair("SIMPLE", GetVector<string>("unsigned long")));
	dataTypes["M"].insert(make_pair("SIMPLE", GetVector<string>("float")));
	dataTypes["N"].insert(make_pair("SIMPLE", GetVector<string>("double")));
	dataTypes["O"].insert(make_pair("SIMPLE", GetVector<string>("long double")));
	dataTypes["P"].insert(make_pair("MODIFIER", GetVector<string>("*")));
	dataTypes["Q"].insert(make_pair("MODIFIER", GetVector<string>("*", "const")));
	dataTypes["R"].insert(make_pair("MODIFIER", GetVector<string>("*", "volatile")));
	dataTypes["S"].insert(make_pair("MODIFIER", GetVector<string>("*", "const volatile")));
	dataTypes["T"].insert(make_pair("COMPLEX", GetVector<string>("union")));
	dataTypes["U"].insert(make_pair("COMPLEX", GetVector<string>("struct")));
	dataTypes["V"].insert(make_pair("COMPLEX", GetVector<string>("class")));
	dataTypes["W"].insert(make_pair("COMPLEX", GetVector<string>("enum")));
	dataTypes["X"].insert(make_pair("SIMPLE", GetVector<string>("void")));
	dataTypes["Y"].insert(make_pair("COMPLEX", GetVector<string>("cointerface")));
	dataTypes["_D"].insert(make_pair("SIMPLE", GetVector<string>("__int8")));
	dataTypes["_E"].insert(make_pair("SIMPLE", GetVector<string>("unsigned __int8")));
	dataTypes["_F"].insert(make_pair("SIMPLE", GetVector<string>("__int16")));
	dataTypes["_G"].insert(make_pair("SIMPLE", GetVector<string>("unsigned __int16")));
	dataTypes["_H"].insert(make_pair("SIMPLE", GetVector<string>("__int32")));
	dataTypes["_I"].insert(make_pair("SIMPLE", GetVector<string>("unsigned __int32")));
	dataTypes["_J"].insert(make_pair("SIMPLE", GetVector<string>("__int64")));
	dataTypes["_K"].insert(make_pair("SIMPLE", GetVector<string>("unsigned __int64")));
	dataTypes["_L"].insert(make_pair("SIMPLE", GetVector<string>("__int128")));
	dataTypes["_M"].insert(make_pair("SIMPLE", GetVector<string>("unsigned __int128")));
	dataTypes["_N"].insert(make_pair("SIMPLE", GetVector<string>("bool")));
	//_O   =SPECIAL CASE= Array
	dataTypes["_S"].insert(make_pair("SIMPLE", GetVector<string>("char16_t")));
	dataTypes["_U"].insert(make_pair("SIMPLE", GetVector<string>("char32_t")));
	dataTypes["_W"].insert(make_pair("SIMPLE", GetVector<string>("wchar_t")));
	dataTypes["_X"].insert(make_pair("COMPLEX", GetVector<string>("coclass")));
	dataTypes["_Y"].insert(make_pair("COMPLEX", GetVector<string>("cointerface")));
	//_$'  =SPECIAL CASE= __w64 type
	//$$A  =TODO= (found by reversing vcruntime140.dll, more reverse is needed)
	//$$B  =SPECIAL CASE= Apparently no effect
	dataTypes["$$C"].insert(make_pair("MODIFIER", GetVector<string>("")));
	dataTypes["$$Q"].insert(make_pair("MODIFIER", GetVector<string>("&&")));
	dataTypes["$$R"].insert(make_pair("MODIFIER", GetVector<string>("&&", "volatile")));
	//$$S  =TODO= (found by reversing vcruntime140.dll, more reverse is needed)
	dataTypes["$$T"].insert(make_pair("SIMPLE", GetVector<string>("std::nullptr_t")));
	//$$Y  =TODO= (found by reversing vcruntime140.dll, more reverse is needed)

	/*
	* Here are the enum types mentioned at
	* https://en.wikiversity.org/wiki/Visual_C%2B%2B_name_mangling
	* Note that only type 4 aka 'int' is used by "modern versions"
	* of Visual Studio.
	*/
	enumTypes.insert(make_pair("0", "char"));
	enumTypes.insert(make_pair("1", "unsigned char"));
	enumTypes.insert(make_pair("2", "short"));
	enumTypes.insert(make_pair("3", "unsigned short"));
	enumTypes.insert(make_pair("4", "int"));
	enumTypes.insert(make_pair("5", "unsigned int"));
	enumTypes.insert(make_pair("6", "long"));
	enumTypes.insert(make_pair("7", "unsigned long"));

	specialFragments.insert(make_pair("0", "?0")); //to be done by name_finalize()
	specialFragments.insert(make_pair("1", "?1")); //to be done by name_finalize()
	specialFragments.insert(make_pair("2", "operator new"));
	specialFragments.insert(make_pair("3", "operator delete"));
	specialFragments.insert(make_pair("4", "operator="));
	specialFragments.insert(make_pair("5", "operator>>"));
	specialFragments.insert(make_pair("6", "operator<<"));
	specialFragments.insert(make_pair("7", "operator!"));
	specialFragments.insert(make_pair("8", "operator=="));
	specialFragments.insert(make_pair("9", "operator!="));
	specialFragments.insert(make_pair("A", "operator[]"));
	specialFragments.insert(make_pair("B", "?B")); //to be done by name_finalize()
	specialFragments.insert(make_pair("C", "operator->"));
	specialFragments.insert(make_pair("D", "operator*"));
	specialFragments.insert(make_pair("E", "operator++"));
	specialFragments.insert(make_pair("F", "operator--"));
	specialFragments.insert(make_pair("G", "operator-"));
	specialFragments.insert(make_pair("H", "operator+"));
	specialFragments.insert(make_pair("I", "operator&"));
	specialFragments.insert(make_pair("J", "operator->*"));
	specialFragments.insert(make_pair("K", "operator/"));
	specialFragments.insert(make_pair("L", "operator%"));
	specialFragments.insert(make_pair("M", "operator<"));
	specialFragments.insert(make_pair("N", "operator<="));
	specialFragments.insert(make_pair("O", "operator>"));
	specialFragments.insert(make_pair("P", "operator>="));
	specialFragments.insert(make_pair("Q", "operator,"));
	specialFragments.insert(make_pair("R", "operator()"));
	specialFragments.insert(make_pair("S", "operator~"));
	specialFragments.insert(make_pair("T", "operator^"));
	specialFragments.insert(make_pair("U", "operator|"));
	specialFragments.insert(make_pair("V", "operator&&"));
	specialFragments.insert(make_pair("W", "operator||"));
	specialFragments.insert(make_pair("X", "operator*="));
	specialFragments.insert(make_pair("Y", "operator+="));
	specialFragments.insert(make_pair("Z", "operator-="));
	specialFragments.insert(make_pair("_0", "operator/="));
	specialFragments.insert(make_pair("_1", "operator%="));
	specialFragments.insert(make_pair("_2", "operator>>="));
	specialFragments.insert(make_pair("_3", "operator<<="));
	specialFragments.insert(make_pair("_4", "operator&="));
	specialFragments.insert(make_pair("_5", "operator|="));
	specialFragments.insert(make_pair("_6", "operator^="));
	specialFragments.insert(make_pair("_7", quoteB + "vftable" + quoteE));
	specialFragments.insert(make_pair("_8", quoteB + "vbtable" + quoteE));
	specialFragments.insert(make_pair("_9", quoteB + "vcall" + quoteE));
	specialFragments.insert(make_pair("_A", quoteB + "typeof" + quoteE));
	specialFragments.insert(make_pair("_B", quoteB + "local static guard" + quoteE));
	//_C just returns 'string' and forgets the rest of the input
	specialFragments.insert(make_pair("_D", quoteB + "vbase destructor" + quoteE));
	specialFragments.insert(make_pair("_E", quoteB + "vector deleting destructor" + quoteE));
	specialFragments.insert(make_pair("_F", quoteB + "default constructor closure" + quoteE));
	specialFragments.insert(make_pair("_G", quoteB + "scalar deleting destructor" + quoteE));
	specialFragments.insert(make_pair("_H", quoteB + "vector constructor iterator" + quoteE));
	specialFragments.insert(make_pair("_I", quoteB + "vector destructor iterator" + quoteE));
	specialFragments.insert(make_pair("_J", quoteB + "vector vbase constructor iterator" + quoteE));
	specialFragments.insert(make_pair("_K", quoteB + "virtual displacement map" + quoteE));
	specialFragments.insert(make_pair("_L", quoteB + "eh vector constructor iterator" + quoteE));
	specialFragments.insert(make_pair("_M", quoteB + "eh vector destructor iterator" + quoteE));
	specialFragments.insert(make_pair("_N", quoteB + "eh vector vbase constructor iterator" + quoteE));
	specialFragments.insert(make_pair("_O", quoteB + "copy constructor closure" + quoteE));
	//_P "udt returning" followed by a special fragment
	//_R0 "RTTI Type Descriptor" followed by a data type
	//_R1 "RTTI Base Class Descriptor" followed by four numbers
	specialFragments.insert(make_pair("_R2", quoteB + "RTTI Base Class Array" + quoteE));
	specialFragments.insert(make_pair("_R3", quoteB + "RTTI Class Hierarchy Descriptor" + quoteE));
	specialFragments.insert(make_pair("_R4", quoteB + "RTTI Complete Object Locator" + quoteE));
	specialFragments.insert(make_pair("_S", quoteB + "local vftable" + quoteE));
	specialFragments.insert(make_pair("_T", quoteB + "local vftable constructor closure" + quoteE));
	specialFragments.insert(make_pair("_U", "operator new[]"));
	specialFragments.insert(make_pair("_V", "operator delete[]"));
	specialFragments.insert(make_pair("_X", quoteB + "placement delete closure" + quoteE));
	specialFragments.insert(make_pair("_Y", quoteB + "placement delete[] closure" + quoteE));
	specialFragments.insert(make_pair("__A", quoteB + "managed vector constructor iterator" + quoteE));
	specialFragments.insert(make_pair("__B", quoteB + "managed vector destructor iterator" + quoteE));
	specialFragments.insert(make_pair("__C", quoteB + "eh vector copy constructor iterator" + quoteE));
	specialFragments.insert(make_pair("__D", quoteB + "eh vector vbase copy constructor iterator" + quoteE));
	specialFragments.insert(make_pair("__E", "?__E")); //to be done by name_finalize()
	specialFragments.insert(make_pair("__F", "?__F")); //to be done by name_finalize()
	specialFragments.insert(make_pair("__G", quoteB + "vector copy constructor iterator" + quoteE));
	specialFragments.insert(make_pair("__H", quoteB + "vector vbase copy constructor iterator" + quoteE));
	specialFragments.insert(make_pair("__I", quoteB + "managed vector copy constructor iterator" + quoteE));
	specialFragments.insert(make_pair("__J", quoteB + "local static thread guard" + quoteE));
	specialFragments.insert(make_pair("__K", "?__K")); //to be done by name_finalize()

	thunkAccesses["A"].insert(make_pair("", "private:"));
	thunkAccesses["B"].insert(make_pair("", "private:"));
	thunkAccesses["C"].insert(make_pair("", "private: static"));
	thunkAccesses["D"].insert(make_pair("", "private: static"));
	thunkAccesses["E"].insert(make_pair("", "private: virtual"));
	thunkAccesses["F"].insert(make_pair("", "private: virtual"));
	thunkAccesses["G"].insert(make_pair("", "[thunk]:private: virtual"));
	thunkAccesses["H"].insert(make_pair("", "[thunk]:private: virtual"));
	thunkAccesses["I"].insert(make_pair("", "protected:"));
	thunkAccesses["J"].insert(make_pair("", "protected:"));
	thunkAccesses["K"].insert(make_pair("", "protected: static"));
	thunkAccesses["K"].insert(make_pair("", "protected: static"));
	thunkAccesses["M"].insert(make_pair("", "protected: virtual"));
	thunkAccesses["N"].insert(make_pair("", "protected: virtual"));
	thunkAccesses["O"].insert(make_pair("", "[thunk]:protected: virtual"));
	thunkAccesses["P"].insert(make_pair("", "[thunk]:protected: virtual"));
	thunkAccesses["Q"].insert(make_pair("", "public:"));
	thunkAccesses["R"].insert(make_pair("", "public:"));
	thunkAccesses["S"].insert(make_pair("", "public: static"));
	thunkAccesses["T"].insert(make_pair("", "public: static"));
	thunkAccesses["U"].insert(make_pair("", "public: virtual"));
	thunkAccesses["V"].insert(make_pair("", "public: virtual"));
	thunkAccesses["W"].insert(make_pair("", "[thunk]:public: virtual"));
	thunkAccesses["X"].insert(make_pair("", "[thunk]:public: virtual"));
	thunkAccesses["Y"].insert(make_pair("", ""));
	thunkAccesses["Z"].insert(make_pair("", ""));
	thunkAccesses["0"].insert(make_pair("VAR", "private: static"));
	thunkAccesses["1"].insert(make_pair("VAR", "protected: static"));
	thunkAccesses["2"].insert(make_pair("VAR", "public: static"));
	thunkAccesses["3"].insert(make_pair("VAR", "")); //private non-static
	thunkAccesses["4"].insert(make_pair("VAR", "")); //protected non-static
	thunkAccesses["5"].insert(make_pair("VAR", "")); //public non-static
	thunkAccesses["6"].insert(make_pair("OPT", ""));
	thunkAccesses["7"].insert(make_pair("OPT", ""));
	thunkAccesses["$0"].insert(make_pair("vtordisp", "[thunk]:private: virtual"));
	thunkAccesses["$1"].insert(make_pair("vtordisp", "[thunk]:private: virtual"));
	thunkAccesses["$2"].insert(make_pair("vtordisp", "[thunk]:protected: virtual"));
	thunkAccesses["$3"].insert(make_pair("vtordisp", "[thunk]:protected: virtual"));
	thunkAccesses["$4"].insert(make_pair("vtordisp", "[thunk]:public: virtual"));
	thunkAccesses["$5"].insert(make_pair("vtordisp", "[thunk]:public: virtual"));
	thunkAccesses["$B"].insert(make_pair("vcall", "[thunk]:"));
	thunkAccesses["$R"].insert(make_pair("vtordispex", "[thunk]:public: virtual"));
}

string MSVCDemangler::DemangleSymbol(string symbol, string& rest, bool verbose)
{
	string result;
	DemangleData data = DemangleData(symbol, verbose);

	result = DemangleReentrantSymbol(&data);
	rest = data.GetValue();

	return result;
}

string MSVCDemangler::DemangleReentrantSymbol(DemangleData* data)
{
	//Reentrant: can be called for nested symbols.

	if (data->GetValue().substr(0, 5) == "__mep@")
	{
		/*
		* undname.exe does not expand symbols beginning with __mep@,
		* but they are generated when using C++ / CLI and correspond
		* to prefixed mangled symbols.
		*/

		data->Advance(6);

		return "[MEP] " + DemangleReentrantSymbol(data);
	}
	else if (data->GetValue()[0] != '?')
	{
		data->Log("Not mangled.");

		data->Advance(static_cast<int>(data->GetValue().length()));

		return data->GetValue();
	}

	data->Advance(1);

	if (data->GetValue()[0] == '$')
	{
		/*
		* Neither a variable nor a function: just a name with a template
		* Example: '?$a@PAUb@@' which means 'a<struct b *>'
		*/

		data->Advance(1);

		string name = ExtractTemplate(data);

		assert(data->GetValue().length() == 0);

		return name;
	}
	else if (data->GetValue()[0] == '@')
	{
		/*
		* Found by reversing vcruntime140.dll
		* Don't know when such names are generated
		*/

		data->Advance(1);
		data->Log("CV: prefix");

		return "CV: " + DemangleReentrantSymbol(data);
	}
	else if (data->GetValue().substr(0, 3) == "?_C")
	{
		/*
		* Neither a variable nor a function: just `string'
		* The rest is ignored
		*/

		string name = quoteB + "string" + quoteE;

		data->Advance(static_cast<int>(data->GetValue().length()));

		return name;
	}

	/*
	* Variable or function: starts with a list of name fragments,
	* continues with type information.
	*/

	vector<string> names;

	if (data->GetValue()[0] == '?')
	{
		data->Advance(1);

		string name = ExtractSpecialName(data);

		names.push_back(name);
	}

	vector<string> names2 = ExtractNamesList(data);

	names.insert(names.end(), names2.begin(), names2.end());

	data->Log("PARAM");

	if (names[0] == quoteB + "local static guard" + quoteE)
	{
		return DemangleLocalStaticGuardSymbol(names, data);
	}

	if ('0' <= data->GetValue()[0] && data->GetValue()[0] <= '9' || data->GetValue().substr(0, 2) == "$B")
	{
		return DemangleVariableSymbol(names, data);
	}

	if ('A' <= data->GetValue()[0] && data->GetValue()[0] <= 'Z' || data->GetValue()[0] == '$')
	{
		return DemangleFunctionSymbol(names, data);
	}

	return "";
}

tuple<DataType, string, string> MSVCDemangler::DemangleFunctionPrototypeSymbol(DemangleData* data)
{
	/*
	* Used when demangling a function, but also for function pointers
	* and member function pointers.
	*/
	tuple<DataType, string, string> result;

	result = make_tuple(DataType(), "", "");

	get<1>(result) = ParseValue(data, callingConventions, "CALL");
	get<0>(result) = GetDataType(data);

	data->Log("RET=%s", get<0>(result).GetValue().c_str());

	vector<string> args2 = ArgList(data, "XZ@");

	get<2>(result) = "(" + Join(args2, ",") + ")";

	if (data->GetValue()[0] == 'Z')
	{
		//No throw

		data->Advance(1);
	}
	else
	{
		//Same output as undname.exe, but Visual Studio 14.0 seems to
		//ignore throw() in function prototypes.

		vector<string> throwArgs = ArgList(data, "@");

		get<2>(result) += " throw(" + Join(throwArgs, ",") + ")";
	}

	return result;
}

string MSVCDemangler::DemangleLocalStaticGuardSymbol(vector<string> names, DemangleData* data)
{
	//We don't know if other value than 5 can appear, and what they mean.

	assert(data->GetValue()[0] == '5');
	data->Advance(1);
	assert('0' <= data->GetValue()[0] && data->GetValue()[0] <= '9');

	string param = to_string(1 + static_cast<int>(data->GetValue()[0]) - static_cast<int>('0'));

	data->Advance(1);
	reverse(names.begin(), names.end());

	string name = Join(names, "::");

	return name + format("{{}}'", param);
}

string MSVCDemangler::DemangleVariableSymbol(vector<string> names, DemangleData* data)
{
	//Access level and storage class
	multimap<string, string> result = ParseValue(data, thunkAccesses, "TYPE=%s ACCESS=%s");
	string thunk = result.begin()->first;
	string access = result.begin()->second;
	string addName;
	DataType returnType;

	if (thunk == "VAR")
	{
		//NB: ret is of type DataType, because it may be a function pointer
		returnType = GetDataType(data);

		data->Log("TYPE=%s", returnType.GetValue().c_str());

		string cv = Join(GetClassModifiers(data), " ");

		if (cv.length())
		{
			cv += " ";
		}

		returnType.Add(" " + cv);
	}
	else if (thunk == "OPT")
	{
		returnType = DataType(make_tuple(Join(GetClassModifiers(data), " "), "", ""), false);

		if (returnType.GetValue().length())
		{
			returnType.Add(" ");
		}

		if (data->GetValue()[0] != '@')
		{
			vector<string> addNames = ExtractNamesList(data);

			reverse(addNames.begin(), addNames.end());

			addName = Join(addNames, "::");
			addName = format("{for %s%s%s}", quoteB, addName, quoteE);
		}

		data->Log("OPT_NAME=%s", addName.c_str());
		assert(data->GetValue()[0] == '@');
		data->Advance(1);
	}
	else if (thunk == "vcall")
	{
		int n1 = DecodeNumber(data);

		data->Log("VCALL{%d}", n1);

		addName = "{" + to_string(n1) + ",{flat}}" + quoteE + " }" + quoteE;

		assert(data->GetValue()[0] == 'A');
		data->Advance(1);

		returnType = DataType(make_tuple(ParseValue(data, callingConventions, "CALL=%s"), "", ""), false);
	}

	reverse(names.begin(), names.end());

	string name = Join(names, "::");

	returnType.Add(name);

	if (access.length())
	{
		access += " ";
	}

	return access + returnType.GetValue() + addName;
}

string MSVCDemangler::DemangleFunctionSymbol(vector<string> names, DemangleData* data)
{
	string prefix;
	vector<string> vtor;

	if (string("$$F").find(data->GetValue().substr(0, 3)) != string::npos ||
		string("$$H").find(data->GetValue().substr(0, 3)) != string::npos)
	{
		/*
		* C++/CLI
		* https://en.wikiversity.org/wiki/Visual_C%2B%2B_name_mangling
		* identifies $$F as a 'function modifier' for C++/CLI meaning
		* the the function is managed. This web page mentions $$F in the
		* data_types section, but it does not appear with data types,
		* it appears before the thunk_access letters.
		* The symbol with $$F is the 'managed entry point', the other
		* being the 'native entry point'.
		*
		* Visual Studio also generates $$H for 'main', with unknown
		* meaning...
		*
		* Apparently this could be ignored, because undname.exe outputs
		* the same decoding when $$F or $$H is present. We add a prefix,
		* it seems more informative.
		*/

		map<string, string> prefixes;

		prefixes.insert(make_pair("$$F", "[managed] "));
		prefixes.insert(make_pair("$$H", "[MANAGED] "));

		prefix = prefixes[data->GetValue().substr(0, 3)];

		data->Advance(3);

		return prefix + DemangleFunctionSymbol(names, data);
	}

	if (data->GetValue().substr(0, 3) == "$$J")
	{
		data->Advance(3);
		assert(data->GetValue().length());

		if (string("0123456789").find(data->GetValue()[0]) != string::npos)
		{
			/*
			* To be analyzed later... does not change the output of undname.exe
			* Visual Studio generates various values ('0', '18', ...)
			*/

			data->Advance(1 + static_cast<int>(data->GetValue()[0]) - static_cast<int>('0'));
		}

		prefix = "extern \"C\" ";
	}
	else
	{
		prefix = "";
	}

	multimap<string, string> result = ParseValue(data, thunkAccesses, "TYPE=%s ACCESS=%s");
	string thunk = result.begin()->first;
	string access = result.begin()->second;

	string adjustor;

	if (access.length() > 8 && access.starts_with("[thunk]:"))
	{
		size_t offset = string("0123456789").find(data->GetValue()[0]) + 1;
		adjustor = "`adjustor{" + to_string(offset) + "}'";

		data->Advance(1);
		data->Log("MOD=%s", adjustor.c_str());
	}

	if (thunk == "vtordisp")
	{
		for (int i = 0; i < 2; i++)
		{
			vtor.push_back(to_string(DecodeNumber(data)));
		}
	}
	else if (thunk == "vtordispex")
	{
		assert(data->GetValue()[0] == '4');
		data->Advance(1);

		for (int i = 0; i < 4; i++)
		{
			vtor.push_back(to_string(DecodeNumber(data)));
		}
	}

	string cv;

	if (access.length() && string(access).find("static") == string::npos)
	{
		map<string, string> cliReturnValues;

		cliReturnValues.insert(make_pair("$A", ""));
		cliReturnValues.insert(make_pair("$C", "%"));

		string cli = ParseValue(data, cliReturnValues);

		if (cli.length())
		{
			data->Log("C++/CLI Return Value");

			cv = cli;
		}

		cv = Join(GetClassModifiers(data), " ") + cv;
	}

	tuple<DataType, string, string> result2 = DemangleFunctionPrototypeSymbol(data);
	FinalizeName(names, get<0>(result2));
	reverse(names.begin(), names.end());

	string name = Join(names, "::");

	if (thunk.length() && thunk.starts_with("vtordisp"))
	{
		name += quoteB + thunk + '{' + Join(vtor, ",") + '}' + quoteE + ' ';
	}

	if (get<0>(result2).GetValue().length() && access.length())
	{
		access += " ";
	}

	get<0>(result2).Add(" " + get<1>(result2) + name + adjustor + get<2>(result2) + cv);

	return prefix + access + get<0>(result2).GetValue();
}

string MSVCDemangler::ExtractTemplate(DemangleData* data)
{
	data->Log("TEMPLATE start");
	data->EnterTemplate();

	string name = ExtractNameString(data);
	vector<string> args = ArgList(data, "Z@");

	data->ExitTemplate();

	string fragment = format("{}<{}>", name, Join(args, ","));

	data->Log("TEMPLATE=%s", fragment.c_str());

	return fragment;
}

string MSVCDemangler::ExtractNameString(DemangleData* data)
{
	string name = data->GetValue();

	assert(name.length());
	assert(name[0] != '?');

	int idx = data->Index('@');
	string fragment = name.substr(0, idx);

	data->Advance(idx + 1);
	data->AddFragment(fragment);
	data->Log("NAME=%s", fragment.c_str());

	return fragment;
}

string MSVCDemangler::ExtractSpecialName(DemangleData* data)
{
	//The symbol's name optionally starts with a special fragment
	string fragment = ParseValue(data, specialFragments);

	if (fragment.length() > 0)
	{
		data->Log("SPEC=%s", fragment.c_str());
	}
	else if (data->GetValue().substr(0, 2) == "_P")
	{
		data->Advance(2);

		fragment = quoteB + "udt returning" + quoteE;
		fragment += ExtractSpecialName(data)[0];
	}
	else if (data->GetValue().substr(0, 3) == "_R0")
	{
		data->Advance(3);

		fragment = GetDataType(data).GetValue();
		fragment += ' ' + quoteB + "RTTI Type Descriptor" + quoteE;
	}
	else if (data->GetValue().substr(0, 3) == "_R1")
	{
		data->Advance(3);

		fragment = quoteB + "RTTI Base Class Descriptor at (";

		for (int i = 0; i < 4; i++)
		{
			fragment += DecodeNumber(data);
		}

		fragment += ")" + quoteE;
	}
	else if (data->GetValue().substr(0, 2) == "$?")
	{
		//operator template
		data->Advance(2);

		fragment = ParseValue(data, specialFragments);

		fragment += format("<{}>", GetDataType(data).GetValue());

		assert(data->GetValue()[0] == '@');

		data->Advance(1);
	}
	else if (data->GetValue()[0] == '$')
	{
		//normal template
		data->Advance(1);

		fragment = ExtractTemplate(data);
	}

	return fragment;
}

vector<string> MSVCDemangler::ExtractNamesList(DemangleData* data)
{
	/*
	* Other fragments cannot be in 'special_fragment' nor operator template.
	* If they begin with '?$' they are normal templates, with '??' they are
	* nested names, and other fragments beginning with '?' are quoted numeric.
	*/

	vector<string> names;

	while (data->GetValue()[0] != '@')
	{
		string fragment = ExtractNameFragment(data);

		names.push_back(fragment);
	}

	assert(data->GetValue()[0] == '@');
	data->Advance(1);
	data->Log("NAME=%s", Join(names, ",").c_str());

	return names;
}

string MSVCDemangler::ExtractNameFragment(DemangleData* data)
{
	string fragment;

	if (data->GetValue().length() && string("0123456789").find(data->GetValue()[0]) != string::npos)
	{
		//fragment backreference
		data->Log("BACKREF_FRG=%c", data->GetValue()[0]);

		fragment = data->GetFragment(static_cast<int>(data->GetValue()[0]) - '0');

		data->Advance(1);
	}
	else if (data->GetValue().substr(0, 2) == "??")
	{
		//nested name
		data->Advance(1);

		fragment = quoteB + DemangleReentrantSymbol(data) + quoteE;
	}
	else if (data->GetValue().substr(0, 2) == "?$")
	{
		//template
		data->Advance(2);

		fragment = ExtractTemplate(data);
		data->AddFragment(fragment);
	}
	else if (data->GetValue().substr(0, 2) == "?A")
	{
		//anonymous namespace
		int idx = data->Index('@');

		data->Advance(idx + 1);

		fragment = quoteB + "anonymous namespace" + quoteE;
	}
	else if (data->GetValue()[0] == '?')
	{
		//numbered namespace
		data->Advance(1);

		int i = DecodeNumber(data);
		fragment = quoteB + to_string(i) + quoteE;
	}
	else
	{
		//name (text)
		fragment = ExtractNameString(data);
	}

	data->Log("FRAGMENT=%s", fragment.c_str());

	return fragment;
}

vector<string> MSVCDemangler::ArgList(DemangleData* data, string stop)
{
	/*
	* For function arguments, 'X' is terminating     => stop = 'XZ@'
	* For template arguments, 'X' is not terminating => stop = 'Z@'
	*/

	data->Log("ARGS start");

	vector<string> args;

	while (data->GetValue().length())
	{
		if (stop.find(data->GetValue()[0]) != string::npos)
		{
			break;
		}

		string str = "CDEFGHIJKMNO";
		bool isPrimitiveType = str.find(data->GetValue()[0]) != string::npos;
		string a = GetDataType(data).GetValue();

		if (a.empty())
		{
			break;
		}

		args.push_back(a);

		if (!isPrimitiveType)
		{
			data->AddArgument(a);
		}
	}

	data->Log("ARGS=%s", Join(args, ",").c_str());

	if (!data->GetValue().length())
	{
		//Neither a variable nor a function : just a type with template

		return args;
	}

	if (data->GetValue()[0] == 'X')
	{
		//void as the only argument

		args.push_back("void");
	}
	else if (data->GetValue().substr(0, 2) == "ZZ")
	{
		//ellipsis only when at the end of the argument list

		args.push_back("...");
	}
	else
	{
		assert(data->GetValue()[0] == '@');
	}

	data->Advance(1);

	return args;
}

DataType MSVCDemangler::GetDataType(DemangleData* data, int depth)
{
	DataType result;

	data->Log("TYPE depth %d", depth);

	if (data->GetValue().length() && string("0123456789").find(data->GetValue()[0]) != string::npos)
	{
		//argument backreference
		int pos = static_cast<int>(data->GetValue()[0]) - '0';

		data->Log("BACKREF_ARG=%d", pos);
		data->Advance(1);
		assert(pos < data->GetArgumentsSize());

		result = DataType(make_tuple(data->GetArgument(pos), "", ""), false);
	}
	else if (string("P6").find(data->GetValue().substr(0, 2)) != string::npos ||
		string("Q6").find(data->GetValue().substr(0, 2)) != string::npos)
	{
		/*
		* Function pointer
		* The result of 'data_type' is not a string, because if it is
		* an argument of a function it needs to be converted to
		* '%s(%s)%s'%result but if it is a return type it needs
		* to be converted to '%s(%s f(args))%s'
		* 'Q6' is probably 'const', but undname.exe does not show it.
		*/

		multimap<string, vector<string>> result2 = ParseValue(data, dataTypes);

		data->Advance(1);

		tuple<DataType, string, string> result3 = DemangleFunctionPrototypeSymbol(data);

		result = DataType(make_tuple(get<0>(result3).GetValue(), get<1>(result3), get<2>(result3)), true);

		result.Add(Join(result2.begin()->second, " "));
	}
	else if (string("P8").find(data->GetValue().substr(0, 2)) != string::npos)
	{
		//Member function pointer
		data->Advance(2);
		assert(data->GetValue().length());

		vector<string> names = ExtractNamesList(data);

		reverse(names.begin(), names.end());

		string name = Join(names, "::");
		string cv = Join(GetClassModifiers(data), " ");

		tuple<DataType, string, string> result3 = DemangleFunctionPrototypeSymbol(data);

		result = DataType(make_tuple(get<0>(result3).GetValue(), get<1>(result3), get<2>(result3)), true);

		result.Add(name + "::*");
		result.Append(cv);
	}
	else if (data->GetValue().substr(0, 3) == "__Z")
	{
		//HACK. do nothing
		data->Advance(3);

		result = GetDataType(data);
	}
	else if (string("A$").find(data->GetValue().substr(0, 2)) != string::npos ||
		string("P$").find(data->GetValue().substr(0, 2)) != string::npos)
	{
		char cli0 = data->GetValue()[0];

		data->Advance(2);
		assert(data->GetValue().length());

		/*
		* Managed C++ properties
		*	https://en.wikipedia.org/wiki/Managed_Extensions_for_C%2B%2B
		*	Now deprecated, was designed for .Net and CLR.
		*	There were __gc, __value, __interface, __abstract, __sealed
		*	and _pin modifiers.
		* C++/CLI
		*	Replaces Managed C++, included in Visual Studio 2005
		*	The both Managed C++ and C++/CLI are well described at
		*	https://msdn.microsoft.com/en-us/library/ms379603(VS.80).aspx
		*/

		data->Log("C++/CLI Arguments");

		char cli1 = data->GetValue()[0];

		if (string("ABC").find(cli1) != string::npos)
		{
			data->Advance(1);

			map<string, map<string, string>> cliArguments;

			cliArguments["PA"].insert(make_pair(" ^", ""));
			cliArguments["AA"].insert(make_pair(" %", ""));
			//Not sure whether these next two are generated by the compiler
			cliArguments["PC"].insert(make_pair(" %", ""));
			cliArguments["AC"].insert(make_pair(" %", ""));
			/*
			* pin_ptr decoding seems invalid, the < is not closed,
			* but that's what undname.exe outputs.
			*/
			cliArguments["PB"].insert(make_pair(" *", "cli::pin_ptr<"));
			cliArguments["AB"].insert(make_pair(" &", "cli::pin_ptr<"));

			string postfix = cliArguments[string(1, cli0) + string(1, cli1)].begin()->first;
			string prefix = cliArguments[string(1, cli0) + string(1, cli1)].begin()->second;
			vector<string> classModifiers = GetClassModifiers(data);

			result = GetDataType(data, depth + 1);

			if (classModifiers[0] != "")
			{
				result.Add(" " + classModifiers[0]);
			}

			result.Add(postfix);
			result.Prepend(prefix);

			if (classModifiers.size() > 1)
			{
				vector<string> classModifiers2;
				vector<string>::iterator it = classModifiers.begin();

				advance(it, 1);

				while (it != classModifiers.end())
				{
					classModifiers2.push_back(*it);

					++it;
				}

				result.Add(" " + Join(classModifiers2, " "));
			}
		}
		else if (string("01").find(cli1) != string::npos)
		{
			int dim;
			stringstream ss;

			ss << hex << data->GetValue().substr(0, 2);
			ss >> dim;

			data->Advance(2);
			data->Advance(1); //ignored, apparently

			result = GetDataType(data, depth + 1);

			result.Prepend("cli::array<");

			if (dim == 1)
			{
				result.Add(" >^");
			}
			else
			{
				result.Add(format(" ,{}>^", dim));
			}
		}
		else
		{
			/*
			* VS 14.0's undname.exe does some additional decoding,
			* e.g. ?a@@$$FYMHP$DFCH@Z
			* becomes 'int __clrcall a(cli::array<int ,342>^)'
			* but this is clearly a bug of undname.exe
			*/

			assert(false);
		}
	}
	else if (data->GetValue()[0] == '?' && data->IsInTemplate())
	{
		//Template parameters
		data->Advance(1);

		int i = DecodeNumber(data);

		result = DataType(make_tuple(quoteB + format("template-parameter-{}", i) + quoteE, "", ""), false);
	}
	else if (data->GetValue()[0] == '$' && data->GetValue().substr(0, 2) != "$$" && data->IsInTemplate())
	{
		//Various types of template parameters
		char templateType = data->GetValue()[1];
		int i;

		data->Advance(2);

		if (templateType == '0')
		{
			/*
			* Template instanciated with a numeric value. Example:
			*	template<int N> struct S1 { int a[N]; };
			*	S1<10> s1;
			*/

			i = DecodeNumber(data);
			result = DataType(make_tuple(to_string(i), "", ""), false);
		}
		else if (templateType == '1')
		{
			/*
			* Template instanciated with a static object. Example:
			*	template<const int&> struct S2 {};
			*	int N = 1;
			*	S2<N> s2;
			*/

			result = DataType(make_tuple(format("&{}", DemangleReentrantSymbol(data)), "", ""), false);
		}
		else if (string("2FG").find(templateType) != string::npos)
		{
			/*
			* Decoding obtained by trial and error with undname.exe,
			* but the result seems meaningless.
			*/

			string h = to_string(static_cast<int>(data->GetValue()[0]) - static_cast<int>('/'));

			data->Advance(1);

			i = DecodeNumber(data);

			if (templateType == '2')
			{
				result = DataType(make_tuple(format("{}.{}e{}", h[0], h.substr(1), i), "", ""), false);
			}
			else if (templateType == 'F')
			{
				result = DataType(make_tuple(format("{%s,%d}", h, i), "", ""), false);
			}
			else if (templateType == 'G')
			{
				int j = DecodeNumber(data);

				result = DataType(make_tuple(format("{%s,%d,%d}", h, i, j), "", ""), false);
			}
		}
		else if (templateType == 'D')
		{
			/*
			* This is compatible with wine's undname, but is not known to
			* the undname.exe of Visual Studio 14.0.
			*/

			int i = DecodeNumber(data);

			result = DataType(make_tuple(quoteB + format("template-parameter{}", i) + quoteE, "", ""), false);
		}
		else
		{
			throw invalid_argument(format("TemplateParameter<{}>", templateType));
		}
	}
	else if (data->GetValue().substr(0, 3) == "$$B")
	{
		/*
		* $$B seems useless because it calls data_type with no changes,
		* but it is needed by undname.exe in some cases.
		*/

		data->Advance(3);

		result = GetDataType(data);
	}
	else if (data->GetValue()[0] == 'Y')
	{
		//Pointer to multidimensional array
		data->Advance(1);

		int dim = DecodeNumber(data);
		vector<string> val;

		for (int i = 0; i < dim; i++)
		{
			val.push_back(format("[{}]", DecodeNumber(data)));
		}

		result = GetDataType(data);
		result = DataType(make_tuple(result.GetValue(), "", Join(val, "")), true);
	}
	else if (data->GetValue().substr(0, 2) == "_$")
	{
		//__w64 type
		data->Advance(2);

		result = GetDataType(data, depth + 1);

		result.Prepend("__w64 ");
	}
	else if (data->GetValue().substr(0, 2) == "_O")
	{
		//Array
		int dimension = 1;

		data->Advance(2);

		string cv = Join(GetClassModifiers(data), " ");

		if (cv.length())
		{
			cv = " " + cv;
		}

		while (data->GetValue().substr(0, 2) == "_O")
		{
			dimension += 1;

			data->Advance(2);

			GetClassModifiers(data);
		}

		result = GetDataType(data, depth + 1);

		result.Append(cv + " ");

		for (int i = 0; i < dimension; i++)
		{
			result.Append("[]");
		}
	}
	else
	{
		multimap<string, vector<string>> result2 = ParseValue(data, dataTypes);
		string category = result2.begin()->first;
		vector<string> result3 = result2.begin()->second;

		if (category == "COMPLEX")
		{
			data->Log("COMPLEX_TYPE");

			if (result3[0] == "enum")
			{
				assert("int" == ParseValue(data, enumTypes));
			}

			result = DataType(make_tuple(result3[0], "", ""), false);

			vector<string> names = ExtractNamesList(data);

			reverse(names.begin(), names.end());

			string name = Join(names, "::");

			result.Add(" " + name);
		}
		else if (category == "MODIFIER")
		{
			/*
			* The type modifier is output in two parts, because the qualifier
			* is not present when there are nested pointer/references,
			* detected by looking at the variable 'depth'.
			* The mixing of 'm' and 'c' outputs the same order as undname.exe
			*/

			vector<string> m = result3;
			vector<string> classModifiers = GetClassModifiers(data);

			data->Log("CVM(%d) %s %s", depth, Join(classModifiers, ",").c_str(), Join(m, ",").c_str());

			vector<string> cm;

			if (depth > 0)
			{
				m.push_back(m[0]);
			}

			if (classModifiers[0] != "")
			{
				cm.push_back(classModifiers[0]);
			}

			if (m[0] != "")
			{
				cm.push_back(m[0]);
			}

			vector<string> classModifiers2, m2;
			vector<string>::iterator it = classModifiers.begin();
			vector<string>::iterator it2 = m.begin();

			advance(it, 1);
			advance(it2, 1);

			while (it != classModifiers.end())
			{
				classModifiers2.push_back(*it);

				++it;
			}

			while (it2 != m.end())
			{
				m2.push_back(*it2);

				++it2;
			}

			cm.insert(cm.end(), classModifiers2.begin(), classModifiers2.end());
			cm.insert(cm.end(), m2.begin(), m2.end());

			string cm2 = Join(cm, " ");

			if (cm2 != "")
			{
				cm2 = " " + cm2;
			}

			result = GetDataType(data);

			result.Add(cm2);
		}
		else
		{
			assert(category == "SIMPLE");

			result = DataType(make_tuple(result3[0], "", ""), false);
		}
	}

	data->Log("TYPE=%s", result.GetValue().c_str());

	return result;
}

void MSVCDemangler::FinalizeName(vector<string>& names, DataType& returnType)
{
	/*
	* Some special fragments need to be replaced after everything has
	* been computed.
	*/

	if (names[0] == "?0")
	{
		//constructor
		assert(names.size() >= 2);

		names[0] = names[1];
		returnType.SetValue(make_tuple("", "", ""));
	}
	else if (names[0] == "?1")
	{
		//destructor
		assert(names.size() >= 2);

		names[0] = '~' + names[1];
		returnType.SetValue(make_tuple("", "", ""));
	}
	else if (names[0] == "?B")
	{
		//operator returntype
		names[0] = "operator " + returnType.GetValue();
		returnType.SetValue(make_tuple("", "", ""));
	}
	else if (string("?__E").find(names[0]) != string::npos ||
		string("?__F").find(names[0]) != string::npos ||
		string("?__K").find(names[0]) != string::npos)
	{
		assert(names.size() >= 2);

		map<string, string> names2;

		names2.insert(make_pair("?__E", quoteB + "dynamic initializer for '%s'" + quoteE));
		names2.insert(make_pair("?__K", quoteB + "dynamic atexit destructor for '%s'" + quoteE));
		names2.insert(make_pair("?__E", "operator "" %s"));

		names[1] = format(names2[names[0]], names[1]);

		names.erase(names.begin());
	}
}

int MSVCDemangler::DecodeNumber(DemangleData* data)
{
	int sign;

	if (data->GetValue()[0] == '?')
	{
		sign = -1;

		data->Advance(1);
	}
	else
	{
		sign = 1;
	}

	if (data->GetValue()[0] == '@')
	{
		data->Advance(1);

		return 0;
	}
	else if (string("0123456789").find(data->GetValue()[0]) != string::npos)
	{
		int val = 1 + static_cast<int>(data->GetValue()[0]) - '0';

		data->Advance(1);

		return sign * val;
	}
	else if (data->GetValue().length() && string("ABCDEFGHIJKLMNOP").find(data->GetValue()[0]) != string::npos)
	{
		int i = 0;

		while (data->GetValue().length() && data->GetValue()[0] != '@')
		{
			i *= 16;
			i += static_cast<int>(data->GetValue()[0]) - static_cast<int>('A');

			data->Advance(1);
		}

		data->Advance(1);

		return sign * i;
	}

	return false;
}

vector<string> MSVCDemangler::GetClassModifiers(DemangleData* data)
{
	vector<string> modifiers;
	map<char, string> letters;

	letters.insert(make_pair('E', "__ptr64"));
	letters.insert(make_pair('F', "__unaligned"));
	letters.insert(make_pair('I', "__restrict"));

	while (data->GetValue().length() && string("EFI").find(data->GetValue()[0]) != string::npos)
	{
		modifiers.push_back(letters[data->GetValue()[0]]);

		data->Advance(1);
	}

	map<string, string> cvTable;

	cvTable.insert(make_pair("A", ""));
	cvTable.insert(make_pair("B", "const"));
	cvTable.insert(make_pair("C", "volatile"));
	cvTable.insert(make_pair("D", "const volatile"));
	cvTable.insert(make_pair("M2", "__based({})"));

	string cv = ParseValue(data, cvTable);

	if (cv == "__based({})")
	{
		/*
		* Note that undname.exe forgets some const qualifiers. Example:
		* int __based(b) * const __cdecl a(short)
		* becomes ?a@@YAQM2b@@HF@Z but undname.exe decodes it as
		* int __based(b) * __cdecl a(short)
		*/

		vector<string> names = ExtractNamesList(data);

		reverse(names.begin(), names.end());

		string name = Join(names, "::");
		cv = format(cv, name);
	}

	data->Log("CVC_MOD=%s %s", cv.c_str(), Join(modifiers, ","));

	modifiers.insert(modifiers.begin(), cv);

	return modifiers;
}

string MSVCDemangler::Join(const vector<string>& vector, string delimiter)
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
