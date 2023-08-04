--- @meta
--- 
--- @class openvr
openvr = {}


--- 
--- @class openvr.ControllerState
--- @field axis0 math.Vector2 
--- @field axis1 math.Vector2 
--- @field packetNum int 
--- @field axis2 math.Vector2 
--- @field axis3 math.Vector2 
--- @field buttonTouched int 
--- @field axis4 math.Vector2 
--- @field buttonPressed int 
openvr.ControllerState = {}


--- 
--- @class openvr.TrackedDevicePose
--- @field poseIsValid bool 
--- @field angularVelocity math.Vector 
--- @field trackingResult vr::ETrackingResult 
--- @field deviceToAbsoluteTracking math.Mat3x4 
--- @field velocity math.Vector 
--- @field deviceIsConnected bool 
openvr.TrackedDevicePose = {}


--- 
--- @class openvr.Eye
openvr.Eye = {}

--- 
--- @param cam ents.CameraComponent
function openvr.Eye:GetViewMatrix(cam) end

--- 
--- @param nearZ number
--- @param farZ number
function openvr.Eye:GetProjectionMatrix(nearZ, farZ) end


