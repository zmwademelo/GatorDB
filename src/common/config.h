#pragma once
#include <cstdint>

//Statndard 4KB
inline constexpr std::uint32_t PAGE_SIZE = 4096; //uint32_t is a specific data type that stands for unsigned integer of exactly 32 bits. 
inline constexpr std::uint32_t INVALID_PAGE_ID = 0xFFFFFFFF; //Max value of uint32_t

using page_id_t = std::uint32_t; //Page ID type

