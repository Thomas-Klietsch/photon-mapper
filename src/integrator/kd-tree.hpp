// Copyright (c) 2026 Thomas Klietsch, all rights reserved.
//
// Licensed under the GNU Lesser General Public License, version 3.0 or later
//
// This program is free software: you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation, either version 3 of
// the License, or ( at your option ) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General
// Public License along with this program.If not, see < https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>
#include <cstdint>
#include <numeric>
#include <queue>
#include <tuple>
#include <vector>

#include "../integrator/photon.hpp"
#include "../mathematics/double3.hpp"

namespace Integrator {

	// Jon Louis Bentley:
	// Divide and conquer algorithms for closest point problems in multidimensional space (1976)

	// A k-d tree for photons.
	// Three (3) dimensional, using 32bit floats internally.
	class KDTree final {

	private:

		// k-d tree node
		struct Node {
			// Round robin axis split, x,y,z,x... (0,1,2,0...)
			std::uint8_t axis{ 0 };

			// Index of hyper plane (split point),
			// as well as particle index
			std::uint32_t centre{ 0 };

			// Node ID of left branch (child)
			std::uint32_t left{ UINT32_MAX }; // Default set as a leaf

			// Node ID of right branch (child)
			std::uint32_t right{ UINT32_MAX }; // Default set as a leaf

			Node() {};

			Node(
				std::uint8_t const axis,
				std::uint32_t const centre
			)
				: axis( axis )
				, centre( centre ) {};
		};

		// k-d tree dimensional data,
		// 32bit float is used for speed and memory use.
		struct Float3 {
			std::float_t axis[3];

			Float3() { axis[0] = axis[1] = axis[2] = 0.f; };

			Float3(
				std::float_t const x,
				std::float_t const y,
				std::float_t const z
			) {
				axis[0] = x;
				axis[1] = y;
				axis[2] = z;
			};

			// Integrator uses Double3, convert to internal data
			explicit Float3(
				Double3 const& point
			) {
				axis[0] = point.x;
				axis[1] = point.y;
				axis[2] = point.z;
			};

			// Access axis (3D)
			std::float_t operator[]( std::uint8_t const i ) const { return axis[std::min( i, static_cast<std::uint8_t>( 2 ) )]; };

			// Calculate the squared distance between two points
			inline friend std::float_t Norm(
				Float3 const& a,
				Float3 const& b
			) {
				return ( a[0] - b[0] ) * ( a[0] - b[0] ) + ( a[1] - b[1] ) * ( a[1] - b[1] ) + ( a[2] - b[2] ) * ( a[2] - b[2] );
			};
		};

		// All nodes in the k-d tree
		std::vector<Node> node;

		// All photons, converted to internal 3D space
		std::vector<Float3> particle;

		// Only used during construction of k-d tree
		std::vector<std::uint32_t> index;

		// Priority queue auto sorts elements added, by default on .first value
		// Sorted list is largest value at [0], to smallest [last element]
		// Note: distance is squared, in the queue
		using Queue = std::priority_queue<std::pair<std::float_t, std::uint32_t>>;

	public:

		KDTree() {};

		void build(
			std::vector<Integrator::Photon> const& photon
		) {
			// Clear old data, if present
			if ( particle.size() )
				std::vector<Float3>().swap( particle );
			if ( node.size() )
				std::vector<Node>().swap( node );

			std::size_t const n_size = photon.size();

			// Convert photon position to internal data type
			particle = std::vector<Float3>( n_size );
			for ( std::uint32_t i{ 0 }; i < n_size; ++i )
				particle[i] = Float3( photon[i].position );

			// Generate index, starting at 0
			index = std::vector<std::uint32_t>( n_size );
			std::iota( index.begin(), index.end(), 0 );

			BuildTree( 0, n_size, 0 );

			// Clear unused memory
			std::vector<std::uint32_t>().swap( index );
		};

		// Find photon(s) within radius
		std::vector<std::uint32_t> range_query(
			Double3 const& point,
			std::float_t const radius
		) const {
			if ( radius <= 0 )
				// Nothing to do
				return {};

			std::vector<std::uint32_t> list;

			RangeSearch( Float3( point ), radius * radius, 0, list );

			return list;
		};

