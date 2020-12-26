/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/
/*!
 * \file confoperator.hpp
 * \brief Operators related definitions
 */
#pragma once
#include "global.hpp"
#include "conffunction.hpp"

namespace confparser {
	/*!
	 * \brief Type of operator
	 * 
	 * Possible types:
	 *  - PRE: prefix, example: ++i where '++' is the operator
	 *  - MID: middle, example: a+b where '+' is the operator
	 *  - POST: postfix, example: i++ where '++' is the operator
	 *  - SUR: surround, example: [i] where '[]' is the operator
	*/
	enum class ConfOperatorType {
		PRE,
		MID,
		POST,
		SUR	
	};

	/*!
	 * \brief Intrinsic operator definition
	 * 
	 * Function defined as operator with intrinsic calling mode.
	 * 
	 * \see ConfFunctionIntrinsic
	*/
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

		/*!
		 * \brief Get the priority of the operator
		 * 
		 * Priority is defined in descending order as priority
		 * 1 is, by default, parenthesis or dot (maximum priority) and 14 is 
		 * equal following the C operators priority model.
		 * 
		 * Priority is not fixed for a specific operator and can be changed
		*/
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

	/*!
	 * \brief Extrinsic operator definition
	 *
	 * Function defined as operator with extrinsic calling mode.
	 *
	 * \see ConfFunctionExtrinsic
	*/
	class ConfFunctionExtrinsicOperator : public ConfFunctionIntrinsicOperator {
	public:
		ConfFunctionExtrinsicOperator(ConfScope* parent, string_t name, std::size_t priority) :
			ConfFunctionIntrinsicOperator{ parent, std::move(name), nullptr, priority } {}
	};
}