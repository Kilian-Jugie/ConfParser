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
#include <msclr\marshal.h>
#include <msclr\marshal_cppstd.h>
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
		return gcnew String(str.c_str(), 0, str.size());
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
	CLIConfInstance^ BuildInstance(ConfInstance* instance, CLIConfScope^ currentScope) {
		CLIConfInstance^ ret;
		if (instance->GetType()->GetName() == TOKEN_STRING_TYPE_STRING) {
			CLIConfInstanceNative^ natRet = gcnew CLIConfInstanceNative();
			auto strInst = reinterpret_cast<ConfInstanceString*>(instance)->Get();
			natRet->Data = gcnew String(strInst.c_str(), 0, strInst.size());
			natRet->Type = CLIConfType::CLIStringType;
			ret = natRet;
		}
		else if (instance->GetType()->GetName() == TOKEN_STRING_TYPE_INT) {
			CLIConfInstanceNative^ natRet = gcnew CLIConfInstanceNative();
			auto strInst = reinterpret_cast<ConfInstanceInt*>(instance)->Get();
			natRet->Data = gcnew int(strInst);
			natRet->Type = CLIConfType::CLIIntType;
			ret = natRet;
		}
		else if (instance->GetType()->GetName() == TOKEN_STRING_TYPE_FLOAT) {
			CLIConfInstanceNative^ natRet = gcnew CLIConfInstanceNative();
			auto strInst = reinterpret_cast<ConfInstanceFloat*>(instance)->Get();
			natRet->Data = gcnew float(strInst);
			natRet->Type = CLIConfType::CLIFloatType;
			ret = natRet;
		}
		else {
			CLIConfInstanceImported^ impRet = gcnew CLIConfInstanceImported();
			impRet->SubInstances = gcnew List<CLIConfInstance^>(instance->GetSubInstances().size());
			impRet->Type = static_cast<CLIConfType^>(currentScope->GetByName(StringFromCpp(instance->GetType()->GetName())));
			for (auto si : instance->GetSubInstances()) {
				impRet->SubInstances->Add(BuildInstance(si, currentScope));
			}
			ret = impRet;
		}
		ret->ObjectType = CLICodeObjectType::INSTANCE;
		ret->Scope = currentScope;
		ret->Name = StringFromCpp(instance->GetName());
		return ret;
	}

	/**
	* @brief Build differents objects from a C++ scope recursively
	* @param scope Pointer to the scope to build from
	* @param parent Handle to the parent scope of the current scope
	* @return Handle to a newly builded scope
	*/
	CLIConfScope^ BuildRecursively(ConfScope* scope, CLIConfScope^ parent = nullptr) {
		CLIConfScope^ ret = gcnew CLIConfScope();
		if (parent) ret->Parent = parent;
		ret->Name = gcnew String(scope->GetName().c_str(), 0, scope->GetName().size());
		ret->Childs = gcnew List<CLIConfScopeable^>(scope->GetChilds().size());
		for (auto& it : scope->GetChilds()) {
			CLIConfScopeable^ obj;
			switch (it->GetCodeObjectType()) {
			case CodeObjectType::FUNCTION:
				break;
			case CodeObjectType::INSTANCE: {
				obj = BuildInstance(static_cast<ConfInstance*>(it), ret);
			}break;
			case CodeObjectType::SCOPE: {
				obj = BuildRecursively(static_cast<ConfScope*>(it), ret);
				obj->ObjectType = CLICodeObjectType::SCOPE;
			}break;
			case CodeObjectType::TYPE: {
				obj = BuildRecursively(static_cast<ConfScope*>(it), ret);
				obj->ObjectType = CLICodeObjectType::TYPE;
			}break;
			default:
				break;
			}
			if (obj) {
				obj->Name = gcnew String(it->GetName().c_str(), 0, it->GetName().size());
				ret->Childs->Add(obj);
			}
		}
		return ret;
	}

	/**
	* @brief Build differents objects from a C++ scope
	* @param scope Pointer to the scope to build from
	* @return Handle to a newly builded scope
	*/
	CLIConfScope^ Build(ConfScope* scope) {
		CLIConfScope^ ret = nullptr;
		CLIConfScope^ parent = nullptr;
		if (scope->GetParent()) {
			ret = Build(scope->GetParent());
			ret->Childs->Add(gcnew CLIConfScope());
			parent = ret;
			ret = (CLIConfScope^)ret->Childs[ret->Childs->Count - 1];
		}
		ret = BuildRecursively(scope);
		if(parent)
			ret->Parent = parent;
		return ret;
	}

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
		static CLIConfScope^ Parse(String^ filename) {
			ConfParser parser;
			auto s = parser.Parse(msclr::interop::marshal_as<std::wstring>(filename));
			auto ret = Build(s);
			delete s;
			return ret;
		}
	};
}