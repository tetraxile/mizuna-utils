#include "main.h"
#include "mizuna/msbp/reader.h"
#include "mizuna/msbt/reader.h"
#include "mizuna/util.h"

hk::Result print_msbt(std::string& out, const msbt::Reader& msbt) {
	out += "filetype msbt\n";
	out += std::format("version {}\n", msbt.getVersion());
	out += "endian ";
	switch (msbt.getByteOrder()) {
	case util::ByteOrder::Big: out += "big\n"; break;
	case util::ByteOrder::Little: out += "little\n"; break;
	}
	out += "encoding ";
	switch (msbt.getEncoding()) {
	case lms::Encoding::UTF8: out += "utf8"; break;
	case lms::Encoding::UTF16: out += "utf16"; break;
	case lms::Encoding::UTF32: out += "utf32"; break;
	}

	const std::map<std::string, msbt::Message*>& messages = msbt.getMessages();
	if (!messages.empty()) {
		out += "\n";
		out += "messages\n";
		for (auto [name, message] : messages)
			out += std::format("  {}: \"{}\"\n", name.c_str(), message->toString());
	}

	return hk::ResultSuccess();
}

hk::Result handle_msbt(s32 argc, char* argv[]) {
	if (argc < 3 || util::isEqual(argv[2], "--help")) {
		fprintf(stderr, "usage: %s msbt r <msbp file> <input file> [output mml file]\n", programName.c_str());
		return hk::ResultInvalidArgument();
	}

	if (util::isEqual(argv[2], "read") || util::isEqual(argv[2], "r")) {
		if (argc < 5) {
			fprintf(stderr, "usage: %s msbt r <msbp file> <input file> [output mml file]\n", programName.c_str());
			return hk::ResultInvalidArgument();
		}

		std::vector<u8> msbpContents;
		HK_TRY(util::readFile(msbpContents, argv[3]));

		msbp::Reader msbp(msbpContents);
		HK_TRY(msbp.read());

		// odyssey-specific hardcoded System tags
		HK_TRY(msbp.replaceTag(0, 0, { { "replace", lms::ParamType::U16 }, { "rt", lms::ParamType::String } }));
		HK_TRY(msbp.replaceTag(0, 2, { { "percent", lms::ParamType::U16 } }));
		HK_TRY(msbp.replaceTag(0, 3, { { "index", lms::ParamType::S16 } }));

		std::vector<u8> msbtContents;
		HK_TRY(util::readFile(msbtContents, argv[4]));

		msbt::Reader msbt(msbtContents, msbp);
		HK_TRY(msbt.read());

		std::string out;
		HK_TRY(print_msbt(out, msbt));

		if (argc < 6)
			printf("%s\n", out.c_str());
		else
			util::writeFile(argv[5], out + '\n');
	} else {
		fprintf(stderr, "error: unrecognized option '%s'\n", argv[2]);
		return hk::ResultInvalidArgument();
	}

	return hk::ResultSuccess();
}
