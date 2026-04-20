cmake_minimum_required(VERSION 3.16)

project(calculator LANGUAGES CXX)

set(CMAKE_INSTALL_PREFIX "/usr/local")

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
    src/args_parser.cpp
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
configure_file(deb-configs/preinst.in ${PRE_INST_SCRIPT})

set(POST_INST_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/postinst)
configure_file(deb-configs/postinst.in ${POST_INST_SCRIPT})

set(PRE_REM_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/prerm)
configure_file(deb-configs/prerm.in ${PRE_REM_SCRIPT})

set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA ${CONF_FILE} ${PRE_INST_SCRIPT} ${POST_INST_SCRIPT} ${PRE_REM_SCRIPT})

install(TARGETS calculator
    RUNTIME DESTINATION /usr/local/bin
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