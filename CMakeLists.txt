cmake_minimum_required(VERSION 3.16)

project(calculator LANGUAGES CXX)


find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED dbus-1)

include(FetchContent)

FetchContent_Declare(
    libmath
    GIT_REPOSITORY git@github.com:DanilDill/libmath.git
    GIT_TAG master
)

FetchContent_MakeAvailable(libmath)

add_executable(calculator
    src/app.cpp
    src/main.cpp
)

target_include_directories(calculator PRIVATE
    ${DBUS_INCLUDE_DIRS}
)
target_link_libraries(calculator
    PRIVATE
        math
        ${DBUS_LIBRARIES}
)
target_compile_options(calculator PRIVATE ${DBUS_CFLAGS_OTHER})
install(TARGETS calculator
    RUNTIME DESTINATION /usr/local/bin
)