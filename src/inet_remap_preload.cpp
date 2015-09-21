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

#include "key.hpp"
#include "parse.hpp"

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

namespace {

	/// Map holding the remappings.
	std::map<inet_remap::key, int> rewrite_map;
	bool verbose = false;

	/// Original bind function.
	int (*original_bind)(int, struct sockaddr const *, int);

	/// Global object to perform one-time initialization.
	struct Init {
		Init() {
			// Get the original bind.
			original_bind = reinterpret_cast<int (*)(int, struct sockaddr const *, int)>(dlsym(RTLD_NEXT, "bind"));

			// Set debug bool depending on INET_REMAP_DEBUG variable.
			char const * debug_env = getenv("INET_REMAP_VERBOSE");
			verbose = debug_env && debug_env[0] != 0 && debug_env[0] != '0';

			// Parse the rewrite map from the environment.
			try {
				char const * remap_str = getenv("INET_REMAP");
				if (remap_str) rewrite_map = inet_remap::parseMap(remap_str);
			} catch (std::exception const & e) {
				std::cerr << "inet_remap: " << e.what() << std::endl;
			}
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
			if (verbose) std::cerr << "Failed to get socket protocol. Error " << errno << ": " << std::strerror(errno) << "\n";
			return original_bind(fd, address, address_length);
		}
	}

	sockaddr_in inet_address = reinterpret_cast<sockaddr_in const &>(*address);
	int port = ntohs(inet_address.sin_port);
	std::map<inet_remap::key, int>::iterator entry = rewrite_map.find(inet_remap::key(protocol, port));

	// Port was not in the map? Call original bind without modifying the arguments.
	if (entry == rewrite_map.end()) {
		if (verbose) std::cerr << "Passing through bind for " << protocol << ":" << port << ".\n";
		return original_bind(fd, address, address_length);
	}

	// Change port and pass modified address to original bind.
	inet_address.sin_port = htons(entry->second);
	if (verbose) std::cerr << "Rebinding " << protocol << ":" << port << " to " << entry->second << " .\n";
	return original_bind(fd, reinterpret_cast<sockaddr *>(&inet_address), address_length);
}
