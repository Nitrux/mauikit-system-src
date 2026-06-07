cmake_minimum_required(VERSION 3.16)

project(MauiKitSystemLibs)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "This project supports Linux only.")
endif()

# Enable x86-64-v3 optimizations only for native x86_64/amd64 builds.
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" MAUIKITSYSTEM_SYSTEM_PROCESSOR)
string(TOLOWER "${CMAKE_HOST_SYSTEM_PROCESSOR}" MAUIKITSYSTEM_HOST_PROCESSOR)
if(MAUIKITSYSTEM_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64)$"
   AND MAUIKITSYSTEM_HOST_PROCESSOR MATCHES "^(x86_64|amd64)$")
    include(CheckCCompilerFlag)
    include(CheckCXXCompilerFlag)
    check_c_compiler_flag("-march=x86-64-v3" MAUIKITSYSTEM_HAS_X86_64_V3_C)
    check_cxx_compiler_flag("-march=x86-64-v3" MAUIKITSYSTEM_HAS_X86_64_V3_CXX)
    if(MAUIKITSYSTEM_HAS_X86_64_V3_C AND MAUIKITSYSTEM_HAS_X86_64_V3_CXX)
        add_compile_options(
            $<$<COMPILE_LANGUAGE:C>:-march=x86-64-v3>
            $<$<COMPILE_LANGUAGE:CXX>:-march=x86-64-v3>
        )
    endif()
endif()


set(MAUIKITSYSTEM_VERSION 4.0.4)

add_subdirectory(libs)

if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE
        @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
endif()
