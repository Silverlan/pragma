--- @meta
--- 
--- @class sound
sound = {}


--- 
--- @class sound.Source
sound.Source = {}

--- 
--- @param arg1 math.Vector
function sound.Source:SetPos(arg1) end

--- 
--- @param arg1 number
function sound.Source:SetFadeOutDuration(arg1) end

--- 
--- @param name string
--- @param gain number
--- @return bool ret0
--- @overload fun(name: string, gain: number, gainHF: number, gainLF: number): bool
function sound.Source:SetEffectParameters(name, gain) end

--- 
--- @overload fun(arg1: ents.BaseEntityBase): 
function sound.Source:SetSource() end

--- 
--- @param arg1 number
function sound.Source:SetMinGain(arg1) end

--- 
--- @return number ret0
function sound.Source:GetDopplerFactor() end

--- 
--- @param arg1 number
function sound.Source:SetDopplerFactor(arg1) end

--- 
--- @param arg1 number
function sound.Source:SetRightStereoAngle(arg1) end

--- 
--- @return ents.BaseEntityBase ret0
function sound.Source:GetSource() end

--- 
--- @param arg1 number
function sound.Source:SetLeftStereoAngle(arg1) end

--- 
--- @return number ret0
function sound.Source:GetLeftStereoAngle() end

--- 
--- @return number ret0
function sound.Source:GetRightStereoAngle() end

--- 
--- @param arg1 number
function sound.Source:SetAirAbsorptionFactor(arg1) end

--- 
--- @return number ret0
function sound.Source:GetAirAbsorptionFactor() end

--- 
function sound.Source:ClearRange() end

--- 
--- @param arg1 bool
--- @param arg2 bool
--- @param arg3 bool
function sound.Source:SetGainAuto(arg1, arg2, arg3) end

--- 
--- @return number ret0_1
--- @return number ret0_2
function sound.Source:GetOuterConeGains() end

--- 
--- @param arg2 unknown
--- @return util.Callback ret0
function sound.Source:CallOnStateChanged(arg2) end

--- 
--- @return bool ret0_1
--- @return bool ret0_2
--- @return bool ret0_3
function sound.Source:GetGainAuto() end

--- 
--- @return number ret0_1
--- @return number ret0_2
function sound.Source:GetRolloffFactors() end

--- 
--- @param arg1 math.Vector
--- @param arg2 math.Vector
function sound.Source:SetOrientation(arg1, arg2) end

--- 
--- @param gain number
--- @overload fun(gain: number, gainHF: number, gainLF: number): 
function sound.Source:SetDirectFilter(gain) end

--- 
--- @param arg1 string
function sound.Source:RemoveEffect(arg1) end

--- 
--- @return number ret0_1
--- @return number ret0_2
function sound.Source:GetRange() end

--- 
--- @return number ret0_1
--- @return number ret0_2
--- @return number ret0_3
function sound.Source:GetDirectFilter() end

--- 
--- @param name string
--- @param gain number
--- @return bool ret0
--- @overload fun(name: string, gain: number, gainHF: number, gainLF: number): bool
function sound.Source:AddEffect(name, gain) end

--- 
--- @param arg1 number
--- @param arg2 number
function sound.Source:SetGainRange(arg1, arg2) end

--- 
--- @param arg1 number
function sound.Source:SetInnerConeAngle(arg1) end

--- 
--- @return number ret0_1
--- @return number ret0_2
function sound.Source:GetDistanceRange() end

--- 
--- @return number ret0_1
--- @return number ret0_2
function sound.Source:GetGainRange() end

--- 
--- @return bool ret0
function sound.Source:GetSendGainAuto() end

--- 
--- @param arg1 number
--- @param arg2 number
function sound.Source:SetDistanceRange(arg1, arg2) end

--- 
--- @return bool ret0
function sound.Source:GetDirectGainHFAuto() end

--- 
--- @param arg1 number
--- @param arg2 number
function sound.Source:SetConeAngles(arg1, arg2) end

--- 
--- @param arg1 number
function sound.Source:SetMaxGain(arg1) end

--- 
--- @param arg1 number
function sound.Source:SetMaxDistance(arg1) end

--- 
--- @return math.Vector ret0
function sound.Source:GetDirection() end

--- 
--- @return number ret0_1
--- @return number ret0_2
function sound.Source:GetConeAngles() end

--- 
--- @param arg1 number
--- @param arg2 number
function sound.Source:SetStereoAngles(arg1, arg2) end

--- 
--- @param arg1 number
--- @param arg2 number
function sound.Source:SetOuterConeGains(arg1, arg2) end

--- 
--- @param arg1 bool
function sound.Source:SetLooping(arg1) end

--- 
--- @return number ret0_1
--- @return number ret0_2
function sound.Source:GetStereoAngles() end

--- 
--- @param arg1 number
function sound.Source:SetOffset(arg1) end

--- 
--- @return math.Vector ret0
function sound.Source:GetVelocity() end

--- 
--- @return bool ret0
function sound.Source:IsLooping() end

