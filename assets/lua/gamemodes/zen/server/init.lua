include("/gamemodes/generic/server/init.lua")
include("../shared.lua")

function game.Zen:OnEntityTakenDamage(ent,dmgInfo,oldHealth,newHealth)
	local n = self:DebugPrintStart("OnEntityTakenDamage")

	console.print_message("Entity " .. ent:GetClass() .. " has taken " .. dmgInfo:GetDamage() .. " damage from ")
	local attacker = dmgInfo:GetAttacker()
	console.print_message((attacker ~= nil) and attacker:GetClass() or "Unknown")
	local inflictor = dmgInfo:GetInflictor()
	if(inflictor ~= nil and inflictor ~= attacker) then console.print_messageln(" using " .. inflictor:GetClass())
	else console.print_messageln("") end
	console.print_messageln("Health has changed from " .. oldHealth .. " to " .. newHealth)

	self:DebugPrintEnd(n)
end
