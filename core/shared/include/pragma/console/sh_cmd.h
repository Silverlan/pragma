#ifndef __SH_CMD_H__
#define __SH_CMD_H__

#include "pragma/networkdefinitions.h"
#include <string>

class NetworkState;
class BaseEntity;
namespace pragma {class BaseCharacterComponent;};
namespace command
{
	DLLNETWORK std::vector<BaseEntity*> find_target_entity(NetworkState *state,pragma::BaseCharacterComponent &pl,std::vector<std::string> &argv,const std::function<void(TraceData&)> &trCallback=nullptr);
	DLLNETWORK std::vector<BaseEntity*> find_trace_targets(NetworkState *state,pragma::BaseCharacterComponent &pl,const std::function<void(TraceData&)> &trCallback=nullptr);
	DLLNETWORK std::vector<BaseEntity*> find_named_targets(NetworkState *state,const std::string &targetName);
};

#endif
