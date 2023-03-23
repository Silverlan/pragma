--- @meta
--- 
--- @class game
game = {}

--- 
function game.clear_unused_materials() end

--- 
--- @return ents.BaseGamemodeComponent ret0
function game.get_game_mode() end

--- 
--- @param mapName string
--- @overload fun(mapName: string, landmarkName: string): 
function game.change_map(mapName) end

--- 
--- @param name string
--- @return bool ret0
--- @overload fun(name: string, damage: int): bool
--- @overload fun(name: string, damage: int, force: number): bool
--- @overload fun(name: string, damage: int, force: number, damageType: enum DAMAGETYPE): bool
function game.register_ammo_type(name) end

--- 
--- @return number ret0
function game.get_time_scale() end

--- 
--- @param arg0 class CGame
function game.clear_gameplay_control_camera(arg0) end

--- 
--- @param name string
--- @return int ret0
function game.get_ammo_type_id(name) end

--- 
--- @param pos math.Vector
--- @return number ret0
--- @overload fun(pos: vector.Vector): number
function game.get_sound_intensity(pos) end

--- 
--- @return bool ret0
function game.is_game_mode_initialized() end

--- 
--- @param typeId int
--- @return string ret0
function game.get_ammo_type_name(typeId) end

--- 
--- @param pos math.Vector
--- @return math.Vector ret0
--- @overload fun(pos: vector.Vector): math.Vector
function game.get_light_color(pos) end

--- 
--- @return nav.Mesh ret0
function game.get_nav_mesh() end

--- 
--- @return bool ret0
--- @overload fun(reload: bool): bool
function game.load_nav_mesh() end

--- 
--- @param identifier string
--- @return game.Material ret0
function game.get_material(identifier) end

--- 
--- @return bool ret0
function game.is_map_loaded() end

--- 
--- @return string ret0
function game.get_map_name() end

--- 
--- @return enum Game::GameFlags ret0
function game.get_game_state_flags() end

--- 
--- @param game class Game
--- @param dt number
function game.update_animations(game, dt) end

--- 
--- @return math.Vector ret0
function game.get_gravity() end

--- 
--- @param gravity math.Vector
function game.set_gravity(gravity) end

--- 
--- @param identifier string
--- @param function unknown
--- @return util.Callback ret0
function game.add_callback(identifier, function_) end

--- 
--- @return enum pragma::rendering::RenderMask ret0_1
--- @return enum pragma::rendering::RenderMask ret0_2
function game.get_primary_camera_render_mask() end

--- 
--- @param name string
--- @return any ret0
function game.load_model(name) end

--- 
--- @param fileName string
--- @overload fun(fileName: string, precache: bool): 
function game.load_sound_scripts(fileName) end

--- 
--- @param mdlName string
--- @overload fun(mdl: string): 
function game.precache_model(mdlName) end

--- 
--- @param mdlName string
--- @return game.Model ret0
function game.get_model(mdlName) end

--- 
--- @param mat string
--- @return game.Material ret0
--- @overload fun(mat: string, reload: bool): game.Material
--- @overload fun(mat: string): game.Material
--- @overload fun(mat: string, reload: bool): game.Material
--- @overload fun(mat: string, reload: bool, loadInstantly: bool): game.Material
function game.load_material(mat) end

--- 
--- @param timeScale number
function game.set_time_scale(timeScale) end

--- 
--- @param game class Game
--- @param identifier string
--- @return any ret0
--- @overload fun(game: class Game, identifier: string, arg0: any): any
--- @overload fun(game: class Game, identifier: string, arg0: any, arg1: any): any
--- @overload fun(game: class Game, identifier: string, arg0: any, arg1: any, arg2: any): any
--- @overload fun(game: class Game, identifier: string, arg0: any, arg1: any, arg2: any, arg3: any): any
--- @overload fun(game: class Game, identifier: string, arg0: any, arg1: any, arg2: any, arg3: any, arg4: any): any
--- @overload fun(game: class Game, identifier: string, arg0: any, arg1: any, arg2: any, arg3: any, arg4: any, arg5: any): any
function game.call_callbacks(game, identifier) end

--- 
--- @param identifier string
function game.clear_callbacks(identifier) end

--- 
--- @param mat string
function game.precache_material(mat) end

--- 
--- @param arg0 class CGame
--- @param arg1 ents.CameraComponent
function game.set_gameplay_control_camera(arg0, arg1) end

--- 
--- @return game.Material ret0
function game.get_error_material() end

--- 
--- @param particle string
--- @return bool ret0
--- @overload fun(particle: string, reload: bool): bool
function game.precache_particle_system(particle) end

--- 
--- @param arg0 class CGame
--- @return int ret0
function game.get_number_of_scenes_queued_for_rendering(arg0) end

--- 
--- @param arg0 class CGame
--- @param arg1 int
--- @return game.DrawSceneInfo ret0
function game.get_queued_scene_render_info(arg0, arg1) end

--- 
--- @param arg0 class CGame
--- @return ents.CameraComponent ret0
function game.get_gameplay_control_camera(arg0) end

--- 
--- @param arg0 class CGame
function game.reset_gameplay_control_camera(arg0) end

--- 
--- @param file file.File
--- @return prosper.Texture ret0
--- @overload fun(file: file.File, loadFlags: enum util::AssetLoadFlags): prosper.Texture
--- @overload fun(file: file.File, cacheName: string): prosper.Texture
--- @overload fun(file: file.File, cacheName: string, loadFlags: enum util::AssetLoadFlags): prosper.Texture
--- @overload fun(name: string): prosper.Texture
--- @overload fun(name: string, loadFlags: enum util::AssetLoadFlags): prosper.Texture
function game.load_texture(file) end

--- 
--- @param shader string
--- @return game.Material ret0
--- @overload fun(identifier: string, shader: string): game.Material
function game.create_material(shader) end


--- 
--- @class game.ValueDriverDescriptor
--- @field expression string 
--- @overload fun(arg2: string):game.ValueDriverDescriptor
--- @overload fun(arg2: string, arg3: map):game.ValueDriverDescriptor
--- @overload fun(arg2: string, arg3: map, arg4: map):game.ValueDriverDescriptor
game.ValueDriverDescriptor = {}

--- 
--- @param arg1 string
--- @param arg2 string
function game.ValueDriverDescriptor:AddReference(arg1, arg2) end

--- 
--- @param name string
--- @param udmType classObject
--- @overload fun(name: string, prop: udm.Property): 
function game.ValueDriverDescriptor:AddConstant(name, udmType) end

--- 
--- @return map ret0
function game.ValueDriverDescriptor:GetConstants() end

--- 
function game.ValueDriverDescriptor:__tostring() end

--- 
--- @return map ret0
function game.ValueDriverDescriptor:GetReferences() end


--- 
--- @class game.SurfaceMaterial
game.SurfaceMaterial = {}

--- 
--- @return string ret0
function game.SurfaceMaterial:GetFootstepSound() end

--- 
--- @return number ret0
function game.SurfaceMaterial:GetWavePropagation() end

--- 
--- @param arg1 number
function game.SurfaceMaterial:SetAudioHighFrequencyTransmission(arg1) end

--- 
--- @return number ret0
function game.SurfaceMaterial:GetPBRMetalness() end

--- 
--- @return number ret0
function game.SurfaceMaterial:GetWaveStiffness() end

--- 
--- @return string ret0
function game.SurfaceMaterial:GetHardImpactSound() end

--- 
--- @param radiusRGB math.Vector
--- @overload fun(radiusRGB: vector.Vector): 
function game.SurfaceMaterial:SetSubsurfaceScatterColor(radiusRGB) end

--- 
--- @param factor number
function game.SurfaceMaterial:SetSubsurfaceFactor(factor) end

--- 
--- @return number ret0
function game.SurfaceMaterial:GetPBRRoughness() end

--- 
--- @return number ret0
function game.SurfaceMaterial:GetLinearDragCoefficient() end

--- 
--- @param arg1 number
function game.SurfaceMaterial:SetAudioScattering(arg1) end

--- 
--- @return number ret0
function game.SurfaceMaterial:GetSubsurfaceFactor() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function game.SurfaceMaterial:GetSubsurfaceScatterColor() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function game.SurfaceMaterial:GetSubsurfaceRadiusMM() end

--- 
--- @return number ret0
function game.SurfaceMaterial:GetAudioMidFrequencyAbsorption() end

--- 
--- @param radiusMM math.Vector
--- @overload fun(radiusMM: vector.Vector): 
function game.SurfaceMaterial:SetSubsurfaceRadiusMM(radiusMM) end

--- 
--- @param arg1 string
function game.SurfaceMaterial:SetImpactParticleEffect(arg1) end

--- 
--- @return util.Color ret0
function game.SurfaceMaterial:GetSubsurfaceColor() end

--- 
--- @param color util.Color
function game.SurfaceMaterial:SetSubsurfaceColor(color) end

--- 
--- @param arg1 number
function game.SurfaceMaterial:SetFriction(arg1) end

--- 
--- @return number ret0
function game.SurfaceMaterial:GetDensity() end

--- 
--- @param arg1 number
function game.SurfaceMaterial:SetLinearDragCoefficient(arg1) end

--- 
--- @param arg1 number
function game.SurfaceMaterial:SetDensity(arg1) end

--- 
--- @return number ret0
function game.SurfaceMaterial:GetTorqueDragCoefficient() end

--- 
--- @param arg1 number
function game.SurfaceMaterial:SetTorqueDragCoefficient(arg1) end

--- 
function game.SurfaceMaterial:ClearIOR() end

--- 
--- @param arg1 string
function game.SurfaceMaterial:SetFootstepSound(arg1) end

--- 
--- @param arg1 string
function game.SurfaceMaterial:SetBulletImpactSound(arg1) end

--- 
--- @param arg1 number
function game.SurfaceMaterial:SetWavePropagation(arg1) end

--- 
--- @return number ret0
function game.SurfaceMaterial:GetRestitution() end

--- 
--- @param arg1 number
function game.SurfaceMaterial:SetStaticFriction(arg1) end

--- 
--- @param arg1 number
function game.SurfaceMaterial:SetDynamicFriction(arg1) end

--- 
--- @return number ret0
function game.SurfaceMaterial:GetAudioHighFrequencyAbsorption() end

--- 
--- @return number ret0
function game.SurfaceMaterial:GetStaticFriction() end

--- 
--- @param arg1 string
function game.SurfaceMaterial:SetHardImpactSound(arg1) end

--- 
--- @return number ret0
function game.SurfaceMaterial:GetDynamicFriction() end

--- 
--- @return number ret0
function game.SurfaceMaterial:GetAudioLowFrequencyAbsorption() end

--- 
--- @return string ret0
function game.SurfaceMaterial:GetName() end

--- 
--- @param arg1 string
function game.SurfaceMaterial:SetSoftImpactSound(arg1) end

--- 
--- @param arg1 number
function game.SurfaceMaterial:SetRestitution(arg1) end

--- 
--- @return number ret0
function game.SurfaceMaterial:GetAudioScattering() end

--- 
--- @return int ret0
function game.SurfaceMaterial:GetIndex() end

--- 
--- @return string ret0
function game.SurfaceMaterial:GetImpactParticleEffect() end

--- 
--- @return string ret0
function game.SurfaceMaterial:GetBulletImpactSound() end

--- 
--- @return string ret0
function game.SurfaceMaterial:GetSoftImpactSound() end

--- 
--- @return number ret0
function game.SurfaceMaterial:GetIOR() end

