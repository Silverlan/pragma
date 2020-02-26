#ifndef __LEVEL_INFO_HPP__
#define __LEVEL_INFO_HPP__

#include <mathutil/color.h>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#define WLD_VERSION 10

namespace util
{
	namespace fgd
	{
		struct Data;
	};
};

namespace pragma
{
	namespace level
	{
		extern const Color WLD_DEFAULT_AMBIENT_COLOR;

		enum class MeshType : uint8_t
		{
			PolyMesh = 0u,
			Displacement,
			FaceMesh
		};

		enum class EntityFlags : uint64_t
		{
			None = 0ull,
			ClientsideOnly = 1ull
		};

		void find_entity_components(const std::unordered_map<std::string,std::string> &keyValues,std::unordered_set<std::string> &outComponents);
		std::vector<util::fgd::Data> load_fgds(class NetworkState &nwState,const std::function<void(const std::string&)> &messageLogger=nullptr);
		void transform_class(
			const std::unordered_map<std::string,std::string> &inKeyValues,
			std::unordered_map<std::string,std::string> &outKeyValues,
			std::string &className
		);
		void transform_keyvalue(
			const std::vector<util::fgd::Data> &fgdData,
			const std::string &className,
			const std::string &key,std::string &val,
			const std::function<void(const std::string&,uint8_t)> &messageLogger=nullptr,
			std::unordered_set<std::string> *msgCache=nullptr
		);
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::level::EntityFlags)

#endif
