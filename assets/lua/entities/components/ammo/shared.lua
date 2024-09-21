util.register_class("ents.AmmoComponent", BaseEntityComponent)
function ents.AmmoComponent:__init()
	BaseEntityComponent.__init(self)
	self.m_ammoType = ""
	self.m_pickupAmount = 0
end
function ents.AmmoComponent:Initialize()
	BaseEntityComponent.Initialize(self)
	self:AddEntityComponent(ents.COMPONENT_PHYSICS)
	self:AddEntityComponent(ents.COMPONENT_RENDER)
	self:AddEntityComponent(ents.COMPONENT_MODEL)

	self:BindEvent(ents.PhysicsComponent.EVENT_ON_PHYSICS_INITIALIZED, "OnPhysicsInitialized")
	if SERVER then
		self:AddEntityComponent(ents.COMPONENT_USABLE)
		self:BindEvent(ents.UsableComponent.EVENT_ON_USE, "OnUse")
		self:BindEvent(ents.UsableComponent.EVENT_CAN_USE, "CanUse")
	end
end
function ents.AmmoComponent:OnPhysicsInitialized(physObj)
	local physComponent = self:GetEntity():GetComponent(ents.COMPONENT_PHYSICS)
	if physComponent ~= nil then
		physComponent:SetCollisionFilterGroup(phys.COLLISIONMASK_ITEM)
	end
end
function ents.AmmoComponent:SetAmmoType(ammoType)
	self.m_ammoType = ammoType
end
function ents.AmmoComponent:GetAmmoType()
	return self.m_ammoType
end
function ents.AmmoComponent:SetPickupAmount(am)
	self.m_pickupAmount = am
end
function ents.AmmoComponent:GetPickupAmount()
	return self.m_pickupAmount
end
ents.register_component("ammo", ents.AmmoComponent, "gameplay")

ents.AmmoComponent.EVENT_ON_PICKED_UP = ents.register_component_event(ents.COMPONENT_AMMO, "on_picked_up")
