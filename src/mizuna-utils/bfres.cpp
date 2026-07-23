#include <cstdio>
#include <filesystem>
#include <hk/ValueOrResult.h>
#include <hk/diag/diag.h>
#include <string>

#include "main.h"
#include "mizuna/bfres/reader.h"
#include "mizuna/util.h"

hk::Result handle_bfres(s32 argc, char* argv[]) {
	if (argc < 3 || util::isEqual(argv[2], "--help")) {
		fprintf(stderr, "usage: %s bfres r <input file>\n", programName.c_str());
		fprintf(stderr, "       %s bfres w <input file>\n", programName.c_str());
		return hk::ResultInvalidArgument();
	}

	if (util::isEqual(argv[2], "read") || util::isEqual(argv[2], "r")) {
		if (argc < 5) {
			fprintf(stderr, "usage: %s bfres r <input file> <output file>\n", programName.c_str());
			return hk::ResultInvalidArgument();
		}

		std::vector<u8> fileContents;
		HK_TRY(util::readFile(fileContents, argv[3]));

		bfres::Reader bfres(fileContents);
		HK_TRY(bfres.read());

		HK_TRY(bfres.exportGLTF(argv[4]));
	} else if (util::isEqual(argv[2], "write") || util::isEqual(argv[2], "w")) {
		if (argc < 4) {
			fprintf(stderr, "usage: %s bfres w <input file>\n", programName.c_str());
			return hk::ResultInvalidArgument();
		}
	} else {
		fprintf(stderr, "error: unrecognized option '%s'\n", argv[2]);
		return hk::ResultInvalidArgument();
	}

	return hk::ResultSuccess();
}
