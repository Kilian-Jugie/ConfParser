/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/

#include "confparser.hpp"
#include <fstream>
#include <sstream>
#include <array>
#include <cwctype>
#include <assert.h>
#include <filesystem>

namespace confparser {
	std::unordered_map<string_t, ApplySpecialFunction_t> ConfParser::SpecialTokensMap;
	std::unordered_map<string_t, ApplyKeywordFunction_t> ConfParser::KeywordsMap;
	ConfScope* ConfParser::IntrinsicScope = nullptr;
	ConfScope* ConfParser::GlobalScope = nullptr;

	ConfType* ConfScope::DefaultStringType = nullptr;
	ConfType* ConfScope::DefaultIntegerType = nullptr;
	ConfType* ConfScope::DefaultDecimalType = nullptr;
	ConfType* ConfScope::DefaultObjectType = nullptr;

	//TODO: uniformization ?
	void removeCariageReturn(string_t& str) {
		for (string_t::iterator it{ str.begin() }; it != str.end(); ++it) {
			if (*it == CP_TEXT('\r')) {
				str.erase(it);
			}
		}
	}

	//TODO: uniformization ?
	void unStringify(string_t& str) {
		str.erase(str.begin(), str.begin() + 1);
		str.erase(str.end() - 1);
	}

	//TODO: uniformization ?
	string_t unStringify(const string_t& str) {
		return str.substr(1, str.size() - 2);
	}

	std::vector<string_t> operatorSplitter(string_t expr) {
		static constexpr char_t TOKENS_CHARS_SUR_OPS[] = CP_TEXT("()[]");
		std::vector<string_t> ret;
		ret.push_back(CP_TEXT(""));
		//TODO: Rework
		bool hasAlnum = false, hasOp = false, isInString = false;
		for (const auto& ch : expr) {
			if (ch == TOKEN_CHAR_STRING) {
				if (hasAlnum || hasOp) {
					hasAlnum = hasOp = false;
					ret.push_back(CP_TEXT(""));
				}
				isInString ^= true;
				ret[ret.size() - 1] += ch;
			}
			else if (isInString) ret[ret.size() - 1] += ch;
			else if (cp_isalnum(ch) || ch == CP_TEXT('_')) {
				if (hasOp) {
					ret.push_back(CP_TEXT(""));
					hasOp = false;
				}
				ret[ret.size() - 1] += ch;
				hasAlnum = true;
				
			}
			else if (std::find(std::begin(TOKENS_CHARS_SUR_OPS), std::end(TOKENS_CHARS_SUR_OPS), ch)
				!= std::end(TOKENS_CHARS_SUR_OPS)) {
				//TODO: rework !
				if (!ret[ret.size() - 1].empty()) ret.push_back(string_t(1, ch));
				else ret[ret.size() - 1] = string_t(1, ch);
				ret.push_back(CP_TEXT(""));
				hasAlnum = false;
				hasOp = false;
			}
			else if (cp_ispunct(ch)) {
				if (hasAlnum) {
					ret.push_back(CP_TEXT(""));
					hasAlnum = false;
				}
				ret[ret.size() - 1] += ch;
				hasOp = true;
			}
			else if (!ret[ret.size() - 1].empty()) {
				hasOp = false;
				hasAlnum = false;
				ret.push_back(CP_TEXT(""));
			}
		}
		return ret;
	}

	class OperatorParserOperation {
		ConfFunctionExtrinsicOperator* m_Operator;
		std::vector<int> m_OperandIndexes;
		int m_InstanceIndex;
	public:
		OperatorParserOperation(ConfFunctionExtrinsicOperator* op, int inst, std::vector<int> operands):
			m_Operator{ op }, m_InstanceIndex{ inst }, m_OperandIndexes{ std::move(operands) } {}
	};

