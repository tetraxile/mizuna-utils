#include <cstdio>
#include <cstdlib>
#include <format>
#include <hk/ValueOrResult.h>
#include <hk/diag/diag.h>
#include <optional>
#include <string>

#include "main.h"
#include "mizuna/byml/reader.h"
#include "mizuna/byml/results.h"
#include "mizuna/byml/writer.h"
#include "mizuna/results.h"
#include "mizuna/util.h"
#include "results.h"

using namespace mizuna_utils;

hk::Result print_byml_node(std::string& out, const byml::Reader& node, s32 level = 0) {
	std::string indent;
	for (s32 i = 0; i < level; i++)
		indent += "  ";

	if (HK_TRY(node.getType()) == byml::NodeType::Array) {
		out += "array [";
		for (u32 i = 0; i < node.getSize(); i++) {
			out += "\n";
			byml::NodeType childType = HK_TRY(node.getTypeByIdx(i));
			out += "  ";

			switch (childType) {
			case byml::NodeType::Array: {
				byml::Reader container = HK_TRY(node.getContainerByIdx(i));
				out += indent;
				HK_TRY(print_byml_node(out, container, level + 1));
				break;
			}
			case byml::NodeType::Hash: {
				byml::Reader container = HK_TRY(node.getContainerByIdx(i));
				out += indent;
				HK_TRY(print_byml_node(out, container, level + 1));
				break;
			}
			case byml::NodeType::String: {
				std::string str;
				HK_TRY(node.getStringByIdx(&str, i));
				out += std::format("{}string \"{}\"", indent, str);
				break;
			}
			case byml::NodeType::Binary: {
				std::vector<u8> value;
				u32 alignment;
				HK_TRY(node.getBinaryByIdx(&value, &alignment, i));
				out += std::format("{}binary `", indent);
				for (u8 byte : value)
					out += std::format("{:02x}", byte);
				out += '`';
				break;
			}
			case byml::NodeType::BinaryAlignment: {
				std::vector<u8> value;
				u32 alignment;
				HK_TRY(node.getBinaryByIdx(&value, &alignment, i));
				out += std::format("{}binary % {:#x} `", indent, alignment);
				for (u8 byte : value)
					out += std::format("{:02x}", byte);
				out += '`';
				break;
			}
			case byml::NodeType::Bool: {
				bool value = HK_TRY(node.getBoolByIdx(i));
				out += std::format("{}bool {}", indent, value);
				break;
			}
			case byml::NodeType::S32: {
				s32 value = HK_TRY(node.getS32ByIdx(i));
				out += std::format("{}s32 {}", indent, value);
				break;
			}
			case byml::NodeType::F32: {
				f32 value = HK_TRY(node.getF32ByIdx(i));
				out += std::format("{}f32 {}", indent, value);
				break;
			}
			case byml::NodeType::U32: {
				u32 value = HK_TRY(node.getU32ByIdx(i));
				out += std::format("{}u32 {}", indent, value);
				break;
			}
			case byml::NodeType::S64: {
				s64 value = HK_TRY(node.getS64ByIdx(i));
				out += std::format("{}s64 {}", indent, value);
				break;
			}
			case byml::NodeType::U64: {
				u64 value = HK_TRY(node.getU64ByIdx(i));
				out += std::format("{}u64 {}", indent, value);
				break;
			}
			case byml::NodeType::F64: {
				f64 value = HK_TRY(node.getF64ByIdx(i));
				out += std::format("{}f64 {}", indent, value);
				break;
			}
			case byml::NodeType::Null: {
				out += std::format("{}null", indent);
				break;
			}
			case byml::NodeType::StringTable: {
				fprintf(stderr, "error: string table node can't be in tree\n");
				return byml::ResultInvalidNodeType();
			}
			}

			if (i == node.getSize() - 1) out += std::format("\n{}", indent);
		}
		out += "]";
	} else if (HK_TRY(node.getType()) == byml::NodeType::Hash) {
		out += "hash {";
		for (u32 i = 0; i < node.getSize(); i++) {
			out += "\n";
			byml::NodeType childType = HK_TRY(node.getTypeByIdx(i));
			std::string key;
			HK_TRY(node.getKeyByIdx(&key, i));
			out += std::format("  {}{}: ", indent, key);

			switch (childType) {
			case byml::NodeType::Array: {
				byml::Reader container = HK_TRY(node.getContainerByIdx(i));
				HK_TRY(print_byml_node(out, container, level + 1));
				break;
			}
			case byml::NodeType::Hash: {
				byml::Reader container = HK_TRY(node.getContainerByIdx(i));
				HK_TRY(print_byml_node(out, container, level + 1));
				break;
			}
			case byml::NodeType::String: {
				std::string str;
				HK_TRY(node.getStringByIdx(&str, i));
				out += std::format("string \"{}\"", str);
				break;
			}
			case byml::NodeType::Binary: {
				std::vector<u8> value;
				u32 alignment;
				HK_TRY(node.getBinaryByIdx(&value, &alignment, i));
				out += "binary `";
				for (u8 byte : value)
					out += std::format("{:02x}", byte);
				out += '`';
				break;
			}
			case byml::NodeType::BinaryAlignment: {
				std::vector<u8> value;
				u32 alignment;
				HK_TRY(node.getBinaryByIdx(&value, &alignment, i));
				out += std::format("binary % {:#x} `", alignment);
				for (u8 byte : value)
					out += std::format("{:02x}", byte);
				out += '`';
				break;
			}
			case byml::NodeType::Bool: {
				bool value = HK_TRY(node.getBoolByIdx(i));
				out += std::format("bool {}", value);
				break;
			}
			case byml::NodeType::S32: {
				s32 value = HK_TRY(node.getS32ByIdx(i));
				out += std::format("s32 {}", value);
				break;
			}
			case byml::NodeType::F32: {
				f32 value = HK_TRY(node.getF32ByIdx(i));
				out += std::format("f32 {}", value);
				break;
			}
			case byml::NodeType::U32: {
				u32 value = HK_TRY(node.getU32ByIdx(i));
				out += std::format("u32 {}", value);
				break;
			}
			case byml::NodeType::S64: {
				s64 value = HK_TRY(node.getS64ByIdx(i));
				out += std::format("s64 {}", value);
				break;
			}
			case byml::NodeType::U64: {
				u64 value = HK_TRY(node.getU64ByIdx(i));
				out += std::format("u64 {}", value);
				break;
			}
			case byml::NodeType::F64: {
				f64 value = HK_TRY(node.getF64ByIdx(i));
				out += std::format("f64 {}", value);
				break;
			}
			case byml::NodeType::Null: {
				out += "null";
				break;
			}
			case byml::NodeType::StringTable: {
				fprintf(stderr, "error: string table node can't be in tree\n");
				return byml::ResultInvalidNodeType();
			}
			}

			if (i == node.getSize() - 1) out += std::format("\n{}", indent);
		}
		out += "}";
	}

	return hk::ResultSuccess();
}

