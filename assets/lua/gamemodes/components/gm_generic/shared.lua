util.register_class("ents.GmGeneric", BaseEntityComponent)
local Component = ents.GmGeneric

game.load_sound_scripts("fx_footsteps.udm")
function Component:__init()
	BaseEntityComponent.Initialize(self)
end

function Component:Initialize()
	BaseEntityComponent.Initialize(self)

	self:BindEvent(ents.GamemodeComponent.EVENT_ON_PLAYER_READY, "OnPlayerReady")
	self:BindEvent(ents.GamemodeComponent.EVENT_ON_PLAYER_SPAWNED, "OnPlayerSpawned")
end

function Component:OnPlayerReady(pl)
	local ent = pl:GetEntity()
	ent:AddComponent(ents.COMPONENT_DAMAGEABLE)
	ent:AddComponent(ents.COMPONENT_RENDER)
	ent:AddComponent(ents.COMPONENT_ANIMATED)
	ent:AddComponent(ents.COMPONENT_MODEL)
	ent:AddComponent(ents.COMPONENT_PHYSICS)
	ent:AddComponent(ents.COMPONENT_CHARACTER)

	self:InitializeDefaultPlayerDimensions(pl)
	self:InitializePlayer(pl)
end

function Component:InitializeDefaultPlayerDimensions(pl)
	pl:SetStandHeight(72.0)
	pl:SetStandEyeLevel(64.0)
	pl:SetCrouchHeight(36.0)
	pl:SetCrouchEyeLevel(28.0)
	pl:SetWalkSpeed(63.33)
	pl:SetRunSpeed(190.0)
	pl:SetSprintSpeed(320.0)
	pl:SetCrouchedWalkSpeed(63.33)
	local charComponent = pl:GetEntity():GetComponent(ents.COMPONENT_CHARACTER)
	if charComponent ~= nil then
		charComponent:SetJumpPower(240.0)
	end
end

function Component:InitializePlayer(pl)
	local ent = pl:GetEntity()
	local charComponent = ent:AddComponent(ents.COMPONENT_CHARACTER)
	if charComponent ~= nil then
		charComponent:AddEventCallback(
			ents.CharacterComponent.EVENT_PLAY_FOOTSTEP_SOUND,
			function(footType, surfMat, intensity)
				local sndComponent = charComponent:GetEntity():GetComponent(ents.COMPONENT_SOUND_EMITTER)
				if sndComponent == nil then
					return
				end
				local maxGain = 0.5
				local sndInfo = ents.SoundEmitterComponent.SoundInfo(maxGain * intensity, 1.0)
				sndInfo.transmit = false
				sndComponent:EmitSound(
					surfMat:GetFootstepSound(),
					bit.bor(sound.TYPE_EFFECT, sound.TYPE_PLAYER),
					sndInfo
				)
			end
		)
	end
	self:InitializePlayerModel(pl)
end

function Component:InitializePlayerModel(pl)
	local ent = pl:GetEntity()
	local mdlComponent = ent:GetComponent(ents.COMPONENT_MODEL)
	if CLIENT == true then
		local vb = ents.get_view_body()
		if vb ~= nil then
			local mdlComponent = vb:GetModelComponent()
			if mdlComponent ~= nil then
				mdlComponent:SetModel("player/soldier_legs.wmd")
			end
		end
		return
	end
	if mdlComponent ~= nil then
		mdlComponent:SetModel("player/soldier.wmd")
	end
end

function Component:OnPlayerSpawned(pl)
	local ent = pl:GetEntity()
	local healthComponent = ent:GetComponent(ents.COMPONENT_HEALTH)
	if healthComponent ~= nil then
		healthComponent:SetMaxHealth(100)
	end

	local physComponent = ent:GetComponent(ents.COMPONENT_PHYSICS)
	if physComponent ~= nil then
		physComponent:SetCollisionBounds(Vector(-16, 0, -16), Vector(16, pl:GetStandHeight(), 16))
		physComponent:InitializePhysics(phys.TYPE_CAPSULECONTROLLER)
	end

	if SERVER == true then
		if healthComponent ~= nil then
			healthComponent:SetHealth(100)
		end

		local pos, ang = self:FindSpawnPoint()
		local trComponent = ent:GetTransformComponent()
		if trComponent ~= nil then
			trComponent:SetPos(pos)
			trComponent:SetAngles(ang)
		end

		local charComponent = ent:GetCharacterComponent()
		if charComponent ~= nil then
			charComponent:SetViewAngles(ang)
		end
	end
end
ents.register_component("gm_generic", Component, "gameplay")
