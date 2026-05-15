#pragma once

#include <hk/Result.h>

namespace mizuna_utils {

HK_RESULT_MODULE(10)
HK_DEFINE_RESULT_RANGE(MizunaUtils, 0, 100)
HK_DEFINE_RESULT(InvalidArgument, 0)
HK_DEFINE_RESULT(WriteMismatch, 1)

} // namespace mizuna_utils
