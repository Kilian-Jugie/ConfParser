/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/
/*!
 * \file global.hpp
 * \brief Contain global definitions and constats which could be used anywhere
*/

#pragma once
#include <unordered_map>

#ifdef UNICODE
#define CP_CHAR_T wchar_t
#define CP_TEXT(x) L##x
#define cp_snprintf_s _snwprintf_s
#define cp_atoi _wtoi
#define cp_atof _wtof
#define cp_isalnum std::iswalnum
#define cp_isspace std::iswspace
#define cp_ispunct std::iswpunct
#define cp_tostring std::to_wstring
#else
#define CP_CHAR_T char
#define CP_TEXT(x) x
#define cp_snprintf_s snprintf_s
#define cp_atoi _atoi
#define cp_atof _atof
#define cp_isalnum std::isalnum
#define cp_isspace std::isspace
#define cp_ispunct std::ispunct
#define cp_tostring std::to_string
#endif // UNICODE

#define CP_SF(x) if(x) {delete x; x = nullptr;}

namespace confparser {
	class ConfParser;
	class ConfScope;
	class ConfScopeable;
	class ConfFunctionIntrinsic;
	class ConfInstance;
	class ConfType;

	using char_t = CP_CHAR_T;
	using string_t = std::basic_string<char_t>;
	using ifstream_t = std::basic_ifstream<char_t>;
	using osstream_t = std::basic_ostringstream<char_t>;

	using StringFormater_t = string_t(*)(string_t); //No reference for C# easy compatibility !

	using ApplySpecialFunction_t = void(*)(ConfParser*, ConfScope*,
		const std::vector<string_t>&, StringFormater_t);
	using ApplyKeywordFunction_t = void(*)(ConfParser*, ConfScope**,
		const std::vector<string_t>&);

	constexpr char_t TOKEN_CHAR_COMMENT = CP_TEXT('#');
	constexpr char_t TOKEN_CHAR_ASSIGNATION_SEPARATOR = CP_TEXT('=');
	constexpr char_t TOKEN_CHAR_SPECIAL = CP_TEXT('%');
	constexpr char_t TOKEN_CHAR_STRING = CP_TEXT('"');
	constexpr char_t TOKEN_CHAR_SCOPE_BEGIN = CP_TEXT('{');
	constexpr char_t TOKEN_CHAR_SCOPE_END = CP_TEXT('}');
	constexpr char_t TOKEN_CHAR_DECIMAL = CP_TEXT('.');
	constexpr char_t TOKEN_CHAR_MEMBER = CP_TEXT('.');

	constexpr char_t TOKEN_STRING_SPECIAL_USE[] = CP_TEXT("use");
	constexpr char_t TOKEN_STRING_SPECIAL_DEFAULT[] = CP_TEXT("default");
	constexpr char_t TOKEN_STRING_SPECIAL_DEFINE[] = CP_TEXT("define");
	constexpr char_t TOKEN_STRING_SPECIAL_TYPE[] = CP_TEXT("type");
	constexpr char_t TOKEN_STRING_PREFIX_OPERATOR[] = CP_TEXT("operator");
	constexpr char_t TOKEN_STRING_PREFIX_FUNCTION[] = CP_TEXT("function");

	constexpr char_t TOKENS_STRING_KEYWORD_CLASS[] = CP_TEXT("class");

	enum class CodeObjectType {
		TYPE, //! \see ConfType
		INSTANCE, //! \see ConfInstance
		RVALUE, //! Temporary value (\see ConfScopeable.IsTemp)
		FUNCTION, //! \see ConfFunctionExtrinsic
		SCOPE, //! \see ConfScope
		NONE //Used in filters or error detection
	};

	/*!
	 * \brief Remove any \r char
	 * \param str String where to remove chars
	*/
	void removeCariageReturn(string_t& str);

	/*!
	 * \brief Remove first and last char as they are quotes
	 * \param str The string to remove the chars from
	 * \todo uniformization
	*/
	void unStringify(string_t& str);

	/*!
	 * \brief Remove first and last char as they are quotes
	 * \param str The string to remove the chars from
	 * \return A copy of str without quotes
	 * \todo uniformization
	*/
	string_t unStringify(const string_t& str);

