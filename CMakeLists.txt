cmake_minimum_required(VERSION 3.16)

project(calculator LANGUAGES CXX)

set(CMAKE_INSTALL_PREFIX "/usr/local")


find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED dbus-1)
set(DBUS_FLAGS "")
foreach(dir ${DBUS_INCLUDE_DIRS})
    list(APPEND DBUS_FLAGS "-I${dir}")
endforeach()

# Настройка clang-tidy
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if(CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE})
        
        # Получаем список всех исходных файлов C++ в проекте
        file(GLOB_RECURSE ALL_SOURCE_FILES 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
        )
        
        # Создаем пользовательскую цель для статического анализа
        add_custom_target(tidy
            COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_BINARY_DIR} ${ALL_SOURCE_FILES} -- ${DBUS_FLAGS}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running clang-tidy static analysis"
            VERBATIM
        )
    else()
        message(WARNING "clang-tidy not found, disabling ENABLE_CLANG_TIDY")
        set(ENABLE_CLANG_TIDY OFF)
    endif()
endif()

# Настройка clang-format
option(ENABLE_CLANG_FORMAT "Enable clang-format code formatting target" ON)
if(ENABLE_CLANG_FORMAT)
    find_program(CLANG_FORMAT_EXE NAMES clang-format)
    if(CLANG_FORMAT_EXE)
        # Получаем список всех исходных файлов C++ в проекте
        file(GLOB_RECURSE ALL_SOURCE_FILES 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
        )
        
        # Создаем пользовательскую цель для форматирования
        add_custom_target(format
            COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_SOURCE_FILES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Formatting source code with clang-format"
            VERBATIM
        )
    else()
        message(WARNING "clang-format not found, 'format' target will not be available")
        set(ENABLE_CLANG_FORMAT OFF)
    endif()
endif()

include(FetchContent)

FetchContent_Declare(
    libmath
    GIT_REPOSITORY git@github.com:DanilDill/libmath.git
    GIT_TAG master
)

FetchContent_MakeAvailable(libmath)

add_executable(calculator
    src/calculator.hpp
    src/calculator.cpp
    src/main.cpp
    src/args_parser.cpp
    src/dbus.hpp
    src/dbus.cpp
    src/task.hpp
    src/task.cpp
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



# Set package information
set(CPACK_PACKAGE_NAME "sys-calculator")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_CONTACT "Danil Dil")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "System calculator service")
set(CPACK_PACKAGE_DESCRIPTION "Calculator application that runs as a systemd service")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Danil Dil")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libdbus-1-3")
set(CPACK_GENERATOR "DEB")
# Define variables for script templates
set(PROJECT_DESCRIPTION "System Calculator Service")
# Configure Debian package control scripts with proper variables

set(PRE_INST_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/preinst)
file(COPY deb-configs/preinst DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(POST_INST_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/postinst)
file(COPY deb-configs/postinst DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(PRE_REM_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/prerm)
file(COPY deb-configs/prerm DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA ${CONF_FILE} ${PRE_INST_SCRIPT} ${POST_INST_SCRIPT} ${PRE_REM_SCRIPT})

install(TARGETS calculator
    RUNTIME DESTINATION /usr/local/bin
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/etc/
  DESTINATION /etc
)

add_custom_target(package_calculator
    COMMAND ${CMAKE_CPACK_COMMAND} -G DEB
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating DEB package using CPack..."
    USES_TERMINAL
)


# Enable CPack
include(CPack)

find_package(Doxygen REQUIRED dot)

if(DOXYGEN_FOUND)
    # Можно использовать шаблон Doxyfile.in для динамической подстановки путей
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Генерация API документации через Doxygen"
        VERBATIM )
endif()