#pragma once
#include <cstddef>

// 2021-08-18 16:45:24.132317500 UTC

namespace hazedumper {
	namespace netvars {
		constexpr ::std::ptrdiff_t m_bSpotted = 0x93D;
		constexpr ::std::ptrdiff_t m_fFlags = 0x104;
		constexpr ::std::ptrdiff_t m_flDetectedByEnemySensorTime = 0x3978;
	} // namespace netvars
	namespace signatures {
		constexpr ::std::ptrdiff_t dwEntityList = 0x4DA31EC;
		constexpr ::std::ptrdiff_t dwForceJump = 0x524CFDC;
		constexpr ::std::ptrdiff_t dwGetAllClasses = 0xDB202C;
		constexpr ::std::ptrdiff_t dwLocalPlayer = 0xD8A2DC;
	} // namespace signatures
} // namespace hazedumper