--- 
--- @return number ret0
function game.SurfaceMaterial:GetAudioLowFrequencyTransmission() end

--- 
--- @param arg1 number
function game.SurfaceMaterial:SetIOR(arg1) end

--- 
--- @param arg1 number
function game.SurfaceMaterial:SetAudioLowFrequencyAbsorption(arg1) end

--- 
--- @param arg1 number
function game.SurfaceMaterial:SetAudioMidFrequencyAbsorption(arg1) end

--- 
--- @param arg1 number
function game.SurfaceMaterial:SetAudioHighFrequencyAbsorption(arg1) end

--- 
function game.SurfaceMaterial:__tostring() end

--- 
--- @param arg1 number
function game.SurfaceMaterial:SetAudioLowFrequencyTransmission(arg1) end

--- 
--- @param arg1 number
function game.SurfaceMaterial:SetWaveStiffness(arg1) end

--- 
--- @param arg1 number
function game.SurfaceMaterial:SetAudioMidFrequencyTransmission(arg1) end

--- 
--- @return number ret0
function game.SurfaceMaterial:GetAudioMidFrequencyTransmission() end

--- 
--- @return number ret0
function game.SurfaceMaterial:GetAudioHighFrequencyTransmission() end

--- 
--- @return enum pragma::nav::PolyFlags ret0
function game.SurfaceMaterial:GetNavigationFlags() end

--- 
--- @param arg1 enum pragma::nav::PolyFlags
function game.SurfaceMaterial:SetNavigationFlags(arg1) end


--- 
--- @class game.DamageInfo
--- @overload fun():game.DamageInfo
game.DamageInfo = {}

--- 
--- @param arg1 enum DAMAGETYPE
function game.DamageInfo:RemoveDamageType(arg1) end

--- 
--- @param arg1 enum DAMAGETYPE
--- @return bool ret0
function game.DamageInfo:IsDamageType(arg1) end

--- 
--- @return int ret0
function game.DamageInfo:GetDamage() end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function game.DamageInfo:SetSource(arg1) end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function game.DamageInfo:SetForce(arg1) end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function game.DamageInfo:GetSource() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function game.DamageInfo:GetForce() end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function game.DamageInfo:SetHitPosition(arg1) end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function game.DamageInfo:GetHitPosition() end

--- 
--- @return enum HitGroup ret0
function game.DamageInfo:GetHitGroup() end

--- 
--- @param arg1 enum HitGroup
function game.DamageInfo:SetHitGroup(arg1) end

--- 
function game.DamageInfo:__tostring() end

--- 
--- @param arg1 int
function game.DamageInfo:SetDamage(arg1) end

--- 
--- @param arg1 int
function game.DamageInfo:AddDamage(arg1) end

--- 
--- @param arg1 number
function game.DamageInfo:ScaleDamage(arg1) end

--- 
--- @return ents.BaseEntityBase ret0
function game.DamageInfo:GetAttacker() end

--- 
--- @param arg1 ents.BaseEntityBase
function game.DamageInfo:SetAttacker(arg1) end

--- 
--- @return ents.BaseEntityBase ret0
function game.DamageInfo:GetInflictor() end

--- 
--- @param arg1 ents.BaseEntityBase
function game.DamageInfo:SetInflictor(arg1) end

--- 
--- @return int ret0
function game.DamageInfo:GetDamageTypes() end

--- 
--- @param arg1 enum DAMAGETYPE
function game.DamageInfo:SetDamageType(arg1) end

--- 
--- @param arg1 enum DAMAGETYPE
function game.DamageInfo:AddDamageType(arg1) end


--- 
--- @class game.BulletInfo
--- @field distance number 
--- @field spread  
--- @field attacker class util::TWeakSharedHandle<class BaseEntity> 
--- @field force number 
--- @field damageType int 
--- @field damage int 
--- @field bulletCount int 
--- @field inflictor class util::TWeakSharedHandle<class BaseEntity> 
--- @field tracerCount int 
--- @field tracerRadius number 
--- @field tracerColor  
--- @field tracerLength number 
--- @field tracerSpeed number 
--- @field tracerMaterial string 
--- @field tracerBloom number 
--- @field ammoType string 
--- @field direction  
--- @field effectOrigin  
--- @overload fun():game.BulletInfo
game.BulletInfo = {}

--- 
function game.BulletInfo:__tostring() end


--- 
--- @class game.GibletCreateInfo
--- @field physHeight number 
--- @field model string 
--- @field physShape int 
--- @field lifetime number 
--- @field physRadius number 
--- @field skin int 
--- @field scale number 
--- @field mass number 
--- @field position  
--- @field rotation  
--- @field velocity  
--- @field angularVelocity  
--- @field physTranslationOffset  
--- @field physRotationOffset  
--- @overload fun():game.GibletCreateInfo
game.GibletCreateInfo = {}


--- @enum PhysShape
game.GibletCreateInfo = {
	PHYS_SHAPE_BOX = 3,
	PHYS_SHAPE_CYLINDER = 4,
	PHYS_SHAPE_SPHERE = 2,
	PHYS_SHAPE_MODEL = 0,
	PHYS_SHAPE_NONE = 1,
}

--- 
--- @class game.Model
game.Model = {}

--- 
--- @param idx int
function game.Model:GetObjectAttachment(idx) end

--- 
--- @param lod int
--- @overload fun(lod: int, o: any): 
function game.Model:TranslateLODMeshes(lod) end

--- 
--- @param activity int
function game.Model:SelectFirstAnimation(activity) end

--- 
function game.Model:GetLODCount() end

--- 
--- @param idx int
--- @return any ret0
function game.Model:GetFlexAnimation(idx) end

--- 
--- @param anim int
function game.Model:GetAnimationActivity(anim) end

--- 
--- @param attId int
--- @param data any
--- @overload fun(name: string, data: any): 
function game.Model:SetAttachmentData(attId, data) end

--- 
function game.Model:GetFlags() end

--- 
--- @param anim int
function game.Model:GetAnimationActivityWeight(anim) end

--- 
function game.Model:GetAnimationCount() end

--- 
--- @param group int
function game.Model:PrecacheTextureGroup(group) end

--- 
--- @param name string
--- @param flexAnim Model.FlexAnimation
--- @overload fun(name: string): 
function game.Model:AddFlexAnimation(name, flexAnim) end

--- 
--- @param name string
function game.Model:LookupAnimation(name) end

--- 
--- @param anim int
function game.Model:GetAnimationDuration(anim) end

--- 
function game.Model:GetEyeOffset() end

--- 
--- @overload fun(lod: int): 
function game.Model:GetLODData() end

--- 
--- @param flags int
function game.Model:SetFlags(flags) end

--- 
function game.Model:GetMaterialNames() end

--- 
--- @param name string
--- @return any ret0
function game.Model:LookupFlexAnimation(name) end

--- 
--- @param flag int
function game.Model:HasFlag(flag) end

--- 
--- @return int ret0
function game.Model:GetFlexAnimationCount() end

--- 
--- @param idx int
--- @return any ret0
function game.Model:GetFlexAnimationName(idx) end

--- 
function game.Model:UpdateCollisionBounds() end

--- 
--- @param name string
function game.Model:LookupObjectAttachment(name) end

--- 
--- @param mgId int
--- @overload fun(meshGroupName: string): 
function game.Model:GetMeshGroup(mgId) end

--- 
function game.Model:ClearFlexAnimations() end

--- 
--- @param idx int
function game.Model:RemoveFlexAnimation(idx) end

--- 
function game.Model:GetSubMeshCount() end

--- 
function game.Model:GetBlendControllerCount() end

--- 
function game.Model:GetAnimationNames() end

--- 
--- @param mgId int
--- @param mId int
--- @param smId int
--- @return Model.Mesh.Sub ret0
function game.Model:GetSubMesh(mgId, mId, smId) end

--- 
--- @param path string
function game.Model:AddMaterialPath(path) end

--- 
--- @param blendControllerId int
--- @overload fun(name: string): 
function game.Model:GetBlendController(blendControllerId) end

--- 
function game.Model:GetCollisionMeshes() end

--- 
function game.Model:GetAnimations() end

--- 
--- @param copyFlags int
--- @overload fun(): 
function game.Model:Copy(copyFlags) end

--- 
function game.Model:ClearCollisionMeshes() end

--- 
function game.Model:GetSkeleton() end

--- 
function game.Model:GetAttachmentCount() end

--- 
function game.Model:ClearMeshGroups() end

--- 
function game.Model:GetAttachments() end

--- 
--- @param attId int
--- @overload fun(name: string): 
function game.Model:GetAttachment(attId) end

--- 
--- @param name string
--- @param boneId int
--- @param offset math.Vector
--- @param ang math.EulerAngles
--- @overload fun(name: string, boneName: string, offset: math.Vector, ang: math.EulerAngles): 
--- @overload fun(name: string, boneId: int, offset: vector.Vector, ang: math.EulerAngles): 
--- @overload fun(name: string, boneName: string, offset: vector.Vector, ang: math.EulerAngles): 
function game.Model:AddAttachment(name, boneId, offset, ang) end

--- 
--- @param eyeball Model.Eyeball
function game.Model:AddEyeball(eyeball) end

--- 
--- @param attId int
--- @overload fun(name: string): 
function game.Model:RemoveAttachment(attId) end

--- 
--- @param o any
function game.Model:SetBaseMeshGroupIds(o) end

--- 
--- @param meshGroup Model.MeshGroup
--- @overload fun(name: string): 
function game.Model:AddMeshGroup(meshGroup) end

--- 
function game.Model:GetRenderBounds() end

--- 
function game.Model:GetObjectAttachments() end

--- 
--- @param activity int
--- @param animIgnore int
--- @overload fun(activity: int): 
function game.Model:SelectWeightedAnimation(activity, animIgnore) end

--- 
function game.Model:AddBaseMeshGroupId() end

--- 
--- @param type int
--- @param name string
--- @param attachment string
--- @param oKeyValues any
function game.Model:AddObjectAttachment(type, name, attachment, oKeyValues) end

--- 
function game.Model:GetObjectAttachmentCount() end

--- 
--- @param idx int
--- @overload fun(name: string): 
function game.Model:RemoveObjectAttachment(idx) end

--- 
function game.Model:PrecacheTextureGroups() end

--- 
function game.Model:GetBlendControllers() end

--- 
--- @param animID int
function game.Model:GetAnimationName(animID) end

--- 
function game.Model:GetBoneCount() end

--- 
function game.Model:GetReferencePose() end

--- 
--- @param o any
--- @overload fun(meshGroup: string): 
function game.Model:GetMeshes(o) end

--- 
--- @param name string
function game.Model:LookupBodyGroup(name) end

--- 
function game.Model:GetMeshGroups() end

--- 
function game.Model:GetBaseMeshGroupIds() end

--- 
--- @param name string
function game.Model:LookupFlexController(name) end

--- 
function game.Model:GetCollisionBounds() end

--- 
--- @param mdl game.Model
--- @param fname string
--- @overload fun(): 
--- @overload fun(assetData: udm.AssetData): 
function game.Model:Save(mdl, fname) end

--- 
--- @param bodyGroupId int
--- @param groupId int
function game.Model:GetMeshGroupId(bodyGroupId, groupId) end

--- 
--- @param offset math.Vector
--- @overload fun(offset: vector.Vector): 
function game.Model:SetEyeOffset(offset) end

