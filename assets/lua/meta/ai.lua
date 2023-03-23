--- @meta
--- 
--- @class ai
ai = {}

--- 
--- @return ai.Schedule ret0
function ai.create_schedule() end

--- 
--- @param name string
--- @return ai.Faction ret0
function ai.register_faction(name) end

--- 
--- @param name string
--- @return any ret0
function ai.find_faction_by_name(name) end

--- 
--- @return any ret0
function ai.get_factions() end

--- 
--- @param taskClass any
--- @return int ret0
--- @overload fun(taskClass: any, taskType: enum pragma::ai::BehaviorNode::Type): int
--- @overload fun(taskClass: any, taskType: enum pragma::ai::BehaviorNode::Type, selectorType: enum pragma::ai::SelectorType): int
function ai.register_task(taskClass) end


--- 
--- @class ai.TaskRandom: ai.BehaviorTask
ai.TaskRandom = {}

--- 
--- @param chance number
function ai.TaskRandom:SetChance(chance) end


--- @enum ParameterChance
ai.TaskRandom = {
	PARAMETER_CHANCE = 0,
}

--- 
--- @class ai.Faction
ai.Faction = {}

--- 
--- @param ent ents.BaseEntityBase
--- @overload fun(className: string): 
--- @overload fun(factionTgt: ai.Faction): 
function ai.Faction:GetDisposition(ent) end

--- 
--- @param factionTgt ai.Faction
--- @overload fun(factionTgt: ai.Faction, revert: bool): 
--- @overload fun(factionTgt: ai.Faction, revert: bool, priority: int): 
function ai.Faction:SetNeutralFaction(factionTgt) end

--- 
--- @param arg1 string
function ai.Faction:AddClass(arg1) end

--- 
--- @param factionTgt ai.Faction
--- @overload fun(factionTgt: ai.Faction, revert: bool): 
--- @overload fun(factionTgt: ai.Faction, revert: bool, priority: int): 
function ai.Faction:SetAlliedFaction(factionTgt) end

--- 
--- @return table ret0
function ai.Faction:GetClasses() end

--- 
--- @param factionTgt ai.Faction
--- @param disposition int
--- @overload fun(factionTgt: ai.Faction, disposition: int, revert: bool): 
--- @overload fun(factionTgt: ai.Faction, disposition: int, revert: bool, priority: int): 
function ai.Faction:SetDisposition(factionTgt, disposition) end

--- 
--- @param factionTgt ai.Faction
--- @overload fun(factionTgt: ai.Faction, revert: bool): 
--- @overload fun(factionTgt: ai.Faction, revert: bool, priority: int): 
function ai.Faction:SetEnemyFaction(factionTgt) end

--- 
--- @param factionTgt ai.Faction
--- @overload fun(factionTgt: ai.Faction, revert: bool): 
--- @overload fun(factionTgt: ai.Faction, revert: bool, priority: int): 
function ai.Faction:SetFearsomeFaction(factionTgt) end

--- 
--- @param arg1 string
--- @return bool ret0
function ai.Faction:HasClass(arg1) end

--- 
--- @param arg1 enum DISPOSITION
function ai.Faction:SetDefaultDisposition(arg1) end

--- 
--- @return enum DISPOSITION ret0
function ai.Faction:GetDefaultDisposition() end

--- 
--- @return string ret0
function ai.Faction:GetName() end


--- 
--- @class ai.Squad
ai.Squad = {}

--- 
--- @return table ret0
function ai.Squad:GetMembers() end


--- 
--- @class ai.TaskDecorator: ai.BehaviorTask
ai.TaskDecorator = {}

--- 
--- @param limit int
function ai.TaskDecorator:SetLimit(limit) end

--- 
--- @param decoratorType int
function ai.TaskDecorator:SetDecoratorType(decoratorType) end


--- @enum Parameter
ai.TaskDecorator = {
	PARAMETER_DECORATOR_TYPE = 0,
	PARAMETER_LIMIT = 1,
}

--- 
--- @class ai.MoveInfo
--- @field destinationTolerance number 
--- @field moveOnPath bool 
--- @field activity int 
--- @field faceTarget vector.Vector 
--- @field turnSpeed number 
--- @field moveSpeed number 
--- @overload fun():ai.MoveInfo
ai.MoveInfo = {}


--- 
--- @class ai.TaskPlayLayeredAnimation: ai.BehaviorTask
ai.TaskPlayLayeredAnimation = {}

--- 
--- @param animation string
--- @overload fun(animation: int): 
function ai.TaskPlayLayeredAnimation:SetAnimation(animation) end

