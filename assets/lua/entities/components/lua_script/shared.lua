-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

util.register_class("ents.LuaScriptComponent", BaseEntityComponent)

ents.LuaScriptComponent:RegisterMember("ScriptFile", udm.TYPE_STRING, "", {
	specializationType = ents.ComponentInfo.MemberInfo.SPECIALIZATION_TYPE_FILE,
	metaData = {
		rootPath = "lua/",
		basePath = "",
		extensions = { "lua" },
		stripExtension = false,
	},
}, "def")
ents.LuaScriptComponent:RegisterMember("Code", udm.TYPE_STRING, "", {}, "def")
ents.LuaScriptComponent:RegisterMember("ExecuteOnSpawn", udm.TYPE_BOOLEAN, false, {}, "def")
function ents.LuaScriptComponent:__init()
	BaseEntityComponent.__init(self)
end

function ents.LuaScriptComponent:Initialize()
	BaseEntityComponent.Initialize(self)

	self:AddEntityComponent(ents.COMPONENT_IO)
	self:BindEvent(ents.IOComponent.EVENT_HANDLE_INPUT, "HandleInput")
end

function ents.LuaScriptComponent:Execute()
	local scriptFile = self:GetScriptFile()
	if #scriptFile > 0 then
		assert(loadstring('include("' .. scriptFile .. '",true)'))()
	end

	local code = self:GetCode()
	if #code > 0 then
		assert(loadstring(code))()
	end
end

function ents.LuaScriptComponent:HandleInput(input, activator, caller, data)
	if input == "execute" then
		self:Execute()
		return util.EVENT_REPLY_HANDLED
	end
	return util.EVENT_REPLY_UNHANDLED
end

function ents.LuaScriptComponent:OnEntitySpawn()
	if bit.band(self:GetEntity():GetSpawnFlags(), 1) ~= 0 or self:GetExecuteOnSpawn() then
		self:Execute()
		local ioComponent = self:GetEntity():GetComponent(ents.COMPONENT_IO)
		if ioComponent ~= nil then
			ioComponent:FireOutput("OnExecuted", self:GetEntity())
		end
	end
end
ents.COMPONENT_LUA_SCRIPT =
	ents.register_component("lua_script", ents.LuaScriptComponent, ents.EntityComponent.FREGISTER_NONE)
