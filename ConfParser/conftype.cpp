/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/

#include "conftype.hpp"
#include "confinstance.hpp"

namespace confparser {
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

	ConfInstance* ConfTypeInt::_CreateIntInstance(ConfType* type, string_t name) {
		return new ConfInstanceInt(type, std::move(name));
	}

	ConfInstance* ConfTypeFloat::_CreateFloatInstance(ConfType* type, string_t name) {
		return new ConfInstanceFloat(type, std::move(name));
	}
}