--- 
--- @param animationSlot int
function ai.TaskPlayLayeredAnimation:SetAnimationSlot(animationSlot) end


--- @enum Parameter
ai.TaskPlayLayeredAnimation = {
	PARAMETER_ANIMATION = 0,
	PARAMETER_ANIMATION_SLOT = 2,
}

--- 
--- @class ai.Schedule
ai.Schedule = {}

--- 
--- @param node class pragma::ai::BehaviorNode
function ai.Schedule:AddTask(node) end

--- 
function ai.Schedule:DebugPrint() end

--- 
function ai.Schedule:Cancel() end

--- 
function ai.Schedule:GetInterruptFlags() end

--- 
--- @param paramIdx int
--- @overload fun(paramIdx: int, def: math.Quaternion): 
function ai.Schedule:GetParameterQuaternion(paramIdx) end

--- 
--- @param flags int
function ai.Schedule:AddInterruptFlags(flags) end

--- 
--- @param paramIdx int
--- @overload fun(paramIdx: int, def: number): 
function ai.Schedule:GetParameterFloat(paramIdx) end

--- 
--- @param parameterId int
function ai.Schedule:HasParameter(parameterId) end

--- 
function ai.Schedule:GetRootTask() end

--- 
--- @param idx int
--- @param b bool
function ai.Schedule:SetParameterBool(idx, b) end

--- 
--- @param paramIdx int
function ai.Schedule:GetParameterType(paramIdx) end

--- 
--- @param idx int
--- @param i int
function ai.Schedule:SetParameterInt(idx, i) end

--- 
--- @param idx int
--- @param f number
function ai.Schedule:SetParameterFloat(idx, f) end

--- 
--- @param idx int
--- @param s string
function ai.Schedule:SetParameterString(idx, s) end

--- 
--- @param idx int
--- @param ent ents.BaseEntityBase
function ai.Schedule:SetParameterEntity(idx, ent) end

--- 
--- @param idx int
--- @param v math.Vector
function ai.Schedule:SetParameterVector(idx, v) end

--- 
--- @param idx int
--- @param rot math.Quaternion
function ai.Schedule:SetParameterQuaternion(idx, rot) end

--- 
--- @param idx int
--- @param ang math.EulerAngles
function ai.Schedule:SetParameterEulerAngles(idx, ang) end

--- 
--- @param paramIdx int
--- @overload fun(paramIdx: int, def: bool): 
function ai.Schedule:GetParameterBool(paramIdx) end

--- 
--- @param paramIdx int
--- @overload fun(paramIdx: int, def: int): 
function ai.Schedule:GetParameterInt(paramIdx) end

--- 
function ai.Schedule:Copy() end

--- 
--- @param paramIdx int
--- @overload fun(paramIdx: int, def: string): 
function ai.Schedule:GetParameterString(paramIdx) end

--- 
--- @param paramIdx int
--- @overload fun(paramIdx: int, def: class util::TWeakSharedHandle<class BaseEntity>): 
function ai.Schedule:GetParameterEntity(paramIdx) end

--- 
--- @param paramIdx int
--- @overload fun(paramIdx: int, def: math.Vector): 
function ai.Schedule:GetParameterVector(paramIdx) end

--- 
--- @param paramIdx int
--- @overload fun(paramIdx: int, def: math.EulerAngles): 
function ai.Schedule:GetParameterEulerAngles(paramIdx) end

--- 
--- @param flags int
function ai.Schedule:SetInterruptFlags(flags) end


--- 
--- @class ai.TaskEvent: ai.BehaviorTask
ai.TaskEvent = {}

--- 
--- @param eventId int
function ai.TaskEvent:SetEventId(eventId) end

--- 
--- @param argIdx int
--- @param arg string
function ai.TaskEvent:SetEventArgument(argIdx, arg) end


--- @enum ParameterEvent
ai.TaskEvent = {
	PARAMETER_EVENT_ARGS_START = 1,
	PARAMETER_EVENT_ID = 0,
}

--- 
--- @class ai.TaskDebugDrawText: ai.BehaviorTask
ai.TaskDebugDrawText = {}

--- 
--- @param msg string
function ai.TaskDebugDrawText:SetMessage(msg) end


--- @enum ParameterMessage
ai.TaskDebugDrawText = {
	PARAMETER_MESSAGE = 0,
}

--- 
--- @class ai.TaskDebugPrint: ai.BehaviorTask
ai.TaskDebugPrint = {}

--- 
--- @param msg string
function ai.TaskDebugPrint:SetMessage(msg) end


--- @enum ParameterMessage
ai.TaskDebugPrint = {
	PARAMETER_MESSAGE = 0,
}

