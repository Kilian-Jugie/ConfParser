/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/
/*!
 * \file confscope.cpp
 * \brief Scopes related implementations
 */

#include "confscope.hpp"
#include "confparser.hpp"
#include "confinstance.hpp"
#include <string>

namespace confparser {
	ConfScope::~ConfScope() {
		for (ConfScopeable* it : m_Childs) {
			//!\deprecated Intrinsic scope should not be any scope child but check needed
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

	ConfScope& ConfScope::operator+=(const ConfScope& scope) {

		for (auto oc : scope.m_Childs) {
			auto c = GetByName(oc->GetName());

			if (c) {
				switch (c->GetCodeObjectType()) {
				case CodeObjectType::INSTANCE:
					*static_cast<ConfInstance*>(c) = *static_cast<ConfInstance*>(oc);
					break;
				case CodeObjectType::TYPE:
					[[fallthrough]];
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
		for (auto c : m_Childs) {
			ret->AddChild(c->Clone(c->GetName(), nullptr));
		}
		return ret;
	}
}