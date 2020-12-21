/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/

#pragma once
#include "global.hpp"
#include "confscope.hpp"
#include <functional>

namespace confparser {
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

		virtual ConfInstance* Call(ConfInstance* _this, std::vector<ConfInstance*> parameters) {
			return m_Callback(_this, parameters);
		}

		virtual ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;

	protected:
		intricfunc_t m_Callback;
		ConfScope* m_Parent;
	};

	class ConfFunctionExtrinsic : public ConfFunctionIntrinsic {
		std::vector<std::vector<string_t>> m_Instructions;

		bool CaptureTokens(const std::vector<string_t>& tokens) {
			m_Instructions.push_back(tokens);
			return true;
		}

	public:
		ConfFunctionExtrinsic(ConfScope* parent, string_t name)
			: ConfFunctionIntrinsic{ parent, name, nullptr } {
			//SetTokenCallback();
		}

		virtual ConfInstance* Call(ConfInstance* _this, std::vector<ConfInstance*> parameters) override {
			return nullptr;
		}
	};
}