--- 
--- @param min math.Vector
--- @param max math.Vector
--- @overload fun(min: vector.Vector, max: vector.Vector): 
function game.Model:SetRenderBounds(min, max) end

--- 
--- @param t math.Vector
--- @overload fun(t: vector.Vector): 
function game.Model:Translate(t) end

--- 
--- @param eyeballIndex int
--- @overload fun(): 
function game.Model:IsStatic(eyeballIndex) end

--- 
--- @param animId int
--- @param frameId int
--- @param boneId int
function game.Model:GetLocalBoneTransform(animId, frameId, boneId) end

--- 
function game.Model:GetEyeballCount() end

--- 
function game.Model:GetName() end

--- 
--- @param 1 game.Model
function game.Model:__eq(arg1) end

--- 
--- @param bgId int
function game.Model:GetBodyGroup(bgId) end

--- 
function game.Model:__tostring() end

--- 
function game.Model:GetBodyGroups() end

--- 
--- @param boneId int
function game.Model:IsRootBone(boneId) end

--- 
--- @param name string
function game.Model:LookupBlendController(name) end

--- 
--- @param name string
function game.Model:LookupAttachment(name) end

--- 
--- @param boneId int
--- @param hitGroup int
--- @param min math.Vector
--- @param max math.Vector
--- @overload fun(boneId: int, hitGroup: int, min: vector.Vector, max: vector.Vector): 
function game.Model:AddHitbox(boneId, hitGroup, min, max) end

--- 
function game.Model:GetHitboxCount() end

--- 
--- @param boneId int
function game.Model:GetHitboxBounds(boneId) end

--- 
--- @param name string
function game.Model:GetVertexAnimation(name) end

--- 
--- @param scale math.Vector
--- @overload fun(scale: vector.Vector): 
function game.Model:Scale(scale) end

--- 
--- @return any ret0
function game.Model:GetFlexAnimationNames() end

--- 
--- @return any ret0
function game.Model:GetFlexAnimations() end

--- 
function game.Model:HasVertexWeights() end

--- 
--- @param o any
function game.Model:SetMaterialPaths(o) end

--- 
--- @param name string
function game.Model:LookupBone(name) end

--- 
--- @param mdlOther game.Model
--- @overload fun(mdlOther: game.Model, mergeFlags: int): 
function game.Model:Merge(mdlOther) end

--- 
function game.Model:UpdateRenderBounds() end

--- 
function game.Model:GetMeshCount() end

--- 
--- @param mass number
function game.Model:SetMass(mass) end

--- 
--- @param colMesh Model.CollisionMesh
function game.Model:AddCollisionMesh(colMesh) end

--- 
function game.Model:GetFlexControllerCount() end

--- 
--- @param name string
function game.Model:SaveLegacy(name) end

--- 
--- @param textureGroup int
--- @param name string
--- @overload fun(textureGroup: int, mat: game.Material): 
function game.Model:AddMaterial(textureGroup, name) end

--- 
--- @param texIdx int
--- @param name string
--- @overload fun(matId: int, mat: game.Material): 
function game.Model:SetMaterial(texIdx, name) end

--- 
function game.Model:GetMaterials() end

--- 
--- @param idx int
function game.Model:GetMaterial(idx) end

--- 
--- @param mesh Model.Mesh.Sub
--- @param skinId int
function game.Model:GetMaterialIndex(mesh, skinId) end

--- 
function game.Model:GetMass() end

--- 
--- @param flags int
--- @overload fun(): 
function game.Model:Update(flags) end

--- 
--- @param min math.Vector
--- @param max math.Vector
--- @overload fun(min: vector.Vector, max: vector.Vector): 
function game.Model:SetCollisionBounds(min, max) end

--- 
function game.Model:GetTriangleCount() end

--- 
--- @param rot math.Quaternion
function game.Model:Rotate(rot) end

--- 
--- @param idx int
function game.Model:GetLOD(idx) end

--- 
function game.Model:GetVertexCount() end

--- 
--- @param boneId int
function game.Model:RemoveHitbox(boneId) end

--- 
--- @param animID int
--- @overload fun(name: int): 
function game.Model:GetAnimation(animID) end

--- 
--- @param smTgt Model.Mesh.Sub
--- @return int ret0_1
--- @return int ret0_2
--- @return int ret0_3
function game.Model:FindSubMeshId(smTgt) end

--- 
function game.Model:GetMaterialCount() end

--- 
function game.Model:GetMeshGroupCount() end

--- 
function game.Model:GetCollisionMeshCount() end

--- 
--- @param bodyGroupName string
function game.Model:GetBodyGroupId(bodyGroupName) end

--- 
function game.Model:GetBodyGroupCount() end

--- 
--- @param boneId int
function game.Model:GetHitboxGroup(boneId) end

--- 
--- @overload fun(hitGroup: int): 
function game.Model:GetHitboxBones() end

--- 
--- @param boneId int
--- @param hitGroup int
function game.Model:SetHitboxGroup(boneId, hitGroup) end

--- 
--- @param boneId int
--- @param min math.Vector
--- @param max math.Vector
--- @overload fun(boneId: int, min: vector.Vector, max: vector.Vector): 
function game.Model:SetHitboxBounds(boneId, min, max) end

--- 
--- @return udm.PropertyWrapper ret0
function game.Model:GetExtensionData() end

--- 
function game.Model:GetTextureGroupCount() end

--- 
function game.Model:GetTextureGroups() end

--- 
--- @param id int
function game.Model:GetTextureGroup(id) end

--- 
function game.Model:GetMaterialPaths() end

--- 
--- @param bReload bool
--- @overload fun(): 
function game.Model:LoadMaterials(bReload) end

--- 
--- @param idx int
function game.Model:RemoveMaterialPath(idx) end

--- 
--- @param idx int
function game.Model:RemoveMaterial(idx) end

--- 
function game.Model:ClearMaterials() end

--- 
--- @param name string
--- @param anim Model.Animation
function game.Model:AddAnimation(name, anim) end

--- 
--- @param idx int
function game.Model:RemoveAnimation(idx) end

--- 
function game.Model:ClearAnimations() end

--- 
--- @param idx int
function game.Model:RemoveMeshGroup(idx) end

--- 
function game.Model:ClearJoints() end

--- 
function game.Model:ClearBaseMeshGroupIds() end

--- 
function game.Model:AddTextureGroup() end

--- 
function game.Model:GetJoints() end

--- 
--- @param type enum JointType
--- @param child int
--- @param parent int
--- @return Model.Joint ret0
function game.Model:AddJoint(type, child, parent) end

--- 
function game.Model:GetVertexAnimations() end

--- 
--- @param name string
function game.Model:AddVertexAnimation(name) end

--- 
--- @param name string
function game.Model:RemoveVertexAnimation(name) end

--- 
--- @overload fun(lod: int): 
--- @overload fun(oBodygroups: any, lod: int): 
function game.Model:GetBodyGroupMeshes() end

--- 
--- @param bodyGroupId int
--- @param bgValue int
function game.Model:GetBodyGroupMesh(bodyGroupId, bgValue) end

--- 
function game.Model:GetFlexControllers() end

--- 
--- @param name string
--- @overload fun(id: int): 
function game.Model:RemoveIKController(name) end

--- 
--- @param id int
--- @overload fun(name: string): 
function game.Model:GetFlexController(id) end

--- 
function game.Model:GetFlexes() end

--- 
--- @param name string
function game.Model:LookupFlex(name) end

--- 
--- @param flexId int
--- @overload fun(flexName: string): 
function game.Model:GetFlexFormula(flexId) end

--- 
function game.Model:GetFlexCount() end

--- 
--- @param flexId int
--- @param oFc any
function game.Model:CalcFlexWeight(flexId, oFc) end

--- 
--- @param attIdx int
function game.Model:CalcReferenceAttachmentPose(attIdx) end

--- 
--- @param boneIdx int
function game.Model:CalcReferenceBonePose(boneIdx) end

--- 
--- @param eyeIdx int
function game.Model:GetEyeball(eyeIdx) end

--- 
function game.Model:GetEyeballs() end

--- 
function game.Model:GetIKControllers() end

--- 
--- @param id int
function game.Model:GetIKController(id) end

--- 
--- @param name string
function game.Model:LookupIKController(name) end

--- 
function game.Model:GetIncludeModels() end

--- 
--- @param animName string
--- @param exportInfo Model.ExportInfo
function game.Model:ExportAnimation(animName, exportInfo) end

--- 
--- @param name string
--- @param chainLength int
--- @param type string
--- @overload fun(name: string, chainLength: int, type: string, method: int): 
function game.Model:AddIKController(name, chainLength, type) end

--- 
--- @param modelName string
function game.Model:AddIncludeModel(modelName) end

--- 
function game.Model:GetPhonemeMap() end

--- 
--- @param o any
function game.Model:SetPhonemeMap(o) end

--- 
--- @param group Model.MeshGroup
--- @param mesh Model.Mesh
--- @param subMesh Model.Mesh.Sub
function game.Model:AssignDistinctMaterial(group, mesh, subMesh) end

--- 
function game.Model:GetVertexAnimationBuffer() end

--- 
--- @param exportInfo Model.ExportInfo
function game.Model:Export(exportInfo) end


--- @enum FupdateVertex
game.Model = {
	FUPDATE_VERTEX_ANIMATION_BUFFER = 256,
	FUPDATE_VERTEX_BUFFER = 16,
}

--- @enum ObjectAttachmentType
game.Model = {
	OBJECT_ATTACHMENT_TYPE_MODEL = 0,
	OBJECT_ATTACHMENT_TYPE_PARTICLE_SYSTEM = 1,
}

--- @enum Fupdate
game.Model = {
	FUPDATE_ALL = 1023,
	FUPDATE_ALL_DATA = 527,
	FUPDATE_INDEX_BUFFER = 32,
	FUPDATE_ALPHA_BUFFER = 128,
	FUPDATE_BUFFERS = 496,
	FUPDATE_BOUNDS = 1,
	FUPDATE_COLLISION_SHAPES = 4,
	FUPDATE_CHILDREN = 512,
	FUPDATE_NONE = 0,
	FUPDATE_PRIMITIVE_COUNTS = 2,
	FUPDATE_TANGENTS = 8,
	FUPDATE_WEIGHT_BUFFER = 64,
}

--- @enum Fcopy
game.Model = {
	FCOPY_DEEP = 31,
	FCOPY_SHALLOW = 0,
	FCOPY_NONE = 0,
}

--- @enum Fmerge
game.Model = {
	FMERGE_ALL = 127,
	FMERGE_ANIMATIONS = 1,
	FMERGE_HITBOXES = 8,
	FMERGE_ATTACHMENTS = 2,
	FMERGE_JOINTS = 16,
	FMERGE_BLEND_CONTROLLERS = 4,
	FMERGE_COLLISION_MESHES = 32,
	FMERGE_MESHES = 64,
	FMERGE_NONE = 0,
}

--- @enum FcopyBit
game.Model = {
	FCOPY_BIT_ANIMATIONS = 2,
	FCOPY_BIT_COLLISION_MESHES = 8,
	FCOPY_BIT_FLEX_ANIMATIONS = 16,
	FCOPY_BIT_MESHES = 1,
	FCOPY_BIT_VERTEX_ANIMATIONS = 4,
}

--- @enum FlagBit
game.Model = {
	FLAG_BIT_INANIMATE = 2,
	FLAG_BIT_DONT_PRECACHE_TEXTURE_GROUPS = 128,
	FLAG_BIT_STATIC = 1,
}

