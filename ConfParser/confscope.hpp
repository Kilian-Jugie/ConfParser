/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/

#pragma once
#include "global.hpp"
#include "confscopeable.hpp"

namespace confparser {
	class ConfScope : public ConfScopeable {
	public:
		using tokcallback_t = bool(*)(const std::vector<string_t>&);

		ConfScope(ConfScope* parent = nullptr) : m_Parent{ parent } {}
		~ConfScope();

		virtual CodeObjectType GetCodeObjectType() const override {
			return CodeObjectType::SCOPE;
		}

		ConfScope* GetParent() const {
			return m_Parent;
		}

		const std::vector<ConfScopeable*>& GetChilds() {
			return m_Childs;
		}

		void SetTokenCallback(tokcallback_t call) {
			m_TokenCallback = call;
		}

		bool TokenCallback(const std::vector<string_t>& tokens) {
			return m_TokenCallback ? m_TokenCallback(tokens) : false;
		}

		ConfScopeable* GetByName(const string_t& name, CodeObjectType filter = CodeObjectType::NONE) const;
		void AddChild(ConfScopeable* child);

		static void SetDefaultStringType(ConfType* ty);
		static void SetDefaultIntegerType(ConfType* ty);
		static void SetDefaultDecimalType(ConfType* ty);
		static void SetDefaultObjectType(ConfType* ty);

		static ConfType* GetDefaultObjectType() {
			return DefaultObjectType;
		}

		ConfType* TypeFromRValue(string_t rvalue);
		ConfInstance* InstanceFromRValue(string_t rvalue);

		ConfScope& operator+=(const ConfScope& scope);

		virtual ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;

	private:
		ConfScope* m_Parent;
		std::vector<ConfScopeable*> m_Childs;
		tokcallback_t m_TokenCallback;
		static ConfType* DefaultStringType;
		static ConfType* DefaultIntegerType;
		static ConfType* DefaultDecimalType;
		static ConfType* DefaultObjectType;
	};
}