	/*!
	 * \brief Remove trailing and front spaces and tab
	 * \param str The string to remove from
	 * \todo isspace
	 * \todo one pass
	*/
	void trim(string_t& str);

	/*!
	 * \brief Split a string into string array following filters
	 * \param from String to split
	 * \param filters Filters to split
	 * \deprecated Will be replaced by filtersplit
	*/
	std::vector<string_t> advsplit(const string_t& from, const string_t& filters);

	/*!
	 * \brief Wrapper for multiple filter methods for filtersplit
	 * 
	 * \todo operators ?
	 * \todo condition fusion ?
	 */
	class FilterSplitFilter {
	public:
		/*!
		 * \brief Returns if we must keep the char or skip it when split
		*/
		using conditionfnc_t = bool(*)(char_t);

		FilterSplitFilter(conditionfnc_t cond) {
			m_FncList.push_back(cond);
		}

		FilterSplitFilter(char_t ch, bool keep = false) {
			m_CharMap[ch] = keep;
		}

		/*!
		 * \brief Contruct filters from char array
		 * \param str Contains each chars where we must split and with default keep condition
		 * \param keep Shall we keep chars or not in split
		*/
		FilterSplitFilter(const string_t& str, bool keep = false) {
			for (const auto& it : str) m_CharMap[it] = keep;
		}

		/*!
		 * \brief Construct filters from char array with custom keep conditions and default
		 * \param str Contains each chars where we must split
		 * \param keep Arrays of bool with index-equivalent keep condition for str. If outofbound, keepDefault used
		 * \param keepDefault Default keep condition if keep isn't defined for all str indexes
		*/
		FilterSplitFilter(const std::string& str, const std::vector<bool>& keep, bool keepDefault = false) {
			for (int i{ 0 }; i < str.size(); ++i) {
				m_CharMap[str[i]] = i < keep.size() ? keep[i] : keepDefault;
			}
		}

		/*!
		 * \brief Check for a char and returns if split and keep needed
		 * \param ch The char to check to
		 * \return A pair where first is shall we split and second is shall we keep the char
		*/
		std::pair<bool, bool> check(char_t ch) {
			for (const auto& it : m_FncList) {
				if (it(ch)) return { true,false };
			}
			return m_CharMap.find(ch) != m_CharMap.end() ? std::make_pair(true, m_CharMap[ch]) :
				std::make_pair(false, false);
		}

	private:
		std::unordered_map<char_t, bool> m_CharMap;
		std::vector<conditionfnc_t> m_FncList;
	};

	/*!
	 * \brief Split using filters
	 * \param in The string to split from
	 * \param condition The filters for split
	 * \param useStrings Strings litterals ('"' surrounded chars) aren't threated
	 * \param keepStringChar Should the '"' be keeped ?
	 * 
	 * Universal reference to allows perfect forward but we need const lvalue ref compatibility
	 * 
	 * \todo sfinae on _StrTy enable_if decay(in) is string_t constructible
	 * \
	*/
	template<typename _StrTy>
	std::vector<string_t> filtersplit(_StrTy&& in, FilterSplitFilter condition,
		bool useStrings = false, bool keepStringChar = false) {
		std::vector<string_t> ret{ {CP_TEXT("")} };
		bool isInString = false;
		const auto end{ in.cend() };
		for (auto it{ in.cbegin() }; it != end; ++it) {
			if (useStrings && *it == TOKEN_CHAR_STRING) {
				isInString ^= true;
				if (keepStringChar) ret[ret.size() - 1].push_back(*it);
			}
			else if (isInString) ret[ret.size() - 1].push_back(*it);
			else if (auto p = condition.check(*it); p.first) {
				if (!ret[ret.size() - 1].empty()) ret.push_back(CP_TEXT(""));
				if (p.second) {
					ret[ret.size() - 1] = string_t(&*it, 1);
					ret.push_back(CP_TEXT(""));
				}
			}
			else ret[ret.size() - 1].push_back(*it);
		}
		return ret;
	}
}