// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:engine.info;

export import pragma.util;

export namespace pragma::engine_info {
	DLLNETWORK std::string get_identifier();
	DLLNETWORK std::string get_name();
	DLLNETWORK std::string get_program_title();
	DLLNETWORK util::Path get_icon_path();
	DLLNETWORK std::string get_executable_name();
	DLLNETWORK std::string get_server_executable_name();
	DLLNETWORK std::string get_author_mail_address();
	DLLNETWORK std::string get_website_url();
	DLLNETWORK std::string get_wiki_url();
	DLLNETWORK std::string get_discord_url();
	DLLNETWORK std::string get_github_url();
	DLLNETWORK uint32_t get_steam_app_id();

	constexpr uint16_t DEFAULT_SERVER_PORT = 29150;
	constexpr uint16_t DEFAULT_QUERY_PORT = 29155;
	constexpr uint16_t DEFAULT_AUTH_PORT = 8766;

	// Returns the extensions for the supported audio formats
	DLLNETWORK const std::vector<std::string> get_supported_audio_formats();

	struct DLLNETWORK GitInfo {
		std::string ref;
		std::string commitSha;
		std::string dateTime;
	};
	DLLNETWORK std::optional<GitInfo> get_git_info();

	enum class Platform : uint8_t {
		Linux = 0,
		Windows,
		Unknown,
	};
	constexpr Platform get_platform()
	{
#ifdef __linux__
		return Platform::Linux;
#elif _WIN32
		return Platform::Windows;
#else
		return Platform::Unknown;
#endif
	}
	constexpr std::string_view get_platform_name()
	{
		switch(get_platform()) {
		case Platform::Linux:
			return "linux";
		case Platform::Windows:
			return "windows";
		}
		return "unknown";
	}

	enum class Compiler : uint8_t {
		Clang = 0,
		ClangCl,
		GCC,
		MSVC,
		Unknown,
	};
	constexpr Compiler get_compiler()
	{
#if defined(__clang__)

#if defined(_MSC_VER)
		return Compiler::ClangCl;
#else
		return Compiler::Clang;
#endif

#elif defined(_MSC_VER)
		return Compiler::MSVC;
#elif defined(__GNUC__) || defined(__GNUG__)
		return Compiler::GCC;
#else
		return Compiler::Unknown;
#endif
	}
	constexpr util::Version get_compiler_version()
	{
#if defined(__clang__)
		return {__clang_major__, __clang_minor__, __clang_patchlevel__};
#elif defined(__GNUC__) || defined(__GNUG__)
		return {__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__};
#elif defined(_MSC_VER)
		constexpr int msc = _MSC_VER;
		constexpr int major = msc / 100;
		constexpr int minor = msc % 100;
		return {major, minor};
#else
		return {};
#endif
	}
	constexpr std::string_view get_compiler_name()
	{
		switch(get_compiler()) {
		case Compiler::Clang:
			return "clang";
		case Compiler::ClangCl:
			return "clang-cl";
		case Compiler::GCC:
			return "gcc";
		case Compiler::MSVC:
			return "msvc";
		}
		return "unknown";
	}

	enum class Architecture : uint8_t {
		Unknown,
		X86,
		X64,
		Arm,
		Arm64,
	};
	constexpr Architecture get_architecture()
	{
#if defined(__x86_64__) || defined(_M_X64)
		return Architecture::X64;
#elif defined(__i386__) || defined(_M_IX86)
		return Architecture::X86;
#elif defined(__aarch64__) || defined(_M_ARM64)
		return Architecture::Arm64;
#elif defined(__arm__) || defined(_M_ARM)
		return Architecture::Arm;
#else
		return Architecture::Unknown;
#endif
	}

	constexpr std::string_view get_architecture_name()
	{
		switch(get_architecture()) {
		case Architecture::X64:
			return "x64";
		case Architecture::X86:
			return "x86";
		case Architecture::Arm:
			return "arm";
		case Architecture::Arm64:
			return "arm64";
		default:
			return "unknown";
		}
	}
};
