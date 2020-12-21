#include "confscope.hpp"
#include "confparser.hpp"
#include "confinstance.hpp"
#include <string>

namespace confparser {
	ConfType* ConfScope::DefaultStringType = nullptr;
	ConfType* ConfScope::DefaultIntegerType = nullptr;
	ConfType* ConfScope::DefaultDecimalType = nullptr;
	ConfType* ConfScope::DefaultObjectType = nullptr;

	ConfScope::~ConfScope() {
		for (ConfScopeable* it : m_Childs) {
			if (_ADDRESSOF(*it) == _ADDRESSOF(*ConfParser::GetIntrinsicScope())) continue;
			CP_SF(it);
		}
	}

	ConfScopeable* ConfScope::GetByName(const string_t& name, CodeObjectType filter) const {
		for (const auto& c : m_Childs) {
			if ((filter != CodeObjectType::NONE ? c->GetCodeObjectType() == filter : true)
				&& c->GetName() == name)
				return c;
		}
		if (m_Parent) return m_Parent->GetByName(name);
		return nullptr;
	}

	void ConfScope::AddChild(ConfScopeable* child) {
		m_Childs.push_back(child);
	}

	void ConfScope::SetDefaultStringType(ConfType* ty) {
		DefaultStringType = ty;
	}

	void ConfScope::SetDefaultIntegerType(ConfType* ty) {
		DefaultIntegerType = ty;
	}

	void ConfScope::SetDefaultDecimalType(ConfType* ty) {
		DefaultDecimalType = ty;
	}

	void ConfScope::SetDefaultObjectType(ConfType* ty) {
		DefaultObjectType = ty;
	}

	ConfType* ConfScope::TypeFromRValue(string_t rvalue) {
		if (rvalue[0] == TOKEN_CHAR_STRING && rvalue[rvalue.size() - 1] == TOKEN_CHAR_STRING) {
			return DefaultStringType;
		}
		try {
			volatile double v = std::stof(rvalue);
		}
		catch (...) {
			return nullptr;
		}
		if (rvalue.find(TOKEN_CHAR_DECIMAL) != string_t::npos)
			return DefaultDecimalType;
		return DefaultIntegerType;
	}

	ConfInstance* ConfScope::InstanceFromRValue(string_t rvalue) {
		static std::size_t VCOUNT = 1;
		ConfType* ty = TypeFromRValue(rvalue);
		if (!ty) return nullptr;
		ConfInstance* ret = nullptr;

		char_t* fbuf = new char_t[32];
		cp_snprintf_s(fbuf, 32, 32, CP_TEXT("__RSTRTMP_%zu"), VCOUNT++);

		if (ty == DefaultStringType) {
			ret = new ConfInstanceString(ty, string_t(fbuf));
			static_cast<ConfInstanceString*>(ret)->Set(rvalue.substr(1, rvalue.size() - 2));
		}
		else if (ty == DefaultIntegerType) {
			ret = new ConfInstanceInt(ty, string_t(fbuf));
			static_cast<ConfInstanceInt*>(ret)->Set(cp_atoi(rvalue.c_str()));
		}
		else if (ty == DefaultDecimalType) {
			ret = new ConfInstanceFloat(ty, string_t(fbuf));
			static_cast<ConfInstanceFloat*>(ret)->Set(cp_atof(rvalue.c_str()));
		}
		delete[] fbuf;
		ret->SetTemp(true);
		return ret;
	}

	ConfScope& ConfScope::operator+=(const ConfScope& scope) {

		for (auto oc : scope.m_Childs) {
			auto c = GetByName(oc->GetName());

			if (c) {
				switch (c->GetCodeObjectType()) {
				case CodeObjectType::INSTANCE:
					*static_cast<ConfInstance*>(c) = *static_cast<ConfInstance*>(oc);
					break;
				case CodeObjectType::TYPE:
					__fallthrough;
				case CodeObjectType::SCOPE:
					*static_cast<ConfScope*>(c) += *static_cast<ConfScope*>(oc);
					break;
				default:
					break;
				}
			}
			else {
				this->AddChild(oc->Clone(oc->GetName(), nullptr));
			}

		}
		return *this;
	}


	ConfScopeable* ConfScope::Clone(string_t name, ConfScopeable* buf) const {
		if (!buf) buf = new ConfScope();
		ConfScope* ret = static_cast<ConfScope*>(buf);
		ret->m_Name = name;
		ret->m_Parent = m_Parent;
		ret->DefaultDecimalType = DefaultDecimalType;
		ret->DefaultIntegerType = DefaultIntegerType;
		ret->DefaultStringType = DefaultStringType;
		for (auto c : m_Childs) {
			ret->AddChild(c->Clone(c->GetName(), nullptr));
		}
		return ret;
	}
}