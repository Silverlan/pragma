-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

util.register_class("gui.BaseHUD", gui.Base)
function gui.BaseHUD:__init()
	gui.Base.__init(self)
end
function gui.BaseHUD:OnInitialize()
	gui.Base.OnInitialize(self)
	local ent = self:GetTargetEntity()
	if util.is_valid(ent) == false then
		return
	end
	local healthComponent = ent:GetComponent(ents.COMPONENT_HEALTH)
	if healthComponent ~= nil then
		self.m_cbHealthChanged = healthComponent:AddEventCallback(
			ents.HealthComponent.EVENT_ON_HEALTH_CHANGED,
			function(oldHealth, health)
				self:OnHealthChanged(oldHealth, health)
			end
		)
		self.m_cbMaxHealthChanged = healthComponent:GetMaxHealthProperty():AddCallback(function(oldMaxHealth, maxHealth)
			self:OnHealthChanged(healthComponent:GetHealth(), healthComponent:GetHealth())
		end)
		self:OnHealthChanged(healthComponent:GetHealth(), healthComponent:GetHealth())
	end

	local charComponent = ent:GetComponent(ents.COMPONENT_CHARACTER)
	if charComponent ~= nil then
		self.m_cbWeaponChanged = charComponent:AddEventCallback(
			ents.CharacterComponent.EVENT_ON_SET_ACTIVE_WEAPON,
			function(oldWep, weapon)
				self:OnWeaponChanged(weapon)
			end
		)
		self:OnWeaponChanged(charComponent:GetActiveWeapon())
	end
end
function gui.BaseHUD:OnRemove()
	gui.Base.OnRemove(self)
	for _, cb in ipairs({ self.m_cbHealthChanged, self.m_cbMaxHealthChanged, self.m_cbWeaponChanged }) do
		if util.is_valid(cb) == true then
			cb:Remove()
		end
	end
	self:ClearWeaponHooks()
end
function gui.BaseHUD:ClearWeaponHooks()
	for _, cb in ipairs({ self.m_cbPrimaryClipChanged, self.m_cbSecondaryClipChanged }) do
		if util.is_valid(cb) == true then
			cb:Remove()
		end
	end
end
function gui.BaseHUD:GetTargetEntity()
	local pl = ents.get_local_player()
	return (pl ~= nil) and pl:GetEntity() or nil
end
function gui.BaseHUD:OnHealthChanged(oldHealth, newHealth) end
function gui.BaseHUD:GetWeapon()
	return self.m_activeWeapon
end
function gui.BaseHUD:OnPrimaryClipSizeChanged(oldSize, newSize) end
function gui.BaseHUD:OnSecondaryClipSizeChanged(oldSize, newSize) end
function gui.BaseHUD:OnWeaponChanged(weapon)
	self:ClearWeaponHooks()
	self.m_activeWeapon = weapon
	if weapon == nil then
		return
	end
	self.m_cbPrimaryClipChanged = weapon:AddEventCallback(
		ents.WeaponComponent.EVENT_ON_PRIMARY_CLIP_SIZE_CHANGED,
		function(oldSize, size)
			self:OnPrimaryClipSizeChanged(oldSize, size)
		end
	)
	self.m_cbSecondaryClipChanged = weapon:AddEventCallback(
		ents.WeaponComponent.EVENT_ON_SECONDARY_CLIP_SIZE_CHANGED,
		function(oldSize, size)
			self:OnSecondaryClipSizeChanged(oldSize, size)
		end
	)
	local clipSize = weapon:GetPrimaryClipSize()
	self:OnPrimaryClipSizeChanged(clipSize, clipSize)

	clipSize = weapon:GetSecondaryClipSize()
	self:OnSecondaryClipSizeChanged(clipSize, clipSize)
end
