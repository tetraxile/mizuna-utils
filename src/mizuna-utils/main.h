#pragma once

#include <hk/ValueOrResult.h>
#include <hk/types.h>
#include <string>

extern std::string programName;

hk::Result handle_byml(s32 argc, char* argv[]);
hk::Result handle_yaz0(s32 argc, char* argv[]);
hk::Result handle_bfres(s32 argc, char* argv[]);
hk::Result handle_sarc(s32 argc, char* argv[]);
hk::Result handle_szs(s32 argc, char* argv[]);
hk::Result handle_bffnt(s32 argc, char* argv[]);
hk::Result handle_bntx(s32 argc, char* argv[]);
hk::Result handle_msbp(s32 argc, char* argv[]);
