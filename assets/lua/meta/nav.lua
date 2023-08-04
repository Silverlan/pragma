--- @meta
--- 
--- @class nav
nav = {}

--- 
--- @return nav.Mesh ret0
function nav.load() end

--- 
function nav.generate() end


--- 
--- @class nav.Mesh
nav.Mesh = {}

--- 
--- @return nav.Config ret0
function nav.Mesh:GetConfig() end

--- 
--- @param fname string
function nav.Mesh:Save(fname) end

--- 
--- @param start math.Vector
--- @param end math.Vector
--- @overload fun(start: vector.Vector, end_: vector.Vector): 
function nav.Mesh:RayCast(start, end_) end


--- 
--- @class nav.Config
--- @field sampleDetailDist number 
--- @field sampleDetailMaxError number 
--- @field walkableRadius number 
--- @field walkableSlopeAngle number 
--- @field samplePartitionType int 
--- @field characterHeight number 
--- @field vertsPerPoly number 
--- @field maxClimbHeight number 
--- @field maxEdgeLength number 
--- @field cellSize number 
--- @field maxSimplificationError number 
--- @field minRegionSize int 
--- @field mergeRegionSize int 
--- @field cellHeight number 
--- @overload fun():nav.Config
--- @overload fun(arg1: number, arg2: number, arg3: number, arg4: number):nav.Config
nav.Config = {}


--- @enum PartitionType
nav.Config = {
	PARTITION_TYPE_LAYERS = 2,
	PARTITION_TYPE_MONOTONE = 1,
	PARTITION_TYPE_WATERSHED = 0,
}

