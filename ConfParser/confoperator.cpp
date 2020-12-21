#include "confoperator.hpp"
#include <string>

namespace confparser {
	ConfScopeable* ConfFunctionIntrinsicOperator::Clone(string_t name, ConfScopeable* buf) const {
		if (!buf) buf = new ConfFunctionIntrinsicOperator(nullptr, name, m_Callback, m_Priority);
		static_cast<ConfFunctionIntrinsicOperator*>(buf)->m_OpType = m_OpType;
		return buf;
	}
}