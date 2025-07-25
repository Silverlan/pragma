// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/console/cvar_handler.h"
#include <algorithm>
#include <pragma/console/convars.h>
#include <pragma/console/convarhandle.h>
#pragma optimize("", off)
static std::unordered_map<std::string, std::shared_ptr<PtrConVar>> *conVarPtrs = NULL;
std::unordered_map<std::string, std::shared_ptr<PtrConVar>> &CVarHandler::GetConVarPtrs() { return *conVarPtrs; }
ConVarHandle CVarHandler::GetConVarHandle(std::unordered_map<std::string, std::shared_ptr<PtrConVar>> &ptrs, std::string scvar)
{
	std::unordered_map<std::string, std::shared_ptr<PtrConVar>>::iterator i = ptrs.find(scvar);
	if(i == ptrs.end()) {
		PtrConVar *p = new PtrConVar;
		std::shared_ptr<PtrConVar> sptr(p);
		ptrs.insert(std::unordered_map<std::string, std::shared_ptr<PtrConVar>>::value_type(scvar, sptr));
		return ConVarHandle(sptr);
	}
	return ConVarHandle(i->second);
}

CVarHandler::CVarHandler() {}

CVarHandler::~CVarHandler() { ClearCommands(); }

void CVarHandler::ClearCommands()
{
	m_conCommandIDs.clear();
	m_cvarCallbacks.clear();
	m_conVars.clear();
}

void CVarHandler::Initialize()
{
	ConVarMap *map = GetConVarMap();
	if(map != NULL) {
		auto &cvars = map->GetConVars();
		for(auto &pair : cvars) {
			auto *cf = pair.second->Copy();
			if(cf->GetType() == ConType::Var) {
				auto &conVarPtrs = GetConVarPtrs();
				auto j = conVarPtrs.find(pair.first);
				if(j != conVarPtrs.end())
					j->second->set(static_cast<ConVar *>(cf));
			}
			m_conVars.insert(decltype(m_conVars)::value_type(pair.first, std::shared_ptr<ConConf>(cf)));
		}

		auto &callbacks = map->GetConVarCallbacks();
		for(auto &pair : callbacks) {
			auto it = m_cvarCallbacks.find(pair.first);
			if(it == m_cvarCallbacks.end())
				it = m_cvarCallbacks.insert(decltype(m_cvarCallbacks)::value_type(pair.first, std::vector<CvarCallback>())).first;
			for(auto &f : pair.second)
				it->second.push_back(CvarCallback {f});
		}
	}
}

std::shared_ptr<ConVar> CVarHandler::RegisterConVar(const std::string &scmd, udm::Type type, const std::string &value, ConVarFlags flags, const std::string &help)
{
	auto it = m_conVars.find(scmd);
	if(it != m_conVars.end()) {
		if(it->second->GetType() != ConType::Var)
			return nullptr;
		return std::static_pointer_cast<ConVar>(it->second);
	}
	udm::visit(type, [this, &it, &scmd, &value, &flags, &help](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(console::is_valid_convar_type_v<T> && udm::is_convertible<std::string, T>())
			it = m_conVars.insert(decltype(m_conVars)::value_type(scmd, ConVar::Create<T>(udm::convert<std::string, T>(value), flags, help))).first;
	});
	if(it == m_conVars.end())
		return nullptr;
	return std::static_pointer_cast<ConVar>(it->second);
}
std::shared_ptr<ConCommand> CVarHandler::RegisterConCommand(const std::string &scmd, const std::function<void(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)> &fc, ConVarFlags flags, const std::string &help)
{
	auto it = m_conVars.find(scmd);
	if(it != m_conVars.end()) {
		if(it->second->GetType() != ConType::Cmd)
			return nullptr;
		return std::static_pointer_cast<ConCommand>(it->second);
	}
	it = m_conVars.insert(decltype(m_conVars)::value_type(scmd, std::make_shared<ConCommand>(fc, flags, help))).first;
	return std::static_pointer_cast<ConCommand>(it->second);
}
template<typename T>
CallbackHandle CVarHandler::RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, const ConVar &, T, T)> &function)
{
	auto it = m_cvarCallbacks.find(scvar);
	if(it == m_cvarCallbacks.end())
		it = m_cvarCallbacks.insert(decltype(m_cvarCallbacks)::value_type(scvar, {})).first;
	auto f = [function](NetworkState *nw, const ConVar &cvar, const void *poldVal, const void *pnewVal) {
		udm::visit(cvar.GetVarType(), [poldVal, pnewVal, &function, &nw, &cvar](auto tag) {
			using TCvar = typename decltype(tag)::type;
			if constexpr(udm::is_convertible<TCvar, T>()) {
				auto oldVal = udm::convert<TCvar, T>(*static_cast<const TCvar *>(poldVal));
				auto newVal = udm::convert<TCvar, T>(*static_cast<const TCvar *>(pnewVal));
				function(nw, cvar, oldVal, newVal);
			}
		});
	};
	auto &callbacks = it->second;
	callbacks.push_back(CvarCallback {f});
	return callbacks.back().GetFunction();
}
CallbackHandle CVarHandler::RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, const ConVar &, int, int)> &function) { return RegisterConVarCallback<int>(scvar, function); }
CallbackHandle CVarHandler::RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, const ConVar &, std::string, std::string)> &function) { return RegisterConVarCallback<std::string>(scvar, function); }
CallbackHandle CVarHandler::RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, const ConVar &, float, float)> &function) { return RegisterConVarCallback<float>(scvar, function); }
CallbackHandle CVarHandler::RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, const ConVar &, bool, bool)> &function) { return RegisterConVarCallback<bool>(scvar, function); }