--- @enum FlagNone
game.Model = {
	FLAG_NONE = 0,
}

--- 
--- @class game.Model.Joint
--- @field collide bool 
game.Model.Joint = {}

--- 
--- @return any ret0
function game.Model.Joint:GetArgs() end

--- 
--- @param bEnabled bool
function game.Model.Joint:SetCollisionsEnabled(bEnabled) end

--- 
function game.Model.Joint:GetType() end

--- 
--- @param t 
function game.Model.Joint:SetArgs(t) end

--- 
--- @param key string
--- @param val string
function game.Model.Joint:SetKeyValue(key, val) end

--- 
function game.Model.Joint:GetCollisionsEnabled() end

--- 
function game.Model.Joint:GetChildBoneId() end

--- 
function game.Model.Joint:GetParentBoneId() end

--- 
function game.Model.Joint:GetKeyValues() end

--- 
--- @param type int
function game.Model.Joint:SetType(type) end

--- 
--- @param meshId int
function game.Model.Joint:SetCollisionMeshId(meshId) end

--- 
--- @param meshId int
function game.Model.Joint:SetParentCollisionMeshId(meshId) end

--- 
--- @param keyValues any
function game.Model.Joint:SetKeyValues(keyValues) end

--- 
--- @param key string
function game.Model.Joint:RemoveKeyValue(key) end


--- @enum Type
game.Model.Joint = {
	TYPE_BALLSOCKET = 2,
	TYPE_FIXED = 1,
	TYPE_CONETWIST = 5,
	TYPE_DOF = 6,
	TYPE_HINGE = 3,
	TYPE_NONE = 0,
	TYPE_SLIDER = 4,
}

--- 
--- @class game.Model.Vertex
--- @field uv  
--- @field normal  
--- @field tangent  
--- @field position  
--- @overload fun():game.Model.Vertex
--- @overload fun(arg1: math.Vector, arg2: math.Vector):game.Model.Vertex
--- @overload fun(arg1: math.Vector, arg2: math.Vector2, arg3: math.Vector):game.Model.Vertex
--- @overload fun(arg1: math.Vector, arg2: math.Vector2, arg3: math.Vector, arg4: math.Vector4):game.Model.Vertex
game.Model.Vertex = {}

--- 
--- @return game.Model.Vertex ret0
function game.Model.Vertex:Copy() end

--- 
function game.Model.Vertex:__tostring() end

--- 
--- @param 1 game.Model.Vertex
function game.Model.Vertex:__eq(arg1) end

--- 
--- @return math.Vector ret0
function game.Model.Vertex:GetBiTangent() end


--- 
--- @class game.Model.CollisionMesh
game.Model.CollisionMesh = {}

--- 
function game.Model.CollisionMesh:ClearTriangles() end

--- 
--- @return number ret0
function game.Model.CollisionMesh:GetMass() end

--- 
--- @return table ret0
function game.Model.CollisionMesh:GetSurfaceMaterialIds() end

--- 
--- @return int ret0
function game.Model.CollisionMesh:GetVertexCount() end

--- 
--- @return table ret0
function game.Model.CollisionMesh:GetVertices() end

--- 
--- @param idx0 int
--- @param idx1 int
--- @param idx2 int
function game.Model.CollisionMesh:AddTriangle(idx0, idx1, idx2) end

--- 
--- @return phys.Shape ret0
function game.Model.CollisionMesh:GetShape() end

--- 
--- @param arg1 bool
function game.Model.CollisionMesh:SetConvex(arg1) end

--- 
--- @return int ret0
function game.Model.CollisionMesh:GetBoneParentId() end

--- 
--- @return math.Vector ret0_1
--- @return math.Vector ret0_2
function game.Model.CollisionMesh:GetAABB() end

--- 
--- @param arg1 number
function game.Model.CollisionMesh:SetMass(arg1) end

--- 
--- @return int ret0
function game.Model.CollisionMesh:GetSurfaceMaterialId() end

--- 
--- @param min math.Vector
--- @param max math.Vector
--- @return bool ret0
function game.Model.CollisionMesh:IntersectAABB(min, max) end

--- 
--- @return bool ret0
function game.Model.CollisionMesh:IsConvex() end

--- 
--- @param subMesh game.Model.Mesh.Sub
function game.Model.CollisionMesh:SetSoftBodyMesh(subMesh) end

--- 
--- @return math.Vector ret0
function game.Model.CollisionMesh:GetOrigin() end

--- 
--- @param arg1 int
function game.Model.CollisionMesh:SetBoneParentId(arg1) end

--- 
function game.Model.CollisionMesh:Centralize() end

--- 
--- @param tTriangles table
function game.Model.CollisionMesh:SetTriangles(tTriangles) end

--- 
--- @return game.Model.CollisionMesh ret0
function game.Model.CollisionMesh:Copy() end

--- 
--- @param arg1 int
function game.Model.CollisionMesh:SetSurfaceMaterialId(arg1) end

--- 
function game.Model.CollisionMesh:Update() end

--- 
--- @param arg1 math.Vector
function game.Model.CollisionMesh:AddVertex(arg1) end

--- 
function game.Model.CollisionMesh:GetTriangles() end

--- 
--- @param o table
function game.Model.CollisionMesh:SetSoftBodyTriangles(o) end

--- 
--- @param arg1 math.Vector
--- @param arg2 number
--- @param arg3 game.Model.CollisionMesh
--- @param arg4 game.Model.CollisionMesh
function game.Model.CollisionMesh:ClipAgainstPlane(arg1, arg2, arg3, arg4) end

--- 
function game.Model.CollisionMesh:ClearSoftBodyAnchors() end

--- 
--- @return number ret0
function game.Model.CollisionMesh:GetVolume() end

--- 
function game.Model.CollisionMesh:ClearVertices() end

--- 
--- @param vertIdx int
--- @param boneIdx int
--- @return int ret0
--- @overload fun(vertIdx: int, boneIdx: int, flags: enum CollisionMesh::SoftBodyAnchor::Flags): int
--- @overload fun(vertIdx: int, boneIdx: int, flags: enum CollisionMesh::SoftBodyAnchor::Flags, influence: number): int
function game.Model.CollisionMesh:AddSoftBodyAnchor(vertIdx, boneIdx) end

--- 
--- @param arg1 number
function game.Model.CollisionMesh:SetVolume(arg1) end

--- 
--- @param tVertices table
function game.Model.CollisionMesh:SetVertices(tVertices) end

--- 
--- @param arg1 bool
function game.Model.CollisionMesh:SetSoftBody(arg1) end

--- 
--- @return bool ret0
function game.Model.CollisionMesh:IsSoftBody() end

--- 
--- @param arg1 math.Vector
function game.Model.CollisionMesh:Translate(arg1) end

--- 
function game.Model.CollisionMesh:GetSoftBodyMesh() end

--- 
function game.Model.CollisionMesh:GetSoftBodyInfo() end

--- 
--- @return table ret0
function game.Model.CollisionMesh:GetSoftBodyTriangles() end

--- 
--- @param arg1 math.Vector
function game.Model.CollisionMesh:SetOrigin(arg1) end

--- 
--- @return table ret0
function game.Model.CollisionMesh:GetSoftBodyAnchors() end

--- 
function game.Model.CollisionMesh:__tostring() end

--- 
--- @param arg1 int
function game.Model.CollisionMesh:RemoveSoftBodyAnchor(arg1) end

--- 
--- @param arg1 math.Quaternion
function game.Model.CollisionMesh:Rotate(arg1) end

--- 
--- @param min math.Vector
--- @param max math.Vector
function game.Model.CollisionMesh:SetAABB(min, max) end


--- @enum FsoftbodyAnchor
game.Model.CollisionMesh = {
	FSOFTBODY_ANCHOR_DISABLE_COLLISIONS = 2,
	FSOFTBODY_ANCHOR_NONE = 0,
	FSOFTBODY_ANCHOR_RIGID = 1,
}

--- 
--- @class game.Model.Flex
game.Model.Flex = {}

--- 
function game.Model.Flex:GetName() end

--- 
function game.Model.Flex:GetOperations() end

--- 
--- @param anim game.Model.VertexAnimation
--- @overload fun(anim: game.Model.VertexAnimation, frameIndex: int): 
function game.Model.Flex:SetVertexAnimation(anim) end

--- 
function game.Model.Flex:GetFrameIndex() end

--- 
function game.Model.Flex:GetVertexAnimation() end

--- 
--- @return string ret0
function game.Model.Flex:__tostring() end


--- @enum OpTwo
game.Model.Flex = {
	OP_TWO_WAY0 = 15,
	OP_TWO_WAY1 = 16,
}

--- @enum OpDme
game.Model.Flex = {
	OP_DME_LOWER_EYELID = 20,
	OP_DME_UPPER_EYELID = 21,
}

--- @enum Op
game.Model.Flex = {
	OP_ADD = 4,
	OP_CONST = 1,
	OP_DOMINATE = 19,
	OP_DIV = 7,
	OP_CLOSE = 11,
	OP_COMBO = 18,
	OP_EXP = 9,
	OP_COMMA = 12,
	OP_MIN = 14,
	OP_FETCH = 2,
	OP_FETCH2 = 3,
	OP_MAX = 13,
	OP_MUL = 6,
	OP_NEG = 8,
	OP_NONE = 0,
	OP_OPEN = 10,
	OP_N_WAY = 17,
	OP_SUB = 5,
}

--- 
--- @class game.Model.Flex.Operation
--- @field index int 
--- @field value number 
--- @field type int 
game.Model.Flex.Operation = {}

--- 
function game.Model.Flex.Operation:GetName() end


--- 
--- @class game.Model.Skeleton
game.Model.Skeleton = {}

--- 
--- @param boneId int
--- @return game.Model.Skeleton.Bone ret0
function game.Model.Skeleton:GetBone(boneId) end

--- 
function game.Model.Skeleton:ClearBones() end

--- 
--- @param boneId int
--- @return bool ret0
--- @overload fun(boneName: string): bool
function game.Model.Skeleton:IsRootBone(boneId) end

--- 
--- @param arg1 game.Model.Skeleton
function game.Model.Skeleton:Merge(arg1) end

--- 
--- @return int ret0
function game.Model.Skeleton:GetBoneCount() end

--- 
--- @param name string
--- @return int ret0
function game.Model.Skeleton:LookupBone(name) end

--- 
--- @param name string
--- @return game.Model.Skeleton.Bone ret0
--- @overload fun(name: string, parent: game.Model.Skeleton.Bone): game.Model.Skeleton.Bone
function game.Model.Skeleton:AddBone(name) end

--- 
--- @return table ret0
function game.Model.Skeleton:GetBones() end

--- 
--- @return map ret0
function game.Model.Skeleton:GetBoneHierarchy() end

--- 
--- @param bone game.Model.Skeleton.Bone
--- @return bool ret0
function game.Model.Skeleton:MakeRootBone(bone) end

--- 
--- @return map ret0
function game.Model.Skeleton:GetRootBones() end


--- 
--- @class game.Model.Skeleton.Bone
game.Model.Skeleton.Bone = {}

--- 
--- @return int ret0
function game.Model.Skeleton.Bone:GetID() end

--- 
function game.Model.Skeleton.Bone:ClearParent() end

--- 
function game.Model.Skeleton.Bone:__tostring() end

--- 
--- @param arg1 game.Model.Skeleton.Bone
--- @return bool ret0
function game.Model.Skeleton.Bone:IsDescendantOf(arg1) end

