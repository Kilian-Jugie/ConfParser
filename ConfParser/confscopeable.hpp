/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/

#pragma once
#include "global.hpp"

namespace confparser {
	class ConfScopeable {
	protected:
		string_t m_Name;
		bool m_IsTemporary;
	public:
		ConfScopeable() = default;

		virtual string_t GetName() const { return m_Name; }
		virtual CodeObjectType GetCodeObjectType() const = 0;
		virtual ConfScopeable* Clone(string_t, ConfScopeable*) const {
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