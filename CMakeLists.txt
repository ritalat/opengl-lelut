cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "..." FORCE)
endif()

project(opengltest)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install CACHE PATH "..." FORCE)
endif()

set(USE_GLES OFF CACHE BOOL "...")

if(EMSCRIPTEN OR ANDROID OR USE_GLES)
    add_compile_definitions(USE_GLES=1)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_INSTALL_BINDIR ".")
set(CMAKE_INSTALL_LIBDIR ".")

add_subdirectory(external)

add_subdirectory(src)

if(MINGW)
    # Find MinGW runtime libraries (only tested with Arch and MSYS2)
    set(MINGW_LIB_DIRS /usr/x86_64-w64-mingw32/bin C:/msys64/mingw64/bin)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .dll)
    find_library(MINGW_STDCPP_LIB NAMES libstdc++-6 HINTS ${MINGW_LIB_DIRS})
    if(MINGW_STDCPP_LIB)
        list(APPEND MINGW_RUNTIME_LIBS "${MINGW_STDCPP_LIB}")
    endif()
    find_library(MINGW_GCCSEH_LIB NAMES libgcc_s_seh-1 HINTS ${MINGW_LIB_DIRS})
    if(MINGW_GCCSEH_LIB)
        list(APPEND MINGW_RUNTIME_LIBS "${MINGW_GCCSEH_LIB}")
    endif()
    find_library(MINGW_WINPTHREAD_LIB NAMES libwinpthread-1 HINTS ${MINGW_LIB_DIRS})
    if(MINGW_WINPTHREAD_LIB)
        list(APPEND MINGW_RUNTIME_LIBS "${MINGW_WINPTHREAD_LIB}")
    endif()
endif()

if(EMSCRIPTEN OR ANDROID OR USE_GLES)
    if(EMSCRIPTEN)
        set(CMAKE_EXECUTABLE_SUFFIX ".html")
        add_compile_options(-sNO_DISABLE_EXCEPTION_CATCHING)
        add_link_options(--embed-file ${CMAKE_SOURCE_DIR}/shaders/@shaders/
                         --preload-file ${CMAKE_SOURCE_DIR}/assets/@assets/
                         --shell-file=${EMSCRIPTEN_ROOT_PATH}/src/shell_minimal.html
                         -sNO_DISABLE_EXCEPTION_CATCHING
                         -sALLOW_MEMORY_GROWTH
                         -sMIN_WEBGL_VERSION=2)
    endif()
    if(ANDROID)
        add_library(main SHARED src/mylly/android_main.cc
                                src/es3_triangle.cc
                                src/es3_bitmap_font.cc
                                src/es3_cubemaps.cc)
        target_link_libraries(main mylly)
    else()
        foreach(ES3_SAMPLE ${ES3_SAMPLES})
            create_sample(${ES3_SAMPLE})
        endforeach()
    endif()
else()
    foreach(SAMPLE ${SAMPLES})
        create_sample(${SAMPLE})
    endforeach()
endif()

if(EMSCRIPTEN)
    foreach(ES3_SAMPLE ${ES3_SAMPLES})
        install(FILES ${CMAKE_BINARY_DIR}/${ES3_SAMPLE}.data
                      ${CMAKE_BINARY_DIR}/${ES3_SAMPLE}.html
                      ${CMAKE_BINARY_DIR}/${ES3_SAMPLE}.js
                      ${CMAKE_BINARY_DIR}/${ES3_SAMPLE}.wasm
                      DESTINATION ".")
    endforeach()
elseif(NOT ANDROID)
    if(USE_GLES)
        install(TARGETS ${ES3_SAMPLES} mylly SDL3-shared RUNTIME DESTINATION ".")
    else()
        install(TARGETS ${SAMPLES} mylly SDL3-shared RUNTIME DESTINATION ".")
    endif()
    if(MINGW)
        if(MINGW_RUNTIME_LIBS)
            install(FILES ${MINGW_RUNTIME_LIBS} DESTINATION ".")
        else()
            message(WARNING "Remember to copy MinGW runtime libraries!")
        endif()
    endif()
    install(DIRECTORY assets shaders DESTINATION ".")
endif()