--- 
--- @return game.Model.Skeleton.Bone ret0
function game.Model.Skeleton.Bone:GetParent() end

--- 
--- @return string ret0
function game.Model.Skeleton.Bone:GetName() end

--- 
--- @param name string
function game.Model.Skeleton.Bone:SetName(name) end

--- 
--- @param arg1 game.Model.Skeleton.Bone
--- @return bool ret0
function game.Model.Skeleton.Bone:IsAncestorOf(arg1) end

--- 
--- @param parent game.Model.Skeleton.Bone
function game.Model.Skeleton.Bone:SetParent(parent) end

--- 
--- @return map ret0
function game.Model.Skeleton.Bone:GetChildren() end


--- 
--- @class game.Model.FlexAnimation
game.Model.FlexAnimation = {}

--- 
--- @param frameId int
--- @return game.Model.FlexAnimation.Frame ret0
function game.Model.FlexAnimation:GetFrame(frameId) end

--- 
--- @return int ret0
function game.Model.FlexAnimation:GetFrameCount() end

--- 
--- @return any ret0
function game.Model.FlexAnimation:GetFlexControllerIds() end

--- 
--- @param id int
--- @return int ret0
function game.Model.FlexAnimation:AddFlexControllerId(id) end

--- 
--- @param tIds 
function game.Model.FlexAnimation:SetFlexControllerIds(tIds) end

--- 
--- @return int ret0
function game.Model.FlexAnimation:GetFlexControllerCount() end

--- 
--- @param id int
--- @return any ret0
function game.Model.FlexAnimation:LookupLocalFlexControllerIndex(id) end

--- 
--- @return game.Model.FlexAnimation.Frame ret0
function game.Model.FlexAnimation:AddFrame() end

--- 
function game.Model.FlexAnimation:ClearFrames() end

--- 
--- @param idx int
function game.Model.FlexAnimation:RemoveFrame(idx) end

--- 
--- @param assetData udm.AssetData
function game.Model.FlexAnimation:Save(assetData) end

--- 
--- @param frameId int
--- @param id int
--- @param val number
function game.Model.FlexAnimation:SetFlexControllerValue(frameId, id, val) end

--- 
--- @return number ret0
function game.Model.FlexAnimation:GetFps() end

--- 
--- @param fps number
function game.Model.FlexAnimation:SetFps(fps) end

--- 
--- @return any ret0
function game.Model.FlexAnimation:GetFrames() end


--- 
--- @class game.Model.FlexAnimation.Frame
game.Model.FlexAnimation.Frame = {}

--- 
--- @return any ret0
function game.Model.FlexAnimation.Frame:GetFlexControllerValues() end

--- 
--- @param t 
function game.Model.FlexAnimation.Frame:SetFlexControllerValues(t) end

--- 
--- @param id int
function game.Model.FlexAnimation.Frame:GetFlexControllerValue(id) end

--- 
--- @return int ret0
function game.Model.FlexAnimation.Frame:GetFlexControllerValueCount() end

--- 
--- @param id int
--- @param val number
function game.Model.FlexAnimation.Frame:SetFlexControllerValue(id, val) end


--- 
--- @class game.Model.Mesh
game.Model.Mesh = {}

--- 
--- @param 1 game.Model.Mesh
function game.Model.Mesh:__eq(arg1) end

--- 
--- @param index int
function game.Model.Mesh:GetSubMesh(index) end

--- 
function game.Model.Mesh:GetSubMeshCount() end

--- 
function game.Model.Mesh:GetCenter() end

--- 
function game.Model.Mesh:GetTriangleCount() end

--- 
--- @param tSubMeshes any
function game.Model.Mesh:SetSubMeshes(tSubMeshes) end

--- 
function game.Model.Mesh:GetBounds() end

--- 
function game.Model.Mesh:GetIndexCount() end

--- 
--- @param uuid string
--- @overload fun(i: int): 
function game.Model.Mesh:RemoveSubMesh(uuid) end

--- 
--- @param flags int
--- @overload fun(): 
function game.Model.Mesh:Update(flags) end

--- 
--- @param subMesh game.Model.Mesh.Sub
--- @param pos int
--- @overload fun(mesh: game.Model.Mesh.Sub): 
function game.Model.Mesh:AddSubMesh(subMesh, pos) end

--- 
function game.Model.Mesh:__tostring() end

--- 
function game.Model.Mesh:GetVertexCount() end

--- 
--- @param uuid string
--- @return game.Model.Mesh.Sub ret0
function game.Model.Mesh:FindSubMesh(uuid) end

--- 
function game.Model.Mesh:Centralize() end

--- 
--- @param rotation math.Quaternion
function game.Model.Mesh:Rotate(rotation) end

--- 
function game.Model.Mesh:ClearSubMeshes() end

--- 
function game.Model.Mesh:GetSubMeshes() end

--- 
--- @param center math.Vector
function game.Model.Mesh:SetCenter(center) end

--- 
--- @param scale math.Vector
function game.Model.Mesh:Scale(scale) end

--- 
--- @param translation math.Vector
function game.Model.Mesh:Translate(translation) end

--- 
function game.Model.Mesh:GetReferenceId() end


--- 
--- @class game.Model.Mesh.Sub
game.Model.Mesh.Sub = {}

--- 
--- @param arg1 util.Uuid
--- @overload fun(uuid: string): 
function game.Model.Mesh.Sub:SetUuid(arg1) end

--- 
--- @return any ret0
function game.Model.Mesh.Sub:GetUVSetNames() end

--- 
--- @param a int
--- @param b int
--- @param c int
--- @overload fun(v1: game.Model.Vertex, v2: game.Model.Vertex, v3: game.Model.Vertex): 
function game.Model.Mesh.Sub:AddTriangle(a, b, c) end

--- 
function game.Model.Mesh.Sub:GetVertexCount() end

--- 
--- @param arg1 int
function game.Model.Mesh.Sub:AddIndex(arg1) end

--- 
function game.Model.Mesh.Sub:GetVertices() end

--- 
--- @param name string
--- @return bool ret0
function game.Model.Mesh.Sub:HasUvSet(name) end

--- 
function game.Model.Mesh.Sub:GetIndices() end

--- 
--- @overload fun(flags: int): 
function game.Model.Mesh.Sub:Update() end

--- 
--- @return bool ret0
function game.Model.Mesh.Sub:HasVertexWeights() end

--- 
function game.Model.Mesh.Sub:NormalizeUVCoordinates() end

--- 
--- @param arg1 int
--- @param arg2 int
--- @return bool ret0
function game.Model.Mesh.Sub:SetIndex(arg1, arg2) end

--- 
--- @param 1 game.Model.Mesh.Sub
function game.Model.Mesh.Sub:__eq(arg1) end

--- 
function game.Model.Mesh.Sub:GetNormals() end

--- 
function game.Model.Mesh.Sub:GetReferenceId() end

--- 
--- @param idx int
--- @param weight game.Model.VertexWeight
function game.Model.Mesh.Sub:SetVertexWeight(idx, weight) end

--- 
function game.Model.Mesh.Sub:GetVertexWeights() end

--- 
--- @param rotation math.Quaternion
function game.Model.Mesh.Sub:Rotate(rotation) end

--- 
--- @param idx int
function game.Model.Mesh.Sub:AddPoint(idx) end

--- 
--- @param texture int
function game.Model.Mesh.Sub:SetSkinTextureIndex(texture) end

--- 
--- @param pose math.ScaledTransform
function game.Model.Mesh.Sub:Transform(pose) end

--- 
--- @param idx int
--- @param alpha math.Vector2
function game.Model.Mesh.Sub:SetVertexAlpha(idx, alpha) end

--- 
function game.Model.Mesh.Sub:GetBounds() end

--- 
--- @param idx int
--- @param v game.Model.Vertex
function game.Model.Mesh.Sub:SetVertex(idx, v) end

--- 
--- @param v game.Model.Vertex
function game.Model.Mesh.Sub:AddVertex(v) end

--- 
function game.Model.Mesh.Sub:ClearExtendedVertexWeights() end

--- 
--- @param idx int
--- @param normal math.Vector
function game.Model.Mesh.Sub:SetVertexNormal(idx, normal) end

--- 
--- @param idx int
--- @param uv math.Vector2
--- @overload fun(uvSetName: string, idx: int, uv: math.Vector2): 
function game.Model.Mesh.Sub:SetVertexUV(idx, uv) end

--- 
--- @param idx int
function game.Model.Mesh.Sub:GetVertexPosition(idx) end

--- 
--- @param idx int
function game.Model.Mesh.Sub:GetVertexAlpha(idx) end

--- 
--- @param idx int
function game.Model.Mesh.Sub:GetVertexWeight(idx) end

--- 
function game.Model.Mesh.Sub:GenerateNormals() end

--- 
--- @param numVerts int
function game.Model.Mesh.Sub:ReserveVertexWeights(numVerts) end

--- 
--- @param nu math.Vector
--- @param nv math.Vector
--- @param w int
--- @param h int
--- @param ou number
--- @param ov number
--- @param su number
--- @param sv number
--- @overload fun(mdl: game.Model, nu: math.Vector, nv: math.Vector, ou: number, ov: number, su: number, sv: number): 
function game.Model.Mesh.Sub:ApplyUVMapping(nu, nv, w, h, ou, ov, su, sv) end

--- 
--- @param verts table
function game.Model.Mesh.Sub:SetVertices(verts) end

--- 
--- @param numTris int
function game.Model.Mesh.Sub:ReserveTriangles(numTris) end

--- 
--- @return string ret0
function game.Model.Mesh.Sub:GetUuid() end

--- 
function game.Model.Mesh.Sub:MakeVerticesUnique() end

--- 
--- @param indices table
function game.Model.Mesh.Sub:SetIndices(indices) end

--- 
function game.Model.Mesh.Sub:GetGeometryType() end

--- 
--- @param geometryType int
function game.Model.Mesh.Sub:SetGeometryType(geometryType) end

--- 
--- @param idx0 int
--- @param idx1 int
function game.Model.Mesh.Sub:AddLine(idx0, idx1) end

--- 
function game.Model.Mesh.Sub:ClearIndices() end

--- 
function game.Model.Mesh.Sub:ClearAlphas() end

--- 
function game.Model.Mesh.Sub:ClearUVSets() end

--- 
--- @param uvSetName string
function game.Model.Mesh.Sub:HasUVSet(uvSetName) end

--- 
function game.Model.Mesh.Sub:ClearVertexWeights() end

--- 
function game.Model.Mesh.Sub:ClearVertexData() end

--- 
--- @param arg1 int
function game.Model.Mesh.Sub:ReserveIndices(arg1) end

--- 
--- @param assetData udm.AssetData
function game.Model.Mesh.Sub:Save(assetData) end

--- 
--- @param numVerts int
function game.Model.Mesh.Sub:ReserveVertices(numVerts) end

--- 
--- @param n int
function game.Model.Mesh.Sub:SetVertexCount(n) end

--- 
--- @param arg1 int
--- @return int ret0
function game.Model.Mesh.Sub:GetIndex(arg1) end

--- 
--- @param translation math.Vector
function game.Model.Mesh.Sub:Translate(translation) end

--- 
function game.Model.Mesh.Sub:GetPose() end

--- 
--- @param idx int
function game.Model.Mesh.Sub:GetVertexNormal(idx) end

--- 
--- @param pose math.ScaledTransform
function game.Model.Mesh.Sub:SetPose(pose) end

