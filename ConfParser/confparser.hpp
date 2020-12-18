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
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <functional>

#ifdef UNICODE
#define CP_CHAR_T wchar_t
#define CP_TEXT(x) L##x
#define cp_snprintf_s _snwprintf_s
#define cp_atoi _wtoi
#define cp_atof _wtof
#define cp_isalnum std::iswalnum
#define cp_isspace std::iswspace
#define cp_ispunct std::iswpunct
#else
#define CP_CHAR_T char
#define CP_TEXT(x) x
#define cp_snprintf_s snprintf_s
#define cp_atoi _atoi
#define cp_atof _atof
#define cp_isalnum std::isalnum
#define cp_isspace std::isspace
#define cp_ispunct std::ispunct
#endif // UNICODE

#define CP_SF(x) if(x) {delete x; x = nullptr;}

namespace confparser {
	using char_t = CP_CHAR_T;
	using string_t = std::basic_string<char_t>;
	using ifstream_t = std::basic_ifstream<char_t>;
	using osstream_t = std::basic_ostringstream<char_t>;

	using StringFormater_t = string_t(*)(string_t); //No reference for C# easy compatibility !

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

	class ConfScopeable;
	class ConfScope;
	class ConfType;
	class ConfInstance;
	class ConfParser;
	class ConfFunctionExtrinsic;
	class ConfFunctionOperator;

	using ApplySpecialFunction_t = void(*)(ConfParser*, ConfScope*,
		const std::vector<string_t>&, StringFormater_t);
	using ApplyKeywordFunction_t = void(*)(ConfParser*, ConfScope**,
		const std::vector<string_t>&);

	enum class CodeObjectType {
		TYPE,
		INSTANCE,
		RVALUE,
		FUNCTION,
		SCOPE,
		NONE
	};


	class ConfMemoryHandle {
		void* m_Data;
		std::size_t m_Size;
	public:
		ConfMemoryHandle(std::size_t size) : m_Size{ size } {
			m_Data = malloc(size);
		}

		ConfMemoryHandle() : ConfMemoryHandle(0) {

		}

		const void* operator->() const {
			return m_Data;
		}

		template<typename _Ty>const _Ty& operator[](std::size_t n) const {
			//if(n*sizeof(_Ty) > m_Size) //OutOfBound
			return static_cast<_Ty*>(m_Data)[n];
		}

		std::size_t Size() const {
			return m_Size;
		}

		void Reallocate(std::size_t newSize) {
			realloc(m_Data, newSize);
		}
	};

	class ConfMemoryManager {
		std::unordered_map<std::size_t, ConfMemoryHandle> m_Memory;
		std::size_t m_CurrentIdentifier;

		ConfMemoryManager() = default;
	public:
		static ConfMemoryManager& Instance() {
			static ConfMemoryManager instance;
			return instance;
		}

		ConfMemoryHandle& Get(std::size_t identifier) {
			return m_Memory[identifier];
		}

		std::size_t Create(std::size_t size) {
			m_Memory[++m_CurrentIdentifier] = { size };
			return m_CurrentIdentifier;
		}
	};

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

	class ConfScope : public ConfScopeable {
	public:
		using tokcallback_t = bool(*)(const std::vector<string_t>&);

		ConfScope(ConfScope* parent = nullptr) : m_Parent{ parent} {}
		~ConfScope();

		virtual CodeObjectType GetCodeObjectType() const override {
			return CodeObjectType::SCOPE;
		}

		ConfScope* GetParent() const {
			return m_Parent;
		}

		const std::vector<ConfScopeable*>& GetChilds() {
			return m_Childs;
		}

		void SetTokenCallback(tokcallback_t call) {
			m_TokenCallback = call;
		}

		bool TokenCallback(const std::vector<string_t>& tokens) {
			return m_TokenCallback ? m_TokenCallback(tokens) : false;
		}

		ConfScopeable* GetByName(const string_t& name, CodeObjectType filter = CodeObjectType::NONE) const;
		void AddChild(ConfScopeable* child);

		static void SetDefaultStringType(ConfType* ty);
		static void SetDefaultIntegerType(ConfType* ty);
		static void SetDefaultDecimalType(ConfType* ty);
		static void SetDefaultObjectType(ConfType* ty);

		static ConfType* GetDefaultObjectType() {
			return DefaultObjectType;
		}

		ConfType* TypeFromRValue(string_t rvalue);
		ConfInstance* InstanceFromRValue(string_t rvalue);

