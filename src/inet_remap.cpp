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

#include <map>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cerrno>

extern "C" {
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
}

#include "inet_remap.hpp"
#include "parse.hpp"

namespace {

	/// Map holding the remappings.
	std::map<inet_remap::key, int> rewrite_map;

	/// Original bind function.
	int (*original_bind)(int, struct sockaddr const *, int);

	/// Global object to perform one-time initialization.
	struct Init {
		Init() {
			// Get the original bind.
			original_bind = reinterpret_cast<int (*)(int, struct sockaddr const *, int)>(dlsym(RTLD_NEXT, "bind"));

			// Parse the rewrite map from the environment.
			rewrite_map = inet_remap::parse_map(getenv("INET_REMAP"));
		}
	} _init;
}

extern "C" int bind(int fd, sockaddr const * address, socklen_t address_length) {
	// Not an INET address or the rewrite map is empty?
	// Skip lookup and call original bind without modifying the arguments.
	if (address->sa_family != AF_INET || rewrite_map.empty()) {
		return original_bind(fd, address, address_length);
	}

	int protocol;
	{
		socklen_t length = sizeof(protocol);
		if (getsockopt(fd, SOL_SOCKET, SO_PROTOCOL, &protocol, &length) != 0) {
			std::cerr << "Failed to get socket protocol. Error " << errno << ": " << std::strerror(errno) << "\n";
			return original_bind(fd, address, address_length);
		}
	}

	sockaddr_in inet_address = reinterpret_cast<sockaddr_in const &>(*address);
	int port = ntohs(inet_address.sin_port);
	std::map<inet_remap::key, int>::iterator entry = rewrite_map.find(inet_remap::key(protocol, port));

	// Port was not in the map? Call original bind without modifying the arguments.
	if (entry == rewrite_map.end()) return original_bind(fd, address, address_length);

	// Change port and pass modified address to original bind.
	inet_address.sin_port = htons(entry->second);
	return original_bind(fd, reinterpret_cast<sockaddr *>(&inet_address), address_length);
}
