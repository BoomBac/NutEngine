#include "../Interface/IRuntimeModule.h"
#include "Buffer.h"

namespace Engine
{
	class AssetLoader : public IRuntimeModule
	{
		
	public:
		virtual ~AssetLoader() {};
		int Initialize() override;
		void Finalize() override;
		void Tick() override;

		using AssetFilePtr = void*;
		enum class EAssetOpenMode
		{
			kOpenText,
			kOpenBinary
		};
		enum class EAssetSeekBase
		{
			kSeekSet,
			kSeekCur,
			kSeekEnd
		};
		bool AddSearchPath(const char* path);
		bool RemoveSearchPath(const char* path);
		bool FileExists(const char* filePath);
		AssetFilePtr OpenFile(const char* name, EAssetOpenMode mode);
		Buffer OpenAndReadTextSync(const char* filePath);
		size_t ReadSync(const AssetFilePtr& fp, Buffer& buf);
		void CloseFile(AssetFilePtr& fp);
		size_t GetSize(const AssetFilePtr& fp);
		int32_t Seek(AssetFilePtr fp, long offset, EAssetSeekBase where);
		inline std::string OpenAndReadTextFileToStringSync(const char* fileName)
		{
			std::string result;
			Buffer buffer = OpenAndReadTextSync(fileName);
			char* content = reinterpret_cast<char*>(buffer.p_data_);

			if (content)
			{
				result = std::string(std::move(content));
			}

			return result;
		}
	private:
		std::vector<std::string> search_path_;
	};
}