		ConfScope& operator+=(const ConfScope& scope);

		virtual ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;
	
	private:
		ConfScope* m_Parent;
		std::vector<ConfScopeable*> m_Childs;
		tokcallback_t m_TokenCallback;
		static ConfType* DefaultStringType;
		static ConfType* DefaultIntegerType;
		static ConfType* DefaultDecimalType;
		static ConfType* DefaultObjectType;
	};

	class ConfType : public ConfScope {
	protected:
		using CreateInstance_t = ConfInstance * (*)(ConfType*,string_t);
		CreateInstance_t CreateInstanceCallback;

	private:

		static ConfInstance* _CreateInstance(ConfType* type, string_t name);
	public:
		ConfType(string_t name, ConfScope* parent = nullptr) : ConfScope{ parent } {
			m_Name = std::move(name);
			CreateInstanceCallback = _CreateInstance;
		}

		virtual CodeObjectType GetCodeObjectType() const override {
			return CodeObjectType::TYPE;
		}

		ConfInstance* CreateInstance(string_t name) {
			return CreateInstanceCallback(this, name);
		}

		ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;
	};

	class ConfFunctionIntrinsic : public ConfScope {
	public:
		using intricfunc_t = std::function<ConfInstance* (ConfInstance*, std::vector<ConfInstance*>)>;

		ConfFunctionIntrinsic(ConfScope* parent, string_t name, intricfunc_t callback):
			m_Callback{ callback }, m_Parent{ parent } {
			m_Name = std::move(name);
		}

		virtual CodeObjectType GetCodeObjectType() const override {
			return CodeObjectType::FUNCTION;
		}

		virtual ConfInstance* Call(ConfInstance* _this, std::vector<ConfInstance*> parameters) {
			return m_Callback(_this, parameters);
		}

		virtual ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;

	protected:
		intricfunc_t m_Callback;
		ConfScope* m_Parent;
	};

	class ConfFunctionExtrinsic : public ConfFunctionIntrinsic {
		std::vector<std::vector<string_t>> m_Instructions;

		bool CaptureTokens(const std::vector<string_t>& tokens) {
			m_Instructions.push_back(tokens);
			return true;
		}

	public:
		ConfFunctionExtrinsic(ConfScope* parent, string_t name)
			: ConfFunctionIntrinsic{ parent, name, nullptr } {
			//SetTokenCallback();
		}

		virtual ConfInstance* Call(ConfInstance* _this, std::vector<ConfInstance*> parameters) override {
			return nullptr;
		}
	};

	enum class ConfOperatorType {
		PRE,	//++i
		MID,	//5+6
		POST,	//i++
		SUR		//[i]
	};

	class ConfFunctionIntrinsicOperator :
		public ConfFunctionIntrinsic {
	protected:
		std::size_t m_Priority;
		ConfOperatorType m_OpType;
	public:
		ConfFunctionIntrinsicOperator(ConfScope* parent, string_t name,
			ConfFunctionIntrinsic::intricfunc_t callback, std::size_t priority) :
			ConfFunctionIntrinsic{ parent, name, callback }, m_Priority{ priority },
			m_OpType{ ConfOperatorType::MID } {}

		std::size_t GetPriority() {
			return m_Priority;
		}

		void SetPriority(std::size_t priority) {
			m_Priority = priority;
		}

		ConfOperatorType GetOpType() {
			return m_OpType;
		}

		void SetOpType(ConfOperatorType ty) {
			m_OpType = ty;
		}

		virtual ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;
	};

	class ConfFunctionExtrinsicOperator : public ConfFunctionIntrinsicOperator {
	public:
		ConfFunctionExtrinsicOperator(ConfScope* parent, string_t name, std::size_t priority):
			ConfFunctionIntrinsicOperator{parent, std::move(name), nullptr, priority} {}
	};

	class ConfInstance : public ConfScopeable {
	protected:
		std::vector<ConfInstance*> m_SubInstances;
		ConfType* m_Type;
	public:
		ConfInstance(ConfType* type, string_t name) {
			m_Type = type;
			m_Name = std::move(name);
		}

		~ConfInstance() {
			ClearSubInstances();
		}

		void ClearSubInstances() {
			for (auto it : m_SubInstances) {
				CP_SF(it);
			}
			m_SubInstances.clear();
		}

		virtual CodeObjectType GetCodeObjectType() const override {
			return CodeObjectType::INSTANCE;
		}

