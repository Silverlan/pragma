util.register_class("ents.LuaScriptComponent",BaseEntityComponent)

ents.LuaScriptComponent:RegisterMember("ScriptFile",util.VAR_TYPE_STRING,"",ents.BaseEntityComponent.MEMBER_FLAG_DEFAULT,1)
ents.LuaScriptComponent:RegisterMember("Code",util.VAR_TYPE_STRING,"",ents.BaseEntityComponent.MEMBER_FLAG_DEFAULT,1)
function ents.LuaScriptComponent:__init()
	BaseEntityComponent.__init(self)
end

function ents.LuaScriptComponent:Initialize()
	BaseEntityComponent.Initialize(self)
	
	self:AddEntityComponent(ents.COMPONENT_IO)
	self:BindEvent(ents.IOComponent.EVENT_HANDLE_INPUT,"HandleInput")
end

function ents.LuaScriptComponent:Execute()
	local scriptFile = self:GetScriptFile()
	if(#scriptFile > 0) then
		assert(loadstring("include(\"" .. scriptFile .. "\")"))()
	end
	
	local code = self:GetCode()
	if(#code > 0) then
		assert(loadstring(code))()
	end
end

function ents.LuaScriptComponent:HandleInput(input,activator,caller,data)
	if(input == "execute") then
		self:Execute()
		return util.EVENT_REPLY_HANDLED
	end
	return util.EVENT_REPLY_UNHANDLED
end

function ents.LuaScriptComponent:OnEntitySpawn()
	if(bit.band(self:GetEntity():GetSpawnFlags(),1) ~= 0) then
		self:Execute()
		local ioComponent = self:GetEntity():GetComponent(ents.COMPONENT_IO)
		if(ioComponent ~= nil) then ioComponent:FireOutput("OnExecuted",self:GetEntity()) end
	end
end
ents.COMPONENT_LUA_SCRIPT = ents.register_component("lua_script",ents.LuaScriptComponent,ents.EntityComponent.FREGISTER_NONE)
