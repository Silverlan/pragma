-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

include("/gui/wibasehud.lua")

util.register_class("gui.ZenHUD", gui.BaseHUD)
function gui.ZenHUD:__init()
	gui.BaseHUD.__init(self)
end
function gui.ZenHUD:CreateText(color, x)
	local p = gui.create("WIText", self)
	p:SetColor(color)
	p:SetX(x or 0)
	p:EnableShadow(true)
	p:SetShadowOffset(Vector2(1, 1))
	p:SetShadowColor(Color.Black)
	return p
end
function gui.ZenHUD:OnInitialize()
	local pHealth = self:CreateText(Color.Red)
	self.m_pHealth = pHealth

	local pAmmo = self:CreateText(Color.Lime, 100)
	self.m_pAmmo = pAmmo

	local pAmmo2 = self:CreateText(Color.Aqua, 200)
	self.m_pAmmo2 = pAmmo2

	gui.BaseHUD.OnInitialize(self)
end
function gui.ZenHUD:OnHealthChanged(oldHealth, newHealth)
	gui.BaseHUD.OnHealthChanged(self, oldHealth, newHealth)
	if util.is_valid(self.m_pHealth) == false then
		return
	end
	local ent = self:GetTargetEntity()
	local healthComponent = util.is_valid(ent) and ent:GetComponent(ents.COMPONENT_HEALTH) or nil
	if healthComponent == nil then
		return
	end
	self.m_pHealth:SetText(newHealth .. " / " .. healthComponent:GetMaxHealth())
	self.m_pHealth:SizeToContents()
end
function gui.ZenHUD:OnPrimaryClipSizeChanged(oldSize, newSize)
	gui.BaseHUD.OnPrimaryClipSizeChanged(self, oldSize, newSize)
	if util.is_valid(self.m_pAmmo) == false then
		return
	end
	local wep = self:GetWeapon()
	self.m_pAmmo:SetText(wep:GetPrimaryClipSize() .. " / " .. wep:GetMaxPrimaryClipSize())
	self.m_pAmmo:SizeToContents()
end
function gui.ZenHUD:OnSecondaryClipSizeChanged(oldSize, newSize)
	gui.BaseHUD.OnSecondaryClipSizeChanged(self, oldSize, newSize)
	if util.is_valid(self.m_pAmmo2) == false then
		return
	end
	local wep = self:GetWeapon()
	self.m_pAmmo2:SetText(wep:GetSecondaryClipSize() .. " / " .. wep:GetMaxSecondaryClipSize())
	self.m_pAmmo2:SizeToContents()
end
function gui.ZenHUD:OnWeaponChanged(weapon)
	gui.BaseHUD.OnWeaponChanged(self, weapon)
end
gui.register("WIZenHUD", gui.ZenHUD)
