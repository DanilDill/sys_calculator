cmake_minimum_required(VERSION 3.10)

project(calculator)
project(libmath LANGUAGES CXX)

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

target_link_libraries(calculator
    PRIVATE
        math
)

install(TARGETS calculator
    RUNTIME DESTINATION /usr/local/bin
)