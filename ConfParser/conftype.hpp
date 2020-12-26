/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/
/*!
 * \file conftype.hpp
 * \brief Types related definitions
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

	/*!
	 * \brief Represent an in-code usable type
	*/
	class ConfType : public ConfScope {
	protected:
		using CreateInstance_t = ConfInstance * (*)(ConfType*, string_t);

		/*!
		 * \brief Function to call to create a new instance of the object
		 * \param type The type of the variable to create
		 * \param name The name of the variable to create
		*/
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

		/*!
		 * \brief Proxy function used to call polymorphic-overrwritten CreateInstanceCallback
		 * \see CreateInstanceCallback
		*/
		ConfInstance* CreateInstance(string_t name) {
			return CreateInstanceCallback(this, name);
		}

		ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;
	};

	/*!
	 * \brief Represent an intrinsic type
	 * 
	 * An intrinsic type is a standard type which can be used to create temporary variables from
	 * rvalues (values which are not properly variables but must be converted to like 1 or "hello")
	*/
	class ConfTypeIntrinsic : public ConfType {
	protected:
		/*!
		 * \brief Registry of all intrinsic types
		*/
		static std::unordered_map<string_t, ConfTypeIntrinsic*> IntrinsicTypesRegistry;
	public:
		static const std::unordered_map<string_t, ConfTypeIntrinsic*>& GetTypesRegistry() {
			return IntrinsicTypesRegistry;
		}

		ConfTypeIntrinsic(string_t name);

		/*!
		 * \brief Returns a compatibility indice where a greater value is a better type compatibility
		 * 
		 * For example 5 will have a 1000 compatibility with int, 500 with float and -1 with string so
		 * we prefer the int, if not available, we choose the float.
		 * \param expr The expression to test for
		 * \param scope The scope where the expression is
		*/
		virtual int IsExprCompatible(string_t expr, ConfScope* scope);

		/*!
		 * \brief Get the best compatible type from an expression otherwise nullptr
		 * \param expr The expression to extract the type from
		 * \param scope The scope where the expression is
		*/
		static ConfTypeIntrinsic* TypeFromExpression(string_t expr, ConfScope* scope);

		/*!
		 * \brief Create a new instance from the expression if compatible otherwise nullptr
		 * \param expr The expression to create the instance from
		 * \param scope The scope where the expression is
		 * \param name The name of the new created instance
		*/
		static ConfInstance* InstanceFromExpression(string_t expr, ConfScope* scope, string_t name);
	};

	/*!
	 * \brief Intrinsic type 'object'
	*/
	class ConfTypeObject : public ConfTypeIntrinsic {
	private:
		static ConfInstance* _CreateObjectInstance(ConfType* type, string_t name);
	public:

		ConfTypeObject() : ConfTypeIntrinsic(NAME_TYPE_OBJECT) {
			CreateInstanceCallback = _CreateObjectInstance;
		}

		virtual int IsExprCompatible(string_t expr, ConfScope* scope) override;
	};

	/*!
	 * \brief Intrinsic type 'string'
	*/
	class ConfTypeString : public ConfTypeIntrinsic {
	private:
		static ConfInstance* _CreateStringInstance(ConfType* type, string_t name);
	public:

		ConfTypeString() : ConfTypeIntrinsic(NAME_TYPE_STRING) {
			CreateInstanceCallback = _CreateStringInstance;
		}

		virtual int IsExprCompatible(string_t expr, ConfScope* scope) override;
	};

	/*!
	 * \brief Intrinsic type 'int'
	*/
	class ConfTypeInt : public ConfTypeIntrinsic {
	private:
		static ConfInstance* _CreateIntInstance(ConfType* type, string_t name);
	public:
		ConfTypeInt() : ConfTypeIntrinsic(NAME_TYPE_INT) {
			CreateInstanceCallback = _CreateIntInstance;
		}

		virtual int IsExprCompatible(string_t expr, ConfScope* scope) override;
	};

	/*!
	 * \brief Intrinsic type 'float'
	*/
	class ConfTypeFloat : public ConfTypeIntrinsic {
	private:
		static ConfInstance* _CreateFloatInstance(ConfType* type, string_t name);
	public:
		ConfTypeFloat() : ConfTypeIntrinsic(NAME_TYPE_FLOAT) {
			CreateInstanceCallback = _CreateFloatInstance;
		}

		virtual int IsExprCompatible(string_t expr, ConfScope* scope) override;
	};

	/*!
	 * \brief Intrinsic type 'expr'
	*/
	class ConfTypeExpr : public ConfTypeIntrinsic {
	private:
		static ConfInstance* _CreateExprInstance(ConfType* type, string_t name);
	public:
		ConfTypeExpr() : ConfTypeIntrinsic(NAME_TYPE_EXPR) {
			CreateInstanceCallback = _CreateExprInstance;
		}

		virtual int IsExprCompatible(string_t expr, ConfScope* scope) override;
	};
}