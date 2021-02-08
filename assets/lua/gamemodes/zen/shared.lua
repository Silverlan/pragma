util.register_class("ents.GmZen",BaseEntityComponent)
local Component = ents.GmZen

function Component:__init()
	BaseEntityComponent.__init(self)
end

function Component:Initialize()
	BaseEntityComponent.Initialize(self)
	
	self:AddEntityComponent("gm_generic")
	if(CLIENT) then self:BindEvent(ents.GamemodeComponent.EVENT_ON_GAME_READY,"InitializeHud") end
end
ents.COMPONENT_GM_ZEN = ents.register_component("gm_zen",Component)

