#include "pch.h"
#include <filesystem>

#include "../Inc/Framework/Common/AssetLoader.h"


using namespace Engine;



int Engine::AssetLoader::Initialize()
{
    int ret = 0;
    AddSearchPath("H:/Project_VS2019/NutEngine/Engine");
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
    std::vector<std::string>::iterator src = search_path_.begin();
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
    std::vector<std::string>::iterator src = search_path_.begin();
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

bool Engine::AssetLoader::FileExists(const char* filePath)
{
    AssetFilePtr fp = OpenFile(filePath, EAssetOpenMode::kOpenBinary);
    if (fp != nullptr) 
    {
        CloseFile(fp);
        return true;
    }
    return false;
}


std::string Engine::AssetLoader::GetAbsolutePath(const char* file_name) const
{
    std::string full_path;
    for(auto& str : search_path_)
    {
        full_path.append(str);
        full_path.append("/Asset/");
        full_path.append(file_name);
        std::filesystem::path _path(full_path);
        if(std::filesystem::exists(_path)) return full_path;
        else continue;
    }
}

Engine::AssetLoader::AssetFilePtr Engine::AssetLoader::OpenFile(const char* name, EAssetOpenMode mode)
{
    FILE* fp = nullptr;
    std::string up_path;
    std::string full_path;
    for (int32_t i = 0; i < 10; i++)
    {
        auto src = search_path_.begin();
        bool bloop = true;
        while (bloop)
        {
            full_path.assign(up_path);
            if (src != search_path_.end())
            {
                full_path.append(*src);
                full_path.append("/Asset/");
                src++;
            }
            else
            {
                bloop = false;
                full_path.append("Asset/");
            }
            full_path.append(name);
            switch (mode)
            {
            case Engine::AssetLoader::EAssetOpenMode::kOpenText:
                fp = fopen(full_path.c_str(), "r");
                break;
            case Engine::AssetLoader::EAssetOpenMode::kOpenBinary:
                fp = fopen(full_path.c_str(), "rb");
                break;
            }
            if (fp) return static_cast<AssetFilePtr>(fp);         
        }
        up_path.append("../");
    }
    return nullptr;
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
