#pragma once

#include <cmath>
#include <ostream>

// 3D vector, using double
class Double3 final {

public:

	std::double_t x{ 0.0 };
	std::double_t y{ 0.0 };
	std::double_t z{ 0.0 };

	Double3() {};

	Double3( std::double_t const x, std::double_t const y, std::double_t const z )
		: x( x )
		, y( y )
		, z( z ) {};

	// Unary minus
	Double3 operator-() const { return Double3( -x, -y, -z ); };

	Double3 operator+( Double3 const& value ) const { return Double3( x + value.x, y + value.y, z + value.z ); };
	Double3 operator-( Double3 const& value ) const { return Double3( x - value.x, y - value.y, z - value.z ); };
	Double3 operator*( std::double_t const value ) const { return Double3( x * value, y * value, z * value ); };
	Double3 operator/( std::double_t const value ) const { return Double3( x / value, y / value, z / value ); };

	Double3 operator=( Double3 const& value ) {
		x = value.x;
		y = value.y;
		z = value.z;
		return *this;
	};

	Double3 normalise() const { return Double3( x, y, z ) / std::sqrt( x * x + y * y + z * z ); };

	std::double_t absdot( Double3 const& value ) const { return std::abs( x * value.x + y * value.y + z * value.z ); };
	std::double_t dot( Double3 const& value ) const { return x * value.x + y * value.y + z * value.z; };

	Double3 cross( Double3 const& value ) const { return Double3( y * value.z - z * value.y, z * value.x - x * value.z, x * value.y - y * value.x ); };

	std::double_t magnitude() const { return std::sqrt( x * x + y * y + z * z ); };

	std::double_t distance( Double3 const& value ) const { return ( *this - value ).magnitude(); };

	friend std::ostream& operator<<( std::ostream& os, Double3 const& value ) {
		os << "( " << value.x << " , " << value.y << " , " << value.z << " )";
		return os;
	};

	friend Double3 Min( Double3 const& left, Double3 const& right ) { return { std::min( left.x, right.x ), std::min( left.y, right.y ), std::min( left.z, right.z ) }; };

	friend Double3 Max( Double3 const& left, Double3 const& right ) { return { std::max( left.x, right.x ), std::max( left.y, right.y ), std::max( left.z, right.z ) }; };

	friend std::double_t Norm( Double3 const& left, Double3 const& right ) {
		Double3 const diff = left - right;
		return diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
	};

	Double3 static const Zero;
	Double3 static const One;
	// Unit vectors
	Double3 static const X;
	Double3 static const Y;
	Double3 static const Z;
};

// ( 0,0,0 )
inline Double3 const Double3::Zero( 0.0, 0.0, 0.0 );
// ( 1,1,1 )
inline Double3 const Double3::One( 1.0, 1.0, 1.0 );
// ( 1,0,0 )
inline Double3 const Double3::X( 1.0, 0.0, 0.0 );
// ( 0,1,0 )
inline Double3 const Double3::Y( 0.0, 1.0, 0.0 );
// ( 0,0,1 )
inline Double3 const Double3::Z( 0.0, 0.0, 1.0 );