	void threatOp(std::vector< ConfScopeable*>& line,
		ConfFunctionExtrinsicOperator* op, std::size_t opIndex) {
		switch (op->GetOpType()) {
		case ConfOperatorType::MID: {
			ConfScopeable* f, *s;
			f = line[opIndex - 1];
			s = line[opIndex + 1];
			line.erase(line.begin() + (opIndex - 1), line.begin() + (opIndex + 2));
			line.insert(line.begin() + (opIndex - 1), op->Call((ConfInstance*)f, {(ConfInstance*)s}) );
			if (f->IsTemp()) CP_SF(f);
			if (s->IsTemp()) CP_SF(s);
		}break;
			//TODO [F:implement on others parsers !]
		case ConfOperatorType::POST:
			__fallthrough;
			//TODO [F:implement on others parsers !]
		case ConfOperatorType::PRE:
			__fallthrough;
			//TODO [F:implement on others parsers !]
		case ConfOperatorType::SUR: break;
		}
	}

	//TODO: PRE, SUB, MID support
	ConfInstance* operatorParser(ConfScope* scope,
		std::unordered_map<int, std::vector<std::vector<string_t>>>& parenthetized, int depth=0, int offset=0) {
		std::vector< ConfScopeable*> currentLine = { };
		for (auto& token : parenthetized[depth][offset]) {
			if (token.empty()) continue;
			if (token[0] == '$') {
				currentLine.push_back(
					operatorParser(scope, parenthetized, depth + 1, token[1] - CP_TEXT('0')) );
			}
			else {
				ConfScopeable* inst = scope->GetByName(token, CodeObjectType::INSTANCE);
				
				if (!inst) {
					inst = scope->InstanceFromRValue(token);
				}
				if (!inst && currentLine.size() > 0 && currentLine[currentLine.size()-1]->GetCodeObjectType()
					!= CodeObjectType::FUNCTION) {
					ConfInstance* _this = (ConfInstance*)currentLine[currentLine.size() - 1];
					ConfFunctionExtrinsicOperator* op = reinterpret_cast<ConfFunctionExtrinsicOperator*>(
						_this->GetFunction(TOKEN_STRING_PREFIX_OPERATOR + token));
					inst = reinterpret_cast<ConfScopeable*>(op);
				}
				if(!inst) {
					//Deprecated ?
					//non code object value ! marked as rvalue because temporary
					inst = new ConfInstance(nullptr, token);
					inst->SetTemp(true);
				}
				currentLine.push_back(inst);
				if (currentLine.size() > 2 && currentLine[currentLine.size() - 2]->GetCodeObjectType()
					== CodeObjectType::FUNCTION) { //TODO: PRE, SUB, MID support
					//We must inline threat priority 1 operators because they can change the _left type
					//Is this optimization ?
					ConfFunctionExtrinsicOperator* op = (ConfFunctionExtrinsicOperator*)
						currentLine[currentLine.size() - 2];
					if (op->GetPriority() == 1) {
						threatOp(currentLine, op, currentLine.size() - 2);
					}
					
				}
			}
		}
		
		while (currentLine.size() > 1) {
			ConfFunctionExtrinsicOperator* currentOp = nullptr;
			std::size_t opIndex = 0, i = 0;
			for (auto& it : currentLine) {
				if (it->GetCodeObjectType() == CodeObjectType::FUNCTION && (!currentOp ||
					currentOp->GetPriority() > ((ConfFunctionExtrinsicOperator*)it)->GetPriority())) {
					currentOp = (ConfFunctionExtrinsicOperator*)it;
					opIndex = i;
				}
				++i;
			}

			threatOp(currentLine, currentOp, opIndex);
		}

		return (ConfInstance*)(currentLine[0]);
	}

