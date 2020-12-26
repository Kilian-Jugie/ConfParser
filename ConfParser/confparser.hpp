/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/
/*!
* \file confparser.hpp
* \brief Main header file, public interface
*/

#pragma once
#include <unordered_map>
#include <filesystem>
#include "global.hpp"

namespace confparser {
	/*!
	 * \brief Main class, public interface
	 * 
	 * A parser is an instance of the compiler which operate on
	 * a source file. It automatically parse other included files
	 * if necessary
	*/
	class ConfParser {
	private:
		bool m_IsInitialized;
		static std::unordered_map<string_t, ApplySpecialFunction_t> SpecialTokensMap;
		static std::unordered_map<string_t, ApplyKeywordFunction_t> KeywordsMap;
		static ConfScope* IntrinsicScope;
		static ConfScope* GetNewIntrinsicScope();
		static ConfScope* GlobalScope;

	public:
		/*!
		 * \brief Get the global's parent scope as singleton
		 * 
		*/
		static ConfScope* GetIntrinsicScope() {
			if (!IntrinsicScope)
				IntrinsicScope = GetNewIntrinsicScope();
			return IntrinsicScope;
		}

		/*!
		 * \brief Get the global scope as singleton
		 * \deprecated Global scope not to be singleton
		*/
		static ConfScope* GetGlobalScope();

		ConfParser() : m_IsInitialized{ false } {}
		~ConfParser();

		/*!
		 * \brief Parse a conf source file into a scope structure
		 * \param file The path to the source file
		 * \param format [NOT IMPLEMENTED, DEPRECATED] A static line pre-formater
		*/
		ConfScope* Parse(std::filesystem::path file, StringFormater_t format=nullptr);

		/*!
		 * \brief ConfParser initialization
		 * 
		 * This function will be called automatically if not be the user
		*/
		void Initialize();
	};
}