		virtual ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;

		virtual ConfInstance* GetMember(const string_t& memberName) {
			ConfInstance* ret = static_cast<ConfInstance*>(m_Type->GetByName(memberName, CodeObjectType::INSTANCE));
			if (ret) return ret;
			for (auto inst : m_SubInstances) {
				if (inst->GetName() == memberName) return inst;
			}
			return nullptr;
		}

		virtual ConfFunctionExtrinsic* GetFunction(const string_t& funcName) {
			return static_cast<ConfFunctionExtrinsic*>(m_Type->GetByName(funcName, CodeObjectType::FUNCTION));
		}

		virtual std::size_t GetSize() const {
			return 0;
		}

		virtual ConfType* GetType() const {
			return m_Type;
		}

		virtual const std::vector<ConfInstance*>& GetSubInstances() {
			return m_SubInstances;
		}

		virtual void AddSubInstance(ConfInstance* inst) {
			m_SubInstances.push_back(inst);
		}

		virtual ConfInstance& operator=(ConfInstance* inst) {
			m_SubInstances = inst->m_SubInstances;
			return *this;
		}
	};

	class ConfTypeString : public ConfType {
	private:
		static ConfInstance* _CreateStringInstance(ConfType* type, string_t name);
	public:
		ConfTypeString(string_t name) : ConfType(std::move(name)) {
			CreateInstanceCallback = _CreateStringInstance;
		}
	};

	class ConfInstanceString : public ConfInstance {
		string_t m_Data;
	public:
		ConfInstanceString(ConfType* strType, string_t name) :
			ConfInstance{ strType,std::move(name) }, m_Data{ CP_TEXT("") } {
		}

		void Set(string_t newData) {
			m_Data = newData;
		}

		string_t Get() const {
			return m_Data;
		}

		virtual ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;

		virtual ConfInstance& operator=(ConfInstance* inst) override {
			ConfInstance::operator=(inst);
			m_Data = static_cast<ConfInstanceString*>(inst)->m_Data;
			return *this;
		}
	};

	class ConfTypeInt : public ConfType {
	private:
		static ConfInstance* _CreateIntInstance(ConfType* type, string_t name);
	public:
		ConfTypeInt(string_t name) : ConfType(std::move(name)) {
			CreateInstanceCallback = _CreateIntInstance;
		}
	};

	class ConfInstanceInt : public ConfInstance {
		int m_Data;
	public:
		ConfInstanceInt(ConfType* strType, string_t name) :
			ConfInstance{ strType,std::move(name) }, m_Data{ 0 } {
		}

		void Set(int newData) {
			m_Data = newData;
		}

		int Get() const {
			return m_Data;
		}

		virtual ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;

		virtual ConfInstance& operator=(ConfInstance* inst) override {
			ConfInstance::operator=(inst);
			m_Data = static_cast<ConfInstanceInt*>(inst)->m_Data;
			return *this;
		}
	};

	class ConfTypeFloat : public ConfType {
	private:
		static ConfInstance* _CreateFloatInstance(ConfType* type, string_t name);
	public:
		ConfTypeFloat(string_t name) : ConfType(std::move(name)) {
			CreateInstanceCallback = _CreateFloatInstance;
		}
	};

	

	class ConfInstanceFloat : public ConfInstance {
		float m_Data;
	public:
		ConfInstanceFloat(ConfType* strType, string_t name) :
			ConfInstance{ strType,std::move(name) }, m_Data{ 0.f } {
		}

		void Set(float newData) {
			m_Data = newData;
		}

		float Get() const {
			return m_Data;
		}

		virtual ConfScopeable* Clone(string_t name, ConfScopeable* buf = nullptr) const override;

		virtual ConfInstance& operator=(ConfInstance* inst) override {
			ConfInstance::operator=(inst);
			m_Data = static_cast<ConfInstanceFloat*>(inst)->m_Data;
			return *this;
		}
	};

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

		static ConfScope* GetGlobalScope() {
			if (!GlobalScope) GlobalScope = new ConfScope(GetIntrinsicScope());
			return GlobalScope;
		}

		ConfParser() : m_IsInitialized{ false } {}
		~ConfParser() {
			if (IntrinsicScope) CP_SF(IntrinsicScope);
		}

		ConfScope* Parse(std::filesystem::path file, StringFormater_t format=nullptr);
		void Initialize();
	};
}