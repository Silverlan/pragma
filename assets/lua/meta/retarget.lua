--- @meta
--- 
--- @class util.retarget
util.retarget = {}

--- 
--- @param retargetFlexData util.retarget.RetargetFlexData
--- @param flexCSrc ents.BaseFlexComponent
--- @param flexCDst ents.BaseFlexComponent
function util.retarget.apply_retarget_flex(retargetFlexData, flexCSrc, flexCDst) end

--- 
--- @param absBonePoses any
--- @param origBindPoseToRetargetBindPose any
--- @param origBindPoseBoneDistances any
--- @param bindPosesOther any
--- @param origBindPoses any
--- @param tmpPoses any
--- @param retargetPoses any
--- @param invRootPose any
--- @param bindPoseTransforms any
--- @param relBindPoses any
--- @param tUntranslatedBones any
--- @param tTranslationTable any
--- @return util.retarget.RetargetData ret0
function util.retarget.initialize_retarget_data(absBonePoses, origBindPoseToRetargetBindPose, origBindPoseBoneDistances, bindPosesOther, origBindPoses, tmpPoses, retargetPoses, invRootPose, bindPoseTransforms, relBindPoses, tUntranslatedBones, tTranslationTable) end

--- 
--- @param retargetData util.retarget.RetargetData
--- @param mdl game.Model
--- @param animSrc ents.BaseAnimatedComponent
--- @param animDst ents.BaseAnimatedComponent
--- @param skeleton Model.Skeleton
function util.retarget.apply_retarget_rig(retargetData, mdl, animSrc, animDst, skeleton) end

--- 
--- @param remapData any
--- @return util.retarget.RetargetFlexData ret0
function util.retarget.initialize_retarget_flex_data(remapData) end


--- 
--- @class util.retarget.RetargetData
util.retarget.RetargetData = {}


--- 
--- @class util.retarget.RetargetFlexData
util.retarget.RetargetFlexData = {}


