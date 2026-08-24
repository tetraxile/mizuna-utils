#pragma once

#include <hk/Result.h>

namespace mizuna_utils {

HK_RESULT_MODULE(10)
HK_DEFINE_RESULT_RANGE(MizunaUtils, 0, 100)
HK_DEFINE_RESULT(InvalidArgument, 0)
HK_DEFINE_RESULT(WriteMismatch, 1)
HK_DEFINE_RESULT(ParseError, 2)
HK_DEFINE_RESULT(MissingVersion, 3)
HK_DEFINE_RESULT(MissingByteOrder, 4)
HK_DEFINE_RESULT(WrongFiletype, 5)
HK_DEFINE_RESULT(DecompressionFailed, 6)

} // namespace mizuna_utils
