util.register_class("ai.TaskFunction",ai.BaseBehaviorTask)
function ai.TaskFunction:__tostring() return "Function" end
function ai.TaskFunction:__init(taskType,selectorType)
	ai.BaseBehaviorTask.__init(self,taskType,selectorType)
end

function ai.TaskFunction:SetStart(f) self.m_fcStart = f end
function ai.TaskFunction:SetThink(f) self.m_fcThink = f end
function ai.TaskFunction:SetStop(f) self.m_fcStop = f end

function ai.TaskFunction:Start(schedule,npc)
	if(self.m_fcStart ~= nil) then
		local r = self.m_fcStart(schedule,npc)
		if(r == ai.BehaviorTask.RESULT_PENDING and self.m_fcThink == nil) then
			console.print_warning("Pending TaskFunction without a Think-function!")
			return ai.BehaviorTask.RESULT_SUCCEEDED
		end
		return r
	end
	if(self.m_fcThink == nil) then return ai.BehaviorTask.RESULT_SUCCEEDED end
	return ai.BehaviorTask.RESULT_PENDING
end

function ai.TaskFunction:Stop()
	if(self.m_fcStop ~= nil) then self.m_fcStop() end
end

function ai.TaskFunction:Think(schedule,ent)
	if(self.m_fcThink == nil) then schedule:DebugPrint() end
	return self.m_fcThink(schedule,ent)
end
ai.TASK_FUNCTION = ai.TASK_FUNCTION or ai.register_task(ai.TaskFunction)