bool CVarHandler::InvokeConVarChangeCallbacks(const std::string &cvarName)
{
	auto nCvarName = cvarName;
	ustring::to_lower(nCvarName);
	auto *cv = GetConVar(nCvarName);
	if(cv == nullptr || cv->GetType() != ConType::Var)
		return false;
	SetConVar(nCvarName, static_cast<ConVar *>(cv)->GetString(), true);
	return true;
}

ConVar *CVarHandler::SetConVar(std::string scmd, std::string value, bool bApplyIfEqual)
{
	ConConf *cv = GetConVar(scmd);
	if(cv == nullptr)
		return nullptr;
	if(cv->GetType() != ConType::Var)
		return nullptr;
	ConVar *cvar = static_cast<ConVar *>(cv);
	std::string prev = cvar->GetString();
	if(bApplyIfEqual == false && prev == value)
		return nullptr;
	udm::visit_c(cvar->GetVarType(), [this, &scmd, cvar, &value](auto tag) {
		using T = typename decltype(tag)::type;
		auto it = m_cvarCallbacks.find(scmd);
		if(it == m_cvarCallbacks.end() || it->second.empty()) {
			// We can skip the callbacks
			cvar->SetValue(value);
			return;
		}
		auto &rawValPrev = cvar->GetRawValue();
		auto prevVal = rawValPrev ? *static_cast<T *>(rawValPrev.get()) : T {};
		cvar->SetValue(value);
		auto &rawValNew = cvar->GetRawValue();
		if(!rawValNew)
			return;
		auto &newVal = *static_cast<T *>(rawValNew.get());
		if(it != m_cvarCallbacks.end()) {
			for(auto itCb = it->second.begin(); itCb != it->second.end();) {
				auto &ptrCb = *itCb;
				auto &fc = const_cast<CvarCallback &>(ptrCb).GetFunction();
				if(!fc.IsValid())
					itCb = it->second.erase(itCb);
				else {
					if(!ptrCb.IsLuaFunction())
						fc.Call<void, NetworkState *, const ConVar &, const void *, const void *>(nullptr, *cvar, &prevVal, &newVal);
					++itCb;
				}
			}
		}
	});
	return cvar;
}

ConConf *CVarHandler::GetConVar(std::string scmd)
{
	std::transform(scmd.begin(), scmd.end(), scmd.begin(), ::tolower);
	auto it = m_conVars.find(scmd);
	if(it == m_conVars.end())
		return nullptr;
	return it->second.get();
}

