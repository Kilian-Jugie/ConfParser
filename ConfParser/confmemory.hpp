/*
* Copyright (C) 2020 Kilian Jugie - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
*/
/*!
 * \file confmemory.hpp
 * \brief Unused possible future memory management implementation
 */

#pragma once
#include <unordered_map>
#include "global.hpp"

namespace confparser {
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
}