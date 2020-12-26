/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/
/*!
 * \file confinstance.cpp
 * \brief Instances related implementations
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
}