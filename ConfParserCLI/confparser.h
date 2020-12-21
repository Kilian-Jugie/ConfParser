/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/
/**			  ConfParser CLI Interface
 *
 * Provide managed version of confparser datastructures
 *
 */

#pragma once
#include <ConfParser/global.hpp>
#include <msclr/marshal.h>
#include <msclr/marshal_cppstd.h>
#include <fstream>

using namespace System;
using Collections::Generic::List;

namespace confparser {
	/**
	* @brief Build new CLI String from C++ string
	* @param str The string to convert
	* @return A handle to a new constructed CLI String
	*
	* TODO: generic c++ string
	*/
	String^ StringFromCpp(string_t str) {
		return gcnew String(str.c_str(), 0, static_cast<int>(str.size()));
	}

	/**
	* @brief Build new C++ string from CLI String
	* @param str A handle to the CLI String to convert
	* @return A static constructed C++ string
	*
	* TODO: generic c++ string
	*/
	string_t StringToCpp(String^ str) {
		return msclr::interop::marshal_as<string_t>(str);
	}

	/**
	* @brief CLI Equivalent of CodeObjectType
	* @see CodeObjectType
	*/
	public enum class CLICodeObjectType {
		TYPE,
		INSTANCE,
		FUNCTION,
		SCOPE
	};

	/**
	* @brief CLI Equivalent of ConfScopeable
	* @see ConfScopeable
	*/
	public ref class CLIConfScopeable {
	public:
		property String^ Name;
		property CLICodeObjectType ObjectType;
	};

	/**
	* @brief CLI Equivalent of ConfScope
	* @see ConfScope
	*/
	public ref class CLIConfScope : CLIConfScopeable {
	public:
		property List<CLIConfScopeable^>^ Childs;
		property CLIConfScope^ Parent;

		/**
		* @brief Search for a child by its name
		* @param CLI String handle to the name searched
		* @return Handle to child with the searched name or nullptr
		*/
		CLIConfScopeable^ GetByName(String^ name) {
			for each (CLIConfScopeable ^ cs in Childs) {
				if (cs->Name && cs->Name == name) return cs;
			}
			if (Parent) return Parent->GetByName(name);
			return nullptr;
		}

	};

	/**
	* @brief CLI Equivalent of ConfType
	* @see ConfType
	*/
	public ref class CLIConfType : CLIConfScope {
	public:
		CLIConfType() {}

		static CLIConfType^ CLIStringType = gcnew CLIConfType();
		static CLIConfType^ CLIIntType = gcnew CLIConfType();
		static CLIConfType^ CLIFloatType = gcnew CLIConfType();
	};

	/**
	* @brief CLI Equivalent of ConfInstance
	* @see ConfInstance
	*/
	public ref class CLIConfInstance : CLIConfScopeable {
	public:
		property CLIConfType^ Type;
		property CLIConfScope^ Scope;
	};

	/**
	* @brief CLI Equivalent of a native/intrinsic instance type
	* @description This is simplier than in the C++ version due to
	*				the fact that we don't have to store the functions
	*				members and all CLI objects inherit Object
	* @see ConfInstanceString, ConfInstanceInt, ConfInstanceFloat
	*/
	public ref class CLIConfInstanceNative : CLIConfInstance {
	public:
		property Object^ Data;
	};

	/**
	* @brief CLI Equivalent of a non native/extrinsic instance type
	* * @description This is simplier than in the C++ version due to
	*				the fact that we don't have to store the functions
	*				members and all CLI objects inherit Object
	* @see ConfInstance
	*/
	public ref class CLIConfInstanceImported : CLIConfInstance {
	public:
		property List<CLIConfInstance^>^ SubInstances;
	};

	/**
	* @brief Build a CLI instance from a C++ instance
	* @param instance Pointer to the C++ instance to build from
	* @param currenScope Handle to the scope where the declaration is
	* @return Handle to a new constructed instance
	* @see CLIConfInstanceNative, CLIConfInstanceImported
	*/
	CLIConfInstance^ BuildInstance(ConfInstance* instance, CLIConfScope^ currentScope);

	/**
	* @brief Build differents objects from a C++ scope recursively
	* @param scope Pointer to the scope to build from
	* @param parent Handle to the parent scope of the current scope
	* @return Handle to a newly builded scope
	*/
	CLIConfScope^ BuildRecursively(ConfScope* scope, CLIConfScope^ parent = nullptr);

	/**
	* @brief Build differents objects from a C++ scope
	* @param scope Pointer to the scope to build from
	* @return Handle to a newly builded scope
	*/
	CLIConfScope^ Build(ConfScope* scope);

	/**
	* @brief Main class from where to call the parser
	*/
	public ref class CLIConfParser {
		static bool m_IsWritterInitialized;
	public:
		/**
		* @brief Parse a file with ConfParser
		* @param filename Handle to the filename to parse from
		* @return Handle to global scope from ConfParser
		*/
		static CLIConfScope^ Parse(String^ filename);
	};
}