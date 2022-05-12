#include "pch.h"
#include <filesystem>

#include "../Inc/Framework/Common/AssetLoader.h"
#include "Framework/Common/Log.h"


using namespace Engine;
namespace fs = std::filesystem;


int Engine::AssetLoader::Initialize()
{
    int ret = 0;
    AddSearchPath("H:/Project_VS2019/NutEngine/Engine/Asset/");
    return ret;
}

void Engine::AssetLoader::Finalize()
{
    search_path_.clear();
}

void Engine::AssetLoader::Tick()
{
}

bool Engine::AssetLoader::AddSearchPath(const char* path)
{
    auto src = search_path_.begin();
    while (src != search_path_.end()) 
    {
        if (!(*src).compare(path))
            return true;
        src++;
    }
    search_path_.push_back(path);
    return true;
}

bool Engine::AssetLoader::RemoveSearchPath(const char* path)
{
    auto src = search_path_.begin();
    while (src != search_path_.end())
    {
        if (!(*src).compare(path)) 
        {
            search_path_.erase(src);
            return true;
        }
        src++;
    }
    return true;
}


std::string Engine::AssetLoader::GetAbsolutePath(const char* file_path) const
{
    fs::path _file_path(file_path);
    if (_file_path.is_absolute())
        return file_path;
    else
    {
        for (auto& s_path : search_path_)
        {
            for (auto& path : fs::recursive_directory_iterator(s_path))
            {
                if (!path.is_directory())
                {
                    auto tar = _file_path.filename().string();
                    auto cur = path.path().filename().string();
                    if(tar == cur)
                        return path.path().string();
                }
            }
        }
    }
    return std::string{};
}

Engine::AssetLoader::AssetFilePtr Engine::AssetLoader::OpenFile(const char* name, EAssetOpenMode mode)
{
    FILE* fp = nullptr;
    std::string abs_path = GetAbsolutePath(name);
    switch (mode)
    {
    case Engine::AssetLoader::EAssetOpenMode::kOpenText:
        fp = fopen(abs_path.c_str(), "r");
        break;
    case Engine::AssetLoader::EAssetOpenMode::kOpenBinary:
        fp = fopen(abs_path.c_str(), "rb");
        break;
    }
    if(auto error = GetLastError(); error == 2 && error == 3)
        NE_LOG(ALL, kError, "[AssetLoader Error]: The specified file :{} was not found, please check if the path is correct or add a search path!", name)
    else if(error == 4)
        NE_LOG(ALL, kError, "[AssetLoader Error]: The system cannot open the file :{}", name)
    else if (error == 5)
        NE_LOG(ALL, kError, "[AssetLoader Error]: File :{} Access denied", name)
    return fp;
}

Buffer Engine::AssetLoader::OpenAndReadTextSync(const char* filePath)
{
    AssetFilePtr fp = OpenFile(filePath, EAssetOpenMode::kOpenText);
    Buffer* pBuff = nullptr;
    if (fp) 
    {
        size_t length = GetSize(fp);
        pBuff = new Buffer(length + 1);
        fread(pBuff->GetData(), length, 1, static_cast<FILE*>(fp));
        pBuff->GetData()[length] = '\0';
        CloseFile(fp);
    }
    else
        pBuff = new Buffer();
    return *pBuff;
}

Buffer Engine::AssetLoader::OpenAndReadBinarySync(const char* filePath)
{
    AssetFilePtr fp = OpenFile(filePath, EAssetOpenMode::kOpenBinary);
    Buffer* pBuff = nullptr;
    if (fp) {
        size_t length = GetSize(fp);
        pBuff = new Buffer(length);
        fread(pBuff->GetData(), length, 1, static_cast<FILE*>(fp));
        CloseFile(fp);
    }
    else {
        fprintf(stderr, "Error opening file '%s'\n", filePath);
        pBuff = new Buffer();
    }
    return *pBuff;
}

size_t Engine::AssetLoader::ReadSync(const AssetFilePtr& fp, Buffer& buf)
{
    size_t sz;
    if (!fp) return 0;
    sz = fread(buf.GetData(), buf.GetDataSize(), 1, static_cast<FILE*>(fp));
    return sz;
}

void Engine::AssetLoader::CloseFile(AssetFilePtr& fp)
{
    fclose((FILE*)fp);
    fp = nullptr;
}

size_t Engine::AssetLoader::GetSize(const AssetFilePtr& fp)
{
    FILE* _fp = static_cast<FILE*>(fp);
    long pos = ftell(_fp);
    fseek(_fp, 0, SEEK_END);
    size_t length = ftell(_fp);
    fseek(_fp, pos, SEEK_SET);
    return length;
}

int32_t Engine::AssetLoader::Seek(AssetFilePtr fp, long offset, EAssetSeekBase where)
{
    return fseek(static_cast<FILE*>(fp), offset, static_cast<int>(where));
}
