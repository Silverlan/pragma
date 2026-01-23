-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

util.register_class("ents.WeaponPhyscannon", BaseEntityComponent)

game.load_sound_scripts("fx_weapon_physcannon.txt", true)

ents.WeaponPhyscannon.ViewModel = "weapons/v_physcannon.wmd"
ents.WeaponPhyscannon.WorldModel = "weapons/w_physics.wmd"
ents.WeaponPhyscannon.ViewFOV = 50

ents.WeaponPhyscannon.PrimaryAttackDelay = 0.5

ents.WeaponPhyscannon.MaxPullDistance = 1500.0
ents.WeaponPhyscannon.PullForce = 600.0

function ents.WeaponPhyscannon:__init()
	BaseEntityComponent.__init(self)
end

local materialsInitialized = false
function ents.WeaponPhyscannon:Initialize()
	self:AddEntityComponent(ents.COMPONENT_WEAPON, "InitializeWeapon")
	self:AddEntityComponent(ents.COMPONENT_PHYSICS)
	self:AddEntityComponent(ents.COMPONENT_SOUND_EMITTER)
	self:AddEntityComponent(ents.COMPONENT_MODEL, "InitializeModel")
	self.m_bCarrying = false

	self:BindEvent(ents.WeaponComponent.EVENT_ON_PRIMARY_ATTACK, "OnPrimaryAttack")
	self:BindEvent(ents.WeaponComponent.EVENT_ON_SECONDARY_ATTACK, "OnSecondaryAttack")
	self:BindEvent(ents.WeaponComponent.EVENT_ON_HOLSTER, "OnHolster")

	self:SetTickPolicy(ents.TICK_POLICY_ALWAYS)
end

function ents.WeaponPhyscannon:InitializeWeapon(component)
	if CLIENT == true then
		component:SetViewModel(self.ViewModel)
		component:SetViewFOV(self.ViewFOV)
		component:SetHideWorldModelInFirstPerson(true)
	end
	component:SetAutomaticPrimary(true)
	component:SetAutomaticSecondary(false)
end

function ents.WeaponPhyscannon:GetOwner()
	local ownableComponent = self:GetEntity():GetComponent(ents.COMPONENT_OWNABLE)
	return (ownableComponent ~= nil) and ownableComponent:GetOwner() or nil
end

function ents.WeaponPhyscannon:FindPhysProp()
	local owner = self:GetOwner()
	local charComponentOwner = (owner ~= nil) and owner:GetComponent(ents.COMPONENT_CHARACTER) or nil
	if charComponentOwner == nil then
		return
	end
	local trData = charComponentOwner:GetAimRayData()
	trData:SetTarget(trData:GetSourceOrigin() + trData:GetDirection() * 400.0)
	local r = phys.raycast(trData)
	if r == false or r.hit == false then
		return
	end
	local physComponent = r.entity:GetComponent(ents.COMPONENT_PHYSICS)
	if physComponent == nil or physComponent:IsPhysicsProp() == false then
		return
	end
	return r.entity, r.position
end

function ents.WeaponPhyscannon:OnPrimaryAttack()
	self:EndHold()
	local wepComponent = self:GetEntity():GetComponent(ents.COMPONENT_WEAPON)
	local prop, hitPos = self:FindPhysProp()
	if CLIENT == true then
		if prop == nil then
			sound.play("weapon_physcannon.dryfire", sound.TYPE_WEAPON)
			if wepComponent ~= nil then
				wepComponent:PlayViewActivity(Animation.ACT_VM_PRIMARY_ATTACK, ents.AnimatedComponent.FPLAYANIM_RESET)
			end
		else
			sound.play("weapon_physcannon.launch", sound.TYPE_WEAPON)
			if wepComponent ~= nil then
				wepComponent:PlayViewActivity(Animation.ACT_VM_SECONDARY_ATTACK, ents.AnimatedComponent.FPLAYANIM_RESET)
			end
			local owner = self:GetOwner()
			if owner ~= nil and owner:IsPlayer() == true then
				local plComponent = owner:GetComponent(ents.COMPONENT_PLAYER)
				if plComponent ~= nil then
					local ang = EulerAngles(math.randomf(-2, -5), math.randomf(-2, 2), 0)
					plComponent:ApplyViewRotationOffset(ang, 0.3)
				end
			end
			self:CreateLaunchParticles(hitPos)
		end
	else
		if prop ~= nil then
			local physComponent = prop:GetComponent(ents.COMPONENT_PHYSICS)
			local owner = self:GetOwner()
			if owner ~= nil and physComponent ~= nil then
				local charComponent = owner:GetComponent(ents.COMPONENT_CHARACTER)
				local trComponent = owner:GetComponent(ents.COMPONENT_TRANSFORM)
				local dir = (charComponent ~= nil) and charComponent:GetViewForward()
					or (trComponent ~= nil) and trComponent:GetForward()
					or nil
				physComponent:ApplyImpulse(dir * 800.0 * physComponent:GetMass(), hitPos)
			end
		end
	end
	if wepComponent ~= nil then
		wepComponent:SetNextAttack(time.cur_time() + self.PrimaryAttackDelay)
	end
