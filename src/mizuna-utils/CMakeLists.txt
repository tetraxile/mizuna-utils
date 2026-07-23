add_executable(mizuna-utils)

find_library(ZSTD_LIBRARY NAMES zstd lzstd libzstd)
target_link_libraries(mizuna-utils PRIVATE ${ZSTD_LIBRARY} LibHakkunCore argspp)

target_sources(mizuna-utils
    PRIVATE
        main.cpp
        byml.cpp
        yaz0.cpp
        bfres.cpp
        sarc.cpp
        szs.cpp
        bffnt.cpp
        bntx.cpp
        msbp.cpp
)

target_link_libraries(mizuna-utils PRIVATE mizuna)
