/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/

#include "confscope.hpp"
#include "confparser.hpp"
#include "confinstance.hpp"
#include <string>

namespace confparser {
	ConfType* ConfScope::DefaultStringType = nullptr;
	ConfType* ConfScope::DefaultIntegerType = nullptr;
	ConfType* ConfScope::DefaultDecimalType = nullptr;
	ConfType* ConfScope::DefaultObjectType = nullptr;

	ConfScope::~ConfScope() {
		for (ConfScopeable* it : m_Childs) {
			if (_ADDRESSOF(*it) == _ADDRESSOF(*ConfParser::GetIntrinsicScope())) continue;
			CP_SF(it);
		}
	}

	ConfScopeable* ConfScope::GetByName(const string_t& name, CodeObjectType filter) const {
		for (const auto& c : m_Childs) {
			if ((filter != CodeObjectType::NONE ? c->GetCodeObjectType() == filter : true)
				&& c->GetName() == name)
				return c;
		}
		if (m_Parent) return m_Parent->GetByName(name);
		return nullptr;
	}

	void ConfScope::AddChild(ConfScopeable* child) {
		m_Childs.push_back(child);
	}

	void ConfScope::SetDefaultStringType(ConfType* ty) {
		DefaultStringType = ty;
	}

	void ConfScope::SetDefaultIntegerType(ConfType* ty) {
		DefaultIntegerType = ty;
	}

	void ConfScope::SetDefaultDecimalType(ConfType* ty) {
		DefaultDecimalType = ty;
	}

	void ConfScope::SetDefaultObjectType(ConfType* ty) {
		DefaultObjectType = ty;
	}

	ConfScope& ConfScope::operator+=(const ConfScope& scope) {

		for (auto oc : scope.m_Childs) {
			auto c = GetByName(oc->GetName());

			if (c) {
				switch (c->GetCodeObjectType()) {
				case CodeObjectType::INSTANCE:
					*static_cast<ConfInstance*>(c) = *static_cast<ConfInstance*>(oc);
					break;
				case CodeObjectType::TYPE:
					__fallthrough;
				case CodeObjectType::SCOPE:
					*static_cast<ConfScope*>(c) += *static_cast<ConfScope*>(oc);
					break;
				default:
					break;
				}
			}
			else {
				this->AddChild(oc->Clone(oc->GetName(), nullptr));
			}

		}
		return *this;
	}


	ConfScopeable* ConfScope::Clone(string_t name, ConfScopeable* buf) const {
		if (!buf) buf = new ConfScope();
		ConfScope* ret = static_cast<ConfScope*>(buf);
		ret->m_Name = name;
		ret->m_Parent = m_Parent;
		ret->DefaultDecimalType = DefaultDecimalType;
		ret->DefaultIntegerType = DefaultIntegerType;
		ret->DefaultStringType = DefaultStringType;
		for (auto c : m_Childs) {
			ret->AddChild(c->Clone(c->GetName(), nullptr));
		}
		return ret;
	}
}