end

function ents.WeaponPhyscannon:OnSecondaryAttack()
	local ent = self:GetEntity()
	local wepComponent = ent:GetComponent(ents.COMPONENT_WEAPON)
	if wepComponent ~= nil then
		wepComponent:SetNextAttack(time.cur_time() + self.PrimaryAttackDelay)
	end
	if self:IsCarrying() == true then
		self:EndHold()
		return
	end
	self.m_bPulling = true
	if SERVER == true then
		return
	end
	local entProp = self:FindPhysProp()
	local sndEmitterComponent = ent:GetComponent(ents.COMPONENT_SOUND_EMITTER)
	if sndEmitterComponent ~= nil then
		sndEmitterComponent:EmitSound("weapon_physcannon.tooheavy", sound.TYPE_WEAPON)
	end
	--if(util.is_valid(entProp) == false) then self:EmitSound("weapon_physcannon.tooheavy",sound.TYPE_WEAPON) end
	self:InitHoldSprites()
end

function ents.WeaponPhyscannon:GetPullTarget()
	return self.m_entTarget
end

function ents.WeaponPhyscannon:InitializeConstraint()
	if util.is_valid(self.m_constraint) == true then
		return
	end
	local tgt = self:GetPullTarget()
	local tgtPhysComponent = util.is_valid(tgt) and tgt:GetComponent(ents.COMPONENT_PHYSICS) or nil
	if tgtPhysComponent == nil then
		return
	end
	local physObj = tgtPhysComponent:GetPhysicsObject()
	if physObj == nil then
		return
	end
	local colObj = physObj:GetCollisionObjects()[1]
	if colObj == nil then
		return
	end
	local shape = phys.create_box_shape(Vector(1, 1, 1), phys.get_generic_material()) -- Dummy shape
	local rigid = phys.create_rigid_body(shape, true)
	rigid:SetPos(self:GetHoldPos())
	rigid:SetSimulationEnabled(false)
	rigid:Spawn()

	self.m_rigidBody = rigid
	self.m_constraint = phys.create_ballsocket_constraint(colObj, Vector(), rigid, Vector())
end

function ents.WeaponPhyscannon:GetHoldPos()
	local owner = self:GetOwner()
	if owner == nil then
		return self:GetPos()
	end
	local charComponent = owner:GetComponent(ents.COMPONENT_CHARACTER)
	local trComponent = owner:GetComponent(ents.COMPONENT_TRANSFORM)
	if charComponent == nil or trComponent == nil then
		return self:GetPos()
	end
	return trComponent:GetEyePos()
		+ charComponent:GetViewForward() * 100.0
		- charComponent:GetViewUp() * 10.0
		+ charComponent:GetViewRight() * 10.0
end

function ents.WeaponPhyscannon:DestroyConstraint()
	if util.is_valid(self.m_constraint) == true then
		self.m_constraint:Remove()
	end
	if util.is_valid(self.m_rigidBody) == true then
		self.m_rigidBody:Remove()
	end
end

function ents.WeaponPhyscannon:UpdateConstraintPos()
	if util.is_valid(self.m_rigidBody) == false then
		return
	end
	self.m_rigidBody:SetPos(self:GetHoldPos())
end

function ents.WeaponPhyscannon:IsCarrying()
	return self.m_bCarrying
end

