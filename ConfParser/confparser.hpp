/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/
/**			
* ==================== ConfParser V1.0 ====================
*		  "Conf" object oriented language interpreter
* 
* 
* Conf capabilities :
*	- Object oriented programming
*	- Intrinsic types as objects and not rvalues
*	- Types extension
*	- Type-specific operators creation and overloading
*	- Operators priority management with volatile priority rules
*	- Intrinsic lambda linking as language functions
*	- Scoped block-lifetime instances
*	- Scope inheritance including classes and files
*	- Basic preprocessor directives to manipulate interpreter
*	- Rvalues managment with anti-wast pattern
*	- Modulable & extensible default types
* 
* ConfParser specs & capabities :
*	- Modern C++17 with advanced moving semantic
*	- Fully modulable intrinsic architecture
*	- UNICODE compatible
*	- Safe-Free pattern
*	- Dependance free interface
*/

#pragma once
#include <unordered_map>
#include <filesystem>
#include "global.hpp"

namespace confparser {
	class ConfParser {
	private:
		bool m_IsInitialized;
		static std::unordered_map<string_t, ApplySpecialFunction_t> SpecialTokensMap;
		static std::unordered_map<string_t, ApplyKeywordFunction_t> KeywordsMap;
		static ConfScope* IntrinsicScope;
		static ConfScope* GetNewIntrinsicScope();
		static ConfScope* GlobalScope;

	public:
		static ConfScope* GetIntrinsicScope() {
			if (!IntrinsicScope)
				IntrinsicScope = GetNewIntrinsicScope();
			return IntrinsicScope;
		}

		static ConfScope* GetGlobalScope();

		ConfParser() : m_IsInitialized{ false } {}
		~ConfParser();

		ConfScope* Parse(std::filesystem::path file, StringFormater_t format=nullptr);
		void Initialize();
	};
}