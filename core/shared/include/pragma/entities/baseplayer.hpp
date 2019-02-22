#ifndef __BASEPLAYER_HPP__
#define __BASEPLAYER_HPP__

#include "pragma/networkdefinitions.h"
#include <sharedutils/util_weak_handle.hpp>

namespace pragma {class BasePlayerComponent;};
class DLLNETWORK BasePlayer
{
public:
	BasePlayer()=default;
	virtual ~BasePlayer()=default;
	util::WeakHandle<pragma::BasePlayerComponent> GetBasePlayerComponent();
protected:
	util::WeakHandle<pragma::BasePlayerComponent> m_basePlayerComponent = {};
};

#endif