--- 
--- @class ai.TaskWait: ai.BehaviorTask
ai.TaskWait = {}

--- 
--- @param t number
function ai.TaskWait:SetMinWaitTime(t) end

--- 
--- @param t number
function ai.TaskWait:SetMaxWaitTime(t) end

--- 
--- @param t number
--- @overload fun(tMin: number, tMax: number): 
function ai.TaskWait:SetWaitTime(t) end


--- @enum Parameter
ai.TaskWait = {
	PARAMETER_MAX_WAIT_TIME = 1,
	PARAMETER_MIN_WAIT_TIME = 0,
}

--- 
--- @class ai.BaseBehaviorTask: ai.BehaviorTask
--- @overload fun():ai.BaseBehaviorTask
--- @overload fun(arg1: int):ai.BaseBehaviorTask
--- @overload fun(arg1: int, arg2: int):ai.BaseBehaviorTask
ai.BaseBehaviorTask = {}

--- 
--- @param arg1 ai.Schedule
--- @param arg2 ents.AIComponent
--- @param arg3 int
--- @return int ret0
function ai.BaseBehaviorTask:Think(arg1, arg2, arg3) end

--- 
--- @param arg1 ai.Schedule
--- @param arg2 int
--- @param arg3 int
function ai.BaseBehaviorTask:OnTaskComplete(arg1, arg2, arg3) end

--- 
--- @param arg1 int
--- @param arg2 int
function ai.BaseBehaviorTask:OnSetScheduleParameter(arg1, arg2) end

--- 
--- @param arg1 ai.Schedule
--- @param arg2 ents.AIComponent
--- @return int ret0
function ai.BaseBehaviorTask:Start(arg1, arg2) end

--- 
function ai.BaseBehaviorTask:Stop() end


--- 
--- @class ai.TaskMoveRandom: ai.BehaviorTask
ai.TaskMoveRandom = {}

--- 
--- @param dist number
function ai.TaskMoveRandom:SetMoveDistance(dist) end

--- 
--- @param activity int
function ai.TaskMoveRandom:SetMoveActivity(activity) end


--- @enum Parameter
ai.TaskMoveRandom = {
	PARAMETER_DISTANCE = 0,
	PARAMETER_MOVE_ACTIVITY = 1,
}

--- 
--- @class ai.Memory
ai.Memory = {}

--- 
--- @return table ret0
function ai.Memory:GetFragments() end

--- 
--- @return int ret0
function ai.Memory:GetFragmentCount() end


--- @enum Type
ai.Memory = {
	TYPE_SMELL = 2,
	TYPE_SOUND = 1,
	TYPE_VISUAL = 0,
}

--- 
--- @class ai.TaskLookAtTarget: ai.BehaviorTask
ai.TaskLookAtTarget = {}

--- 
--- @param duration number
function ai.TaskLookAtTarget:SetLookDuration(duration) end


--- @enum ParameterLookDuration
ai.TaskLookAtTarget = {
	PARAMETER_LOOK_DURATION = 1,
}

--- 
--- @class ai.TaskPlayAnimation: ai.BehaviorTask
ai.TaskPlayAnimation = {}

--- 
function ai.TaskPlayAnimation:SetFacePrimaryTarget() end

--- 
--- @param target ents.BaseEntityBase
--- @overload fun(target: math.Vector): 
function ai.TaskPlayAnimation:SetFaceTarget(target) end

--- 
--- @param animation string
--- @overload fun(animation: int): 
function ai.TaskPlayAnimation:SetAnimation(animation) end


--- @enum Parameter
ai.TaskPlayAnimation = {
	PARAMETER_ANIMATION = 0,
	PARAMETER_FACE_TARGET = 1,
}

--- 
--- @class ai.MemoryFragment
ai.MemoryFragment = {}

--- 
--- @return number ret0
function ai.MemoryFragment:GetLastTimeSensed() end

--- 
--- @return number ret0
function ai.MemoryFragment:GetLastCheckTime() end

--- 
--- @return ents.BaseEntityBase ret0
function ai.MemoryFragment:GetEntity() end

--- 
--- @return bool ret0
function ai.MemoryFragment:IsInView() end

--- 
--- @return math.Vector ret0
function ai.MemoryFragment:GetLastKnownPosition() end

--- 
--- @return math.Vector ret0
function ai.MemoryFragment:GetLastKnownVelocity() end

--- 
--- @return number ret0
function ai.MemoryFragment:GetLastTimeSeen() end

--- 
--- @return number ret0
function ai.MemoryFragment:GetLastTimeHeared() end


