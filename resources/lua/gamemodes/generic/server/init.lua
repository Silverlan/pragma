include("../shared.lua")

function game.Generic:Think()
	game.Base.Think(self)
end

function game.Generic:Tick()
	game.Base.Tick(self)
end

function game.Generic:OnEntityTakeDamage(ent,dmgInfo)
end

function game.Generic:OnPlayerDeath(victim,attacker,inflictor)
end

function game.Generic:FindSpawnPoint()
	local tEnts = ents.get_all({ents.IteratorFilterComponent(ents.COMPONENT_PLAYER_SPAWN),ents.IteratorFilterComponent(ents.COMPONENT_TRANSFORM)})
	local numSpawnPoints = #tEnts
	if(numSpawnPoints == 0) then return Vector(0,0,0),EulerAngles(0,0,0) end
	local r = math.random(1,numSpawnPoints)
	local ent = tEnts[r]
	if(ent:IsValid() == false) then return Vector(0,0,0),EulerAngles(0,0,0) end
	local trComponent = ent:GetTransformComponent()
	return trComponent:GetPos(),EulerAngles(0,trComponent:GetYaw(),0)
end

function game.Generic:OnActionInput(pl,action,bPressed)
	local charComponent = pl:GetEntity():GetCharacterComponent()
	if(charComponent ~= nil and charComponent:IsDead() and (action == input.ACTION_ATTACK or action == input.ACTION_ATTACK2)) then
		pl:Respawn()
		return false
	end
	return true
end
