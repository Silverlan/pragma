util.register_class("ents.ProjectileComponent", BaseEntityComponent)

function ents.ProjectileComponent:Initialize()
	BaseEntityComponent.Initialize(self)

	self:AddEntityComponent(ents.COMPONENT_PHYSICS, "InitializePhysics")
	self:AddEntityComponent(ents.COMPONENT_VELOCITY)
	self:AddEntityComponent(ents.COMPONENT_MODEL)
	if SERVER then
		self:AddEntityComponent(ents.COMPONENT_HEALTH, "InitializeHealth")
		self:AddEntityComponent(ents.COMPONENT_TOUCH)

		self:BindEvent(ents.TouchComponent.EVENT_ON_START_TOUCH, "OnStartTouch")
	end
end

function ents.ProjectileComponent:SetupPhysics(radius, mass)
	if self:BroadcastEvent(self.EVENT_INITIALIZE_PHYSICS, { radius, mass }) == util.EVENT_REPLY_HANDLED then
		return
	end
	local physComponent = self:GetEntity():GetComponent(ents.COMPONENT_PHYSICS)
	if physComponent == nil then
		return
	end
	radius = self.m_physRadius or 2.0
	mass = self.m_physMass or 1.0
	self.m_physRadius = nil
	self.m_physMass = nil
	local shape = phys.create_sphere_shape(radius)
	if shape ~= nil then
		local phys = physComponent:InitializePhysics(shape, mass)
		if phys ~= nil then
			phys:WakeUp()
		end
	end
end

function ents.ProjectileComponent:SetPhysicsProperties(radius, mass)
	self.m_physRadius = radius
	self.m_physMass = mass
end

function ents.ProjectileComponent:InitializePhysics(component)
	if SERVER then
		component:SetCollisionCallbacksEnabled(true)
	end
	self:SetupPhysics()
end
ents.register_component(
	"projectile",
	ents.ProjectileComponent,
	"gameplay",
	ents.EntityComponent.FREGISTER_BIT_HIDE_IN_EDITOR
)

ents.ProjectileComponent.EVENT_ON_PROJECTILE_IMPACT = ents.register_component_event(ents.COMPONENT_PROJECTILE, "impact")
ents.ProjectileComponent.EVENT_ON_PROJECTILE_HIT_DAMAGE =
	ents.register_component_event(ents.COMPONENT_PROJECTILE, "hit_damage")
ents.ProjectileComponent.EVENT_INITIALIZE_PHYSICS =
	ents.register_component_event(ents.COMPONENT_PROJECTILE, "initialize_physics")
