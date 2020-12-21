/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/

#include "conffunction.hpp"

namespace confparser {
	ConfScopeable* ConfFunctionIntrinsic::Clone(string_t name, ConfScopeable* buf) const {
		if (!buf) buf = new ConfFunctionIntrinsic(nullptr, name, m_Callback);
		return buf;
	}
}