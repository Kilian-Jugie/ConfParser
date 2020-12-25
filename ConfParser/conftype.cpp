/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/

#include "conftype.hpp"
#include "confinstance.hpp"
#include <cctype>
#include <algorithm>

namespace confparser {
	std::unordered_map<string_t, ConfTypeIntrinsic*> ConfTypeIntrinsic::IntrinsicTypesRegistry;

	ConfInstance* ConfType::_CreateInstance(ConfType* type, string_t name) {
		ConfInstance* inst = new ConfInstance(type, std::move(name));
		for (auto c : type->GetChilds()) {
			if (c->GetCodeObjectType() == CodeObjectType::INSTANCE) {
				ConfInstance* subInst = static_cast<ConfInstance*>(c);
				inst->AddSubInstance(subInst->GetType()->CreateInstance(c->GetName()));
			}
		}
		return inst;
	}

	ConfScopeable* ConfType::Clone(string_t name, ConfScopeable* buf) const {
		if (!buf) buf = new ConfType(name);
		static_cast<ConfType*>(buf)->CreateInstanceCallback = CreateInstanceCallback;
		return buf;
	}

	ConfInstance* ConfTypeString::_CreateStringInstance(ConfType* type, string_t name) {
		return new ConfInstanceString(type, std::move(name));
	}

	int ConfTypeString::IsExprCompatible(string_t expr, ConfScope* scope) {
		return expr.size() > 0 && expr[0] == TOKEN_CHAR_STRING &&
			expr[expr.size() - 1] == TOKEN_CHAR_STRING ? 1000 : -1;
	}

	ConfInstance* ConfTypeInt::_CreateIntInstance(ConfType* type, string_t name) {
		return new ConfInstanceInt(type, std::move(name));
	}

	int ConfTypeInt::IsExprCompatible(string_t expr, ConfScope* scope) {
		return (!expr.empty() && std::all_of(expr.begin(), expr.end(), 
			[](int i) { return std::isdigit(i) || i == '-'; })) ? 1000 : -1;
	}

	ConfInstance* ConfTypeFloat::_CreateFloatInstance(ConfType* type, string_t name) {
		return new ConfInstanceFloat(type, std::move(name));
	}

	int ConfTypeFloat::IsExprCompatible(string_t expr, ConfScope* scope) {
		bool isNum = (!expr.empty() && std::all_of(expr.begin(), expr.end(),
			[](int i) { return std::isdigit(i) || i == '-' || i == '.'; }));
		if (!isNum)
			return -1;
		if (expr.find_first_of('.') != string_t::npos) {
			if (expr.size() == 1) return -1; //if there is only a dot (operator.)
			return 1000;
		}
		return 500;
	}

	ConfTypeIntrinsic::ConfTypeIntrinsic(string_t name) : ConfType{ name } {
		IntrinsicTypesRegistry[name] = this;
	}

	int ConfTypeIntrinsic::IsExprCompatible(string_t expr, ConfScope* scope) {
		return -1;
	}

	ConfTypeIntrinsic* ConfTypeIntrinsic::TypeFromExpression(string_t expr, ConfScope* scope) {
		int betterCompat = -1;
		ConfTypeIntrinsic* betterType = nullptr;
		for (const auto& ty : IntrinsicTypesRegistry) {
			if (int curComp = ty.second->IsExprCompatible(expr, scope); curComp > betterCompat) {
				betterType = ty.second;
				betterCompat = curComp;
			}
		}
		return betterType;
	}

	ConfInstance* ConfTypeIntrinsic::InstanceFromExpression(string_t expr, ConfScope* scope, string_t name) {
		return TypeFromExpression(expr, scope)->CreateInstance(std::move(name));
	}

	ConfInstance* ConfTypeObject::_CreateObjectInstance(ConfType* type, string_t name) {
		return new ConfInstanceObject(type, std::move(name));
	}

	int ConfTypeObject::IsExprCompatible(string_t expr, ConfScope* scope) {
		return IntrinsicTypesRegistry[NAME_TYPE_FLOAT]->IsExprCompatible(expr, scope) > 0 || 
			IntrinsicTypesRegistry[NAME_TYPE_STRING]->IsExprCompatible(expr, scope) > 0 ||
			IntrinsicTypesRegistry[NAME_TYPE_INT]->IsExprCompatible(expr, scope) > 0 ? 1 : -1;
	}
	ConfInstance* ConfTypeExpr::_CreateExprInstance(ConfType* type, string_t name) {
		return nullptr;
	}
	int ConfTypeExpr::IsExprCompatible(string_t expr, ConfScope* scope) {
		return IntrinsicTypesRegistry[NAME_TYPE_OBJECT] ? -1 : 1;
	}
}