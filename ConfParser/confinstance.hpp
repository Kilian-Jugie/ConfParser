/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/
/*!
 * \file confinstance.hpp
 * \brief Instances related definitions
 */

#pragma once
#include "global.hpp"
#include "confscopeable.hpp"
#include <string>
#include <cassert>

namespace confparser {
	/*!
	 * \brief In-code instance of an object
	 * 
	 * An instance is only a container of sub-instances and does not
	 * contain real data.
	 */
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

		/*!
		 * \brief Safe delete all subinstances
		 */
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

		/*!
		 * \brief Get a subinstance by its name
		 * \param memberName The name of the subinstance
		*/
		virtual ConfInstance* GetMember(const string_t& memberName);

		/*!
		 * \brief Get a method by its name
		 * \param funcName The name of the method to get
		*/
		virtual ConfFunctionIntrinsic* GetFunction(const string_t& funcName);

		/*!
		 * \brief [Unused: to be removed] Get the memory cost of the instance
		*/
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

		virtual void SetFromString(const string_t& v) {
			//Possible alternative : Anonymous structure ?
			assert(false && "SetFromString is reserved for intrinsic usage");
		}
	};

	/*!
	 * \brief In-code accessible instance of an object
	 *
	 * Instance of an intrinsic type. These instances, unlike others, contains
	 * data in a form which depends of their type.
	 */
	template<typename _Ty>class ConfIntrinsicInstance : public ConfInstance {
	protected:
		_Ty m_Data;
	public:
		ConfIntrinsicInstance<_Ty>(ConfType* strType, string_t name) : ConfInstance{ strType,std::move(name) } {

		}

		/*!
		 * \brief Set the raw value
		 * \param data The new value
		*/
		inline void Set(_Ty data) {
			m_Data = data;
		}

		/*!
		 * \brief Get the raw value
		*/
		inline _Ty Get() const {
			return m_Data;
		}

		/*!
		 * \brief Set the raw value from a string
		 * 
		 * This function is intended to be template specialized for each
		 * possible raw value type
		 * 
		 * \param v The string to set the value from
		*/
		virtual void SetFromString(const string_t& v) override final {
			ConfInstance::SetFromString(v);
		}
	};

	using ConfInstanceString = ConfIntrinsicInstance<string_t>;
	using ConfInstanceInt = ConfIntrinsicInstance<int>;
	using ConfInstanceFloat = ConfIntrinsicInstance<float>;
	using ConfInstanceObject = ConfIntrinsicInstance<ConfInstance*>;

	template<> void ConfInstanceString::SetFromString(const string_t& v) {
		m_Data = v;
	}

	template<> void ConfInstanceInt::SetFromString(const string_t& v) {
		m_Data = std::stoi(v);
	}

	template<> void ConfInstanceFloat::SetFromString(const string_t& v) {
		m_Data = std::stof(v);
	}

	template<> void ConfInstanceObject::SetFromString(const string_t& v) {
		//Peek instance addr by scope lookaround
		assert(false && "WIP");
	}
}