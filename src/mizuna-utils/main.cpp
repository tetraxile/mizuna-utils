#include "main.h"

#include <cstdio>
#include <filesystem>
#include <hk/ValueOrResult.h>
#include <hk/diag/diag.h>
#include <string>
#include <zstd/zstd.h>

#include "mizuna/results.h"
#include "mizuna/util.h"

namespace fs = std::filesystem;

std::string programName;

s32 main(s32 argc, char* argv[]) {
	programName = "./" + fs::path(argv[0]).filename().string();

	if (argc < 2) {
		fprintf(stderr, "usage: %s <format> <options...>\n", programName.c_str());
		fprintf(stderr, "\tformats: yaz0, sarc, szs, byml, msbp\n");
		fprintf(stderr, "\nrun `%s <format> --help` for more info on a specific format\n", programName.c_str());
		return 1;
	}

	hk::Result r;

	if (util::isEqual(argv[1], "yaz0"))
		r = handle_yaz0(argc, argv);
	else if (util::isEqual(argv[1], "sarc"))
		r = handle_sarc(argc, argv);
	else if (util::isEqual(argv[1], "szs"))
		r = handle_szs(argc, argv);
	else if (util::isEqual(argv[1], "bffnt"))
		r = handle_bffnt(argc, argv);
	else if (util::isEqual(argv[1], "bntx"))
		r = handle_bntx(argc, argv);
	else if (util::isEqual(argv[1], "byml"))
		r = handle_byml(argc, argv);
	else if (util::isEqual(argv[1], "bfres"))
		r = handle_bfres(argc, argv);
	else if (util::isEqual(argv[1], "msbp"))
		r = handle_msbp(argc, argv);
	else {
		fprintf(stderr, "error: unrecognized format '%s'\n\n", argv[1]);
		fprintf(stderr, "usage: %s <format> <options...>\n", argv[0]);
		fprintf(stderr, "\tformats: yaz0, sarc, szs, byml, msbp\n");
		return 1;
	}

	if (r == hk::ResultInvalidArgument() || r == mizuna::ResultUnimplementedVersion()) return 1;

	if (r.failed()) fprintf(stderr, "error: %s\n", hk::diag::getResultName(r));

	return 0;
}