hk::Result print_byml(std::string& out, const byml::Reader& node) {
	out += "filetype byml\n";
	out += std::format("version {}\n", node.getVersion());
	out += "endian ";
	switch (node.getByteOrder()) {
	case util::ByteOrder::Big: out += "big\n"; break;
	case util::ByteOrder::Little: out += "little\n"; break;
	}
	out += "root ";

	return print_byml_node(out, node);
}

bool is_whitespace(char c) {
	return c == ' ' || c == '\t';
}

bool is_num(char c, s32 radix = 10) {
	if (radix == 2) {
		return c == '0' || c == '1';
	} else if (radix == 8) {
		return c >= '0' && c <= '7';
	} else if (radix == 10) {
		return c >= '0' && c <= '9';
	} else if (radix == 16) {
		char c2 = c | 0x20;
		return (c >= '0' && c <= '9') || (c2 >= 'a' && c2 <= 'f');
	} else {
		return false;
	}
}

bool is_alpha(char c) {
	c |= 0x20;
	return c >= 'a' && c <= 'z';
}

bool is_alnum(char c) {
	return is_num(c) || is_alpha(c);
}

hk::Result parse_mml(byml::Writer& writer, const std::string& contents) {
	std::string tmp;
	std::string key;
	size_t idx = 0;
	size_t line = 1;

	std::optional<byml::NodeType> containerType = std::nullopt;
	std::vector<byml::NodeType> containerStack;
	std::optional<byml::NodeType> childNodeType = std::nullopt;
	std::optional<util::ByteOrder> byteOrder = std::nullopt;
	std::optional<byml::Writer::Version> version = std::nullopt;
	bool seenFiletype = false;

	enum { START, FILETYPE, VERSION, ENDIAN, ROOT } state = START;

	auto logError = [](size_t line, const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);
		fprintf(stderr, "parse error on line %zu: ", line);
		vfprintf(stderr, fmt, args);
		fprintf(stderr, "\n");
		va_end(args);
	};

	while (true) {
		if (idx >= contents.size()) break;

		char c = contents[idx];

		if (is_alpha(c)) {
			// printf("ALNUM\n");

			while (tmp += c, c = contents[++idx], is_alnum(c))
				;

			// printf("  '%s'\n", tmp.c_str());

			if (state == START) {
				if (tmp == "filetype")
					state = FILETYPE;
				else if (tmp == "version")
					state = VERSION;
				else if (tmp == "endian")
					state = ENDIAN;
				else if (tmp == "root") {
					state = ROOT;
					if (!seenFiletype) {
						logError(line, "no file type given (`filetype` line)");
						return ResultWrongFiletype();
					}
					if (!version.has_value()) {
						logError(line, "no BYML version given (`version` line)");
						return ResultMissingVersion();
					}
					if (!byteOrder.has_value()) {
						logError(line, "no byte order given (`endian` line)");
						return ResultMissingByteOrder();
					}
					writer.init(version.value());
				} else {
					logError(line, "unexpected name in header '%s'\n", tmp.c_str());
					return ResultParseError();
				}
			} else if (state == FILETYPE) {
				if (tmp == "byml")
					seenFiletype = true;
				else {
					logError(line, "filetype must be `byml`");
					return ResultWrongFiletype();
				}
			} else if (state == ENDIAN) {
				if (tmp == "little")
					byteOrder = util::ByteOrder::Little;
				else if (tmp == "big")
					byteOrder = util::ByteOrder::Big;
				else {
					logError(line, "endian must be `big` or `little`");
					return mizuna::ResultBadByteOrder();
				}
			} else if (state == ROOT) {
				if (tmp == "hash") {
					if (!containerStack.empty() && containerType == byml::NodeType::Hash && key.empty()) {
						logError(line, "expected key for hash node");
						return ResultParseError();
					}
					if (containerType.has_value()) containerStack.push_back(containerType.value());
					containerType = byml::NodeType::Hash;
				} else if (tmp == "array") {
					if (!containerStack.empty() && containerType == byml::NodeType::Hash && key.empty()) {
						logError(line, "expected key for array node");
						return ResultParseError();
					}
					if (containerType.has_value()) containerStack.push_back(containerType.value());
					containerType = byml::NodeType::Array;
				} else if (tmp == "null") {
					// if root node is null then stop parsing
					if (!containerType.has_value()) break;

					if (containerType.value() == byml::NodeType::Array)
						HK_TRY(writer.addNull());
					else if (!key.empty()) {
						HK_TRY(writer.addNull(key));
						key.clear();
					} else {
						logError(line, "expected key for null node\n");
						return ResultParseError();
					}
					childNodeType.reset();
				} else if (!containerType.has_value()) {
					logError(line, "unexpected name '%s'\n", tmp.c_str());
					return ResultParseError();
				} else if (tmp == "string") {
					if (!containerStack.empty() && containerType == byml::NodeType::Hash && key.empty()) {
						logError(line, "expected key for string node");
						return ResultParseError();
					}
					childNodeType = byml::NodeType::String;
				} else if (tmp == "f32") {
					if (!containerStack.empty() && containerType == byml::NodeType::Hash && key.empty()) {
						logError(line, "expected key for f32 node");
						return ResultParseError();
					}
					childNodeType = byml::NodeType::F32;
				} else if (tmp == "f64") {
					if (!containerStack.empty() && containerType == byml::NodeType::Hash && key.empty()) {
						logError(line, "expected key for f64 node");
						return ResultParseError();
					}
					childNodeType = byml::NodeType::F64;
				} else if (tmp == "s32") {
					if (!containerStack.empty() && containerType == byml::NodeType::Hash && key.empty()) {
						logError(line, "expected key for s32 node");
						return ResultParseError();
					}
					childNodeType = byml::NodeType::S32;
				} else if (tmp == "u32") {
					if (!containerStack.empty() && containerType == byml::NodeType::Hash && key.empty()) {
						logError(line, "expected key for u32 node");
						return ResultParseError();
					}
					childNodeType = byml::NodeType::U32;
				} else if (tmp == "s64") {
					if (!containerStack.empty() && containerType == byml::NodeType::Hash && key.empty()) {
						logError(line, "expected key for s64 node");
						return ResultParseError();
					}
					childNodeType = byml::NodeType::S64;
				} else if (tmp == "u64") {
					if (!containerStack.empty() && containerType == byml::NodeType::Hash && key.empty()) {
						logError(line, "expected key for u64 node");
						return ResultParseError();
					}
					childNodeType = byml::NodeType::U64;
				} else if (tmp == "binary") {
					if (!containerStack.empty() && containerType == byml::NodeType::Hash && key.empty()) {
						logError(line, "expected key for binary node");
						return ResultParseError();
					}
					childNodeType = byml::NodeType::Binary;
				} else if (tmp == "bool") {
					if (!containerStack.empty() && containerType == byml::NodeType::Hash && key.empty()) {
						logError(line, "expected key for bool node");
						return ResultParseError();
					}
					childNodeType = byml::NodeType::Bool;
				} else if (childNodeType == byml::NodeType::Bool) {
					if (!containerType.has_value() || key.empty()) {
						logError(line, "unrecognised value for bool '%s'\n", tmp.c_str());
						return ResultParseError();
					}

					bool value;
					if (tmp == "true")
						value = true;
					else if (tmp == "false")
						value = false;
					else {
						logError(line, "invalid value for bool");
						return ResultParseError();
					}

					if (containerType == byml::NodeType::Array)
						writer.addBool(value);
					else {
						writer.addBool(key, value);
						key.clear();
					}
					childNodeType.reset();
				} else {
					// printf("  '%s' (container: %02x)\n", tmp.c_str(), containerType.value());
					if (contents[idx++] != ':') {
						logError(line, "expected colon after key");
						return ResultParseError();
					}
					key = tmp;
				}
			} else {
				logError(line, "unrecognised name '%s'\n", tmp.c_str());
				return ResultParseError();
			}

			tmp.clear();
		} else if (is_whitespace(c)) {
			// printf("WHITESPACE\n");
			while (is_whitespace(contents[++idx]))
				;
		} else if (is_num(c) || c == '-') {
			// printf("NUMBER\n");

			bool isNegative = c == '-';
			if (isNegative) idx++;

			s32 radix = 10;
			if (c == '0') {
				char c2 = contents[idx + 1];
				if (c2 == 'b')
					radix = 2;
				else if (c2 == 'o')
					radix = 8;
				else if (c2 == 'x')
					radix = 16;

				if (radix != 10) c = c2;
			}

			bool hasSeparator = false;
			while (true) {
				c = contents[idx];
				if (radix == 10 && is_num(c)) {
					tmp += c;
				} else if (radix == 8 && is_num(c, 8)) {
					tmp += c;
				} else if (radix == 2 && is_num(c, 2)) {
					tmp += c;
				} else if (radix == 16 && is_num(c, 16)) {
					tmp += c;
				} else if (c == '.' || c == ',') {
					if (hasSeparator) break;
					hasSeparator = true;
				} else {
					break;
				}
				idx++;
			}

			// printf("  `%s` ('%c')\n", tmp.c_str(), contents[idx]);

			if (state == VERSION) {
				u32 num = std::stoul(tmp, 0, radix);
				// printf("  version = %u\n", num);
				version = (byml::Writer::Version)num;
				tmp.clear();
				continue;
			} else if (state != ROOT) {
				logError(line, "unexpected number");
				return ResultParseError();
			}

			if (!containerType.has_value()) {
				logError(line, "no container for value node");
				return ResultParseError();
			}

			if (key.empty()) {
				logError(line, "unexpected number");
				return ResultParseError();
			}

			if (childNodeType == byml::NodeType::S32) {
				if (isNegative) tmp.insert(0, 1, '-');
				s32 value = std::stoi(tmp, 0, radix);
				if (containerType == byml::NodeType::Array) {
					HK_TRY(writer.addS32(value));
				} else {
					HK_TRY(writer.addS32(key, value));
					key.clear();
				}
				// printf("  `%d` (%u)\n", value, radix);
			} else if (childNodeType == byml::NodeType::S64) {
				if (isNegative) tmp.insert(0, 1, '-');
				s64 value = std::stol(tmp, 0, radix);
				if (containerType == byml::NodeType::Array) {
					HK_TRY(writer.addS64(value));
				} else {
					HK_TRY(writer.addS64(key, value));
					key.clear();
				}
				// printf("  `%ld` (%u)\n", value, radix);
			} else if (childNodeType == byml::NodeType::U32) {
				if (isNegative) {
					logError(line, "unsigned integer cannot be negative");
					return ResultParseError();
				}
				u32 value = std::stoul(tmp, 0, radix);
				if (containerType == byml::NodeType::Array) {
					HK_TRY(writer.addU32(value));
				} else {
					HK_TRY(writer.addU32(key, value));
					key.clear();
				}
				// printf("  `%u` (%u)\n", value, radix);
			} else if (childNodeType == byml::NodeType::U64) {
				if (isNegative) {
					logError(line, "unsigned integer cannot be negative");
					return ResultParseError();
				}
				u64 value = std::stoull(tmp, 0, radix);
				if (containerType == byml::NodeType::Array) {
					HK_TRY(writer.addU64(value));
				} else {
					HK_TRY(writer.addU64(key, value));
					key.clear();
				}
				// printf("  `%lu` (%u)\n", value, radix);
			} else if (childNodeType == byml::NodeType::F32) {
				if (isNegative) tmp.insert(0, 1, '-');
				// printf("%s\n", tmp.c_str());
				if (radix != 10) {
					logError(line, "floats only support base 10");
					return ResultParseError();
				}

				f32 value = std::stof(tmp);
				if (containerType == byml::NodeType::Array) {
					HK_TRY(writer.addF32(value));
				} else {
					HK_TRY(writer.addF32(key, value));
					key.clear();
				}
				// printf("  `%f`\n", value);
			} else if (childNodeType == byml::NodeType::F64) {
				if (isNegative) tmp.insert(0, 1, '-');
				if (radix != 10) {
					logError(line, "floats only support base 10");
					return ResultParseError();
				}

				f64 value = std::stod(tmp);
				if (containerType == byml::NodeType::Array) {
					HK_TRY(writer.addF64(value));
				} else {
					HK_TRY(writer.addF64(key, value));
					key.clear();
				}
				// printf("  `%f`\n", value);
			} else {
				logError(line, "unexpected number");
				return ResultParseError();
			}

			childNodeType.reset();
			tmp.clear();
		} else if (c == '\n') {
			// printf("NEWLINE\n");
			idx++;
			line++;
			if (state == VERSION || state == ENDIAN) state = START;
			if (!key.empty()) {
				logError(line, "expected value for key '%s'", key.c_str());
				return ResultParseError();
			}
			childNodeType.reset();
		} else if (c == '{') {
			// printf("LBRACE\n");
			idx++;
			if (state != ROOT || containerType != byml::NodeType::Hash) {
				logError(line, "unexpected character '{'\n");
				return ResultParseError();
			}

			if (containerStack.empty() || containerStack.back() == byml::NodeType::Array) {
				HK_TRY(writer.pushHash());
			} else if (!key.empty()) {
				HK_TRY(writer.pushHash(key));
				key.clear();
			} else {
				HK_ABORT("unreachable");
			}
		} else if (c == '}') {
			// printf("RBRACE\n");
			idx++;
			if (state != ROOT || containerType != byml::NodeType::Hash) {
				logError(line, "unexpected character '}'\n");
				return ResultParseError();
			}
			HK_TRY(writer.pop());
			if (containerStack.empty()) break;

			// printf("  ");
			for (byml::NodeType& nodeType : containerStack)
				// printf("%02x ", (u32)nodeType);
				// printf("\n");

				containerType = containerStack.back();
			containerStack.pop_back();
		} else if (c == '[') {
			// printf("LBRACKET\n");
			idx++;
			if (state != ROOT || containerType != byml::NodeType::Array) {
				logError(line, "unexpected character '['\n");
				return ResultParseError();
			}
			if (containerStack.empty() || containerStack.back() == byml::NodeType::Array) {
				HK_TRY(writer.pushArray());
			} else if (!key.empty()) {
				HK_TRY(writer.pushArray(key));
				key.clear();
			} else {
				HK_ABORT("unreachable");
			}
			childNodeType.reset();
		} else if (c == ']') {
			// printf("RBRACKET\n");
			idx++;
			if (state != ROOT || containerType != byml::NodeType::Array) {
				logError(line, "unexpected character ']'\n");
				return ResultParseError();
			}
			HK_TRY(writer.pop());
			if (containerStack.empty()) break;

			// printf("  ");
			for (byml::NodeType& nodeType : containerStack)
				// printf("%02x ", (u32)nodeType);
				// printf("\n");

				containerType = containerStack.back();
			containerStack.pop_back();
		} else if (c == '"') {
			// printf("QUOTE\n");
			if (!containerType.has_value()) {
				logError(line, "no container for value node");
				return ResultParseError();
			}

			std::string str;
			size_t startLine = line;
			while (true) {
				if (idx >= contents.size()) {
					logError(startLine, "string without matching closing quote");
					return ResultParseError();
				}
				c = contents[++idx];
				if (c == '\\') {
					c = contents[++idx];
					if (c == '\\') {
						str += c;
					} else if (c == 'n') {
						str += '\n';
					} else if (c == 't') {
						str += '\t';
					} else if (c == '\n') {
						line++;
					} else {
						logError(line, "invalid escape character '%c'", c);
						return ResultParseError();
					}
				} else if (c == '"') {
					break;
				} else {
					str += c;
				}
			}

			if (containerType == byml::NodeType::Array) {
				HK_TRY(writer.addString(str));
			} else if (!key.empty()) {
				HK_TRY(writer.addString(key, str));
				key.clear();
			} else {
				logError(line, "expected key for string node");
				return ResultParseError();
			}

			// printf("  str `%s`\n", str.c_str());
			childNodeType.reset();
			idx++;
		} else if (c == ',') {
			// ignore any commas
			idx++;
		} else {
			logError(line, "unexpected character '%c'", c);
			return ResultParseError();
		}
	}

	return hk::ResultSuccess();
}

