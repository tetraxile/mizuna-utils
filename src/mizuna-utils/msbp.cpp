#include <cstdio>
#include <filesystem>
#include <format>
#include <hk/ValueOrResult.h>
#include <hk/diag/diag.h>
#include <string>

#include "main.h"
#include "mizuna/msbp/reader.h"
#include "mizuna/msbp/results.h"
#include "mizuna/util.h"

hk::Result print_msbp(std::string& out, const msbp::Reader& msbp) {
	out += "filetype msbp\n";
	out += std::format("version {}\n", msbp.getVersion());
	out += "endian ";
	switch (msbp.getByteOrder()) {
	case util::ByteOrder::Big: out += "big\n"; break;
	case util::ByteOrder::Little: out += "little\n"; break;
	}

	const std::map<std::string, msbp::Reader::Colour>& colours = msbp.getColours();
	if (!colours.empty()) {
		out += "\n";
		out += "colours\n";
		for (auto [name, c] : colours) {
			out += std::format(
				"  {}: ({}, {}, {}, {})\n", name.c_str(), (c >> 24) & 0xff, (c >> 16) & 0xff, (c >> 8) & 0xff, c & 0xff
			);
		}
	}

	const std::vector<msbp::Reader::Attribute>& attributes = msbp.getAttributes();
	if (!attributes.empty()) {
		return msbp::ResultUnimplemented();
	}

	const std::map<u16, msbp::Reader::TagGroup>& tagGroups = msbp.getTagGroups();
	if (!tagGroups.empty()) {
		out += "\n";
		out += "tags\n";
		for (const auto& [id, group] : tagGroups) {
			out += std::format("  {}: {}\n", id, group.name.c_str());
			for (const auto& tag : group.tags) {
				out += std::format("    {}\n", tag.name.c_str());
				for (const auto& param : tag.params) {
					std::string typeStr;
					if (param.type == lms::ParamType::U8)
						typeStr = "u8";
					else if (param.type == lms::ParamType::U16)
						typeStr = "u16";
					else if (param.type == lms::ParamType::S16)
						typeStr = "s16";
					else if (param.type == lms::ParamType::U32)
						typeStr = "u32";
					else if (param.type == lms::ParamType::F32)
						typeStr = "f32";
					else if (param.type == lms::ParamType::String)
						typeStr = "string";
					else if (param.type == lms::ParamType::Null)
						typeStr = "null";
					else {
						fprintf(stderr, "unimplemented param type `%#x`\n", (u8)param.type);
						return msbp::ResultInvalidParamType();
					}
					out += std::format("      {} {}\n", typeStr.c_str(), param.name.c_str());

					for (const auto& string : param.strings) {
						out += std::format("        {}\n", string.c_str());
					}
				}
			}
		}
	}

	const std::map<std::string, msbp::Reader::Style>& styles = msbp.getStyles();
	if (!styles.empty()) {
		out += "\n";
		out += "styles\n";
		for (const auto& [name, style] : styles) {
			out += std::format("  {}\n", name.c_str());
			out += std::format("    width: {}\n", style.regionWidth);
			out += std::format("    lines: {}\n", style.lineNum);
			out += std::format("    font: {}\n", style.fontIndex);
			out += std::format("    base colour: {}\n", style.baseColourIndex);
		}
	}

	const std::vector<std::string>& filenames = msbp.getFilenames();
	if (!filenames.empty()) {
		out += "\n";
		out += "filenames\n";
		for (const std::string& filename : filenames)
			out += std::format("  {}\n", filename.c_str());
	}

	return hk::ResultSuccess();
}

hk::Result handle_msbp(s32 argc, char* argv[]) {
	if (argc < 3 || util::isEqual(argv[2], "--help")) {
		fprintf(stderr, "usage: %s msbp r <project file> [output mml file]\n", programName.c_str());
		return hk::ResultInvalidArgument();
	}

	if (util::isEqual(argv[2], "read") || util::isEqual(argv[2], "r")) {
		if (argc < 4) {
			fprintf(stderr, "usage: %s msbp r <project file> [output mml file]\n", programName.c_str());
			return hk::ResultInvalidArgument();
		}

		std::vector<u8> fileContents;
		HK_TRY(util::readFile(fileContents, argv[3]));

		msbp::Reader msbp(fileContents);
		HK_TRY(msbp.read());

		std::string out;
		HK_TRY(print_msbp(out, msbp));

		if (argc < 5)
			printf("%s\n", out.c_str());
		else
			util::writeFile(argv[4], out + '\n');
	} else {
		fprintf(stderr, "error: unrecognized option '%s'\n", argv[2]);
		return hk::ResultInvalidArgument();
	}

	return hk::ResultSuccess();
}