--- 
function game.Model.Mesh.Sub:GetCenter() end

--- 
function game.Model.Mesh.Sub:GetTriangleCount() end

--- 
--- @param idx int
--- @param pos math.Vector
function game.Model.Mesh.Sub:SetVertexPosition(idx, pos) end

--- 
function game.Model.Mesh.Sub:ClearVertices() end

--- 
function game.Model.Mesh.Sub:__tostring() end

--- 
function game.Model.Mesh.Sub:GetSkinTextureIndex() end

--- 
--- @param uvSetName string
--- @return any ret0
--- @overload fun(): any
function game.Model.Mesh.Sub:GetUVs(uvSetName) end

--- 
function game.Model.Mesh.Sub:FlipTriangleWindingOrder() end

--- 
--- @param n int
function game.Model.Mesh.Sub:SetIndexCount(n) end

--- 
function game.Model.Mesh.Sub:GetIndexCount() end

--- 
--- @return game.Model.Mesh.Sub ret0
--- @overload fun(fullCopy: bool): game.Model.Mesh.Sub
function game.Model.Mesh.Sub:Copy() end

--- 
--- @return enum pragma::model::IndexType ret0
function game.Model.Mesh.Sub:GetIndexType() end

--- 
--- @param arg1 enum pragma::model::IndexType
function game.Model.Mesh.Sub:SetIndexType(arg1) end

--- 
--- @param uvSetName string
function game.Model.Mesh.Sub:AddUVSet(uvSetName) end

--- 
--- @param scale math.Vector
function game.Model.Mesh.Sub:Scale(scale) end

--- 
--- @return string ret0
function game.Model.Mesh.Sub:GetName() end

--- 
--- @param arg1 string
function game.Model.Mesh.Sub:SetName(arg1) end

--- 
--- @param idx int
function game.Model.Mesh.Sub:GetVertex(idx) end

--- 
--- @param idx int
--- @overload fun(uvSetName: string, idx: int): 
function game.Model.Mesh.Sub:GetVertexUV(idx) end

--- 
--- @param arg1 number
--- @overload fun(arg1: number): 
function game.Model.Mesh.Sub:Optimize(arg1) end

--- 
--- @param n math.Vector
--- @param d number
--- @overload fun(n: math.Vector, d: number, bSplitCoverMeshes: bool): 
--- @overload fun(n: math.Vector, d: number, bSplitCoverMeshes: bool, tBoneMatrices: any): 
function game.Model.Mesh.Sub:ClipAgainstPlane(n, d) end

--- 
--- @param idx int
--- @return int ret0_1
--- @return int ret0_2
--- @return int ret0_3
function game.Model.Mesh.Sub:GetTriangle(idx) end

--- 
--- @param idx int
--- @param t math.Vector4
function game.Model.Mesh.Sub:SetVertexTangent(idx, t) end

--- 
function game.Model.Mesh.Sub:GetTangents() end

--- 
function game.Model.Mesh.Sub:GetVertexBuffer() end

--- 
function game.Model.Mesh.Sub:GetVertexWeightBuffer() end

--- 
function game.Model.Mesh.Sub:GetIndexBuffer() end

--- 
function game.Model.Mesh.Sub:GetSceneMesh() end

--- 
--- @return udm.PropertyWrapper ret0
function game.Model.Mesh.Sub:GetExtensionData() end

--- 
function game.Model.Mesh.Sub:GetAlphaBuffer() end

--- 
function game.Model.Mesh.Sub:GetBiTangents() end


--- @enum Max
game.Model.Mesh.Sub = {
	MAX_INDEX16 = 65535,
	MAX_INDEX32 = -1,
}

--- @enum GeometryType
game.Model.Mesh.Sub = {
	GEOMETRY_TYPE_LINES = 1,
	GEOMETRY_TYPE_POINTS = 2,
	GEOMETRY_TYPE_TRIANGLES = 0,
}

--- @enum IndexType
game.Model.Mesh.Sub = {
	INDEX_TYPE_UINT16 = 0,
	INDEX_TYPE_UINT32 = 1,
}

--- 
--- @class game.Model.Eyeball
--- @field boneIndex int 
--- @field zOffset number 
--- @field up  
--- @field origin  
--- @field name string 
--- @field maxDilationFactor number 
--- @field irisScale number 
--- @field irisUvRadius number 
--- @field forward  
--- @field radius number 
--- @field irisMaterialIndex int 
--- @overload fun():game.Model.Eyeball
game.Model.Eyeball = {}

--- 
--- @return any ret0
function game.Model.Eyeball:GetUpperLidFlexIndices() end

--- 
--- @return any ret0
function game.Model.Eyeball:GetUpperLidFlexAngles() end

--- 
--- @return any ret0
function game.Model.Eyeball:GetLowerLidFlexIndices() end

--- 
--- @return any ret0
function game.Model.Eyeball:GetLowerLidFlexAngles() end

--- 
--- @return int ret0
function game.Model.Eyeball:GetLowerLidFlexIndex() end

--- 
--- @return int ret0
function game.Model.Eyeball:GetUpperLidFlexIndex() end


--- 
--- @class game.Model.Animation
game.Model.Animation = {}

--- 
--- @return table ret0
function game.Model.Animation:GetBoneList() end

--- 
--- @param boneId int
function game.Model.Animation:LookupBone(boneId) end

--- 
--- @param arg1 math.Vector
function game.Model.Animation:Scale(arg1) end

--- 
--- @param arg1 int
--- @return number ret0
function game.Model.Animation:GetBoneWeight(arg1) end

--- 
--- @param frame Animation.Frame
--- @overload fun(frame: game.Model.Animation.Frame): 
function game.Model.Animation:AddFrame(frame) end

--- 
function game.Model.Animation:GetBlendController() end

--- 
--- @return int ret0
function game.Model.Animation:GetActivityWeight() end

--- 
--- @return enum FAnim ret0
function game.Model.Animation:GetFlags() end

--- 
--- @param ID int
function game.Model.Animation:GetFrame(ID) end

--- 
--- @return int ret0
function game.Model.Animation:GetBoneCount() end

--- 
--- @param arg1 enum FAnim
function game.Model.Animation:SetFlags(arg1) end

--- 
--- @return int ret0
function game.Model.Animation:GetFrameCount() end

--- 
function game.Model.Animation:ClearFrames() end

--- 
--- @param frameId int
--- @param idx int
--- @param oArgs any
function game.Model.Animation:SetEventArgs(frameId, idx, oArgs) end

--- 
--- @return enum Activity ret0
function game.Model.Animation:GetActivity() end

--- 
--- @param mdl game.Model
function game.Model.Animation:CalcRenderBounds(mdl) end

--- 
--- @param arg1 game.Model.Skeleton
--- @param arg2 math.Vector
function game.Model.Animation:Translate(arg1, arg2) end

--- 
--- @return any ret0
function game.Model.Animation:GetActivityName() end

--- 
function game.Model.Animation:Reverse() end

--- 
--- @param arg1 int
--- @param arg2 number
function game.Model.Animation:SetBoneWeight(arg1, arg2) end

--- 
--- @param idx int
--- @return int ret0
function game.Model.Animation:GetBoneId(idx) end

--- 
--- @param arg1 enum Activity
function game.Model.Animation:SetActivity(arg1) end

--- 
--- @param arg1 int
function game.Model.Animation:SetActivityWeight(arg1) end

--- 
--- @return int ret0
function game.Model.Animation:GetFPS() end

--- 
--- @param arg1 int
function game.Model.Animation:SetFPS(arg1) end

--- 
--- @param arg1 enum FAnim
function game.Model.Animation:AddFlags(arg1) end

--- 
--- @param arg1 enum FAnim
function game.Model.Animation:RemoveFlags(arg1) end

--- 
--- @return number ret0
function game.Model.Animation:GetDuration() end

--- 
--- @param frameId int
--- @param eventId int
--- @param args any
function game.Model.Animation:AddEvent(frameId, eventId, args) end

--- 
--- @overload fun(frameId: int): 
function game.Model.Animation:GetEvents() end

--- 
--- @overload fun(frameId: int): 
function game.Model.Animation:GetEventCount() end

--- 
--- @return number ret0
function game.Model.Animation:GetFadeInTime() end

--- 
--- @return number ret0
function game.Model.Animation:GetFadeOutTime() end

--- 
--- @param arg1 game.Model.Skeleton
--- @param arg2 math.Quaternion
function game.Model.Animation:Rotate(arg1, arg2) end

--- 
--- @param  game.Model
function game.Model.Animation:GetRenderBounds(arg) end

--- 
--- @param arg1 number
function game.Model.Animation:SetFadeOutTime(arg1) end

--- 
--- @param frameId int
--- @param idx int
function game.Model.Animation:RemoveEvent(frameId, idx) end

--- 
--- @param arg1 int
--- @return int ret0
function game.Model.Animation:AddBoneId(arg1) end

--- 
--- @param frameId int
--- @param idx int
--- @param type int
--- @param oArgs any
function game.Model.Animation:SetEventData(frameId, idx, type, oArgs) end

--- 
--- @param frameId int
--- @param idx int
--- @param type int
function game.Model.Animation:SetEventType(frameId, idx, type) end

--- 
--- @param o any
function game.Model.Animation:SetBoneList(o) end

--- 
function game.Model.Animation:GetFrames() end

--- 
--- @param arg1 number
function game.Model.Animation:SetFadeInTime(arg1) end

--- 
function game.Model.Animation:GetBoneWeights() end

--- 
--- @param assetData udm.AssetData
function game.Model.Animation:Save(assetData) end


--- @enum Flag
game.Model.Animation = {
	FLAG_MOVEZ = 64,
	FLAG_AUTOPLAY = 128,
	FLAG_GESTURE = 256,
	FLAG_LOOP = 1,
	FLAG_MOVEX = 32,
	FLAG_NOREPEAT = 2,
}

--- 
--- @class game.Model.Animation.Frame
game.Model.Animation.Frame = {}

--- 
--- @param scale math.Vector
function game.Model.Animation.Frame:Scale(scale) end

--- 
function game.Model.Animation.Frame:GetMoveTranslation() end

--- 
function game.Model.Animation.Frame:GetMoveTranslationX() end

--- 
--- @param object any
function game.Model.Animation.Frame:SetFlexControllerWeights(object) end

--- 
--- @param x number
--- @param z number
function game.Model.Animation.Frame:SetMoveTranslation(x, z) end

--- 
--- @param z number
function game.Model.Animation.Frame:SetMoveTranslationZ(z) end

--- 
--- @param anim game.Model.Animation
--- @param mdl game.Model
function game.Model.Animation.Frame:CalcRenderBounds(anim, mdl) end

--- 
--- @param boneId int
--- @param scale math.Vector
function game.Model.Animation.Frame:SetBoneScale(boneId, scale) end

--- 
--- @param anim game.Model.Animation
--- @param skeleton game.Model.Skeleton
--- @param t math.Vector
function game.Model.Animation.Frame:Translate(anim, skeleton, t) end

--- 
--- @param x number
function game.Model.Animation.Frame:SetMoveTranslationX(x) end

--- 
--- @param boneID int
--- @param pos math.Vector
--- @param rot math.Quaternion
--- @overload fun(boneID: int, pos: math.Vector, rot: math.Quaternion, scale: math.Vector): 
function game.Model.Animation.Frame:SetBoneTransform(boneID, pos, rot) end

--- 
--- @param boneID int
function game.Model.Animation.Frame:GetBoneRotation(boneID) end