function ents.WeaponPhyscannon:StartCarry(ent)
	if self:IsCarrying() == true then
		return
	end
	self.m_entTarget = ent
	if CLIENT == true then
		local entThis = self:GetEntity()
		local wepComponent = entThis:GetComponent(ents.COMPONENT_WEAPON)
		if wepComponent ~= nil then
			wepComponent:PlayViewActivity(game.Model.Animation.ACT_VM_RELOAD, ents.AnimatedComponent.FPLAYANIM_RESET)
		end
		local sndEmitterComponent = entThis:GetComponent(ents.COMPONENT_SOUND_EMITTER)
		if sndEmitterComponent ~= nil then
			self.m_sndHoldLoop = sndEmitterComponent:CreateSound("weapon_physcannon.hold_loop", sound.TYPE_WEAPON)
			if self.m_sndHoldLoop ~= nil then
				self.m_sndHoldLoop:SetLooping(true)
				self.m_sndHoldLoop:Play()
			end
		end
	end
	self:InitializeConstraint()
	self.m_bCarrying = true
end

function ents.WeaponPhyscannon:EndHold()
	if CLIENT == true then
		self:DestroyHoldSprites()
	end
	self:EndCarry()
end

function ents.WeaponPhyscannon:IsPulling()
	return self.m_bPulling and true or false
end

function ents.WeaponPhyscannon:EndCarry()
	self.m_bPulling = false
	if self:IsCarrying() == false then
		return
	end
	if CLIENT == true then
		local ent = self:GetEntity()
		local wepComponent = ent:GetComponent(ents.COMPONENT_WEAPON)
		if wepComponent ~= nil then
			wepComponent:PlayViewActivity(Animation.ACT_VM_IDLE)
		end
		local sndEmitterComponent = ent:GetComponent(ents.COMPONENT_SOUND_EMITTER)
		if sndEmitterComponent ~= nil then
			sndEmitterComponent:EmitSound("weapon_physcannon.drop", sound.TYPE_WEAPON)
		end
	end
	self:DestroyConstraint()
	self.m_entTarget = nil
	self.m_bCarrying = false
	if self.m_sndHoldLoop ~= nil then
		self.m_sndHoldLoop:Stop()
		self.m_sndHoldLoop = nil
	end
end

function ents.WeaponPhyscannon:OnHolster()
	self:EndHold()
	self.m_tLastTarget = nil
end

function ents.WeaponPhyscannon:OnRemove()
	self:EndHold()
	self.m_tLastTarget = nil
end

function ents.WeaponPhyscannon:OnTick()
	local wepComponent = self:GetEntity():GetComponent(ents.COMPONENT_WEAPON)
	if wepComponent == nil or wepComponent:IsDeployed() == false then
		return
	end
	if CLIENT == true then
		self:UpdateProngs()
	end
	local entPullTarget = self.m_entTarget
	if util.is_valid(entPullTarget) == false then
		entPullTarget = self:FindPhysProp()
	end
	if CLIENT == true then
		if util.is_valid(entPullTarget) == true then
			self.m_tLastTarget = time.cur_time()
			self:OpenProngs()
		elseif self.m_tLastTarget == nil or time.cur_time() - self.m_tLastTarget > 0.8 then
			self:CloseProngs()
		end
	end

	local owner = self:GetOwner()
	local actionInputC = owner:GetComponent(ents.COMPONENT_ACTION_INPUT_CONTROLLER)
	if
		(actionInputC == nil or actionInputC:GetActionInput(input.ACTION_ATTACK2) == false)
		and self:IsCarrying() == false
	then
		self:EndHold()
		return
	end
	local trComponent = util.is_valid(owner) and owner:GetComponent(ents.COMPONENT_TRANSFORM) or nil
	if trComponent ~= nil and self:IsPulling() == true then
		if util.is_valid(entPullTarget) == true then
			local posObj = entPullTarget:GetPos()
			local posDst = trComponent:GetEyePos()
			local dir = posDst - posObj
			local physComponent = entPullTarget:GetComponent(ents.COMPONENT_PHYSICS)
			local velComponent = entPullTarget:GetComponent(ents.COMPONENT_VELOCITY)
			if physComponent ~= nil and velComponent ~= nil then
				local phys = physComponent:GetPhysicsObject()
				local dist = dir:Length()
				if dist > 0.0 and dist <= self.MaxPullDistance then
					dir = dir / dist
					velComponent:AddVelocity(
						dir * self.PullForce * (1.0 - (dist / self.MaxPullDistance)) * time.delta_time()
					) -- TODO: With relative position!
					if dist <= 200.0 then
						self:StartCarry(entPullTarget)
					end
				end
				self:UpdateConstraintPos()
			end
		end
	else
		self:EndHold()
	end
end
ents.COMPONENT_WEAPON_PHYSCANNON =
	ents.register_component("weapon_physcannon", ents.WeaponPhyscannon, ents.EntityComponent.FREGISTER_BIT_NETWORKED)
