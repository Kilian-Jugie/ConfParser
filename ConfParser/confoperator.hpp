/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/

#pragma once
#include "global.hpp"
#include "conffunction.hpp"

namespace confparser {

	enum class ConfOperatorType {
		PRE,	//++i
		MID,	//5+6
		POST,	//i++
		SUR		//[i]
	};

	class ConfFunctionIntrinsicOperator :
		public ConfFunctionIntrinsic {
	protected:
		std::size_t m_Priority;
		ConfOperatorType m_OpType;
	public:
		ConfFunctionIntrinsicOperator(ConfScope* parent, string_t name,
			ConfFunctionIntrinsic::intricfunc_t callback, std::size_t priority) :
			ConfFunctionIntrinsic{ parent, name, callback }, m_Priority{ priority },
			m_OpType{ ConfOperatorType::MID } {}

		std::size_t GetPriority() {
			return m_Priority;
		}

		void SetPriority(std::size_t priority) {
			m_Priority = priority;
		}

		ConfOperatorType GetOpType() {
			return m_OpType;
		}

		void SetOpType(ConfOperatorType ty) {
			m_OpType = ty;
		}

		virtual ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;
	};

	class ConfFunctionExtrinsicOperator : public ConfFunctionIntrinsicOperator {
	public:
		ConfFunctionExtrinsicOperator(ConfScope* parent, string_t name, std::size_t priority) :
			ConfFunctionIntrinsicOperator{ parent, std::move(name), nullptr, priority } {}
	};

	class OperatorParserOperation {
		ConfFunctionExtrinsicOperator* m_Operator;
		std::vector<int> m_OperandIndexes;
		int m_InstanceIndex;
	public:
		OperatorParserOperation(ConfFunctionExtrinsicOperator* op, int inst, std::vector<int> operands) :
			m_Operator{ op }, m_InstanceIndex{ inst }, m_OperandIndexes{ std::move(operands) } {}
	};
}