		// Find the k nearest photon(s)
		// Return: list of photon id(s), and squared distance to furthest photon found
		std::tuple<std::vector<std::uint32_t>, std::float_t> knn_query(
			Double3 const& point,
			std::uint16_t const k
		) const {
			if ( !k )
				// Nothing to do
				return {};

			Queue queue;

			QueueSearch( Float3( point ), k, 0, queue );

			std::uint32_t const n_count = queue.size();

			if ( n_count ) {
				std::vector<std::uint32_t> list( n_count );

				std::float_t max_distance_squared = queue.top().first;

				for ( std::uint32_t i{ 0 }; i < n_count; ++i ) {
					auto const& p = queue.top();
					list[i] = p.second;
					queue.pop();
				}

				return { list, max_distance_squared };
			}

			return {};
		};

	private:

		void BuildTree(
			std::uint32_t const start,
			std::uint32_t const end,
			std::uint8_t const depth
		) {
			// Leaf
			if ( end <= start )
				return;

			// Round Robin axis select
			std::uint8_t const axis = depth % 3;

			// Sort, from start to end, along axis
			std::sort( index.begin() + start, index.begin() + end,
				// Using custom Lambda function, with capture
				[&]( std::uint32_t const id1, std::uint32_t const id2 )
				{
					return particle[id1][axis] < particle[id2][axis];
				} );

			// Split index
			std::uint32_t const centre = start + ( end - start ) / 2u;

			// Create parent node
			std::uint32_t const parent_id = node.size();
			node.push_back( Node( axis, index[centre] ) );

			// Process left node
			std::uint32_t const left_id = node.size();
			BuildTree( start, centre, depth + 1 );
			if ( left_id == node.size() )
				// No left child
				node[parent_id].left = UINT32_MAX;
			else
				node[parent_id].left = left_id;

			// Process right node
			std::uint32_t const right_id = node.size();
			BuildTree( centre + 1, end, depth + 1 );
			if ( right_id == node.size() )
				// No right child
				node[parent_id].right = UINT32_MAX;
			else
				node[parent_id].right = right_id;
		};

		void RangeSearch(
			Float3 const& point,
			std::float_t const radius2, // Radius squared
			std::uint32_t const node_index,
			std::vector<std::uint32_t>& list
		) const {
			// Leaf
			if ( node_index == UINT32_MAX )
				return;

			Node const& current_node = node[node_index];

			Float3 const& centre_point = particle[current_node.centre];

			// Add centre point to list, if within range
			if ( Norm( point, centre_point ) < radius2 )
				list.push_back( current_node.centre );

			// Select which child to search first
			bool const f_lower = point[current_node.axis] < centre_point[current_node.axis];

			// Always search down, to leaf node, first
			if ( f_lower )
				RangeSearch( point, radius2, current_node.left, list );
			else
				RangeSearch( point, radius2, current_node.right, list );

			// If other child is within range, search it
			std::float_t const difference = centre_point[current_node.axis] - point[current_node.axis];
			if ( difference * difference < radius2 ) {
				if ( f_lower )
					RangeSearch( point, radius2, current_node.right, list );
				else
					RangeSearch( point, radius2, current_node.left, list );
			}
		};

		void QueueSearch(
			Float3 const& point,
			std::uint16_t const k,
			std::uint32_t const node_index,
			Queue& queue
		) const {
			// Leaf
			if ( node_index == UINT32_MAX )
				return;

			Node const& current_node = node[node_index];

			Float3 const& centre_point = particle[current_node.centre];

			// Add centre point to queue
			queue.emplace( Norm( point, centre_point ), current_node.centre );

			// Remove top element (largest distance^2), if queue is too large
			if ( queue.size() > k )
				queue.pop();

			// Select which child to search first
			bool const f_lower = point[current_node.axis] < centre_point[current_node.axis];

			// Always search down, to leaf node, first
			if ( f_lower )
				QueueSearch( point, k, current_node.left, queue );
			else
				QueueSearch( point, k, current_node.right, queue );

			// If the other child is within range, search it
			std::float_t const difference = centre_point[current_node.axis] - point[current_node.axis];
			if ( difference * difference < queue.top().first ) {
				if ( f_lower )
					QueueSearch( point, k, current_node.right, queue );
				else
					QueueSearch( point, k, current_node.left, queue );
			}
		};

	}; // end class

};
