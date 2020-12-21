/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/

#pragma once
#include "global.hpp"
#include "confscope.hpp"

namespace confparser {
	class ConfType : public ConfScope {
	protected:
		using CreateInstance_t = ConfInstance * (*)(ConfType*, string_t);
		CreateInstance_t CreateInstanceCallback;

	private:

		static ConfInstance* _CreateInstance(ConfType* type, string_t name);
	public:
		ConfType(string_t name, ConfScope* parent = nullptr) : ConfScope{ parent } {
			m_Name = std::move(name);
			CreateInstanceCallback = _CreateInstance;
		}

		virtual CodeObjectType GetCodeObjectType() const override {
			return CodeObjectType::TYPE;
		}

		ConfInstance* CreateInstance(string_t name) {
			return CreateInstanceCallback(this, name);
		}

		ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;
	};

	class ConfTypeString : public ConfType {
	private:
		static ConfInstance* _CreateStringInstance(ConfType* type, string_t name);
	public:
		ConfTypeString(string_t name) : ConfType(std::move(name)) {
			CreateInstanceCallback = _CreateStringInstance;
		}
	};

	class ConfTypeInt : public ConfType {
	private:
		static ConfInstance* _CreateIntInstance(ConfType* type, string_t name);
	public:
		ConfTypeInt(string_t name) : ConfType(std::move(name)) {
			CreateInstanceCallback = _CreateIntInstance;
		}
	};

	class ConfTypeFloat : public ConfType {
	private:
		static ConfInstance* _CreateFloatInstance(ConfType* type, string_t name);
	public:
		ConfTypeFloat(string_t name) : ConfType(std::move(name)) {
			CreateInstanceCallback = _CreateFloatInstance;
		}
	};
}