--- 
--- @param arg1 number
--- @param arg2 number
function sound.Source:SetRolloffFactors(arg1, arg2) end

--- 
--- @return math.Vector ret0_1
--- @return math.Vector ret0_2
function sound.Source:GetOrientation() end

--- 
--- @param arg1 math.Vector
function sound.Source:SetVelocity(arg1) end

--- 
--- @param arg1 math.Vector
--- @return number ret0
function sound.Source:GetIntensity(arg1) end

--- 
--- @param arg1 string
--- @param arg2 number
function sound.Source:SetEffectGain(arg1, arg2) end

--- 
--- @return bool ret0
function sound.Source:GetSendGainHFAuto() end

--- 
--- @return number ret0
function sound.Source:GetFadeOutDuration() end

--- 
--- @param arg1 number
function sound.Source:SetPitch(arg1) end

--- 
--- @param arg1 number
function sound.Source:SetGain(arg1) end

--- 
--- @param arg1 number
function sound.Source:SetRolloffFactor(arg1) end

--- 
--- @return number ret0
function sound.Source:GetOffset() end

--- 
--- @param arg1 number
function sound.Source:SetTimeOffset(arg1) end

--- 
--- @return number ret0
function sound.Source:GetDuration() end

--- 
function sound.Source:Play() end

--- 
--- @param arg1 number
function sound.Source:SetReferenceDistance(arg1) end

--- 
--- @return number ret0
function sound.Source:GetOuterConeAngle() end

--- 
function sound.Source:Stop() end

--- 
--- @return bool ret0
function sound.Source:IsPlaying() end

--- 
--- @return bool ret0
function sound.Source:IsPaused() end

--- 
--- @return number ret0
function sound.Source:GetFadeInDuration() end

--- 
function sound.Source:Pause() end

--- 
--- @return number ret0
function sound.Source:GetTimeOffset() end

--- 
--- @param arg1 number
--- @param arg2 number
function sound.Source:SetRange(arg1, arg2) end

--- 
--- @return math.Vector ret0
function sound.Source:GetPos() end

--- 
--- @return number ret0
function sound.Source:GetPitch() end

--- 
--- @return number ret0
function sound.Source:GetInnerConeAngle() end

--- 
--- @param arg1 number
function sound.Source:SetOuterConeAngle(arg1) end

--- 
--- @return number ret0
function sound.Source:GetGain() end

--- 
function sound.Source:Rewind() end

--- 
--- @return number ret0
function sound.Source:GetReferenceDistance() end

--- 
function sound.Source:__tostring() end

--- 
--- @return bool ret0
function sound.Source:IsStopped() end

--- 
--- @return enum ALState ret0
function sound.Source:GetState() end

--- 
--- @return enum ALSoundType ret0
function sound.Source:GetType() end

--- 
--- @param arg1 math.Vector
function sound.Source:SetDirection(arg1) end

--- 
--- @param arg1 bool
function sound.Source:SetRelative(arg1) end

--- 
--- @return bool ret0
function sound.Source:IsRelative() end

--- 
--- @param arg1 enum ALSoundType
function sound.Source:SetType(arg1) end

--- 
--- @return number ret0
function sound.Source:GetRolloffFactor() end

--- 
--- @return number ret0
function sound.Source:GetRoomRolloffFactor() end

--- 
--- @param arg1 number
function sound.Source:SetRoomRolloffFactor(arg1) end

--- 
--- @return number ret0
function sound.Source:GetMaxDistance() end

--- 
--- @return number ret0
function sound.Source:GetMinGain() end

--- 
--- @return number ret0
function sound.Source:GetMaxGain() end

--- 
--- @return number ret0
function sound.Source:GetOuterConeGain() end

--- 
--- @param arg1 number
function sound.Source:SetOuterConeGain(arg1) end

--- 
--- @return number ret0
function sound.Source:GetOuterConeGainHF() end

--- 
--- @param arg1 number
function sound.Source:SetOuterConeGainHF(arg1) end

--- 
--- @param arg1 enum ALSoundType
function sound.Source:AddType(arg1) end

--- 
--- @param arg1 int
function sound.Source:SetPriority(arg1) end

--- 
--- @return int ret0
function sound.Source:GetPriority() end

--- 
--- @param arg1 number
function sound.Source:FadeIn(arg1) end

--- 
--- @param arg1 number
function sound.Source:FadeOut(arg1) end

--- 
--- @return number ret0
function sound.Source:GetMaxAudibleDistance() end

--- 
--- @return bool ret0
function sound.Source:HasRange() end

--- 
--- @return number ret0_1
--- @return number ret0_2
function sound.Source:GetRangeOffsets() end

--- 
--- @param arg1 number
function sound.Source:SetFadeInDuration(arg1) end

--- 
--- @return table ret0
function sound.Source:GetPhonemeData() end


--- 
--- @class sound.Effect
sound.Effect = {}