	//TODO: non joker string version ?
	std::unordered_map<int, std::vector<std::vector<string_t>>> parenthesisOperatorParser(
		const std::vector<string_t>& expression) {
		std::unordered_map<int, std::vector<std::vector<string_t>>> parenthetized;
		parenthetized[0].push_back({ });
		int depth = 0;
		for (string_t e : expression) {
			if (e == L"(") parenthetized[++depth].push_back({ });
			else if (e == L")") {
				--depth;
				parenthetized[depth][parenthetized[depth].size() - 1].push_back(
					(L"$" + std::to_wstring(parenthetized[depth+1].size()-1)));
			}
			else parenthetized[depth][parenthetized[depth].size()-1].push_back(e);
		}
		return parenthetized;
	}

	std::vector<string_t> advsplit(const string_t& from, const string_t& filters) {
		static constexpr wchar_t includeChar = L'^', skipChar = L'$';
		bool exclude = false;
		std::vector<string_t> ret;
		ret.push_back({});
		const string_t::const_iterator fend{ filters.end() };
		for (string_t::const_iterator it{ from.begin() }; it != from.end(); ++it) {
			for (string_t::const_iterator fit{ filters.begin() }; fit != fend; ++fit) {
				if (*fit == includeChar || *fit == skipChar) continue;
				if (*it == *fit) {
					//TODO: Rework
					if ((fit != filters.begin() && *(fit - 1) == skipChar) || !ret[ret.size() - 1].empty())
						ret.push_back({});
					if (fit != filters.begin() && *(fit - 1) == includeChar) {
						ret[ret.size() - 1] = string_t(&(*fit), 1);
						ret.push_back({});
					}
					exclude = true;
				}
			}
			if (!exclude) ret[ret.size() - 1] += *it;
			else exclude = false;
		}
		return ret;
	}

	//TODO: operators ?
	//TODO: condition fusion ?
	class FilterSplitFilter {
	public:
		using conditionfnc_t = bool(*)(char_t);

		FilterSplitFilter(conditionfnc_t cond) {
			m_FncList.push_back(cond);
		}

		FilterSplitFilter(char_t ch, bool keep=false) {
			m_CharMap[ch] = keep;
		}

		FilterSplitFilter(const string_t& str, bool keep = false) {
			for (const auto& it : str) m_CharMap[it] = keep;
		}

		FilterSplitFilter(const std::string &str, const std::vector<bool>& keep, bool keepDefault=false) {
			for (int i{ 0 }; i < str.size(); ++i) {
				m_CharMap[str[i]] = i < keep.size() ? keep[i] : keepDefault;
			}
		}

