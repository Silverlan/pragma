#ifndef __UTIL_VMF_HPP__
#define __UTIL_VMF_HPP__

#include <string>
#include <functional>
#include <fsys/filesystem.h>

class PolyMesh;
class MaterialManager;
class NetworkState;
namespace vmf
{
	enum class ResultCode : uint16_t
	{
		Success = 0u,
		ErrFileNotFound,
		ErrUnableToParseFile,
		ErrNoWorldFound,
		ErrNoWorldSolidsFound,
		ErrOpenOutputFile
	};
	ResultCode load(NetworkState &nwState,const std::string &fileName,const std::function<void(const std::string&)> &messageLogger=nullptr);

	namespace impl
	{
		struct DataFileBlock
		{
			std::unordered_map<std::string,std::vector<std::string>> keyvalues;
			std::unordered_map<std::string,std::vector<std::shared_ptr<DataFileBlock>>> blocks;
			std::string KeyValue(const std::string &key,int32_t i=0);
		};

		class DataFile
		{
		private:
			DataFile()=delete;
		public:
			static std::shared_ptr<DataFileBlock> ReadBlock(VFilePtr f);
			static std::shared_ptr<DataFileBlock> Read(const char *f);
		};
		std::shared_ptr<PolyMesh> build_mesh(const std::vector<std::shared_ptr<DataFileBlock>> &sides,NetworkState *nwState);
	};
};

#endif
