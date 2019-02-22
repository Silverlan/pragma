#ifndef __GAME_CALLBACK_H__
#define __GAME_CALLBACK_H__

#include <sharedutils/functioncallback.h>
#include <unordered_map>
#include <string>

#define DECLARE_STATIC_GAME_CALLBACKS(dlldec,state) \
	dlldec std::unordered_map<std::string,CallbackHandle> &get_static_##state##_callbacks(); \
	dlldec bool __register_##state##_game_callback(const std::string &name,CallbackHandle hCallback);

#define DEFINE_STATIC_GAME_CALLBACKS(dlldec,state) \
	static std::unordered_map<std::string,CallbackHandle> __static_##state##_callbacks; \
	std::unordered_map<std::string,CallbackHandle> &get_static_##state##_callbacks() {return __static_##state##_callbacks;} \
	bool __register_##state##_game_callback(const std::string &name,CallbackHandle hCallback) \
	{ \
		__static_##state##_callbacks.insert(std::unordered_map<std::string,CallbackHandle>::value_type(name,hCallback)); \
		return true; \
	}

#define ADD_STATIC_GAME_CALLBACK(name,callback,state) \
	auto __##state##_game_callback_##name##__LINE__##__COUNTER__ = __register_##state##_game_callback(#name,callback);

#endif