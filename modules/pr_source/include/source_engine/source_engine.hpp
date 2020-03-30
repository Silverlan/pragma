#ifndef __PR_SOURCE_SOURCE_ENGINE_HPP__
#define __PR_SOURCE_SOURCE_ENGINE_HPP__

#include <memory>
#include <unordered_set>
#include <string>

class Game;
class VFilePtrInternal;
namespace pragma::asset {class WorldData; class EntityData;};
namespace util::fgd {struct Data;};
namespace source_engine
{
	namespace convert
	{
		std::shared_ptr<pragma::asset::WorldData> convert_map(Game &game,std::shared_ptr<VFilePtrInternal> &f);
	};
	void translate_class(
		const std::unordered_map<std::string,std::string> &inKeyValues,
		std::unordered_map<std::string,std::string> &outKeyValues,
		std::string &className,bool isSource2
	);
	static void translate_key_value(
		const std::vector<util::fgd::Data> &fgdData,
		const std::string &className,
		const std::string &key,std::string &val,bool isSource2,
		const std::function<void(const std::string&,uint8_t)> &messageLogger=nullptr,
		std::unordered_set<std::string> *msgCache=nullptr
	);
	std::vector<util::fgd::Data> load_fgds(class NetworkState &nwState,const std::function<void(const std::string&)> &messageLogger=nullptr);
	void find_entity_components(const std::unordered_map<std::string,std::string> &keyValues,std::unordered_set<std::string> &outComponents);
	void translate_entity_data(
		pragma::asset::WorldData &worldData,const std::vector<util::fgd::Data> &fgdData,bool isSource2,
		const std::function<void(const std::string&)> &messageLogger=nullptr,
		std::unordered_set<std::string> *optMsgCache=nullptr
	);
	void translate_entity_data(
		NetworkState &nw,pragma::asset::EntityData &entData,const std::vector<util::fgd::Data> &fgdData,bool isSource2,
		const std::function<void(const std::string&)> &messageLogger=nullptr,
		std::unordered_set<std::string> *optMsgCache=nullptr
	);
};

#endif
