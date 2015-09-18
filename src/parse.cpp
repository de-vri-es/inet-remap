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

#include "parse.hpp"

#include <string>
#include <utility>
#include <iostream>

extern "C" {
#include <netinet/in.h>
}

namespace inet_remap {

namespace {
	bool isDigit(char c) {
		return c >= '0' && c <= '9';
	}

	short toProto(std::string const & protocol) {
		if (protocol == "tcp") {
			return IPPROTO_TCP;
		} else if (protocol == "udp") {
			return IPPROTO_UDP;
		} else {
			throw std::runtime_error("Unrecognized protocol: " + protocol);
		}
	}

	bool isSeperator(char c) {
		return c == ' ' || c == ',' || c == ';';
	}

}

std::pair<key, int> parseEntry(char const * data, char const * & new_begin) {
	std::string protocol;
	int old_port = 0;
	int new_port = 0;

	char const * finger = data;

	// Parse protocol.
	for (; *finger != 0 && *finger != ':'; ++finger) {
		protocol.push_back(*finger);
	}
	++finger;

	// Parse source port.
	for (; *finger != 0 && *finger != ':'; ++finger) {
		if (isDigit(*finger)) {
			old_port = old_port * 10 + (*finger - '0');
		} else {
			throw std::runtime_error(std::string("Unexpected character in remap port: ") + *finger + " ("+ std::to_string(int(*finger)) + ")");
		}
	}
	++finger;

	// New port.
	for (; *finger != 0 && !isSeperator(*finger); ++finger) {
		if (isDigit(*finger)) {
			new_port = new_port * 10 + (*finger - '0');
		} else {
			throw std::runtime_error(std::string("Unexpected character in remap port: ") + *finger + " ("+ std::to_string(int(*finger)) + ")");
		}
	}
	++finger;

	new_begin = finger;
	return std::make_pair(key(toProto(protocol), old_port), new_port);
}

/// Parse a rewrite map from a string.
std::map<key, int> parseMap(char const * data) {
	std::map<key, int> result;

	char const * finger = data;
	while (*finger != 0) result.insert(parseEntry(finger, finger));
	return result;
}


}
