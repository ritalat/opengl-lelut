cmake_minimum_required(VERSION 3.16)

set(SOURCES gllelu.cc
            gllelucamera.cc
            himmeli.cc
            model.cc
            path.cc
            shader.cc
            text_bitmap.cc
            texture.cc)

set(HEADERS gllelu.hh
            gllelu_main.hh
            gllelucamera.hh
            himmeli.hh
            model.hh
            path.hh
            shader.hh
            shapes.hh
            text_bitmap.hh
            texture.hh)

if(EMSCRIPTEN OR USE_GLES)
    list(APPEND SOURCES glad/src/gles2.c)
    list(APPEND HEADERS glad/include/glad/gles2.h
                        glad/include/KHR/khrplatform.h)
else()
    list(APPEND SOURCES glad/src/gl.c)
    list(APPEND HEADERS glad/include/glad/gl.h
                        glad/include/KHR/khrplatform.h)
endif()

add_library(single_header STATIC lib_impl.cc)
set_target_properties(single_header PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(single_header stb tinyobjloader)

add_library(mylly SHARED ${SOURCES} ${HEADERS})

if(WIN32)
    set_target_properties(mylly PROPERTIES PREFIX "")
    if(MSVC)
        set_target_properties(mylly PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
    else()
        target_link_options(mylly PUBLIC "-Wl,--export-all-symbols")
    endif()
endif()

if(UNIX AND NOT EMSCRIPTEN)
    target_link_options(mylly PUBLIC "-Wl,--enable-new-dtags")
    set_target_properties(mylly PROPERTIES INSTALL_RPATH "\${ORIGIN}")
endif()

if(MSVC)
    target_compile_options(mylly PRIVATE /W4)
    target_compile_definitions(mylly PRIVATE _CRT_SECURE_NO_WARNINGS)
elseif(NOT EMSCRIPTEN)
    target_compile_options(mylly PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()

target_compile_definitions(mylly PUBLIC GLAD_API_CALL_EXPORT
                                 PRIVATE GLAD_API_CALL_EXPORT_BUILD)

target_include_directories(mylly PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                        ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)

target_link_libraries(mylly SDL2::SDL2
                            glm::glm
                            single_header)
