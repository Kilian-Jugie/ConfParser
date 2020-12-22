/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/

#pragma once
#include "global.hpp"
#include "confscopeable.hpp"

namespace confparser {
	class ConfInstance : public ConfScopeable {
	protected:
		std::vector<ConfInstance*> m_SubInstances;
		ConfType* m_Type;
	public:
		ConfInstance(ConfType* type, string_t name) {
			m_Type = type;
			m_Name = std::move(name);
		}

		~ConfInstance() {
			ClearSubInstances();
		}

		void ClearSubInstances() {
			for (auto it : m_SubInstances) {
				CP_SF(it);
			}
			m_SubInstances.clear();
		}

		virtual CodeObjectType GetCodeObjectType() const override {
			return CodeObjectType::INSTANCE;
		}

		virtual ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;
		virtual ConfInstance* GetMember(const string_t& memberName);

		virtual ConfFunctionIntrinsic* GetFunction(const string_t& funcName);

		virtual std::size_t GetSize() const {
			return 0;
		}

		virtual ConfType* GetType() const {
			return m_Type;
		}

		virtual const std::vector<ConfInstance*>& GetSubInstances() {
			return m_SubInstances;
		}

		virtual void AddSubInstance(ConfInstance* inst) {
			m_SubInstances.push_back(inst);
		}

		virtual ConfInstance& operator=(ConfInstance* inst) {
			m_SubInstances = inst->m_SubInstances;
			return *this;
		}
	};

	class ConfIntrinsicInstance : public ConfInstance {
	public:
		ConfIntrinsicInstance(ConfType* strType, string_t name) : ConfInstance{ strType,std::move(name) } {

		}

		virtual void SetFromString(const string_t& v);
	};

	class ConfInstanceString : public ConfIntrinsicInstance {
		string_t m_Data;
	public:
		ConfInstanceString(ConfType* strType, string_t name) :
			ConfIntrinsicInstance{ strType,std::move(name) }, m_Data{ CP_TEXT("") } {
		}

		void Set(string_t newData) {
			m_Data = newData;
		}

		string_t Get() const {
			return m_Data;
		}

		virtual ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;

		virtual ConfInstance& operator=(ConfInstance* inst) override {
			ConfInstance::operator=(inst);
			m_Data = static_cast<ConfInstanceString*>(inst)->m_Data;
			return *this;
		}

		virtual void SetFromString(const string_t& v) override;
	};

	class ConfInstanceInt : public ConfIntrinsicInstance {
		int m_Data;
	public:
		ConfInstanceInt(ConfType* strType, string_t name) :
			ConfIntrinsicInstance{ strType,std::move(name) }, m_Data{ 0 } {
		}

		void Set(int newData) {
			m_Data = newData;
		}

		int Get() const {
			return m_Data;
		}

		virtual ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;

		virtual ConfInstance& operator=(ConfInstance* inst) override {
			ConfInstance::operator=(inst);
			m_Data = static_cast<ConfInstanceInt*>(inst)->m_Data;
			return *this;
		}

		virtual void SetFromString(const string_t& v) override;
	};

	class ConfInstanceFloat : public ConfIntrinsicInstance {
		float m_Data;
	public:
		ConfInstanceFloat(ConfType* strType, string_t name) :
			ConfIntrinsicInstance{ strType,std::move(name) }, m_Data{ 0.f } {
		}

		void Set(float newData) {
			m_Data = newData;
		}

		float Get() const {
			return m_Data;
		}

		virtual ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;

		virtual ConfInstance& operator=(ConfInstance* inst) override {
			ConfInstance::operator=(inst);
			m_Data = static_cast<ConfInstanceFloat*>(inst)->m_Data;
			return *this;
		}

		virtual void SetFromString(const string_t& v) override;
	};

	class ConfInstanceObject : public ConfIntrinsicInstance {
		ConfInstance* m_Data;
	public:
		ConfInstanceObject(ConfType* strType, string_t name) :
			ConfIntrinsicInstance{ strType,std::move(name) }, m_Data{ nullptr } {
		}

		void Set(ConfInstance* newData) {
			m_Data = newData;
		}

		ConfInstance* Get() const {
			return m_Data;
		}

		virtual ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;
		virtual void SetFromString(const string_t& v) override;
	};
}