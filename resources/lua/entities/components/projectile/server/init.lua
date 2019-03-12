include("../shared.lua")

ents.ProjectileComponent.EVENT_ON_PROJECTILE_IMPACT = ents.register_component_event("ON_PROJECTILE_IMPACT")
ents.ProjectileComponent.EVENT_ON_PROJECTILE_HIT_DAMAGE = ents.register_component_event("ON_PROJECTILE_HIT_DAMAGE")
function ents.ProjectileComponent:__init()
	BaseEntityComponent.__init(self)
	self.m_tRemove = 0
end

function ents.ProjectileComponent:InitializeHealth(component)
	component:SetHealth(4)
	local healthProp = component:GetHealthProperty()
	self:FlagCallbackForRemoval(healthProp:AddCallback(function(oldHealth,newHealth)
		if(newHealth > 0) then return end
		self:OnHit()
	end),ents.EntityComponent.CALLBACK_TYPE_ENTITY)
end

function ents.ProjectileComponent:OnHit()
	if(self.m_bHit == true) then return end
	self.m_bHit = true
	self:GetEntity().RemoveSafely() -- We might be in the middle of the simulation; Remove the entity in the next tick instead
end

function ents.ProjectileComponent:OnStartTouch(entTouch,physTouch)
	--if(self.m_bHit == true) then return end -- Prevent multiple hits at once
	if(entTouch ~= nil) then self:Hit(entTouch,physTouch) end
	self:OnHit()
end

function ents.ProjectileComponent:Hit(ent,phys)
	local pos = self:GetPos()
	local dmg = game.DamageInfo()
	if(self.m_attacker ~= nil) then dmg:SetAttacker(self.m_attacker) end
	dmg:SetInflictor(self)
	dmg:SetDamage(1)
	dmg:SetForce(self:GetVelocity())
	dmg:SetHitPosition(pos)
	dmg:SetSource(pos)
	self:BroadcastEvent(self.EVENT_ON_PROJECTILE_HIT_DAMAGE,dmg,ent,phys)
	ent:TakeDamage(dmg)
	if(self.m_bHit ~= true) then self:BroadcastEvent(self.EVENT_ON_PROJECTILE_IMPACT) end
end

function ents.ProjectileComponent:SetAttacker(attacker)
	local physComponent = self:GetEntity():GetComponent(ents.COMPONENT_PHYSICS)
	if(util.is_valid_entity(self.m_attacker) == true) then
		local physComponentAttacker = self.m_attacker:GetComponent(ents.COMPONENT_PHYSICS)
		if(physComponent ~= nil and physComponentAttacker ~= nil) then
			physComponent:ResetCollisions(physComponentAttacker)
		end
	end
	self.m_attacker = attacker
	if(util.is_valid_entity(attacker) == false or physComponent == nil) then return end
	local physComponentAttacker = attacker:GetComponent(ents.COMPONENT_PHYSICS)
	if(physComponentAttacker == nil) then return end
	physComponent:DisableCollisions(physComponentAttacker)
end
function ents.ProjectileComponent:GetAttacker() return self.m_attacker end

function ents.ProjectileComponent:OnEntitySpawn()
	BaseEntityComponent.OnEntitySpawn(self)
	self.m_tRemove = time.cur_time() +15 -- Remove the projectile after a certain amount of time, in case it got stuck or it's in the void
end

function ents.ProjectileComponent:Think(tDelta)
	BaseEntityComponent.Think(self,tDelta)
	local t = time.cur_time()
	self:SetNextThink(t +0.25)
	if(t >= self.m_tRemove) then self:Remove() end
end

function ents.ProjectileComponent:CanTrigger(phys) return true end
