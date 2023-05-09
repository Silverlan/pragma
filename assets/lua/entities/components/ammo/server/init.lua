include("../shared.lua")

function ents.AmmoComponent:OnUse(ent)
	if self.m_bPickedUp == true then
		return
	end
	local charComponent = ent:GetComponent(ents.COMPONENT_CHARACTER)
	if charComponent == nil then
		return
	end
	self.m_bPickedUp = true
	charComponent:AddAmmo(self:GetAmmoType(), self:GetPickupAmount())
	self:BroadcastEvent(ents.AmmoComponent.EVENT_ON_PICKED_UP, { ent })
	self:GetEntity():RemoveSafely()
end

function ents.AmmoComponent:CanUse(entUser)
	return true
end

function ents.AmmoComponent:CanUse(pl)
	return true
end
