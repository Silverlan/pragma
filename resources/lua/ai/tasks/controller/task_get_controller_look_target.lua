util.register_class("ai.TaskGetControllerLookTarget",ai.BaseBehaviorTask)
ai.TaskGetControllerLookTarget.RETURN_LOOK_TARGET = 0
function ai.TaskGetControllerLookTarget:__tostring() return "Get Controller Look Target" end
function ai.TaskGetControllerLookTarget:__init(taskType,selectorType)
	ai.BaseBehaviorTask.__init(self,taskType,selectorType)
end

function ai.TaskGetControllerLookTarget:Start(schedule,npc)
	local controller = npc:GetController()
	local charComponentCtrl = (controller ~= nil) and controller:GetComponent(ents.COMPONENT_CHARACTER) or nil
	local trComponent = npc:GetEntity():GetComponent(ents.COMPONENT_TRANSFORM)
	if(charComponentCtrl == nil or trComponent == nil) then return ai.BehaviorTask.RESULT_FAILED end
	self:SetParameterVector(self.RETURN_LOOK_TARGET,trComponent:GetEyePos() +charComponentCtrl:GetViewForward() *8192)
	return ai.BehaviorTask.RESULT_SUCCEEDED
end
ai.TASK_GET_CONTROLLER_LOOK_TARGET = ai.TASK_GET_CONTROLLER_LOOK_TARGET or ai.register_task(ai.TaskGetControllerLookTarget)
