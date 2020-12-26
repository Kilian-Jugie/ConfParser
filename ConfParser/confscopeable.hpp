/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/
/*!
 * \file confscopeable.hpp
 * \brief Scopeables related definitions
 */

#pragma once
#include "global.hpp"

namespace confparser {
	/*!
	 * \brief Represent a in-code scopeable object
	 * 
	 * A scopeable object is any object capable of being declarated and used
	 * in a particular scope. In other words, it's a potential scope child
	*/
	class ConfScopeable {
	protected:
		/*!
		 * \brief The name used to designate the object
		*/
		string_t m_Name;

		/*!
		 * \brief Define wherever the object is temporary and should be delete at
		 *		  the end of the instruction (functions returns for example)
		*/
		bool m_IsTemporary;
	public:
		ConfScopeable() = default;

		virtual string_t GetName() const { return m_Name; }

		/*!
		 * \brief Get the object type of the current object
		 * \see CodeObjectType
		*/
		virtual CodeObjectType GetCodeObjectType() const = 0;

		/*!
		 * \brief Clone the current object and its properties in another
		 * 
		 * \warning Even if the scope is passed as parameter, the new created 
		 *			object is not automatically registered in the destination
		 *			scope. It only serves as other objects registry
		 * \param name The name of the new created object
		 * \param scope The scope where the object will be created
		*/
		virtual ConfScopeable* Clone(string_t name, ConfScopeable* scope) const {
			/* This method should be abstract pure but compiler could not
			understand that every time it is called, a child version is called.
			So link failed if this function is not defined but calling this
			version has no sense*/
			return nullptr;
		}

		void SetTemp(bool v) {
			m_IsTemporary = v;
		}

		bool IsTemp() const {
			return m_IsTemporary;
		}
	};
}