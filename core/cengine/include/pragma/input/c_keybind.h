#ifndef __C_KEYBIND_H__
#define __C_KEYBIND_H__
#include "pragma/c_enginedefinitions.h"
#include <string>
#include <memory>
#include <vector>
#include <iglfw/glfw_keys.h>
#pragma warning(push)
#pragma warning(disable : 4251)
struct DLLCENGINE KeyBind
{
public:
	enum class DLLCENGINE Type : uint8_t
	{
		Invalid = std::numeric_limits<uint8_t>::max(),
		Regular = 0,
		Function = 1
	};
private:
	std::unique_ptr<std::string> m_bind = nullptr;
	Type m_type = Type::Invalid;
	int m_function;

	struct Command
	{
		std::string cmd;
		std::vector<std::string> argv;
	};
	std::vector<Command> m_cmds;

	void Initialize();
public:
	KeyBind();
	KeyBind(std::string bind);
	KeyBind(int function);
	Type GetType() const;
	const std::string &GetBind() const;
	int32_t GetFunction() const;
	bool Execute(GLFW::KeyState inputState,GLFW::KeyState pressState,GLFW::Modifier mods,float magnitude=1.f);
};
#pragma warning(pop)
#endif
