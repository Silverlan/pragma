--- @meta
--- 
--- @class nav
nav = {}

--- 
function nav.generate() end

--- 
--- @return nav.Mesh ret0
function nav.load() end


--- 
--- @class nav.Config
--- @field maxSimplificationError number 
--- @field cellHeight number 
--- @field sampleDetailDist number 
--- @field minRegionSize int 
--- @field mergeRegionSize int 
--- @field sampleDetailMaxError number 
--- @field cellSize number 
--- @field maxEdgeLength number 
--- @field vertsPerPoly number 
--- @field samplePartitionType int 
--- @field walkableRadius number 
--- @field characterHeight number 
--- @field maxClimbHeight number 
--- @field walkableSlopeAngle number 
--- @overload fun():nav.Config
--- @overload fun(arg1: number, arg2: number, arg3: number, arg4: number):nav.Config
nav.Config = {}


--- @enum PartitionType
nav.Config = {
	PARTITION_TYPE_LAYERS = 2,
	PARTITION_TYPE_MONOTONE = 1,
	PARTITION_TYPE_WATERSHED = 0,
}

--- 
--- @class nav.Mesh
nav.Mesh = {}

--- 
--- @param fname string
function nav.Mesh:Save(fname) end

--- 
--- @param start math.Vector
--- @param end math.Vector
function nav.Mesh:RayCast(start, end_) end

--- 
--- @return nav.Config ret0
function nav.Mesh:GetConfig() end


