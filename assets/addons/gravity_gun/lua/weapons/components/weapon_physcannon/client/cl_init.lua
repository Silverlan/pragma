include("../shared.lua")

function ents.WeaponPhyscannon:AttachParticleEntity(ent, att)
	local vm = ents.get_view_model()
	local attComponent = ent:AddComponent(ents.COMPONENT_ATTACHMENT)
	if attComponent ~= nil then
		local attInfo = ents.AttachmentComponent.AttachmentInfo()
		attInfo.flags = bit.bor(
			ents.AttachmentComponent.FATTACHMENT_MODE_POSITION_ONLY,
			ents.AttachmentComponent.FATTACHMENT_MODE_UPDATE_EACH_FRAME,
			ents.AttachmentComponent.FATTACHMENT_MODE_SNAP_TO_ORIGIN
		)
		attComponent:AttachToAttachment(vm, att, attInfo)
	end
end

function ents.WeaponPhyscannon:CreateSprite(att, texture, scale, bloomScale, color, blackToAlpha)
	local vm = ents.get_view_model()
	local ent = ents.create("env_sprite")
	ent:SetKeyValue("texture", texture)
	ent:SetKeyValue("scale", scale)
	ent:SetKeyValue("bloom_scale", bloomScale)
	ent:SetKeyValue("color", color)
	ent:SetKeyValue("fade_in_time", "0.1")
	ent:SetKeyValue("fade_out_time", "0.1")
	ent:SetKeyValue("render_mode", tostring(game.SCENE_RENDER_PASS_VIEW))
	if blackToAlpha == true then
		ent:SetKeyValue("spawnflags", "4096")
	end
	self:AttachParticleEntity(ent, att)
	ent:Spawn()
	return ent
end

function ents.WeaponPhyscannon:CreateLaunchParticles(hitPos)
	local vm = ents.get_view_model()
	local mdlComponentVm = (vm ~= nil) and vm:GetComponent(ents.COMPONENT_MODEL) or nil
	local trComponentVm = (vm ~= nil) and vm:GetComponent(ents.COMPONENT_TRANSFORM) or nil
	if mdlComponentVm == nil or trComponentVm == nil then
		return
	end
	local posMuzzle, rotMuzzle = mdlComponentVm:GetAttachmentTransform("muzzle")
	posMuzzle, rotMuzzle = trComponentVm:LocalToWorld(posMuzzle, rotMuzzle)

	local ptBeam = game.create_particle_system({
		maxparticles = 1,
		max_node_count = 2,
		emission_rate = 10000,
		material = "sprites/orangelight1",
		radius = 10.0,
		bloom_scale = 0,
		color = "2048 1400 1400 2048",
		black_to_alpha = 1,
	})
	ptBeam:AddRenderer("beam", {
		node_start = 1,
		node_end = 2,
		curvature = 0.1,
	})
	ptBeam:AddInitializer("lifetime_random", {
		lifetime_min = 0.2,
		lifetime_max = 0.2,
	})
	ptBeam:AddOperator("color_fade", {
		color = "255 255 255 0",
		fade_start = 0,
		fade_end = 0.2,
	})
	ptBeam:SetRemoveOnComplete(true)
	ptBeam:SetNodeTarget(1, posMuzzle)
	ptBeam:SetNodeTarget(2, hitPos)
	self:AttachParticleEntity(ptBeam:GetEntity(), "muzzle")
	ptBeam:SetSceneRenderPass(game.SCENE_RENDER_PASS_VIEW)
	ptBeam:Start()

	local ptFlash = game.create_particle_system({
		maxparticles = 1,
		max_node_count = 2,
		emission_rate = 10000,
		material = "sprites/orangecore2",
		radius = 50.0,
		bloom_scale = 10,
		color = "255 200 200 255",
		black_to_alpha = 1,
	})
	ptFlash:AddRenderer("sprite", {})
	ptFlash:AddInitializer("lifetime_random", {
		lifetime_min = 0.2,
		lifetime_max = 0.2,
	})
	ptFlash:AddOperator("color_fade", {
		color = "255 255 255 0",
		fade_start = 0,
		fade_end = 0.2,
	})
	ptFlash:SetRemoveOnComplete(true)
	local trComponent = ptFlash:GetEntity():GetComponent(ents.COMPONENT_TRANSFORM)
	if trComponent ~= nil then
		trComponent:SetPos(posMuzzle)
	end
	self:AttachParticleEntity(ptFlash:GetEntity(), "muzzle")
	ptFlash:SetSceneRenderPass(game.SCENE_RENDER_PASS_VIEW)
	ptFlash:Start()
end

function ents.WeaponPhyscannon:InitHoldSprites()
	if self.m_tHoldSprites ~= nil then
		return
	end
	local vm = ents.get_view_model()
	if util.is_valid(vm) == false then
		return
	end
	self.m_tHoldSprites = {}
	for _, att in ipairs({ "fork1t", "fork2t" }) do
		local ent = self:CreateSprite(att, "sprites/orangeflare1", "4", "2", "1024 1024 1024 255", true)
		table.insert(self.m_tHoldSprites, ent)
	end

	local ent = self:CreateSprite("muzzle", "sprites/light_glow02", "12", "20", "256 128 0 255", true)
	table.insert(self.m_tHoldSprites, ent)
end

function ents.WeaponPhyscannon:DestroyHoldSprites()
	if self.m_tHoldSprites == nil then
		return
	end
	for _, ent in ipairs(self.m_tHoldSprites) do
		if util.is_valid(ent) == true then
			local spriteComponent = ent:GetComponent(ents.COMPONENT_SPRITE)
			if spriteComponent ~= nil then
				spriteComponent:StopAndRemoveEntity()
			else
				ent:RemoveSafely()
			end
		end
	end
	self.m_tHoldSprites = nil
end

function ents.WeaponPhyscannon:AreProngsOpen()
	return (self.m_bProngsOpen == true) and true or false
end

function ents.WeaponPhyscannon:OpenProngs()
	if self:AreProngsOpen() == true then
		return
	end
	self.m_bProngsOpen = true
	local sndEmitterComponent = self:GetEntity():GetComponent(ents.COMPONENT_SOUND_EMITTER)
	if sndEmitterComponent ~= nil then
		sndEmitterComponent:EmitSound("weapon_physcannon.claws_open", sound.TYPE_WEAPON)
	end
end

function ents.WeaponPhyscannon:CloseProngs()
	if self:AreProngsOpen() == false then
		return
	end
	self.m_bProngsOpen = false
	local sndEmitterComponent = self:GetEntity():GetComponent(ents.COMPONENT_SOUND_EMITTER)
	if sndEmitterComponent ~= nil then
		sndEmitterComponent:EmitSound("weapon_physcannon.claws_close", sound.TYPE_WEAPON)
	end
end

function ents.WeaponPhyscannon:UpdateProngs()
	local vm = ents.get_view_model()
	local animComponent = (vm ~= nil) and vm:GetComponent(ents.COMPONENT_ANIMATED) or nil
	if animComponent == nil then
		return
	end
	local v = animComponent:GetBlendController("active")
	local vDst = self:AreProngsOpen() and 1.0 or 0.0
	v = math.approach(v, vDst, time.delta_time() * 8.0)
	animComponent:SetBlendController("active", v)
end
