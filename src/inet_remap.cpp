#include "parse.hpp"

#include <vector>
#include <map>
#include <iostream>

extern "C" {
#include <unistd.h>
#include <netinet/in.h>
}

namespace {
	void usage(char const * name) {
		std::cerr
			<< "usage: " << name << " [options] command [command args]\n"
			<< "\n"
			<< "Options:\n"
			<< "  -b protocol:old_port:new_port    Add a bind remap\n"
			<< "  -v                               Print verbose messages\n"
		;
	}

	char const * protocolToString(int protocol) {
		switch (protocol) {
			case IPPROTO_TCP: return "tcp";
			case IPPROTO_UDP: return "tcp";
		}

		// TODO: Maybe we should just throw when the protocol is unknown.
		return "unk";
	}

	std::string remapsToString(std::map<inet_remap::key, int> const & remappings) {
		std::string result;
		result.reserve(remappings.size() * 18); // Heuristic guess.
		for (auto const & entry : remappings) {
			result.append(protocolToString(entry.first.protocol));
			result.push_back(':');
			result.append(std::to_string(entry.first.port));
			result.push_back(':');
			result.append(std::to_string(entry.second));
			result.push_back(',');
		}
		return result;
	}
}


int main(int argc, char * * argv) {
	if (argc < 2) {
		usage(argv[0]);
		return 0;
	}

	bool verbose = false;
	std::map<inet_remap::key, int> remaps;


	// Parse command line options.
	while (true) {
		int option = getopt(argc, argv, "+b:v");
		if (option == -1) break;
		switch (option) {
		case 'b':
			try {
				remaps.insert(inet_remap::parseEntry(optarg));
			} catch (std::exception const & e) {
				std::cerr << e.what() << std::endl;
				return -1;
			}
			break;

		case 'v':
			verbose = true;
			break;

		case ':':
		case '?':
			usage(argv[0]);
			return 1;
		}
	}

	if (optind >= argc) {
		std::cerr << argv[0] << ": no command specified.\n\n";
		usage(argv[0]);
		return 1;
	}

	// Set environment for preloaded library.
	if (verbose) setenv("INET_REMAP_VERBOSE", "1", true);
	setenv("INET_REMAP", remapsToString(remaps).c_str(), true);

	// Determine new LD_PRELOAD value.
	char const * ld_preload_env = getenv("LD_PRELOAD");
	std::string ld_preload = std::string(PRELOAD_PATH);
	if (ld_preload_env && ld_preload_env[0]) {
		ld_preload.push_back(':');
		ld_preload.append(ld_preload_env);
	}

	// Set LD_PRELOAD and run the command.
	setenv("LD_PRELOAD", ld_preload.c_str(), true);
	execvp(argv[optind], &argv[optind]);
}