--- 
--- @param anim game.Model.Animation
--- @param skeleton game.Model.Skeleton
--- @param rot math.Quaternion
function game.Model.Animation.Frame:Rotate(anim, skeleton, rot) end

--- 
--- @param skeleton game.Model.Skeleton
--- @param boneId int
function game.Model.Animation.Frame:GetLocalBoneTransform(skeleton, boneId) end

--- 
--- @param boneCount int
function game.Model.Animation.Frame:SetBoneCount(boneCount) end

--- 
--- @param boneId int
--- @param pose math.Transform
--- @overload fun(boneId: int, pose: math.ScaledTransform): 
function game.Model.Animation.Frame:SetBonePose(boneId, pose) end

--- 
function game.Model.Animation.Frame:Copy() end

--- 
function game.Model.Animation.Frame:GetFlexControllerIds() end

--- 
function game.Model.Animation.Frame:GetFlexControllerWeights() end

--- 
function game.Model.Animation.Frame:GetMoveTranslationZ() end

--- 
--- @param boneId int
function game.Model.Animation.Frame:GetBonePose(boneId) end

--- 
function game.Model.Animation.Frame:GetBoneCount() end

--- 
--- @param boneId int
function game.Model.Animation.Frame:GetBoneScale(boneId) end

--- 
--- @param boneID int
function game.Model.Animation.Frame:GetBoneMatrix(boneID) end

--- 
--- @param boneID int
function game.Model.Animation.Frame:GetBoneTransform(boneID) end

--- 
--- @param boneID int
--- @param pos math.Vector
function game.Model.Animation.Frame:SetBonePosition(boneID, pos) end

--- 
--- @param boneID int
--- @param rot math.Quaternion
function game.Model.Animation.Frame:SetBoneRotation(boneID, rot) end

--- 
--- @param skeleton game.Model.Skeleton
--- @overload fun(anim: game.Model.Animation, skeleton: game.Model.Skeleton): 
function game.Model.Animation.Frame:Localize(skeleton) end

--- 
--- @param skeleton game.Model.Skeleton
--- @overload fun(anim: game.Model.Animation, skeleton: game.Model.Skeleton): 
function game.Model.Animation.Frame:Globalize(skeleton) end


--- 
--- @class game.Model.MeshGroup
game.Model.MeshGroup = {}

--- 
function game.Model.MeshGroup:GetName() end

--- 
--- @param index int
function game.Model.MeshGroup:GetMeshCount(index) end

--- 
--- @param mesh game.Model.Mesh
function game.Model.MeshGroup:AddMesh(mesh) end

--- 
--- @param 1 game.Model.MeshGroup
function game.Model.MeshGroup:__eq(arg1) end

--- 
function game.Model.MeshGroup:ClearMeshes() end

--- 
--- @param tMeshes any
function game.Model.MeshGroup:SetMeshes(tMeshes) end

--- 
--- @param index int
function game.Model.MeshGroup:GetMesh(index) end

--- 
function game.Model.MeshGroup:GetMeshes() end


--- 
--- @class game.Model.VertexAnimation
game.Model.VertexAnimation = {}

--- 
function game.Model.VertexAnimation:GetName() end

--- 
function game.Model.VertexAnimation:GetMeshAnimations() end

--- 
--- @param rot math.Quaternion
function game.Model.VertexAnimation:Rotate(rot) end


--- 
--- @class game.Model.VertexAnimation.MeshAnimation
game.Model.VertexAnimation.MeshAnimation = {}

--- 
function game.Model.VertexAnimation.MeshAnimation:GetMesh() end

--- 
--- @param rot math.Quaternion
function game.Model.VertexAnimation.MeshAnimation:Rotate(rot) end

--- 
function game.Model.VertexAnimation.MeshAnimation:GetFrames() end


--- 
--- @class game.Model.VertexAnimation.MeshAnimation.Frame
game.Model.VertexAnimation.MeshAnimation.Frame = {}

--- 
--- @param flags int
function game.Model.VertexAnimation.MeshAnimation.Frame:SetFlags(flags) end

--- 
--- @param vertIdx int
--- @param pos math.Vector
function game.Model.VertexAnimation.MeshAnimation.Frame:SetVertexPosition(vertIdx, pos) end

--- 
--- @param vertIdx int
--- @param n math.Vector
function game.Model.VertexAnimation.MeshAnimation.Frame:SetVertexNormal(vertIdx, n) end

--- 
--- @param rot math.Quaternion
function game.Model.VertexAnimation.MeshAnimation.Frame:Rotate(rot) end

--- 
--- @param vertIdx int
function game.Model.VertexAnimation.MeshAnimation.Frame:GetVertexNormal(vertIdx) end

--- 
function game.Model.VertexAnimation.MeshAnimation.Frame:GetVertexCount() end

--- 
function game.Model.VertexAnimation.MeshAnimation.Frame:GetVertices() end

--- 
--- @param vertId int
--- @param value number
function game.Model.VertexAnimation.MeshAnimation.Frame:SetDeltaValue(vertId, value) end

--- 
--- @param vertId int
function game.Model.VertexAnimation.MeshAnimation.Frame:GetDeltaValue(vertId) end

--- 
--- @param vertIdx int
function game.Model.VertexAnimation.MeshAnimation.Frame:GetVertexPosition(vertIdx) end

--- 
function game.Model.VertexAnimation.MeshAnimation.Frame:GetFlags() end

--- 
--- @param numVerts int
function game.Model.VertexAnimation.MeshAnimation.Frame:SetVertexCount(numVerts) end


--- @enum Flag
game.Model.VertexAnimation.MeshAnimation.Frame = {
	FLAG_BIT_HAS_DELTA_VALUES = 1,
	FLAG_NONE = 0,
}

--- 
--- @class game.Model.VertexWeight
--- @field boneIds  
--- @field weights  
--- @overload fun():game.Model.VertexWeight
--- @overload fun(arg1: math.Vector4i, arg2: math.Vector4):game.Model.VertexWeight
game.Model.VertexWeight = {}

--- 
--- @return game.Model.VertexWeight ret0
function game.Model.VertexWeight:Copy() end

--- 
function game.Model.VertexWeight:__tostring() end

--- 
--- @param 1 game.Model.VertexWeight
function game.Model.VertexWeight:__eq(arg1) end


--- 
--- @class game.Model.ExportInfo
--- @field exportAnimations bool 
--- @field enableExtendedDDS bool 
--- @field exportSkinnedMeshData bool 
--- @field exportMorphTargets bool 
--- @field exportImages bool 
--- @field embedAnimations bool 
--- @field normalizeTextureNames bool 
--- @field fullExport bool 
--- @field saveAsBinary bool 
--- @field verbose bool 
--- @field generateAo bool 
--- @field aoSamples int 
--- @field aoResolution int 
--- @field scale number 
--- @field mergeMeshesByMaterial bool 
--- @field imageFormat int 
--- @field aoDevice int 
--- @overload fun():game.Model.ExportInfo
game.Model.ExportInfo = {}

--- 
--- @param oTable any
function game.Model.ExportInfo:SetAnimationList(oTable) end


--- @enum ImageFormat
game.Model.ExportInfo = {
	IMAGE_FORMAT_JPG = 3,
	IMAGE_FORMAT_BMP = 1,
	IMAGE_FORMAT_DDS = 5,
	IMAGE_FORMAT_HDR = 4,
	IMAGE_FORMAT_KTX = 6,
	IMAGE_FORMAT_PNG = 0,
	IMAGE_FORMAT_TGA = 2,
}

--- @enum Device
game.Model.ExportInfo = {
	DEVICE_CPU = 0,
	DEVICE_GPU = 1,
}

--- 
--- @class game.Material
game.Material = {}

--- 
--- @return bool ret0
function game.Material:IsTranslucent() end

--- 
--- @return number ret0
function game.Material:GetAlphaCutoff() end

--- 
--- @return string ret0
function game.Material:GetName() end

--- 
--- @param textureID string
function game.Material:GetTextureInfo(textureID) end

--- 
--- @return enum AlphaMode ret0
function game.Material:GetAlphaMode() end

--- 
--- @param shader string
function game.Material:SetShader(shader) end

--- 
function game.Material:GetData() end

--- 
function game.Material:__tostring() end

--- 
--- @param mat game.Material
--- @param fname string
--- @return variant ret0
--- @overload fun(): variant
--- @overload fun(assetData: udm.AssetData): variant
function game.Material:Save(mat, fname) end

--- 
function game.Material:Reset() end

--- 
--- @return bool ret0
function game.Material:IsError() end

--- 
--- @return bool ret0
function game.Material:IsValid() end

--- 
function game.Material:GetSpriteSheetAnimation() end

--- 
function game.Material:Copy() end

--- 
--- @return string ret0
function game.Material:GetShaderName() end

--- 
--- @return util.DataBlock ret0
function game.Material:GetDataBlock() end

--- 
--- @return bool ret0
function game.Material:IsLoaded() end

--- 
--- @param arg1 bool
function game.Material:SetLoaded(arg1) end

--- 
--- @param data udm.LinkedPropertyWrapper
--- @return bool ret0
function game.Material:MergeData(data) end

--- 
function game.Material:UpdateTextures() end

--- 
function game.Material:ClearSpriteSheetAnimation() end

--- 
--- @overload fun(reload: bool): 
function game.Material:InitializeShaderDescriptorSet() end

--- 
--- @param textureID string
--- @param hTex prosper.Texture
--- @param name string
--- @overload fun(textureID: string, hTex: prosper.Texture): 
--- @overload fun(textureID: string, tex: util.Texture): 
--- @overload fun(textureID: string, tex: string): 
function game.Material:SetTexture(textureID, hTex, name) end


--- @enum DetailBlendModeTranslucent
game.Material = {
	DETAIL_BLEND_MODE_TRANSLUCENT_BASE = 4,
	DETAIL_BLEND_MODE_TRANSLUCENT_DETAIL = 2,
}

--- @enum DetailBlendMode
game.Material = {
	DETAIL_BLEND_MODE_BASE_MASK_VIA_DETAIL_ALPHA = 9,
	DETAIL_BLEND_MODE_ADDITIVE = 1,
	DETAIL_BLEND_MODE_MULTIPLY = 8,
	DETAIL_BLEND_MODE_BLEND_FACTOR_FADE = 3,
	DETAIL_BLEND_MODE_INVALID = 255,
	DETAIL_BLEND_MODE_SELF_SHADOWED_BUMPMAP = 10,
	DETAIL_BLEND_MODE_DECAL_MODULATE = 0,
	DETAIL_BLEND_MODE_COUNT = 12,
	DETAIL_BLEND_MODE_SSBUMP_ALBEDO = 11,
	DETAIL_BLEND_MODE_TWO_PATTERN_DECAL_MODULATE = 7,
}

--- @enum DetailBlendModeUnlit
game.Material = {
	DETAIL_BLEND_MODE_UNLIT_ADDITIVE = 5,
	DETAIL_BLEND_MODE_UNLIT_ADDITIVE_THRESHOLD_FADE = 6,
}

--- @enum AlphaMode
game.Material = {
	ALPHA_MODE_OPAQUE = 0,
	ALPHA_MODE_BLEND = 2,
	ALPHA_MODE_MASK = 1,
}

--- 
--- @class game.Material.SpriteSheetAnimation
game.Material.SpriteSheetAnimation = {}

--- 
--- @param seqIdx int
--- @return game.Material.SpriteSheetAnimation.Sequence ret0
function game.Material.SpriteSheetAnimation:GetSequence(seqIdx) end

