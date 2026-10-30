#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <memory>
#include <string>

#include "../colour/colour.hpp"
#include "../render/config.hpp"
#include "../render/sensor.hpp"

namespace Render {

	// TODO
	// Uncompressed 24bit TGA
	inline bool SaveImage(
		std::string const& file_name,
		Render::Sensor& sensor,
		Render::Config const& config,
		bool f_libgdk = false
	) {
		// File output only
		std::ofstream tga_file;
		// If already open, delete content. Binary mode is needed
		tga_file.open( file_name + ".tga", std::ios::trunc | std::ios::binary );
		// Check if file is open
		if ( !tga_file.is_open() )
			return false;

		std::uint32_t const tga_header_size = 18 + ( f_libgdk ? 1 : 0 );
		std::uint32_t const n_pixel = config.image_width * config.image_height;
		std::uint32_t const tga_data_size = tga_header_size + n_pixel * 3;

		std::shared_ptr<std::uint8_t[]> p_data = std::make_shared<std::uint8_t[]>( tga_data_size, 0 );
		if ( !p_data )
			return false;

		// Set header
		// Comment data size
		p_data[0] = f_libgdk ? 1 : 0;
		// Colourmap type
		p_data[1] = 0;
		// Datatype
		p_data[2] = 2;
		// Colourmap origin
		p_data[3] = 0;
		p_data[4] = 0;
		// Colourmap length
		p_data[5] = 0;
		p_data[6] = 0;
		// Colourmap depth
		p_data[7] = 0;
		// X origin
		p_data[8] = 0;
		p_data[9] = 0;
		// Y origin
		p_data[10] = 0;
		p_data[11] = 0;
		// X size
		p_data[12] = static_cast<std::uint8_t>( config.image_width % 256 );
		p_data[13] = static_cast<std::uint8_t>( config.image_width / 256 );
		// Y size
		p_data[14] = static_cast<std::uint8_t>( config.image_height % 256 );
		p_data[15] = static_cast<std::uint8_t>( config.image_height / 256 );
		// Bits per pixel
		p_data[16] = 24;
		// Image descriptor
		// 32 (bit 5) is screen origin, 0 lower left, 1 upper left
		p_data[17] = 32;
		if ( f_libgdk )
			// Nonzero length bug fix for libgdk
			p_data[18] = 0;

		for ( std::uint16_t y{ 0 }; y < config.image_height; ++y )
			for ( std::uint16_t x{ 0 }; x < config.image_width; ++x ) {
				Colour const colour = sensor.get_colour( x, y );
				std::uint32_t index = ( x + y * config.image_width ) * 3;
				// TGA uses BGR colour order
				p_data[index + tga_header_size] = static_cast<std::uint8_t>( std::pow( std::clamp( colour.b, 0.f, 1.f ), 1.f / 2.2f ) * 255 );
				p_data[index + 1 + tga_header_size] = static_cast<std::uint8_t>( std::pow( std::clamp( colour.g, 0.f, 1.f ), 1.f / 2.2f ) * 255 );
				p_data[index + 2 + tga_header_size] = static_cast<std::uint8_t>( std::pow( std::clamp( colour.r, 0.f, 1.f ), 1.f / 2.2f ) * 255 );
			}

		// Dump p_data from memory to file
		tga_file.write( (char*)( &p_data[0] ), tga_data_size );
		tga_file.close();

		return true;
	};

};
