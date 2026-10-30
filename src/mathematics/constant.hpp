#pragma once

#include <cmath>
#include <numbers>

// pi
constexpr std::double_t pi = static_cast<std::double_t>( std::numbers::pi );

// 1 / pi
constexpr std::double_t inv_pi = static_cast<std::double_t>( std::numbers::inv_pi );

// 1 / ( 2 * pi )
constexpr std::double_t inv_2pi = static_cast<std::double_t>( std::numbers::inv_pi * 0.5 );

// 2 * pi
constexpr std::double_t two_pi = static_cast<std::double_t>( std::numbers::pi * 2. );

// square root of 2
constexpr std::double_t sqrt2 = std::numbers::sqrt2;

// Convert degree to radian
constexpr std::double_t deg_to_rad = static_cast<std::double_t>( std::numbers::pi / 180. );