--- 
--- @return table ret0
function game.Material.SpriteSheetAnimation:GetSequences() end

--- 
--- @return int ret0
function game.Material.SpriteSheetAnimation:GetSequenceCount() end


--- 
--- @class game.Material.SpriteSheetAnimation.Sequence
game.Material.SpriteSheetAnimation.Sequence = {}

--- 
--- @return number ret0
function game.Material.SpriteSheetAnimation.Sequence:GetDuration() end

--- 
--- @return int ret0
function game.Material.SpriteSheetAnimation.Sequence:GetFrameCount() end

--- 
--- @param ptTime number
--- @return int ret0_1
--- @return int ret0_2
--- @return number ret0_3
function game.Material.SpriteSheetAnimation.Sequence:GetInterpolatedFrameData(ptTime) end

--- 
--- @return int ret0
function game.Material.SpriteSheetAnimation.Sequence:GetFrameOffset() end

--- 
--- @param frameIdx int
--- @return game.Material.SpriteSheetAnimation.Sequence.Frame ret0
function game.Material.SpriteSheetAnimation.Sequence:GetFrame(frameIdx) end

--- 
--- @return table ret0
function game.Material.SpriteSheetAnimation.Sequence:GetFrames() end

--- 
--- @return bool ret0
function game.Material.SpriteSheetAnimation.Sequence:IsLooping() end


--- 
--- @class game.Material.SpriteSheetAnimation.Sequence.Frame
game.Material.SpriteSheetAnimation.Sequence.Frame = {}

--- 
--- @return number ret0
function game.Material.SpriteSheetAnimation.Sequence.Frame:GetDuration() end

--- 
--- @return math.Vector2 ret0_1
--- @return math.Vector2 ret0_2
function game.Material.SpriteSheetAnimation.Sequence.Frame:GetUVBounds() end


--- 
--- @class game.BaseRenderProcessor
game.BaseRenderProcessor = {}

--- 
--- @param d number
--- @param delta number
function game.BaseRenderProcessor:SetDepthBias(d, delta) end


--- 
--- @class game.WorldEnvironment
game.WorldEnvironment = {}

--- 
--- @return bool ret0
function game.WorldEnvironment:IsUnlit() end

--- 
--- @return int ret0
function game.WorldEnvironment:GetShaderQuality() end

--- 
--- @param start number
function game.WorldEnvironment:SetFogStart(start) end

--- 
--- @param end number
function game.WorldEnvironment:SetFogEnd(end_) end

--- 
function game.WorldEnvironment:GetFogStart() end

--- 
function game.WorldEnvironment:GetShaderQualityProperty() end

--- 
function game.WorldEnvironment:GetFogEnd() end

--- 
--- @param bEnabled bool
function game.WorldEnvironment:SetFogEnabled(bEnabled) end

--- 
--- @param color util.Color
function game.WorldEnvironment:SetFogColor(color) end

--- 
function game.WorldEnvironment:IsFogEnabled() end

--- 
function game.WorldEnvironment:GetFogColor() end

--- 
--- @param density number
function game.WorldEnvironment:SetFogDensity(density) end

--- 
function game.WorldEnvironment:GetFogDensity() end

--- 
--- @param type enum util::FogType
function game.WorldEnvironment:SetFogType(type) end

--- 
function game.WorldEnvironment:GetFogType() end

--- 
function game.WorldEnvironment:GetFogFarDistance() end

--- 
function game.WorldEnvironment:GetUnlitProperty() end

--- 
function game.WorldEnvironment:GetShadowResolutionProperty() end

--- 
function game.WorldEnvironment:GetFogColorProperty() end

--- 
--- @return int ret0
function game.WorldEnvironment:GetShadowResolution() end

--- 
function game.WorldEnvironment:GetFogStartProperty() end

--- 
function game.WorldEnvironment:GetFogEndProperty() end

--- 
--- @param arg1 int
function game.WorldEnvironment:SetShaderQuality(arg1) end

--- 
function game.WorldEnvironment:GetFogMaxDensityProperty() end

--- 
function game.WorldEnvironment:GetFogTypeProperty() end

--- 
function game.WorldEnvironment:GetFogEnabledProperty() end

--- 
--- @param arg1 bool
function game.WorldEnvironment:SetUnlit(arg1) end

--- 
--- @param arg1 int
function game.WorldEnvironment:SetShadowResolution(arg1) end


--- @enum FogType
game.WorldEnvironment = {
	FOG_TYPE_LINEAR = 0,
	FOG_TYPE_EXPONENTIAL = 1,
	FOG_TYPE_EXPONENTIAL2 = 2,
}

--- 
--- @class game.RenderPassStats
--- @overload fun():game.RenderPassStats
game.RenderPassStats = {}

--- 
--- @return game.RenderPassStats ret0
function game.RenderPassStats:Copy() end

--- 
--- @param timer enum RenderPassStats::Timer
--- @return number ret0
function game.RenderPassStats:GetTime(timer) end

--- 
--- @param 1 game.RenderPassStats
function game.RenderPassStats:__add(arg1) end

--- 
--- @param counter enum RenderPassStats::Counter
--- @return int ret0
function game.RenderPassStats:GetCount(counter) end


--- @enum TimerCpu
game.RenderPassStats = {
	TIMER_CPU_COUNT = 6,
	TIMER_CPU_END = 6,
	TIMER_CPU_EXECUTION = 2,
	TIMER_CPU_START = 1,
}

--- @enum CounterEntity
game.RenderPassStats = {
	COUNTER_ENTITY_BUFFER_UPDATES = 7,
	COUNTER_ENTITY_STATE_CHANGES = 2,
}

--- @enum CounterInstanced
game.RenderPassStats = {
	COUNTER_INSTANCED_MESHES = 10,
	COUNTER_INSTANCED_SKIPPED_RENDER_ITEMS = 11,
}

--- @enum CounterInstance
game.RenderPassStats = {
	COUNTER_INSTANCE_SETS = 8,
	COUNTER_INSTANCE_SET_MESHES = 9,
}

--- @enum TimerGpu
game.RenderPassStats = {
	TIMER_GPU_COUNT = 1,
	TIMER_GPU_END = 0,
	TIMER_GPU_EXECUTION = 0,
	TIMER_GPU_START = 0,
}

--- @enum CounterDrawn
game.RenderPassStats = {
	COUNTER_DRAWN_MESHES = 4,
	COUNTER_DRAWN_TRIANGLES = 6,
	COUNTER_DRAWN_VERTICES = 5,
}

--- @enum Counter
game.RenderPassStats = {
	COUNTER_COUNT = 13,
	COUNTER_DRAW_CALLS = 3,
	COUNTER_ENTITIES_WITHOUT_INSTANCING = 12,
	COUNTER_SHADER_STATE_CHANGES = 0,
	COUNTER_MATERIAL_STATE_CHANGES = 1,
}

--- @enum Timer
game.RenderPassStats = {
	TIMER_ENTITY_BIND = 4,
	TIMER_COUNT = 7,
	TIMER_DRAW_CALL = 5,
	TIMER_RENDER_THREAD_WAIT = 1,
	TIMER_MATERIAL_BIND = 3,
	TIMER_SHADER_BIND = 6,
}

--- 
--- @class game.RenderStats
--- @overload fun():game.RenderStats
game.RenderStats = {}

--- 
--- @return game.RenderStats ret0
function game.RenderStats:Copy() end

--- 
--- @param pass enum RenderStats::RenderPass
--- @return game.RenderPassStats ret0
function game.RenderStats:GetPassStats(pass) end

--- 
--- @param timer enum RenderStats::RenderStage
--- @return number ret0
function game.RenderStats:GetTime(timer) end

--- 
--- @param 1 game.RenderStats
function game.RenderStats:__add(arg1) end


--- @enum TimerPostProcessingGpu
game.RenderStats = {
	TIMER_POST_PROCESSING_GPU = 1,
	TIMER_POST_PROCESSING_GPU_TONE_MAPPING = 4,
	TIMER_POST_PROCESSING_GPU_BLOOM = 3,
	TIMER_POST_PROCESSING_GPU_FOG = 2,
	TIMER_POST_PROCESSING_GPU_FXAA = 5,
	TIMER_POST_PROCESSING_GPU_SSAO = 6,
}

--- @enum RenderPassLighting
game.RenderStats = {
	RENDER_PASS_LIGHTING_PASS = 0,
	RENDER_PASS_LIGHTING_PASS_TRANSLUCENT = 1,
}

--- @enum Timer
game.RenderStats = {
	TIMER_UPDATE_RENDER_BUFFERS_CPU = 11,
	TIMER_COUNT = 12,
	TIMER_PREPASS_EXECUTION_CPU = 8,
	TIMER_POST_PROCESSING_EXECUTION_CPU = 10,
	TIMER_LIGHTING_PASS_EXECUTION_CPU = 9,
}

--- @enum TimerLightCulling
game.RenderStats = {
	TIMER_LIGHT_CULLING_CPU = 7,
	TIMER_LIGHT_CULLING_GPU = 0,
}

--- @enum RenderPass
game.RenderStats = {
	RENDER_PASS_COUNT = 4,
	RENDER_PASS_SHADOW_PASS = 3,
	RENDER_PASS_PREPASS = 2,
}

--- 
--- @class game.DrawSceneInfo
--- @field renderStats any 
--- @field renderFlags int 
--- @field commandBuffer prosper.CommandBuffer 
--- @field flags int 
--- @field scene any 
--- @field toneMapping any 
--- @field renderTarget prosper.RenderTarget 
--- @field exclusionMask pragma::rendering::RenderMask 
--- @field inclusionMask pragma::rendering::RenderMask 
--- @field outputImage prosper.Image 
--- @field outputLayerId int 
--- @field clearColor util.Color 
--- @field clipPlane math.Vector4 
--- @field pvsOrigin math.Vector 
--- @overload fun():game.DrawSceneInfo
game.DrawSceneInfo = {}

--- 
--- @param arg1 game.DrawSceneInfo
function game.DrawSceneInfo:AddSubPass(arg1) end

--- 
--- @param f any
function game.DrawSceneInfo:SetEntityRenderFilter(f) end

--- 
--- @param f any
function game.DrawSceneInfo:SetEntityPrepassFilter(f) end


--- @enum FlagDisable
game.DrawSceneInfo = {
	FLAG_DISABLE_LIGHTING_PASS_BIT = 16,
	FLAG_DISABLE_PREPASS_BIT = 8,
	FLAG_DISABLE_RENDER_BIT = 2,
}

--- @enum Flag
game.DrawSceneInfo = {
	FLAG_DISABLE_RENDER_BIT = 2,
	FLAG_FLIP_VERTICALLY_BIT = 1,
}

--- 
--- @class game.RenderQueue
game.RenderQueue = {}

--- 
function game.RenderQueue:WaitForCompletion() end

--- 
--- @return bool ret0
function game.RenderQueue:IsComplete() end


--- 
--- @class game.DepthStageRenderProcessor: game.BaseRenderProcessor
game.DepthStageRenderProcessor = {}

--- 
--- @param renderQueue game.RenderQueue
function game.DepthStageRenderProcessor:Render(renderQueue) end


--- 
--- @class game.LightingStageRenderProcessor: game.BaseRenderProcessor
game.LightingStageRenderProcessor = {}

--- 
--- @param renderQueue game.RenderQueue
function game.LightingStageRenderProcessor:Render(renderQueue) end