--- 
--- @class ai.TaskPlaySound: ai.BehaviorTask
ai.TaskPlaySound = {}

--- 
--- @param sndName string
function ai.TaskPlaySound:SetSoundName(sndName) end

--- 
--- @param gain number
function ai.TaskPlaySound:SetGain(gain) end

--- 
--- @param pitch number
function ai.TaskPlaySound:SetPitch(pitch) end


--- @enum Parameter
ai.TaskPlaySound = {
	PARAMETER_GAIN = 1,
	PARAMETER_PITCH = 2,
	PARAMETER_SOUND_NAME = 0,
}

--- 
--- @class ai.TaskPlayActivity: ai.BehaviorTask
ai.TaskPlayActivity = {}

--- 
function ai.TaskPlayActivity:SetFacePrimaryTarget() end

--- 
--- @param target ents.BaseEntityBase
--- @overload fun(target: math.Vector): 
function ai.TaskPlayActivity:SetFaceTarget(target) end

--- 
--- @param activity int
function ai.TaskPlayActivity:SetActivity(activity) end


--- @enum Parameter
ai.TaskPlayActivity = {
	PARAMETER_FACE_TARGET = 1,
	PARAMETER_ACTIVITY = 0,
}

--- 
--- @class ai.TaskPlayLayeredActivity: ai.BehaviorTask
ai.TaskPlayLayeredActivity = {}

--- 
--- @param activity int
function ai.TaskPlayLayeredActivity:SetActivity(activity) end

--- 
--- @param animationSlot int
function ai.TaskPlayLayeredActivity:SetAnimationSlot(animationSlot) end


--- @enum Parameter
ai.TaskPlayLayeredActivity = {
	PARAMETER_ACTIVITY = 0,
	PARAMETER_ANIMATION_SLOT = 2,
}

--- 
--- @class ai.TaskMoveToTarget: ai.BehaviorTask
ai.TaskMoveToTarget = {}

--- 
--- @param ent ents.BaseEntityBase
--- @overload fun(target: math.Vector): 
function ai.TaskMoveToTarget:SetMoveTarget(ent) end

--- 
--- @param dist number
function ai.TaskMoveToTarget:SetMoveDistance(dist) end

--- 
--- @param activity int
function ai.TaskMoveToTarget:SetMoveActivity(activity) end


--- @enum Parameter
ai.TaskMoveToTarget = {
	PARAMETER_DISTANCE = 1,
	PARAMETER_TARGET = 0,
	PARAMETER_MOVE_ACTIVITY = 2,
}

--- 
--- @class ai.BehaviorTask
ai.BehaviorTask = {}

--- 
--- @param sched ai.Schedule
--- @param paramIdx int
--- @overload fun(sched: ai.Schedule, paramIdx: int, def: math.EulerAngles): 
function ai.BehaviorTask:GetParameterEulerAngles(sched, paramIdx) end

--- 
function ai.BehaviorTask:IsActive() end

--- 
function ai.BehaviorTask:GetTasks() end

--- 
function ai.BehaviorTask:GetSelectorType() end

--- 
--- @param sched ai.Schedule
--- @param paramIdx int
--- @overload fun(sched: ai.Schedule, paramIdx: int, def: math.Quaternion): 
function ai.BehaviorTask:GetParameterQuaternion(sched, paramIdx) end

--- 
--- @param nodeId int
function ai.BehaviorTask:GetTask(nodeId) end

--- 
--- @param sched ai.Schedule
--- @param paramIdx int
--- @overload fun(sched: ai.Schedule, paramIdx: int, def: ents.BaseEntityBase): 
function ai.BehaviorTask:GetParameterEntity(sched, paramIdx) end

--- 
function ai.BehaviorTask:GetDebugName() end

--- 
--- @param taskParamId int
--- @param scheduleParamId int
function ai.BehaviorTask:SetScheduleParameter(taskParamId, scheduleParamId) end

--- 
--- @param paramIdx int
--- @param taskOther ai.BehaviorTask
--- @overload fun(paramIdx: int, taskOther: ai.BehaviorTask, paramIdxOther: int): 
function ai.BehaviorTask:LinkParameter(paramIdx, taskOther) end

--- 
--- @param name string
function ai.BehaviorTask:SetDebugName(name) end

--- 
--- @param o any
--- @overload fun(o: any, taskType: int): 
--- @overload fun(o: any, taskType: int, selectorType: int): 
function ai.BehaviorTask:CreateTask(o) end

--- 
--- @param decoratorType int
--- @param o any
--- @overload fun(decoratorType: int, o: any, taskType: int): 
--- @overload fun(decoratorType: int, o: any, taskType: int, selectorType: int): 
function ai.BehaviorTask:CreateDecoratedTask(decoratorType, o) end