--- 
--- @param props sound.Effect.EqualizerProperties
--- @overload fun(props: sound.Effect.CompressorProperties): 
--- @overload fun(props: sound.Effect.AutoWahProperties): 
--- @overload fun(props: sound.Effect.RingModulatorProperties): 
--- @overload fun(props: sound.Effect.PitchShifterProperties): 
--- @overload fun(props: sound.Effect.VocalMorpherProperties): 
--- @overload fun(props: sound.Effect.FrequencyShifterProperties): 
--- @overload fun(props: sound.Effect.FlangerProperties): 
--- @overload fun(props: sound.Effect.EchoProperties): 
--- @overload fun(props: sound.Effect.DistortionProperties): 
--- @overload fun(props: sound.Effect.ChorusProperties): 
--- @overload fun(props: sound.Effect.ReverbProperties): 
function sound.Effect:SetProperties(props) end


--- 
--- @class sound.Effect.EchoProperties
--- @field spread number 
--- @field feedback number 
--- @field lrDelay number 
--- @field delay number 
--- @field damping number 
--- @overload fun():sound.Effect.EchoProperties
sound.Effect.EchoProperties = {}


--- 
--- @class sound.Effect.ReverbProperties
--- @field density number 
--- @field modulationDepth number 
--- @field diffusion number 
--- @field gainHF number 
--- @field decayTime number 
--- @field gain number 
--- @field decayHFRatio number 
--- @field gainLF number 
--- @field decayLFRatio number 
--- @field reflectionsGain number 
--- @field reflectionsDelay number 
--- @field reflectionsPan math.Vector 
--- @field lateReverbDelay number 
--- @field lateReverbGain number 
--- @field lateReverbPan math.Vector 
--- @field echoTime number 
--- @field echoDepth number 
--- @field modulationTime number 
--- @field airAbsorptionGainHF number 
--- @field hfReference number 
--- @field lfReference number 
--- @field roomRolloffFactor number 
--- @field decayHFLimit int 
--- @overload fun():sound.Effect.ReverbProperties
sound.Effect.ReverbProperties = {}


--- 
--- @class sound.Effect.ChorusProperties
--- @field rate number 
--- @field feedback number 
--- @field depth number 
--- @field waveform int 
--- @field delay number 
--- @field phase int 
--- @overload fun():sound.Effect.ChorusProperties
sound.Effect.ChorusProperties = {}


--- 
--- @class sound.Effect.DistortionProperties
--- @field gain number 
--- @field eqCenter number 
--- @field edge number 
--- @field lowpassCutoff number 
--- @field eqBandwidth number 
--- @overload fun():sound.Effect.DistortionProperties
sound.Effect.DistortionProperties = {}


--- 
--- @class sound.Effect.AutoWahProperties
--- @field releaseTime number 
--- @field resonance number 
--- @field peakGain number 
--- @field attackTime number 
--- @overload fun():sound.Effect.AutoWahProperties
sound.Effect.AutoWahProperties = {}


--- 
--- @class sound.Effect.FlangerProperties
--- @field rate number 
--- @field feedback number 
--- @field depth number 
--- @field waveform int 
--- @field delay number 
--- @field phase int 
--- @overload fun():sound.Effect.FlangerProperties
sound.Effect.FlangerProperties = {}


--- 
--- @class sound.Effect.FrequencyShifterProperties
--- @field leftDirection int 
--- @field frequency number 
--- @field rightDirection int 
--- @overload fun():sound.Effect.FrequencyShifterProperties
sound.Effect.FrequencyShifterProperties = {}


--- 
--- @class sound.Effect.VocalMorpherProperties
--- @field phonemeB int 
--- @field phonemeBCoarseTuning int 
--- @field rate number 
--- @field phonemeACoarseTuning int 
--- @field phonemeA int 
--- @field waveform int 
--- @overload fun():sound.Effect.VocalMorpherProperties
sound.Effect.VocalMorpherProperties = {}


--- 
--- @class sound.Effect.EqualizerProperties
--- @field highGain number 
--- @field mid2Gain number 
--- @field highCutoff number 
--- @field mid1Width number 
--- @field lowGain number 
--- @field lowCutoff number 
--- @field mid1Gain number 
--- @field mid1Center number 
--- @field mid2Center number 
--- @field mid2Width number 
--- @overload fun():sound.Effect.EqualizerProperties
sound.Effect.EqualizerProperties = {}


--- 
--- @class sound.Effect.PitchShifterProperties
--- @field coarseTune int 
--- @field fineTune int 
--- @overload fun():sound.Effect.PitchShifterProperties
sound.Effect.PitchShifterProperties = {}


--- 
--- @class sound.Effect.CompressorProperties
--- @field onOff int 
--- @overload fun():sound.Effect.CompressorProperties
sound.Effect.CompressorProperties = {}


--- 
--- @class sound.Effect.RingModulatorProperties
--- @field highpassCutoff number 
--- @field waveform int 
--- @field frequency number 
--- @overload fun():sound.Effect.RingModulatorProperties
sound.Effect.RingModulatorProperties = {}


