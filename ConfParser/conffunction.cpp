#include "conffunction.hpp"

namespace confparser {
	ConfScopeable* ConfFunctionIntrinsic::Clone(string_t name, ConfScopeable* buf) const {
		if (!buf) buf = new ConfFunctionIntrinsic(nullptr, name, m_Callback);
		return buf;
	}
}