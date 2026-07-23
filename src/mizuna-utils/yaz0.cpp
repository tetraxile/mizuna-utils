#include "mizuna/yaz0.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <hk/ValueOrResult.h>
#include <hk/diag/diag.h>
#include <iostream>
#include <string>

#include "main.h"
#include "mizuna/results.h"
#include "mizuna/util.h"

hk::Result handle_yaz0(s32 argc, char* argv[]) {
	if (argc < 3 || util::isEqual(argv[2], "--help")) {
		fprintf(stderr, "usage: %s yaz0 r <compressed file> <decompressed file>\n", programName.c_str());
		fprintf(stderr, "       %s yaz0 w <decompressed file> <compressed file> [alignment]\n", programName.c_str());
		fprintf(stderr, "       %*s         (default alignment: 0x80)\n", (s32)programName.length(), "");
		return hk::ResultInvalidArgument();
	}

	if (util::isEqual(argv[2], "read") || util::isEqual(argv[2], "r")) {
		if (argc < 5) {
			fprintf(stderr, "usage: %s yaz0 r <compressed file> <decompressed file>\n", programName.c_str());
			return hk::ResultInvalidArgument();
		}

		std::vector<u8> fileContents;
		HK_TRY(util::readFile(fileContents, argv[3]));

		std::vector<u8> outputBuffer;
		HK_TRY(yaz0::decompress(outputBuffer, fileContents));

		std::ofstream outfile(argv[4], std::ios::out | std::ios::binary);
		outfile.write(reinterpret_cast<const char*>(outputBuffer.data()), outputBuffer.size());
	} else if (util::isEqual(argv[2], "write") || util::isEqual(argv[2], "w")) {
		if (argc < 5) {
			fprintf(
				stderr, "usage: %s yaz0 w <decompressed file> <compressed file> [alignment]\n", programName.c_str()
			);
			return hk::ResultInvalidArgument();
		}

		fs::path inFile = argv[3];
		if (!fs::is_regular_file(inFile)) {
			return mizuna::ResultFileNotFound();
		}

		u32 alignment;
		if (argc > 5) {
			std::string alignmentStr = argv[5];
			s32 radix = alignmentStr.starts_with("0x") ? 16 : 10;
			alignment = std::stoul(alignmentStr, 0, radix);
		} else {
			alignment = 0x80;
		}

		std::vector<u8> fileContents;
		HK_TRY(util::readFile(fileContents, inFile));

		std::vector<u8> outputBuffer;
		yaz0::compress(outputBuffer, fileContents, alignment);

		util::writeFile(argv[4], outputBuffer);
	} else {
		fprintf(stderr, "error: unrecognized option '%s'\n", argv[2]);
		return hk::ResultInvalidArgument();
	}

	return hk::ResultSuccess();
}
