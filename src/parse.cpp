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

#include <string>
#include <utility>
#include <iostream>

extern "C" {
#include <netinet/in.h>
}

#include "parse.hpp"

namespace inet_remap {

namespace {
	bool is_digit(char c) {
		return c >= '0' && c <= '9';
	}

	short toProto(std::string const & protocol) {
		if (protocol == "tcp") {
			return IPPROTO_TCP;
		} else if (protocol == "udp") {
			return IPPROTO_UDP;
		} else {
			std::cerr << "Unrecognized protocol: " << protocol << "\n";
			return -1;
		}
	}

}

/// Parse a rewrite map from a string.
std::map<key, int> parse_map(char const * data) {
	std::map<key, int> result;

	std::string protocol;
	int old_port = 0;
	int new_port = 0;
	int stage    = 0;

	for (char const * i = data; *i != 0; ++i) {
		char c = *i;
		// Protocol name
		if (stage == 0) {
			if (c == ':') {
				++stage;
			} else {
				protocol += c;
			}

		// Original port
		} else if (stage == 1) {
			if (c == ':') {
				++stage;
			} else if (is_digit(c)) {
				old_port = old_port * 10 + (c - '0');
			} else {
				std::cerr << "Unexpected character in remap port: " << int(c) << "\n";
				return std::map<key, int>();
			}

		// New port.
		} else if (stage == 2) {
			if (c == ';' || c ==  ' ' || c == ',') {
				result.insert(std::make_pair(key(toProto(protocol), old_port), new_port));
				protocol.clear();
				old_port = 0;
				new_port = 0;
				stage    = 0;
			} else if (is_digit(c)) {
				new_port = new_port * 10 + (c - '0');
			} else {
				std::cerr << "Unexpected character in remap port: " << int(c) << "\n";
				return std::map<key, int>();
			}

		}
	}

	// In case the string didn't end with a space.
	if (stage == 2) {
		result.insert(std::make_pair(key(toProto(protocol), old_port), new_port));
	}

	return result;
}


}
