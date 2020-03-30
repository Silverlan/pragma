#ifndef __PR_SOURCE_SOURCE2_HPP__
#define __PR_SOURCE_SOURCE2_HPP__

#include <memory>
#include <functional>

class NetworkState;
class Game;
class Model;
class ModelSubMesh;

namespace pragma::asset {class WorldData;};
class VFilePtrInternal;
namespace source2
{
	namespace resource
	{
		class Resource;
		class Mesh;
		class Model;
	};
	namespace convert
	{
		std::shared_ptr<Model> convert_model(
			Game &game,source2::resource::Model &s2Mdl,source2::resource::Resource *optResource=nullptr
		);
		std::shared_ptr<pragma::asset::WorldData> convert_map(Game &game,std::shared_ptr<VFilePtrInternal> &f,const std::string &mapName);
	};
};

#endif
