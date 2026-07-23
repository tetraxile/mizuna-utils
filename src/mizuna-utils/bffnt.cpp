#include "mizuna/bffnt.h"

#include <cstdio>
#include <filesystem>
#include <hk/ValueOrResult.h>
#include <hk/diag/diag.h>
#include <string>

#include "main.h"
#include "mizuna/util.h"

hk::Result handle_bffnt(s32 argc, char* argv[]) {
	if (argc < 3 || util::isEqual(argv[2], "--help")) {
		fprintf(stderr, "usage: %s bffnt r <font file>\n", programName.c_str());
		return hk::ResultInvalidArgument();
	}

	if (util::isEqual(argv[2], "read") || util::isEqual(argv[2], "r")) {
		if (argc < 4) {
			fprintf(stderr, "usage: %s bffnt r <font file>\n", programName.c_str());
			return hk::ResultInvalidArgument();
		}

		std::vector<u8> fileContents;
		HK_TRY(util::readFile(fileContents, argv[3]));

		BFFNT bffnt(fileContents);
		HK_TRY(bffnt.read());
	} else {
		fprintf(stderr, "error: unrecognized option '%s'\n", argv[2]);
		return hk::ResultInvalidArgument();
	}

	return hk::ResultSuccess();
}
