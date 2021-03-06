/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/

#include "confparser.h"
#include <ConfParser/confinstance.hpp>
#include <ConfParser/confscope.hpp>
#include <ConfParser/confparser.hpp>
#include <ConfParser/conftype.hpp>

namespace confparser {

	/**
	* @brief Parse a file with ConfParser
	* @param filename Handle to the filename to parse from
	* @return Handle to global scope from ConfParser
	*/
	CLIConfScope^ confparser::CLIConfParser::Parse(String^ filename) {
		ConfParser parser;
		auto s = parser.Parse(msclr::interop::marshal_as<std::wstring>(filename));
		auto ret = Build(s);
		delete s;
		return ret;
	}

	/**
	* @brief Build a CLI instance from a C++ instance
	* @param instance Pointer to the C++ instance to build from
	* @param currenScope Handle to the scope where the declaration is
	* @return Handle to a new constructed instance
	* @see CLIConfInstanceNative, CLIConfInstanceImported
	*/
	CLIConfInstance^ confparser::BuildInstance(ConfInstance* instance, CLIConfScope^ currentScope) {
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
	CLIConfScope^ confparser::BuildRecursively(ConfScope* scope, CLIConfScope^ parent) {
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
	CLIConfScope^ confparser::Build(ConfScope* scope) {
		CLIConfScope^ ret = nullptr;
		CLIConfScope^ parent = nullptr;
		if (scope->GetParent()) {
			ret = Build(scope->GetParent());
			ret->Childs->Add(gcnew CLIConfScope());
			parent = ret;
			ret = (CLIConfScope^)ret->Childs[ret->Childs->Count - 1];
		}
		ret = BuildRecursively(scope);
		if (parent)
			ret->Parent = parent;
		return ret;
	}
}