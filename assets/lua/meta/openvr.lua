--- @meta
--- 
--- @class openvr
openvr = {}


--- 
--- @class openvr.ControllerState
--- @field axis1 math.Vector2 
--- @field packetNum int 
--- @field axis2 math.Vector2 
--- @field axis3 math.Vector2 
--- @field buttonTouched int 
--- @field axis4 math.Vector2 
--- @field buttonPressed int 
--- @field axis0 math.Vector2 
openvr.ControllerState = {}


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


--- 
--- @class openvr.TrackedDevicePose
--- @field trackingResult vr::ETrackingResult 
--- @field deviceToAbsoluteTracking math.Mat3x4 
--- @field velocity math.Vector 
--- @field deviceIsConnected bool 
--- @field angularVelocity math.Vector 
--- @field poseIsValid bool 
openvr.TrackedDevicePose = {}


