find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_HIREDIS REQUIRED IMPORTED_TARGET hiredis)

if(NOT TARGET hiredis::hiredis)
    add_library(hiredis::hiredis ALIAS PkgConfig::PC_HIREDIS)
endif()

set(hiredis_FOUND TRUE)