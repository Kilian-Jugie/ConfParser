/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/

#include "confinstance.hpp"
#include "conftype.hpp"
#include "conffunction.hpp"
#include <string>

namespace confparser {
	ConfScopeable* ConfInstance::Clone(string_t name, ConfScopeable* buf) const {
		ConfInstance* ret = m_Type->CreateInstance(std::move(name));
		ret->ClearSubInstances();
		for (auto s : m_SubInstances)
			ret->AddSubInstance(static_cast<ConfInstance*>(s->Clone(s->GetName())));
		return ret;
	}

	ConfInstance* ConfInstance::GetMember(const string_t& memberName) {
		ConfInstance* ret = static_cast<ConfInstance*>(m_Type->GetByName(memberName, CodeObjectType::INSTANCE));
		if (ret) return ret;
		for (auto inst : m_SubInstances) {
			if (inst->GetName() == memberName) return inst;
		}
		return nullptr;
	}

	ConfFunctionIntrinsic* ConfInstance::GetFunction(const string_t& funcName) {
		return static_cast<ConfFunctionIntrinsic*>(m_Type->GetByName(funcName, CodeObjectType::FUNCTION));
	}

	ConfScopeable* ConfInstanceString::Clone(string_t name, ConfScopeable* buf) const {
		auto r = static_cast<ConfInstanceString*>(
			ConfInstance::Clone(std::move(name), buf));
		r->Set(Get());
		return r;
	}

	void ConfInstanceString::SetFromString(const string_t& v) {
		m_Data = v;
	}

	ConfScopeable* ConfInstanceInt::Clone(string_t name, ConfScopeable* buf) const {
		auto r = static_cast<ConfInstanceInt*>(
			ConfInstance::Clone(std::move(name), buf));
		r->Set(Get());
		return r;
	}

	void ConfInstanceInt::SetFromString(const string_t& v) {
		m_Data = std::stoi(v);
	}

	ConfScopeable* ConfInstanceFloat::Clone(string_t name, ConfScopeable* buf) const {
		auto r = static_cast<ConfInstanceFloat*>(
			ConfInstance::Clone(std::move(name), buf));
		r->Set(Get());
		return r;
	}

	void ConfInstanceFloat::SetFromString(const string_t& v) {
		m_Data = stof(v);
	}

	ConfScopeable* ConfInstanceObject::Clone(string_t name, ConfScopeable* buf) const {
		auto r = static_cast<ConfInstanceObject*>(
			ConfInstance::Clone(std::move(name), buf));
		r->Set(Get());
		return r;
	}

	void ConfInstanceObject::SetFromString(const string_t& v)
	{
	}

	void ConfIntrinsicInstance::SetFromString(const string_t& v)
	{
	}
}