--- 
--- @param idx int
--- @param s string
function ai.BehaviorTask:SetParameterString(idx, s) end

--- 
--- @param idx int
--- @param v math.Vector
function ai.BehaviorTask:SetParameterVector(idx, v) end

--- 
--- @param sched ai.Schedule
--- @param paramIdx int
function ai.BehaviorTask:HasParameter(sched, paramIdx) end

--- 
--- @param idx int
--- @param b bool
function ai.BehaviorTask:SetParameterBool(idx, b) end

--- 
--- @param sched ai.Schedule
--- @param paramIdx int
function ai.BehaviorTask:GetParameterType(sched, paramIdx) end

--- 
--- @param idx int
--- @param i int
function ai.BehaviorTask:SetParameterInt(idx, i) end

--- 
--- @param idx int
--- @param f number
function ai.BehaviorTask:SetParameterFloat(idx, f) end

--- 
--- @param idx int
--- @param ent ents.BaseEntityBase
function ai.BehaviorTask:SetParameterEntity(idx, ent) end

--- 
function ai.BehaviorTask:__tostring() end

--- 
--- @param idx int
--- @param rot math.Quaternion
function ai.BehaviorTask:SetParameterQuaternion(idx, rot) end

--- 
--- @param idx int
--- @param ang math.EulerAngles
function ai.BehaviorTask:SetParameterEulerAngles(idx, ang) end

--- 
--- @param sched ai.Schedule
--- @param paramIdx int
--- @overload fun(sched: ai.Schedule, paramIdx: int, def: bool): 
function ai.BehaviorTask:GetParameterBool(sched, paramIdx) end

--- 
--- @param sched ai.Schedule
--- @param paramIdx int
--- @overload fun(sched: ai.Schedule, paramIdx: int, def: int): 
function ai.BehaviorTask:GetParameterInt(sched, paramIdx) end

--- 
--- @param sched ai.Schedule
--- @param paramIdx int
--- @overload fun(sched: ai.Schedule, paramIdx: int, def: number): 
function ai.BehaviorTask:GetParameterFloat(sched, paramIdx) end

--- 
--- @param sched ai.Schedule
--- @param paramIdx int
--- @overload fun(sched: ai.Schedule, paramIdx: int, def: string): 
function ai.BehaviorTask:GetParameterString(sched, paramIdx) end

--- 
--- @param sched ai.Schedule
--- @param paramIdx int
--- @overload fun(sched: ai.Schedule, paramIdx: int, def: math.Vector): 
function ai.BehaviorTask:GetParameterVector(sched, paramIdx) end

--- 
function ai.BehaviorTask:GetType() end


--- @enum Result
ai.BehaviorTask = {
	RESULT_PENDING = 0,
	RESULT_FAILED = 1,
	RESULT_INITIAL = -1,
	RESULT_SUCCEEDED = 2,
}

--- @enum SelectorType
ai.BehaviorTask = {
	SELECTOR_TYPE_RANDOM_SHUFFLE = 1,
	SELECTOR_TYPE_SEQUENTIAL = 0,
}

--- @enum DecoratorTypeAlways
ai.BehaviorTask = {
	DECORATOR_TYPE_ALWAYS_FAIL = 1,
	DECORATOR_TYPE_ALWAYS_SUCCEED = 2,
}

--- @enum DecoratorType
ai.BehaviorTask = {
	DECORATOR_TYPE_LIMIT = 4,
	DECORATOR_TYPE_INHERIT = 0,
	DECORATOR_TYPE_INVERT = 3,
	DECORATOR_TYPE_REPEAT = 5,
}

--- @enum DecoratorTypeUntil
ai.BehaviorTask = {
	DECORATOR_TYPE_UNTIL_FAIL = 6,
	DECORATOR_TYPE_UNTIL_SUCCESS = 7,
}

--- @enum Type
ai.BehaviorTask = {
	TYPE_SELECTOR = 0,
	TYPE_SEQUENCE = 1,
}

--- @enum ParameterType
ai.BehaviorTask = {
	PARAMETER_TYPE_BOOL = 1,
	PARAMETER_TYPE_ENTITY = 8,
	PARAMETER_TYPE_EULER_ANGLES = 7,
	PARAMETER_TYPE_FLOAT = 3,
	PARAMETER_TYPE_NONE = 0,
	PARAMETER_TYPE_STRING = 4,
	PARAMETER_TYPE_INT = 2,
	PARAMETER_TYPE_QUATERNION = 6,
	PARAMETER_TYPE_VECTOR = 5,
}

