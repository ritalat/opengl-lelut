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

if(EMSCRIPTEN OR USE_GLES)
    add_compile_definitions(USE_GLES=1)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_INSTALL_BINDIR ".")
set(CMAKE_INSTALL_LIBDIR ".")

add_subdirectory(external)

add_subdirectory(src)

if(EMSCRIPTEN OR USE_GLES)
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
    foreach(ES3_SAMPLE ${ES3_SAMPLES})
        create_sample(${ES3_SAMPLE})
    endforeach()
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
else()
    if(USE_GLES)
        install(TARGETS ${ES3_SAMPLES} mylly SDL3-shared RUNTIME DESTINATION ".")
    else()
        install(TARGETS ${SAMPLES} mylly SDL3-shared RUNTIME DESTINATION ".")
    endif()
    install(DIRECTORY assets shaders DESTINATION ".")
endif()
