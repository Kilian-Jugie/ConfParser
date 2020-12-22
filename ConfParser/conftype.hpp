/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/

#pragma once
#include "global.hpp"
#include "confscope.hpp"
#include <unordered_map>

namespace confparser {
	constexpr char_t NAME_TYPE_STRING[] = CP_TEXT("string");
	constexpr char_t NAME_TYPE_INT[] = CP_TEXT("int");
	constexpr char_t NAME_TYPE_FLOAT[] = CP_TEXT("float");
	constexpr char_t NAME_TYPE_OBJECT[] = CP_TEXT("object");
	constexpr char_t NAME_TYPE_EXPR[] = CP_TEXT("expr");

	class ConfType : public ConfScope {
	protected:
		using CreateInstance_t = ConfInstance * (*)(ConfType*, string_t);
		CreateInstance_t CreateInstanceCallback;

	private:

		static ConfInstance* _CreateInstance(ConfType* type, string_t name);
	public:
		ConfType(string_t name, ConfScope* parent = nullptr) : ConfScope{ parent } {
			m_Name = std::move(name);
			CreateInstanceCallback = _CreateInstance;
		}

		virtual CodeObjectType GetCodeObjectType() const override {
			return CodeObjectType::TYPE;
		}

		ConfInstance* CreateInstance(string_t name) {
			return CreateInstanceCallback(this, name);
		}

		ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;
	};

	class ConfTypeIntrinsic : public ConfType {
	protected:
		static std::unordered_map<string_t, ConfTypeIntrinsic*> IntrinsicTypesRegistry;
	public:
		static const std::unordered_map<string_t, ConfTypeIntrinsic*>& GetTypesRegistry() {
			return IntrinsicTypesRegistry;
		}

		ConfTypeIntrinsic(string_t name);
		virtual int IsExprCompatible(string_t expr, ConfScope* scope);
		static ConfTypeIntrinsic* TypeFromExpression(string_t expr, ConfScope* scope);
		static ConfInstance* InstanceFromExpression(string_t expr, ConfScope* scope, string_t name);
	};

	class ConfTypeObject : public ConfTypeIntrinsic {
	private:
		static ConfInstance* _CreateObjectInstance(ConfType* type, string_t name);
	public:
		ConfTypeObject() : ConfTypeIntrinsic(NAME_TYPE_OBJECT) {
			CreateInstanceCallback = _CreateObjectInstance;
		}

		virtual int IsExprCompatible(string_t expr, ConfScope* scope) override;
	};

	class ConfTypeString : public ConfTypeIntrinsic {
	private:
		static ConfInstance* _CreateStringInstance(ConfType* type, string_t name);
	public:
		ConfTypeString() : ConfTypeIntrinsic(NAME_TYPE_STRING) {
			CreateInstanceCallback = _CreateStringInstance;
		}

		virtual int IsExprCompatible(string_t expr, ConfScope* scope) override;
	};

	class ConfTypeInt : public ConfTypeIntrinsic {
	private:
		static ConfInstance* _CreateIntInstance(ConfType* type, string_t name);
	public:
		ConfTypeInt() : ConfTypeIntrinsic(NAME_TYPE_INT) {
			CreateInstanceCallback = _CreateIntInstance;
		}

		virtual int IsExprCompatible(string_t expr, ConfScope* scope) override;
	};

	class ConfTypeFloat : public ConfTypeIntrinsic {
	private:
		static ConfInstance* _CreateFloatInstance(ConfType* type, string_t name);
	public:
		ConfTypeFloat() : ConfTypeIntrinsic(NAME_TYPE_FLOAT) {
			CreateInstanceCallback = _CreateFloatInstance;
		}

		virtual int IsExprCompatible(string_t expr, ConfScope* scope) override;
	};

	class ConfTypeExpr : public ConfTypeIntrinsic {
	private:
		static ConfInstance* _CreateExprInstance(ConfType* type, string_t name);
	public:
		ConfTypeExpr() : ConfTypeIntrinsic(NAME_TYPE_FLOAT) {
			CreateInstanceCallback = _CreateExprInstance;
		}

		virtual int IsExprCompatible(string_t expr, ConfScope* scope) override;
	};
}