hk::Result handle_byml(s32 argc, char* argv[]) {
	if (argc < 3 || util::isEqual(argv[2], "--help")) {
		fprintf(stderr, "usage: %s byml r <input file> [output mml file]\n", programName.c_str());
		fprintf(stderr, "       %s byml w <input file> <output byml file>\n", programName.c_str());
		return hk::ResultInvalidArgument();
	}

	if (util::isEqual(argv[2], "read") || util::isEqual(argv[2], "r")) {
		if (argc < 4) {
			fprintf(stderr, "usage: %s byml r <input file> [output mml file]\n", programName.c_str());
			return hk::ResultInvalidArgument();
		}

		std::vector<u8> fileContents;
		HK_TRY(util::readFile(fileContents, argv[3]));

		byml::Reader byml;
		HK_TRY(byml.init(fileContents.data(), fileContents.size()));

		std::string out;
		HK_TRY(print_byml(out, byml));

		if (argc < 5)
			printf("%s\n", out.c_str());
		else
			util::writeFile(argv[4], out + '\n');
	} else if (util::isEqual(argv[2], "write") || util::isEqual(argv[2], "w")) {
		if (argc < 5) {
			fprintf(stderr, "usage: %s byml w <input file> <output byml file>\n", programName.c_str());
			return hk::ResultInvalidArgument();
		}

		std::string fileContents;
		HK_TRY(util::readFile(fileContents, argv[3]));

		byml::Writer writer;
		HK_TRY(parse_mml(writer, fileContents));

		writer.save(argv[4]);
	} else {
		fprintf(stderr, "error: unrecognized option '%s'\n", argv[2]);
		return hk::ResultInvalidArgument();
	}

	return hk::ResultSuccess();
}
