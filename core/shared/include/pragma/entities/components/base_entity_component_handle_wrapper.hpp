#ifndef __BASE_ENTITY_COMPONENT_HANDLE_WRAPPER_HPP__
#define __BASE_ENTITY_COMPONENT_HANDLE_WRAPPER_HPP__

#include <sharedutils/util_weak_handle.hpp>

namespace pragma {class BaseEntityComponent;};
struct DLLNETWORK BaseEntityComponentHandleWrapper
{
	BaseEntityComponentHandleWrapper(const util::WeakHandle<pragma::BaseEntityComponent> &wh)
		: handle(wh)
	{}
	BaseEntityComponentHandleWrapper()=default;
	util::WeakHandle<pragma::BaseEntityComponent> handle = {};
	bool expired() const {return handle.expired();}

	pragma::BaseEntityComponent *operator->() {return get();}
	const pragma::BaseEntityComponent *get() const {return handle.get();}
	pragma::BaseEntityComponent *get() {return handle.get();}
};

#endif
