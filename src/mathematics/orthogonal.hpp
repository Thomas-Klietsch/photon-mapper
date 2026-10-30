#pragma once

#include "../mathematics/double3.hpp"

class Orthogonal final {

private:

	Double3 x_axis{ Double3::X };
	Double3 y_axis{ Double3::Y };
	Double3 z_axis{ Double3::Z };

public:

	Orthogonal() {};

	Orthogonal(
		Double3 const& vector
	) {
		// TODO test magnitude
		// Define orthogonal space, using vector given as notmal (z axis)
		z_axis = vector.normalise();
		Double3 const tmp = ( std::abs( z_axis.x ) > 0.995f ) ? Double3::Y : Double3::X;
		y_axis = ( z_axis.cross( tmp ) ).normalise(); // Right hand
		x_axis = y_axis.cross( z_axis ); // y and z are normalised and perpendicular, so x is length 1 by default
	};

	Double3 to_world(
		Double3 const& value
	) const {
		return x_axis * value.x + y_axis * value.y + z_axis * value.z;
	};

	Double3 to_local(
		Double3 const& value
	) const {
		return { x_axis.dot( value ), y_axis.dot( value ), z_axis.dot( value ) };
	};

	// Tangent plane vector (x axis)
	Double3 const& tangent() const { return x_axis; }
	// Tangent plane vector (y axis)
	Double3 const& bitangent() const { return y_axis; }
	// Normal plane vector (z axis)
	Double3 const& normal() const { return z_axis; }
};
