#include "mdl.h"
#include "wv_source.hpp"
#include "source2/pr_impl.hpp"
#include "source2/source2.hpp"
#include <util_source2.hpp>
#include <source2/resource.hpp>
#include <source2/resource_data.hpp>
#include <source2/resource_edit_info.hpp>
#include <fsys/filesystem.h>
#include <util_archive.hpp>
#include <sharedutils/util_string.h>
#include <sharedutils/util_path.hpp>
#include <pragma/model/vertex.h>
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>
#include <pragma/networkstate/networkstate.h>
#include <pragma/game/game_resources.hpp>
#include <pragma/game/game_resources.hpp>
#include <pragma/asset_types/world.hpp>
#include <unordered_set>

#pragma optimize("",off)
Vector3 source2::impl::convert_source2_vector_to_pragma(const Vector3 &v)
{
	return Vector3{v.x,v.z,-v.y};
}

std::shared_ptr<Model> import::load_source2_mdl(
	Game &game,VFilePtr f,
	const std::function<bool(const std::shared_ptr<Model>&,const std::string&,const std::string&)> &fCallback,bool bCollision,
	std::vector<std::string> &textures,std::ostream *optLog
)
{
	auto resource = source2::load_resource(f);
	auto *s2Mdl = resource ? dynamic_cast<source2::resource::Model*>(resource->FindBlock(source2::BlockType::DATA)) : nullptr;
	if(s2Mdl == nullptr)
		return nullptr;
	return source2::convert::convert_model(game,*s2Mdl,resource.get());
}
#pragma optimize("",on)
