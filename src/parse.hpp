/*
 *  Copyright 2015 Maarten de Vries <maarten@de-vri.es>
 *
 *  This file is part of inet_remap.
 *
 *  inet_remap is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  inet_remap is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with inet_remap.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "key.hpp"

#include <map>


namespace inet_remap {

/// Parse a remap entry from a string.
std::pair<key, int> parseEntry(char const * data, char const * & new_begin);
inline std::pair<key, int> parseEntry(char const * data) {
	char const * finger;
	return parseEntry(data, finger);
}

/// Parse a remap map from a string.
std::map<key, int> parseMap(char const * data);

}
