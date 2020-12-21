/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/

#include "confoperator.hpp"
#include <string>

namespace confparser {
	ConfScopeable* ConfFunctionIntrinsicOperator::Clone(string_t name, ConfScopeable* buf) const {
		if (!buf) buf = new ConfFunctionIntrinsicOperator(nullptr, name, m_Callback, m_Priority);
		static_cast<ConfFunctionIntrinsicOperator*>(buf)->m_OpType = m_OpType;
		return buf;
	}
}