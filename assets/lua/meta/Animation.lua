--- @meta
--- 
--- @class Animation
Animation = {}


--- 
--- @class Animation.Frame
Animation.Frame = {}

--- 
--- @param x number
function Animation.Frame:SetMoveTranslationX(x) end

--- 
--- @param boneID int
function Animation.Frame:GetBoneRotation(boneID) end

--- 
--- @param boneID int
--- @param pos math.Vector
--- @param rot math.Quaternion
--- @overload fun(boneID: int, pos: math.Vector, rot: math.Quaternion, scale: math.Vector): 
--- @overload fun(boneID: int, pos: vector.Vector, rot: math.Quaternion): 
--- @overload fun(boneID: int, pos: vector.Vector, rot: math.Quaternion, scale: vector.Vector): 
function Animation.Frame:SetBoneTransform(boneID, pos, rot) end

--- 
--- @param anim Model.Animation
--- @param skeleton Model.Skeleton
--- @param t math.Vector
--- @overload fun(anim: Model.Animation, skeleton: Model.Skeleton, t: vector.Vector): 
function Animation.Frame:Translate(anim, skeleton, t) end

--- 
function Animation.Frame:Copy() end

--- 
function Animation.Frame:GetFlexControllerIds() end

--- 
--- @param skeleton Model.Skeleton
--- @overload fun(anim: Model.Animation, skeleton: Model.Skeleton): 
function Animation.Frame:Localize(skeleton) end

--- 
--- @param boneID int
--- @param pos math.Vector
--- @overload fun(boneID: int, pos: vector.Vector): 
function Animation.Frame:SetBonePosition(boneID, pos) end

--- 
--- @param boneID int
--- @param rot math.Quaternion
function Animation.Frame:SetBoneRotation(boneID, rot) end

--- 
--- @param skeleton Model.Skeleton
--- @overload fun(anim: Model.Animation, skeleton: Model.Skeleton): 
function Animation.Frame:Globalize(skeleton) end

--- 
--- @param anim Model.Animation
--- @param mdl game.Model
function Animation.Frame:CalcRenderBounds(anim, mdl) end

--- 
--- @param boneId int
--- @param scale math.Vector
--- @overload fun(boneId: int, scale: vector.Vector): 
function Animation.Frame:SetBoneScale(boneId, scale) end

--- 
function Animation.Frame:GetMoveTranslation() end

--- 
--- @param scale math.Vector
--- @overload fun(scale: vector.Vector): 
function Animation.Frame:Scale(scale) end

--- 
function Animation.Frame:GetMoveTranslationX() end

--- 
function Animation.Frame:GetMoveTranslationZ() end

--- 
function Animation.Frame:GetFlexControllerWeights() end

--- 
--- @param x number
--- @param z number
function Animation.Frame:SetMoveTranslation(x, z) end

--- 
--- @param object any
function Animation.Frame:SetFlexControllerWeights(object) end

--- 
--- @param z number
function Animation.Frame:SetMoveTranslationZ(z) end

--- 
--- @param boneCount int
function Animation.Frame:SetBoneCount(boneCount) end

--- 
--- @param boneId int
--- @param pose math.Transform
--- @overload fun(boneId: int, pose: math.ScaledTransform): 
function Animation.Frame:SetBonePose(boneId, pose) end

--- 
--- @param boneID int
function Animation.Frame:GetBoneMatrix(boneID) end

--- 
--- @param boneID int
function Animation.Frame:GetBoneTransform(boneID) end

--- 
--- @param boneId int
function Animation.Frame:GetBonePose(boneId) end

--- 
function Animation.Frame:GetBoneCount() end

--- 
--- @param skeleton Model.Skeleton
--- @param boneId int
function Animation.Frame:GetLocalBoneTransform(skeleton, boneId) end

--- 
--- @param anim Model.Animation
--- @param skeleton Model.Skeleton
--- @param rot math.Quaternion
function Animation.Frame:Rotate(anim, skeleton, rot) end

--- 
--- @param boneId int
function Animation.Frame:GetBoneScale(boneId) end


