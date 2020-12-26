/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/
/*!
 * \file conffunction.hpp
 * \brief Functions related definitions
 */

#pragma once
#include "global.hpp"
#include "confscope.hpp"
#include <functional>

namespace confparser {
	/*!
	 * \brief Intrinsic function definition
	 * 
	 * Intrinsic function designate an in-code callable function with a compiler-implemented
	 * definition. When the interpreter encounters a call to this function it directly call
	 * C++ code linked to.
	 */
	class ConfFunctionIntrinsic : public ConfScope {
	public:
		using intricfunc_t = std::function<ConfInstance* (ConfInstance*, std::vector<ConfInstance*>)>;

		ConfFunctionIntrinsic(ConfScope* parent, string_t name, intricfunc_t callback) :
			m_Callback{ callback }, m_Parent{ parent } {
			m_Name = std::move(name);
		}

		virtual CodeObjectType GetCodeObjectType() const override {
			return CodeObjectType::FUNCTION;
		}

		/*!
		 * \brief Call the code linked to this function
		 * \param _this The instance from where the method is called from
		 * \param parameters List of parameters passed as arguments for the function call
		 */
		virtual ConfInstance* Call(ConfInstance* _this, std::vector<ConfInstance*> parameters) {
			return m_Callback(_this, parameters);
		}

		virtual ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;

	protected:
		intricfunc_t m_Callback;
		ConfScope* m_Parent;
	};

	/*!
	 * \brief Extrinsic function
	 * 
	 *  !!! WIP !!!
	 * 
	 * An extrinsic function is a in-code callable function linked to in-code expression
	 * 
	 * \todo Rework polymorphism on inheritance to avoid useless Callback member !
	 */
	class ConfFunctionExtrinsic : public ConfFunctionIntrinsic {
		std::vector<std::vector<string_t>> m_Instructions;

	public:
		ConfFunctionExtrinsic(ConfScope* parent, string_t name)
			: ConfFunctionIntrinsic{ parent, name, nullptr } {
		}

		/*!
		 * \brief Call the code linked to this function
		 * \param _this The instance from where the method is called from
		 * \param parameters List of parameters passed as arguments for the function call
		 * 
		 * \todo Implement this (need \a expr to be OK !)
		 */
		virtual ConfInstance* Call(ConfInstance* _this, std::vector<ConfInstance*> parameters) override {
			return nullptr;
		}
	};
}