bool CVarHandler::GetConVarInt(std::string scmd, int32_t &outVal)
{
	ConConf *cv = GetConVar(scmd);
	if(cv == NULL || cv->GetType() != ConType::Var)
		return false;
	ConVar *cvar = static_cast<ConVar *>(cv);
	outVal = cvar->GetInt();
	return true;
}
bool CVarHandler::GetConVarString(std::string scmd, std::string &outVal)
{
	ConConf *cv = GetConVar(scmd);
	if(cv == NULL || cv->GetType() != ConType::Var)
		return false;
	ConVar *cvar = static_cast<ConVar *>(cv);
	outVal = cvar->GetString();
	return true;
}
bool CVarHandler::GetConVarFloat(std::string scmd, float &outVal)
{
	ConConf *cv = GetConVar(scmd);
	if(cv == NULL || cv->GetType() != ConType::Var)
		return false;
	ConVar *cvar = static_cast<ConVar *>(cv);
	outVal = cvar->GetFloat();
	return true;
}
bool CVarHandler::GetConVarBool(std::string scmd, bool &outVal)
{
	ConConf *cv = GetConVar(scmd);
	if(cv == NULL || cv->GetType() != ConType::Var)
		return false;
	ConVar *cvar = static_cast<ConVar *>(cv);
	outVal = cvar->GetBool();
	return true;
}
bool CVarHandler::GetConVarFlags(std::string scmd, ConVarFlags &outVal)
{
	ConConf *cv = GetConVar(scmd);
	if(cv == NULL || cv->GetType() != ConType::Var)
		return false;
	ConVar *cvar = static_cast<ConVar *>(cv);
	outVal = cvar->GetFlags();
	return true;
}

int CVarHandler::GetConVarInt(std::string scmd)
{
	auto val = 0;
	GetConVarInt(scmd, val);
	return val;
}
std::string CVarHandler::GetConVarString(std::string scmd)
{
	std::string val {};
	GetConVarString(scmd, val);
	return val;
}
float CVarHandler::GetConVarFloat(std::string scmd)
{
	auto val = 0.f;
	GetConVarFloat(scmd, val);
	return val;
}
bool CVarHandler::GetConVarBool(std::string scmd)
{
	auto val = false;
	GetConVarBool(scmd, val);
	return val;
}
ConVarFlags CVarHandler::GetConVarFlags(std::string scmd)
{
	auto val = ConVarFlags::None;
	GetConVarFlags(scmd, val);
	return val;
}
unsigned int CVarHandler::GetConVarID(std::string scmd)
{
	std::unordered_map<std::string, unsigned int>::iterator i = m_conCommandIDs.find(scmd);
	if(i != m_conCommandIDs.end())
		return i->second;
	ConVarMap *map = GetConVarMap();
	if(map == NULL)
		return 0;
	return map->GetConVarID(scmd);
}

std::map<std::string, std::shared_ptr<ConConf>> &CVarHandler::GetConVars() { return m_conVars; }
const std::map<std::string, std::shared_ptr<ConConf>> &CVarHandler::GetConVars() const { return const_cast<CVarHandler *>(this)->GetConVars(); }

ConVarMap *CVarHandler::GetConVarMap() { return NULL; }

void CVarHandler::FindSimilarConVars(const std::string &input, const std::map<std::string, std::shared_ptr<ConConf>> &cvars, std::vector<SimilarCmdInfo> &similarCmds) const
{
	auto bWasEmpty = similarCmds.empty();
	for(auto &pair : cvars) {
		auto len = ustring::longest_common_substring(input, pair.first);
		auto percentage = len / static_cast<float>(input.length());
		if(percentage < 0.45f) // Only accept results with at least 45% similarity
			continue;
		for(auto &info : similarCmds) {
			if(info.cmd == nullptr || len > info.similarity) {
				info.cmd = &pair.first;
				info.similarity = len;
				break;
			}
			else if(bWasEmpty == false && len == info.similarity && *info.cmd == pair.first) // Avoid duplicates
				break;
		}
	}
}

void CVarHandler::implFindSimilarConVars(const std::string &input, std::vector<SimilarCmdInfo> &similarCmds) const { FindSimilarConVars(input, GetConVars(), similarCmds); }

std::vector<std::string> CVarHandler::FindSimilarConVars(const std::string &input, std::size_t maxCount) const
{
	std::vector<SimilarCmdInfo> similarCmds(maxCount);
	implFindSimilarConVars(input, similarCmds);

	std::vector<std::string> r;
	r.reserve(maxCount);
	for(auto &info : similarCmds) {
		if(info.cmd == nullptr)
			break;
		r.push_back(*info.cmd);
	}
	return r;
}
