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
#include <cstdint>

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

	/// Check if a sockaddr is AF_INET or AF_INET6.
	bool is_inet_addr(sockaddr const & address) {
		return address.sa_family == AF_INET || address.sa_family == AF_INET6;
	}

	/// Get the port number of a AF_INET or AF_INET6 sockaddr, or 0.
	std::uint16_t get_port(sockaddr const & address) {
		switch (address.sa_family) {
			case AF_INET:  return ntohs(reinterpret_cast<sockaddr_in  const &>(address).sin_port);
			case AF_INET6: return ntohs(reinterpret_cast<sockaddr_in6 const &>(address).sin6_port);
		}
		return 0;
	}

	/// Set the port number on a AF_INET or AF_INET6 sockaddr.
	void set_port(sockaddr & address, std::uint16_t port) {
		switch (address.sa_family) {
		case AF_INET:
			reinterpret_cast<sockaddr_in &>(address).sin_port = ntohs(port);
			break;
		case AF_INET6:
			reinterpret_cast<sockaddr_in6 &>(address).sin6_port = ntohs(port);
			break;
		}
	}

	/// Union capable of holding IPv4 and IPv6 socket addresses.
	union address_t {
		sockaddr generic;
		sockaddr_in in;
		sockaddr_in6 in6;
	};

	/// Copy a AF_INET or AF_INET6 address safely.
	address_t copy_address(sockaddr const & address) {
		address_t result;
		switch (address.sa_family) {
			case AF_INET:  result.in  = reinterpret_cast<sockaddr_in  const &>(address); break;
			case AF_INET6: result.in6 = reinterpret_cast<sockaddr_in6 const &>(address); break;
			default: result.generic = address; break;
		}
		return result;
	}
}

extern "C" int bind(int fd, sockaddr const * address, socklen_t address_length) {
	// Not an INET address or the rewrite map is empty?
	// Skip lookup and call original bind without modifying the arguments.
	if (!is_inet_addr(*address) || rewrite_map.empty()) {
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

	int port = get_port(*address);
	std::map<inet_remap::key, int>::iterator entry = rewrite_map.find(inet_remap::key(protocol, port));

	// Port was not in the map? Call original bind without modifying the arguments.
	if (entry == rewrite_map.end()) {
		if (verbose) std::cerr << "Passing through bind for " << protocol << ":" << port << ".\n";
		return original_bind(fd, address, address_length);
	}

	// Change port and pass modified address to original bind.
	address_t modified_address = copy_address(*address);
	set_port(modified_address.generic, entry->second);
	if (verbose) std::cerr << "Rebinding " << protocol << ":" << port << " to " << entry->second << " .\n";
	return original_bind(fd, &modified_address.generic, address_length);
}
