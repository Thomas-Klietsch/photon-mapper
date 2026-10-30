#pragma once

#include <algorithm>
#include <cmath>
#include <ostream>

#include "../epsilon.hpp"

struct Colour {

	std::float_t r{ 0.f };
	std::float_t g{ 0.f };
	std::float_t b{ 0.f };

	Colour() {};

	Colour( std::float_t const r, std::float_t const g, std::float_t const b )
		: r( r )
		, g( g )
		, b( b ) {};

	Colour operator+( Colour const& value ) const { return Colour( r + value.r, g + value.g, b + value.b ); };

	Colour operator*( Colour const& value ) const { return Colour( r * value.r, g * value.g, b * value.b ); };
	Colour operator*( std::float_t const value ) const { return Colour( r * value, g * value, b * value ); };

	Colour operator/( std::float_t const value ) const { return Colour( r / value, g / value, b / value ); };

	Colour operator=( Colour const& value ) {
		r = value.r;
		g = value.g;
		b = value.b;
		return *this;
	};
	Colour operator+=( Colour const& value ) {
		r += value.r;
		g += value.g, b += value.b;
		return *this;
	};
	Colour operator*=( Colour const& value ) {
		r *= value.r;
		g *= value.g, b *= value.b;
		return *this;
	};

	bool is_black() const { return std::max( { r, g, b } ) < EPSILON_BLACK; };

	friend std::ostream& operator<<( std::ostream& os, Colour const& value ) {
		os << "( " << value.r << " , " << value.g << " , " << value.b << " )";
		return os;
	};

	Colour static const Black;
	Colour static const White;
	Colour static const Red;
	Colour static const Green;
	Colour static const Blue;
};

inline Colour const Colour::Black( 0.0f, 0.0f, 0.0f );
inline Colour const Colour::White( 1.0f, 1.0f, 1.0f );
inline Colour const Colour::Red( 1.0f, 0.0f, 0.0f );
inline Colour const Colour::Green( 0.0f, 1.0f, 0.0f );
inline Colour const Colour::Blue( 0.0f, 0.0f, 1.0f );
