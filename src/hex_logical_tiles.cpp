/*
	Copyright 2014 Kristina Simpson <sweet.kristas@gmail.com>

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

#include <tuple>

#include "asserts.hpp"
#include "hex_logical_tiles.hpp"

namespace hex 
{
	namespace logical
	{
		namespace
		{
			typedef std::map<std::string, tile_ptr> tile_mapping_t;
			tile_mapping_t& get_loaded_tiles()
			{
				static tile_mapping_t res;
				return res;
			}
		}

		void loader(const node& n)
		{
			get_loaded_tiles().clear();

			auto& tiles = n["tiles"];
			for(auto& p : tiles.as_map()) {
				std::string id = p.first.as_string();
				float cost = p.second["cost"].as_float(1.0f);
				float height = p.second["height"].as_float(1.0f);
				std::string name = p.second["name"].as_string();
				get_loaded_tiles()[id] = std::make_shared<tile>(id, name, cost, height);
			}
		}

		tile::tile(const std::string& id, const std::string& name, float cost, float height) 
			: name_(name),
			  id_(id), 
			  cost_(cost), 
			  height_(height)
		{
		}

		tile_ptr tile::factory(const std::string& name)
		{
			auto it = get_loaded_tiles().find(name);
			ASSERT_LOG(it != get_loaded_tiles().end(), "Unable to find a tile with name: " << name);
			return it->second;
		}

		map_ptr map::factory(const node& n)
		{
			return std::make_shared<map>(n);
		}

		map::map(const node& n)
			: x_(n["x"].as_int32(0)),
		      y_(n["y"].as_int32(0)),
			  width_(n["width"].as_int32()), 
			  height_(0)
		{
			tiles_.reserve(width_ * width_);	// approximation
			for (auto& tile_str : n["tiles"].as_list_strings()) {
				tiles_.emplace_back(tile::factory(tile_str));
			}
			height_ = tiles_.size() / width_;
		}

		map::map(const map& m)
			: x_(m.x_),
			  y_(m.y_),
			  width_(m.width_),
			  height_(m.height_),
			  tiles_(m.tiles_)
		{
			// XX if we ever have a case where we need to modify tiles differently between the
			// internal server and here then we need to clone all the elements in m.tiles_.
		}

		const_tile_ptr map::get_hex_tile(direction d, int xx, int yy) const
		{
			int ox = xx;
			int oy = yy;
			ASSERT_LOG(x() == 0 && y() == 0, "x/y values not zero (" << x() << "," << y() << ")");
			xx -= x();
			yy -= y();
			if(d == NORTH) {
				yy -= 1;
			} else if(d == SOUTH) {
				yy += 1;
			} else if(d == NORTH_WEST) {
				yy -= (abs(ox)%2==0) ? 1 : 0;
				xx -= 1;
			} else if(d == NORTH_EAST) {
				yy -= (abs(ox)%2==0) ? 1 : 0;
				xx += 1;
			} else if(d == SOUTH_WEST) {
				yy += (abs(ox)%2) ? 1 : 0;
				xx -= 1;
			} else if(d == SOUTH_EAST) {
				yy += (abs(ox)%2) ? 1 : 0;
				xx += 1;
			} else {
				ASSERT_LOG(false, "Unrecognised direction: " << d);
			}
			if (xx < 0 || yy < 0 || yy >= height() || xx >= width()) {
				return nullptr;
			}

			const int index = yy * width() + xx;
			ASSERT_LOG(index >= 0 && index < static_cast<int>(tiles_.size()), "Index out of bounds." << index << " >= " << tiles_.size());
			return tiles_[index];
		}

		point map::get_coordinates_in_dir(direction d, int xx, int yy) const
		{
			int ox = xx;
			int oy = yy;
			xx -= x();
			yy -= y();
			switch (d) {
				case NORTH:			yy -= 1; break;
				case NORTH_EAST:
					yy -= (abs(ox)%2==0) ? 1 : 0;
					xx += 1;
					break;
				case SOUTH_EAST:
					yy += (abs(ox)%2) ? 1 : 0;
					xx += 1;
					break;
				case SOUTH:			yy += 1; break;
				case SOUTH_WEST:
					yy += (abs(ox)%2) ? 1 : 0;
					xx -= 1;
					break;
				case NORTH_WEST:
					yy -= (abs(ox)%2==0) ? 1 : 0;
					xx -= 1;
					break;
				default:
					ASSERT_LOG(false, "Unrecognised direction: " << d);
					break;
			}
			return point(xx, yy) + point(x(), y());
		}

		std::vector<const_tile_ptr> map::get_surrounding_tiles(int x, int y) const
		{
			std::vector<const_tile_ptr> res;
			for(auto dir : { NORTH, NORTH_EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, NORTH_WEST }) {
				auto hp = get_hex_tile(dir, x, y);
				if(hp != nullptr) {
					res.emplace_back(hp);
				}
			}
			return res;
		}

		std::vector<point> map::get_surrounding_positions(int xx, int yy) const
		{
			std::vector<point> res;
			for(auto dir : { NORTH, NORTH_EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, NORTH_WEST }) {
				auto p = get_coordinates_in_dir(dir, xx, yy);
				if(p.x >= 0 && p.x >= 0 && p.x < width() && p.y < height()) {
					res.emplace_back(p);
				}
			}
			return res;
		}

		std::vector<point> map::get_surrounding_positions(const point& p) const
		{
			return get_surrounding_positions(p.x, p.y);
		}

		const_tile_ptr map::get_tile_at(int xx, int yy) const
		{
			xx -= x();
			yy -= y();
			if (xx < 0 || yy < 0 || yy >= height() || xx >= width()) {
				return nullptr;
			}

			const int index = yy * width() + xx;
			ASSERT_LOG(index >= 0 && index < static_cast<int>(tiles_.size()), "");
			return tiles_[index];
		}

		const_tile_ptr map::get_tile_at(const point& p) const
		{
			return get_tile_at(p.x, p.y);
		}

		map_ptr map::clone()
		{
			return map_ptr(new map(*this));
		}

		std::tuple<int,int,int> oddq_to_cube_coords(const point& p)
		{
			int x1 = p.x;
			int z1 = p.y - (p.x - (p.x & 1)) / 2;
			int y1 = -(x1 + z1);
			return std::make_tuple(x1,y1,z1);
		}

		int distance(int x1, int y1, int z1, int x2, int y2, int z2)
		{
			return (abs(x1 - x2) + abs(y1 - y2) + abs(z1 - z2)) / 2;
		}

		int distance(const point& p1, const point& p2)
		{
			int x1, y1, z1;
			std::tie(x1,y1,z1) = oddq_to_cube_coords(p1);
			int x2, y2, z2;
			std::tie(x2,y2,z2) = oddq_to_cube_coords(p2);
			return distance(x1, y1, z1, x2, y2, z2);
		}

		std::tuple<int,int,int> hex_round(float x, float y, float z) 
		{
			int rx = static_cast<int>(std::round(x));
			int ry = static_cast<int>(std::round(y));
			int rz = static_cast<int>(std::round(z));

			float x_diff = std::abs(rx - x);
			float y_diff = std::abs(ry - y);
			float z_diff = std::abs(rz - z);

			if(x_diff > y_diff && x_diff > z_diff) {
				rx = -(ry+rz);
			} else if(y_diff > z_diff) {
				ry = -(rx+rz);
			} else {
				rz = -(rx+ry);
			}
			return std::make_tuple(rx, ry, rz);
		}

		point cube_to_oddq_coords(const std::tuple<int,int,int>& xyz)
		{
			return point(std::get<0>(xyz), std::get<2>(xyz) + (std::get<0>(xyz) - (std::get<0>(xyz) & 1)) / 2);
		}

		std::vector<point> line(const point& p1, const point& p2)
		{
			std::vector<point> res;
			int n = distance(p1, p2);
			int x1, y1, z1;
			std::tie(x1,y1,z1) = oddq_to_cube_coords(p1);
			int x2, y2, z2;
			std::tie(x2,y2,z2) = oddq_to_cube_coords(p2);
			for(int i = 0; i <= n; ++i) {
				const float i_over_n  = static_cast<float>(i)/n;
				const float xt = x1 * (1.0f - i_over_n) + x2 * i_over_n + 1e-6f;
				const float yt = y1 * (1.0f - i_over_n) + y2 * i_over_n + 1e-6f;
				const float zt = z1 * (1.0f - i_over_n) + z2 * i_over_n - 2e-6f;
				res.emplace_back(cube_to_oddq_coords(hex_round(xt, yt, zt)));
			}

			return res;
		}

		float rotation_between(const point& p1, const point& p2)
		{
			// hack it somewhat to just work for p1 and p2 being adjacent.
			const int dx = p2.x - p1.x;
			const int dy = p2.y - p1.y;
			ASSERT_LOG(dx <= 1 && dx >= -1 && dy >= -1 && dy <= 1, "hex::logical::rotation_between only works for adjacent tiles.");
			if(dx == 0 && dy == 0) {
				return 0.0f;
			}
			if(dy == 0) {
				return dx > 0 ? 60.0f : 300.0f;
			} else if(dy > 0) {
				return dx == 0 ? 180.0f : dx > 0 ? 120.0f : dx == 240.0f;
			} else {
				// dy < 0
				return 0.0f;
			}
		}
	}
}
