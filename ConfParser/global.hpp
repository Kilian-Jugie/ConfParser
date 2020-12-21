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

	constexpr char_t TOKEN_STRING_TYPE_STRING[] = CP_TEXT("string");
	constexpr char_t TOKEN_STRING_TYPE_INT[] = CP_TEXT("int");
	constexpr char_t TOKEN_STRING_TYPE_FLOAT[] = CP_TEXT("float");

	enum class CodeObjectType {
		TYPE,
		INSTANCE,
		RVALUE,
		FUNCTION,
		SCOPE,
		NONE
	};

	void removeCariageReturn(string_t& str);
	void unStringify(string_t& str);
	string_t unStringify(const string_t& str);
	void trim(string_t& str);
	std::vector<string_t> advsplit(const string_t& from, const string_t& filters);

	//TODO: operators ?
	//TODO: condition fusion ?
	class FilterSplitFilter {
	public:
		using conditionfnc_t = bool(*)(char_t);

		FilterSplitFilter(conditionfnc_t cond) {
			m_FncList.push_back(cond);
		}

		FilterSplitFilter(char_t ch, bool keep = false) {
			m_CharMap[ch] = keep;
		}

		FilterSplitFilter(const string_t& str, bool keep = false) {
			for (const auto& it : str) m_CharMap[it] = keep;
		}

		FilterSplitFilter(const std::string& str, const std::vector<bool>& keep, bool keepDefault = false) {
			for (int i{ 0 }; i < str.size(); ++i) {
				m_CharMap[str[i]] = i < keep.size() ? keep[i] : keepDefault;
			}
		}

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

	//Here we need universal reference because some values are passed as rvalue reference & some as const
	//lvalue references
	//TODO: SFINAE !
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