		std::pair<bool,bool> check(char_t ch) {
			for(const auto& it : m_FncList) {
				if (it(ch)) return { true,false };
			}
			return m_CharMap.find(ch) != m_CharMap.end() ? std::make_pair(true, m_CharMap[ch] ) :
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

	void trim(string_t& str) {
		//TODO: isspace
		//TODO: One pass ?
		while (str.size() > 0 && (str[0] == ' ' || str[0] == '\t')) str.erase(str.begin());
		while (str.size() > 0 && (str[str.size()-1] == ' ' || str[str.size() - 1] == '\t')) str.erase(str.end()-1);
	}

	void ConfParser::Initialize() {
		SpecialTokensMap[TOKEN_STRING_SPECIAL_DEFINE] = [](ConfParser* _this, ConfScope* scope,
			const std::vector<string_t>& tokens, StringFormater_t formater) {
				//TODO
		};
		SpecialTokensMap[TOKEN_STRING_SPECIAL_USE] = [](ConfParser* _this, ConfScope* scope,
			const std::vector<string_t>& tokens, StringFormater_t formater) {
				//TODO
				SpecialTokensMap[TOKEN_STRING_SPECIAL_DEFAULT](_this, scope, tokens, formater);
		};
		SpecialTokensMap[TOKEN_STRING_SPECIAL_DEFAULT] = [](ConfParser* _this, ConfScope* scope,
			const std::vector<string_t>& tokens, StringFormater_t formater) {
				auto oscope = _this->Parse(unStringify(tokens[2]));
		};
		SpecialTokensMap[TOKEN_STRING_SPECIAL_TYPE] = [](ConfParser* _this, ConfScope* scope,
			const std::vector<string_t>& tokens, StringFormater_t formater) {
		};
		SpecialTokensMap[TOKEN_STRING_PREFIX_FUNCTION] = [](ConfParser* _this, ConfScope* scope,
			const std::vector<string_t>& tokens, StringFormater_t formater) {

		};

		KeywordsMap[TOKENS_STRING_KEYWORD_CLASS] = [](ConfParser* _this, ConfScope** currentScope,
			const std::vector<string_t>& tokens) {
				ConfType* ty = new ConfType(tokens[1], *currentScope);
				*ty += *ConfScope::GetDefaultObjectType();
				(*currentScope)->AddChild(ty);
				*currentScope = ty;
		};
	}

	ConfScope* ConfParser::Parse(std::filesystem::path file, StringFormater_t format) {
		if (!m_IsInitialized) Initialize();
		ConfScope* ret = GetGlobalScope();
		
		ConfScope* currentScope = ret;

		ifstream_t ifs{ file };
		osstream_t sstream;
		sstream << ifs.rdbuf();
		string_t rawText{ sstream.str() };
		ifs.close();

		removeCariageReturn(rawText);
		auto stext = filtersplit(std::move(rawText), { '\n',false });

		for (auto& it : stext) {
			trim(it);
			if (it.empty()) continue;
			string_t text = format ? format(it) : std::move(it); //No overhead haha
			std::vector<string_t> tokenizedText = filtersplit(text,
				{ " =#%+-*/.", {false}, true}, true, true);
			switch (text[0]) {
			case TOKEN_CHAR_COMMENT: break;
			case TOKEN_CHAR_SPECIAL: {
				SpecialTokensMap[tokenizedText[1]](this, currentScope, tokenizedText, format);
			}break;
				//TODO
			case TOKEN_CHAR_SCOPE_BEGIN: break;
			case TOKEN_CHAR_SCOPE_END: 
				currentScope = currentScope->GetParent();
				break;
			default: {
				if (currentScope->TokenCallback(tokenizedText)) continue;

				if (KeywordsMap.find(tokenizedText[0]) != KeywordsMap.end()) {
					KeywordsMap[tokenizedText[0]](this, &currentScope, tokenizedText);
					continue;
				}

				ConfScopeable* firstToken = currentScope->GetByName(tokenizedText[0]);
				if (!firstToken) {
					//Unresolved symbol
					assert(false);
				}

				if (firstToken->GetCodeObjectType() == CodeObjectType::TYPE) {
					ConfType* type = static_cast<ConfType*>(firstToken);
					ConfInstance* inst = type->CreateInstance(tokenizedText[1]);
					currentScope->AddChild(inst);
					text = text.substr(text.find(' ')+1);
				}

				auto splitted = parenthesisOperatorParser(operatorSplitter(text));
				ConfInstance* r = operatorParser(currentScope, splitted);
				if (r->IsTemp()) CP_SF(r);
			}break;
			}
		}
		return ret;
	}

	ConfScopeable* ConfInstance::Clone(string_t name, ConfScopeable* buf) const {
		ConfInstance* ret = m_Type->CreateInstance(std::move(name));
		ret->ClearSubInstances();
		for (auto s : m_SubInstances) 
			ret->AddSubInstance(static_cast<ConfInstance*>(s->Clone(s->GetName())));
		return ret;
	}

	ConfInstance* ConfType::_CreateInstance(ConfType* type, string_t name) {
		ConfInstance* inst = new ConfInstance(type, std::move(name));
		for (auto c : type->GetChilds()) {
			if (c->GetCodeObjectType() == CodeObjectType::INSTANCE) {
				ConfInstance* subInst = static_cast<ConfInstance*>(c);
				inst->AddSubInstance(subInst->GetType()->CreateInstance(c->GetName()));
			}
		}
		return inst;
	}

	ConfScopeable* ConfType::Clone(string_t name, ConfScopeable* buf) const {
		if (!buf) buf = new ConfType(name);
		static_cast<ConfType*>(buf)->CreateInstanceCallback = CreateInstanceCallback;
		return buf;
	}

	ConfInstance* ConfTypeString::_CreateStringInstance(ConfType* type, string_t name) {
		return new ConfInstanceString(type, std::move(name));
	}

	ConfScopeable* ConfInstanceString::Clone(string_t name, ConfScopeable* buf) const {
		auto r = static_cast<ConfInstanceString*>(
			ConfInstance::Clone(std::move(name), buf));
		r->Set(Get());
		return r;
	}

	ConfInstance* ConfTypeInt::_CreateIntInstance(ConfType* type,  string_t name) {
		return new ConfInstanceInt(type, std::move(name));
	}

	ConfScopeable* ConfInstanceInt::Clone(string_t name, ConfScopeable* buf) const {
		auto r = static_cast<ConfInstanceInt*>(
			ConfInstance::Clone(std::move(name), buf));
		r->Set(Get());
		return r;
	}

	ConfInstance* ConfTypeFloat::_CreateFloatInstance(ConfType* type,  string_t name) {
		return new ConfInstanceFloat(type, std::move(name));
	}

	ConfScopeable* ConfInstanceFloat::Clone(string_t name, ConfScopeable* buf) const {
		auto r = static_cast<ConfInstanceFloat*>(
			ConfInstance::Clone(std::move(name), buf));
		r->Set(Get());
		return r;
	}

	ConfScope::~ConfScope() {
		for (ConfScopeable* it : m_Childs) {
			if (_ADDRESSOF(*it) == _ADDRESSOF(*ConfParser::GetIntrinsicScope())) continue;
			CP_SF(it);
		}
	}

	ConfScopeable* confparser::ConfScope::GetByName(const string_t& name, CodeObjectType filter) const {
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
		if(rvalue.find(TOKEN_CHAR_DECIMAL) != string_t::npos)
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
			static_cast<ConfInstanceString*>(ret)->Set(rvalue.substr(1, rvalue.size()-2));
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

		for (auto oc : scope.m_Childs ) {
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

	ConfScopeable* ConfFunctionIntrinsic::Clone(string_t name, ConfScopeable* buf) const {
		if (!buf) buf = new ConfFunctionIntrinsic(nullptr, name, m_Callback);
		return buf;
	}

	ConfScopeable* ConfFunctionIntrinsicOperator::Clone(string_t name, ConfScopeable* buf) const {
		if (!buf) buf = new ConfFunctionIntrinsicOperator(nullptr, name, m_Callback, m_Priority);
		static_cast<ConfFunctionIntrinsicOperator*>(buf)->m_OpType = m_OpType;
		return buf;
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

	ConfScope* ConfParser::GetNewIntrinsicScope() {
		ConfScope* ret = new ConfScope();
		ConfType* tyStr = new ConfTypeString(TOKEN_STRING_TYPE_STRING);
		auto tyStrSet = new ConfFunctionIntrinsicOperator(tyStr, CP_TEXT("operator="),
			[](ConfInstance* _this, std::vector<ConfInstance*> parmeters) {
				ConfInstanceString* strThis = static_cast<ConfInstanceString*>(_this);
				strThis->Set(static_cast<ConfInstanceString*>(parmeters[0])->Get());
				return _this;
			}, 14
		);
		tyStr->AddChild(tyStrSet);
		ret->AddChild(tyStr);
		ret->SetDefaultStringType(tyStr);

		ConfType* tyInt = new ConfTypeInt(TOKEN_STRING_TYPE_INT);
		auto tyIntSet = new ConfFunctionIntrinsicOperator(tyInt, CP_TEXT("operator="),
			[](ConfInstance* _this, std::vector<ConfInstance*> parameters) {
				ConfInstanceInt* strThis = static_cast<ConfInstanceInt*>(_this);
				strThis->Set(static_cast<ConfInstanceInt*>(parameters[0])->Get());
				return _this;
			}, 14
		);

		auto tyIntAdd = new ConfFunctionIntrinsicOperator(tyInt, CP_TEXT("operator+"),
			[](ConfInstance* _this, std::vector<ConfInstance*> parameters) {
				ConfInstanceInt* intThis = static_cast<ConfInstanceInt*>(_this);
				ConfInstanceInt* ret = static_cast<ConfInstanceInt*>(
					intThis->Clone(CP_TEXT("__RV")));
				ret->Set(intThis->Get() + static_cast<ConfInstanceInt*>(parameters[0])->Get());
				ret->SetTemp(true);
				return ret;
			}, 4
		);

		tyInt->AddChild(tyIntAdd);

		auto tyIntMult = new ConfFunctionIntrinsicOperator(tyInt, CP_TEXT("operator*"),
			[](ConfInstance* _this, std::vector<ConfInstance*> parameters) {
				ConfInstanceInt* intThis = static_cast<ConfInstanceInt*>(_this);
				ConfInstanceInt* ret = static_cast<ConfInstanceInt*>(
					intThis->Clone(CP_TEXT("__RV")));
				ret->Set(intThis->Get() * static_cast<ConfInstanceInt*>(parameters[0])->Get());
				ret->SetTemp(true);
				return ret;
			}, 3
		);

		tyInt->AddChild(tyIntMult);

		auto tyIntAddSet = new ConfFunctionIntrinsicOperator(tyInt, CP_TEXT("operator+="),
			[](ConfInstance* _this, std::vector<ConfInstance*> parameters) {
				ConfInstanceInt* strThis = static_cast<ConfInstanceInt*>(_this);
				strThis->Set(strThis->Get() + static_cast<ConfInstanceInt*>(parameters[0])->Get());
				return _this;
			}, 14
		);
		tyInt->AddChild(tyIntSet);
		tyInt->AddChild(tyIntAddSet);
		ret->AddChild(tyInt);
		ret->SetDefaultIntegerType(tyInt);

		ConfType* tyFloat = new ConfTypeFloat(TOKEN_STRING_TYPE_FLOAT);
		auto tyFloSet = new ConfFunctionIntrinsicOperator(tyStr, CP_TEXT("operator="),
			[](ConfInstance* _this, std::vector<ConfInstance*> parameters) {
				ConfInstanceFloat* strThis = static_cast<ConfInstanceFloat*>(_this);
				strThis->Set(static_cast<ConfInstanceFloat*>(parameters[0])->Get());
				return _this;
			}, 14
		);
		tyFloat->AddChild(tyFloSet);
		ret->AddChild(tyFloat);
		ret->SetDefaultDecimalType(tyFloat);

		ConfType* tyObject = new ConfTypeFloat(CP_TEXT("object"));
		auto tyObjDot = new ConfFunctionIntrinsicOperator(tyStr, CP_TEXT("operator."),
			[](ConfInstance* _this, std::vector<ConfInstance*> parameters) {
				for (auto c : _this->GetSubInstances())
					if (c->GetName() == parameters[0]->GetName()) return c;
				return static_cast<ConfInstance*>(nullptr);
			}, 1
		);

		auto tyObjEqu = new ConfFunctionIntrinsicOperator(tyStr, CP_TEXT("operator="),
			[](ConfInstance* _this, std::vector<ConfInstance*> parameters) {
				_this->ClearSubInstances();
				for (auto c : parameters[0]->GetSubInstances())
					_this->AddSubInstance(static_cast<ConfInstance*>(c->Clone(c->GetName())));
				return _this;
			}, 14
		);
		
		tyObject->AddChild(tyObjDot);
		tyObject->AddChild(tyObjEqu);
		ret->AddChild(tyObject);
		ret->SetDefaultObjectType(tyObject);

		return ret;
	}
}


