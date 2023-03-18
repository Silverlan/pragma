--- @meta
--- 
--- @class light
light = {}

--- 
--- @param type enum ulighting::NaturalLightType
--- @param outMin int
--- @param outMax int
function light.get_color_temperature(type, outMin, outMax) end

--- 
--- @param type enum ulighting::NaturalLightType
--- @return int ret0
function light.get_average_color_temperature(type) end

--- 
--- @param temperature int
--- @return math.Vector ret0
function light.color_temperature_to_color(temperature) end

--- 
--- @param wavelength int
--- @return math.Vector ret0
function light.wavelength_to_color(wavelength) end

--- 
--- @param lux number
--- @return number ret0
function light.lux_to_irradiance(lux) end

--- 
--- @param type enum ulighting::LightSourceType
--- @return number ret0
function light.get_luminous_efficacy(type) end

--- 
--- @param watt number
--- @return number ret0
--- @overload fun(watt: number, EFFICACY: number): number
function light.watts_to_lumens(watt) end

--- 
--- @param irradiance number
--- @return number ret0
function light.irradiance_to_lux(irradiance) end

--- 
--- @param lumen number
--- @return number ret0
--- @overload fun(lumen: number, EFFICACY: number): number
function light.lumens_to_watts(lumen) end

--- 
--- @param intensity number
--- @param srcType enum pragma::BaseEnvLightComponent::LightIntensityType
--- @param dstType enum pragma::BaseEnvLightComponent::LightIntensityType
--- @return number ret0
--- @overload fun(intensity: number, srcType: enum pragma::BaseEnvLightComponent::LightIntensityType, dstType: enum pragma::BaseEnvLightComponent::LightIntensityType, coneAngle: number): number
function light.convert_light_intensity(intensity, srcType, dstType) end


