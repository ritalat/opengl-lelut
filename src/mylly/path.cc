#include "path.hh"

#if defined(__ANDROID__)
#include "SDL3/SDL.h"
#endif

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

const Path BASE_DIR = [](){
    const char *env = getenv("GLLELU_DATADIR");
    if (env)
        return env;
    return ".";
}();

Path datadir()
{
    return BASE_DIR;
}

Path assetdir()
{
#if defined(__ANDROID__)
    return "";
#else
    return BASE_DIR / "assets";
#endif
}

Path shaderdir()
{
#if defined(__ANDROID__)
    return "";
#else
    return BASE_DIR / "shaders";
#endif
}

Path getPath(std::string_view file)
{
    return datadir() / file;
}

Path getAssetPath(std::string_view file)
{
    return assetdir() / file;
}

Path getShaderPath(std::string_view file)
{
    return shaderdir() / file;
}

std::string slurpFile(const Path &path)
{
#if defined(__ANDROID__)
    char *contents = (char *)SDL_LoadFile(cpath(path), NULL);
    if (!contents)
        throw std::runtime_error("Failed to read file: " + std::string(path));
    std::string s(contents);
    SDL_free(contents);
    return s;
#else
    std::ifstream stream(path);

    if (!stream)
        throw std::runtime_error("Failed to read file: " + path.string() + FILE_ERROR_HINT);

    return std::string( (std::istreambuf_iterator<char>(stream)),
                        (std::istreambuf_iterator<char>()) );
#endif
}
