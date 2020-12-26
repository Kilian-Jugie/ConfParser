/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/
/*!
 *	\file confscope.hpp
 *	\brief Scopes related definitions
*/

#pragma once
#include "global.hpp"
#include "confscopeable.hpp"

namespace confparser {
	/*!
	 * \brief Represent an in-code scope
	 * 
	 * A scope is any structure which is capable to contain sub objects
	 * and execute code. A class, a function, a block is a scope and the
	 * sub-variables declarations are threated as same. The inheritance applies
	 * on scopes thats mean that technically, function can inherite classes and
	 * vice versa and more generally, any scope can inherite any scope
	*/
	class ConfScope : public ConfScopeable {
	public:
		ConfScope(ConfScope* parent = nullptr) : m_Parent{ parent } {}
		~ConfScope();

		virtual CodeObjectType GetCodeObjectType() const override {
			return CodeObjectType::SCOPE;
		}

		/*!
		 * \brief Get the scope parent
		 * 
		 * The parent is the scope where 'this' is declared. It allows 'this' to
		 * interact with others objects declared in the same (or upper) scopes as
		 * him
		*/
		ConfScope* GetParent() const {
			return m_Parent;
		}

		/*!
		 * \brief Get the full childs list
		 * 
		 * A child is a sub-object declared in the current scope. child.GetParent() == this
		*/
		const std::vector<ConfScopeable*>& GetChilds() {
			return m_Childs;
		}

		void AddChild(ConfScopeable* child);

		/*!
		 * \brief Return a child or upper child by its name
		 * \param name The name of the child to retrieve
		 * \param filter An optional filter to retrieve a specific CodeObjectType child
		*/
		ConfScopeable* GetByName(const string_t& name, CodeObjectType filter = CodeObjectType::NONE) const;

		/*!
		 * \brief Fusion 2 scopes by overriding left by right
		 * 
		 * Can be used to make basic inheritance like with file inclusion.
		 * \param scope The other scope to fusion with
		*/
		ConfScope& operator+=(const ConfScope& scope);

		virtual ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;

	private:
		ConfScope* m_Parent;
		std::vector<ConfScopeable*> m_Childs;
	};
}