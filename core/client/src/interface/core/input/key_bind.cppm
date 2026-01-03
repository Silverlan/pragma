// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:core.key_bind;

export import luabind;
export import pragma.platform;

#pragma warning(push)
#pragma warning(disable : 4251)
export struct DLLCLIENT KeyBind {
  public:
	enum class DLLCLIENT Type : uint8_t { Invalid = std::numeric_limits<uint8_t>::max(), Regular = 0, Function = 1 };
  private:
	std::unique_ptr<std::string> m_bind = nullptr;
	Type m_type = Type::Invalid;
	std::optional<luabind::function<>> m_function {};

	struct Command {
		std::string cmd;
		std::vector<std::string> argv;
	};
	std::vector<Command> m_cmds;

	void Initialize();
  public:
	KeyBind();
	KeyBind(std::string bind);
	KeyBind(luabind::function<> function);
	KeyBind(const KeyBind &other);
	KeyBind &operator=(const KeyBind &other);
	Type GetType() const;
	const std::string &GetBind() const;
	std::optional<luabind::function<>> GetFunction() const;
	bool Execute(pragma::platform::KeyState inputState, pragma::platform::KeyState pressState, pragma::platform::Modifier mods, float magnitude = 1.f);
};
#pragma warning(pop)
