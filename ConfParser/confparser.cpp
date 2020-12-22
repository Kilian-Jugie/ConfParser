/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/

#include "global.hpp"
#include "confparser.hpp"
#include "confscope.hpp"
#include "conftype.hpp"
#include "confoperator.hpp"
#include "confscopeable.hpp"
#include "confinstance.hpp"
#include <fstream>
#include <sstream>
#include <cwctype>

#include <cassert>

namespace confparser {
	std::unordered_map<string_t, ApplySpecialFunction_t> ConfParser::SpecialTokensMap;
	std::unordered_map<string_t, ApplyKeywordFunction_t> ConfParser::KeywordsMap;
	ConfScope* ConfParser::IntrinsicScope = nullptr;
	ConfScope* ConfParser::GlobalScope = nullptr;

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

	void trim(string_t& str) {
		//TODO: isspace
		//TODO: One pass ?
		while (str.size() > 0 && (str[0] == ' ' || str[0] == '\t')) str.erase(str.begin());
		while (str.size() > 0 && (str[str.size()-1] == ' ' || str[str.size() - 1] == '\t')) str.erase(str.end()-1);
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

	void threatOp(std::vector< ConfScopeable*>& line,
		ConfFunctionExtrinsicOperator* op, std::size_t opIndex) {
		switch (op->GetOpType()) {
		case ConfOperatorType::MID: {
			ConfScopeable* f, * s;
			f = line[opIndex - 1];
			s = line[opIndex + 1];
			line.erase(line.begin() + (opIndex - 1), line.begin() + (opIndex + 2));
			line.insert(line.begin() + (opIndex - 1), op->Call((ConfInstance*)f, { (ConfInstance*)s }));
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
		std::unordered_map<int, std::vector<std::vector<string_t>>>& parenthetized, int depth = 0, int offset = 0) {
		std::vector< ConfScopeable*> currentLine = { };
		for (auto& token : parenthetized[depth][offset]) {
			if (token.empty()) continue;
			if (token[0] == '$') {
				currentLine.push_back(
					operatorParser(scope, parenthetized, depth + 1, token[1] - CP_TEXT('0')));
			}
			else {
				ConfScopeable* inst = scope->GetByName(token, CodeObjectType::INSTANCE);

				if (!inst) {
					auto ty = ConfTypeIntrinsic::TypeFromExpression(token, nullptr);
					if (ty && ty->GetName() != NAME_TYPE_EXPR) {
						inst = ty->CreateInstance(token);
						static_cast<ConfIntrinsicInstance*>(inst)->SetFromString(token);
					}
				}
				if (!inst && currentLine.size() > 0 && currentLine[currentLine.size() - 1]->GetCodeObjectType()
					!= CodeObjectType::FUNCTION) {
					ConfInstance* _this = (ConfInstance*)currentLine[currentLine.size() - 1];
					ConfFunctionExtrinsicOperator* op = reinterpret_cast<ConfFunctionExtrinsicOperator*>(
						_this->GetFunction(TOKEN_STRING_PREFIX_OPERATOR + token));
					inst = reinterpret_cast<ConfScopeable*>(op);
				}
				if (!inst) {
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
					(L"$" + cp_tostring(parenthetized[depth + 1].size() - 1)));
			}
			else parenthetized[depth][parenthetized[depth].size() - 1].push_back(e);
		}
		return parenthetized;
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

	ConfScope* ConfParser::GetGlobalScope() {
		if (!GlobalScope) GlobalScope = new ConfScope(GetIntrinsicScope());
		return GlobalScope;
	}

	ConfParser::~ConfParser() {
		if (IntrinsicScope) CP_SF(IntrinsicScope);
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

	ConfScope* ConfParser::GetNewIntrinsicScope() {
		ConfScope* ret = new ConfScope();
		ConfType* tyStr = new ConfTypeString();
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

		ConfType* tyInt = new ConfTypeInt();
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

		ConfType* tyFloat = new ConfTypeFloat();
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

		ConfType* tyObject = new ConfTypeObject();
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


