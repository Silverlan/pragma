--- @meta
--- 
--- @class Animation
Animation = {}


--- 
--- @class Animation.Frame
Animation.Frame = {}

--- 
--- @param boneID int
function Animation.Frame:GetBoneTransform(boneID) end

--- 
--- @param boneId int
function Animation.Frame:GetBonePose(boneId) end

--- 
--- @param skeleton game.Model.Skeleton
--- @param boneId int
function Animation.Frame:GetLocalBoneTransform(skeleton, boneId) end

--- 
function Animation.Frame:Copy() end

--- 
function Animation.Frame:GetFlexControllerIds() end

--- 
--- @param x number
--- @param z number
function Animation.Frame:SetMoveTranslation(x, z) end

--- 
--- @param object any
function Animation.Frame:SetFlexControllerWeights(object) end

--- 
--- @param boneID int
function Animation.Frame:GetBoneRotation(boneID) end

--- 
--- @param boneID int
--- @param pos math.Vector
--- @param rot math.Quaternion
--- @overload fun(boneID: int, pos: math.Vector, rot: math.Quaternion, scale: math.Vector): 
function Animation.Frame:SetBoneTransform(boneID, pos, rot) end

--- 
--- @param x number
function Animation.Frame:SetMoveTranslationX(x) end

--- 
function Animation.Frame:GetFlexControllerWeights() end

--- 
function Animation.Frame:GetMoveTranslationZ() end

--- 
--- @param boneId int
--- @param pose math.Transform
--- @overload fun(boneId: int, pose: math.ScaledTransform): 
function Animation.Frame:SetBonePose(boneId, pose) end

--- 
--- @param scale math.Vector
function Animation.Frame:Scale(scale) end

--- 
function Animation.Frame:GetMoveTranslation() end

--- 
function Animation.Frame:GetBoneCount() end

--- 
--- @param boneId int
function Animation.Frame:GetBoneScale(boneId) end

--- 
--- @param boneId int
--- @param scale math.Vector
function Animation.Frame:SetBoneScale(boneId, scale) end

--- 
--- @param anim game.Model.Animation
--- @param mdl game.Model
function Animation.Frame:CalcRenderBounds(anim, mdl) end

--- 
--- @param boneID int
--- @param pos math.Vector
function Animation.Frame:SetBonePosition(boneID, pos) end

--- 
--- @param boneID int
--- @param rot math.Quaternion
function Animation.Frame:SetBoneRotation(boneID, rot) end

--- 
--- @param skeleton game.Model.Skeleton
--- @overload fun(anim: game.Model.Animation, skeleton: game.Model.Skeleton): 
function Animation.Frame:Localize(skeleton) end

--- 
--- @param skeleton game.Model.Skeleton
--- @overload fun(anim: game.Model.Animation, skeleton: game.Model.Skeleton): 
function Animation.Frame:Globalize(skeleton) end

--- 
--- @param anim game.Model.Animation
--- @param skeleton game.Model.Skeleton
--- @param rot math.Quaternion
function Animation.Frame:Rotate(anim, skeleton, rot) end

--- 
function Animation.Frame:GetMoveTranslationX() end

--- 
--- @param z number
function Animation.Frame:SetMoveTranslationZ(z) end

--- 
--- @param boneCount int
function Animation.Frame:SetBoneCount(boneCount) end

--- 
--- @param anim game.Model.Animation
--- @param skeleton game.Model.Skeleton
--- @param t math.Vector
function Animation.Frame:Translate(anim, skeleton, t) end

--- 
--- @param boneID int
function Animation.Frame:GetBoneMatrix(boneID) end


