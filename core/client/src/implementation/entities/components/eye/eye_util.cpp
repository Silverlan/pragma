// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.eye;
import :game;

static auto g_debugPrint = false;
void pragma::CEyeComponent::UpdateEyeballsMT()
{
	auto mdlC = GetEntity().GetModelComponent();
	if(!mdlC)
		return;
	auto &mdl = mdlC->GetModel();
	if(mdl == nullptr)
		return;
	auto &eyeballs = mdl->GetEyeballs();
	for(auto eyeballIndex = decltype(eyeballs.size()) {0u}; eyeballIndex < eyeballs.size(); ++eyeballIndex)
		UpdateEyeballMT(eyeballs.at(eyeballIndex), eyeballIndex);
	// InvokeEventCallbacks(EVENT_ON_EYEBALLS_UPDATED);
}

const pragma::CEyeComponent::EyeballConfig *pragma::CEyeComponent::GetEyeballConfig(uint32_t eyeballIndex) const { return const_cast<CEyeComponent *>(this)->GetEyeballConfig(eyeballIndex); }
pragma::CEyeComponent::EyeballConfig *pragma::CEyeComponent::GetEyeballConfig(uint32_t eyeballIndex)
{
	auto *data = GetEyeballData(eyeballIndex);
	return data ? &data->config : nullptr;
}
void pragma::CEyeComponent::SetEyeballConfig(const EyeballConfig &eyeballConfig) { m_eyeballConfig = eyeballConfig; }
const pragma::CEyeComponent::EyeballData *pragma::CEyeComponent::GetEyeballData(uint32_t eyeballIndex) const { return const_cast<CEyeComponent *>(this)->GetEyeballData(eyeballIndex); }
pragma::CEyeComponent::EyeballData *pragma::CEyeComponent::GetEyeballData(uint32_t eyeballIndex) { return (eyeballIndex < m_eyeballData.size()) ? &m_eyeballData.at(eyeballIndex) : nullptr; }

std::optional<pragma::math::Transform> pragma::CEyeComponent::GetEyePose() const
{
	auto &ent = GetEntity();
	if(m_eyeAttachmentIndex == std::numeric_limits<uint32_t>::max())
		return {};
	auto mdlC = ent.GetModelComponent();
	if(!mdlC)
		return {};
	Vector3 attPos {};
	auto attRot = uquat::identity();
	if(mdlC->GetAttachment(m_eyeAttachmentIndex, &attPos, &attRot) == false)
		return {};
	// attRot = uquat::identity();
	auto attPose = ent.GetPose();
	attPose *= math::Transform {attPos, attRot};
	return attPose;
}

void pragma::CEyeComponent::SetLocalViewTargetFactor(float f) { m_localViewTargetFactor = f; }
float pragma::CEyeComponent::GetLocalViewTargetFactor() const { return m_localViewTargetFactor; }

Vector3 pragma::CEyeComponent::GetViewTarget() const
{
	auto pose = GetEntity().GetPose();
	pose.TranslateLocal(m_viewTarget);
	return ClampViewTarget(pose.GetOrigin());
}

void pragma::CEyeComponent::ClearViewTarget() { m_viewTarget = {}; }

Vector3 pragma::CEyeComponent::ClampViewTarget(const Vector3 &viewTarget) const
{
	auto flexC = GetEntity().GetComponent<CFlexComponent>();
	auto mdlC = GetEntity().GetComponent<CModelComponent>();
	auto mdl = GetEntity().GetModel();
	if(flexC.expired() || mdlC.expired() || mdl == nullptr)
		return Vector3 {};
	// Test
	/*auto t = std::fmodf(pragma::get_cgame()->RealTime(),6.0) /6.0;
	auto t2 = std::fmodf(pragma::get_cgame()->RealTime(),3.0) /3.0;
	auto s = pragma::math::sin(t *pragma::math::pi *2);
	auto s2 = pragma::math::sin(t2 *pragma::math::pi *2);
	auto f = 1.0;
	flexC->SetFlexController("eyes_updown",f *(s *90 -45));
	flexC->SetFlexController("eyes_rightleft",f *(s2 *90 -45));*/

	auto tmp = viewTarget;
	if(m_eyeAttachmentIndex != std::numeric_limits<uint32_t>::max()) {
		Vector3 pos {};
		auto rot = uquat::identity();
		mdlC->GetAttachment(m_eyeAttachmentIndex, &pos, &rot);

		auto attPose = GetEntity().GetPose();
		attPose *= math::Transform {pos, rot};
		auto invPose = attPose.GetInverse();

		auto posAbs = invPose * tmp;
		constexpr Vector3 posLocal {0.f, 0.f, 500.f}; // Distance is arbitrarily chosen, just has to point forward

		auto localPos = uvec::lerp(posAbs, posLocal, m_localViewTargetFactor);

		if(localPos.z < 6)
			localPos.z = 6;

		auto flDist = uvec::length(localPos);
		uvec::normalize(&localPos);

		// Eye deflection
		Vector3 eyeDeflect {};

		auto eyeAng = EulerAngles {};
		if(m_eyeUpDownFlexController != std::numeric_limits<uint32_t>::max())
			flexC->GetFlexController(m_eyeUpDownFlexController, eyeAng.p);
		if(m_eyeLeftRightFlexController != std::numeric_limits<uint32_t>::max())
			flexC->GetFlexController(m_eyeLeftRightFlexController, eyeAng.y);

		math::negate(eyeAng.p);
		math::negate(eyeAng.y);
		eyeDeflect = eyeAng.Forward();
		eyeDeflect.z = 0;

		eyeDeflect = eyeDeflect * (localPos.z * localPos.z);

		localPos = localPos + eyeDeflect;
		uvec::normalize(&localPos);

		auto maxEyeDeflection = math::cos(math::deg_to_rad(mdl->GetMaxEyeDeflection()));
		static auto testDeflection = true;
		if(testDeflection) {
			if(localPos.z < maxEyeDeflection) {
				// TODO: Unsure if this is correct, further testing required
				localPos.z = 0;
				auto d = uvec::length_sqr(localPos);
				if(d > 0.0) {
					d = math::sqrt((1.0 - maxEyeDeflection * maxEyeDeflection) / (localPos.y * localPos.y + localPos.x * localPos.x));
					localPos.z = maxEyeDeflection;
					localPos.y = localPos.y * d;
					localPos.x = localPos.x * d;
				}
				else
					localPos.z = 1.0;
			}
		}
		localPos = localPos * flDist;
		localPos = attPose * localPos;
		tmp = localPos;
	}
	return tmp;
}
void pragma::CEyeComponent::SetViewTarget(const Vector3 &viewTarget)
{
	m_viewTarget = viewTarget;
	//static auto clamp = true;
	//if(clamp)
	//	m_viewTarget = ClampViewTarget(viewTarget);
}
pragma::math::Transform pragma::CEyeComponent::CalcEyeballPose(uint32_t eyeballIndex, math::Transform *optOutBonePose) const
{
	if(m_animC.expired())
		return {};
	auto *eyeballData = GetEyeballData(eyeballIndex);
	auto mdl = GetEntity().GetModel();
	auto *eyeball = mdl ? mdl->GetEyeball(eyeballIndex) : nullptr;
	if(eyeballData == nullptr || eyeball == nullptr) {
		if(optOutBonePose)
			*optOutBonePose = {};
		return math::Transform {};
	}
	auto &config = eyeballData->config;
	auto tmp = eyeball->origin;
	tmp.x = tmp.x + config.eyeShift.x * math::sign(tmp.x);
	tmp.y = tmp.y + config.eyeShift.y * math::sign(tmp.y);
	tmp.z = tmp.z + config.eyeShift.z * math::sign(tmp.z);
	tmp *= GetEntity().GetScale();

	Vector3 bonePos;
	Quat boneRot;
	m_animC->GetBonePose(eyeball->boneIndex, &bonePos, &boneRot, nullptr, math::CoordinateSpace::World);
	math::Transform bonePose {bonePos, boneRot};
	if(optOutBonePose)
		*optOutBonePose = bonePose;
	return bonePose * math::Transform {tmp, uquat::identity()};
}
bool pragma::CEyeComponent::GetEyeballProjectionVectors(uint32_t eyeballIndex, Vector4 &outProjU, Vector4 &outProjV) const
{
	auto *eyeballData = GetEyeballData(eyeballIndex);
	if(eyeballData == nullptr)
		return false;
	outProjU = eyeballData->state.irisProjectionU;
	outProjV = eyeballData->state.irisProjectionV;
	return true;
}
void pragma::CEyeComponent::UpdateEyeMaterialData()
{
	auto &mdl = GetEntity().GetModel();
	for(auto &data : m_eyeballData) {
		data.config.irisScale = 1.f;
		data.config.irisUvClampRange = {0.f, 1.f};
	}
	auto numEyeballs = math::min(mdl->GetEyeballCount(), static_cast<uint32_t>(m_eyeballData.size()));
	for(auto eyeballIndex = decltype(numEyeballs) {0u}; eyeballIndex < numEyeballs; ++eyeballIndex) {
		auto &eyeball = *mdl->GetEyeball(eyeballIndex);
		auto *mat = mdl->GetMaterial(0, eyeball.irisMaterialIndex);
		if(!mat)
			continue;
		auto &config = m_eyeballData[eyeballIndex].config;
		config.irisScale = mat->GetProperty("iris_scale", 1.f);
		mat->GetProperty("iris_uv_clamp_range", &config.irisUvClampRange);
	}
}
void pragma::CEyeComponent::UpdateEyeballMT(const asset::Eyeball &eyeball, uint32_t eyeballIndex)
{
	if(eyeballIndex >= m_eyeballData.size() || m_animC.expired())
		return;
	if(g_debugPrint)
		Con::COUT << "Eyeball state " << eyeballIndex << Con::endl;
	auto &eyeballData = m_eyeballData.at(eyeballIndex);
	auto &state = eyeballData.state;
	auto &config = eyeballData.config;

	auto viewTarget = GetViewTarget();

	// To world space
	math::Transform bonePose {};
	state.origin = CalcEyeballPose(eyeballIndex, &bonePose).GetOrigin();
	state.up = eyeball.up;
	uvec::rotate(&state.up, bonePose.GetRotation());

	// Look at target
	state.forward = viewTarget - state.origin;
	uvec::normalize(&state.forward);

	if(config.eyeMove == false) {
		state.forward = -state.forward;
	}

	state.right = uvec::cross(state.forward, state.up);
	uvec::normalize(&state.right);

	state.forward = state.forward + (eyeball.zOffset * 2.f) * state.right;

	// Jitter
	state.forward += math::random(-config.jitter.x, config.jitter.x) * state.right;
	state.forward += math::random(-config.jitter.y, config.jitter.y) * state.up;

	uvec::normalize(&state.forward);

	state.right = uvec::cross(state.forward, state.up);
	uvec::normalize(&state.right);

	state.up = uvec::cross(state.right, state.forward);
	uvec::normalize(&state.up);

	auto &vScale = GetEntity().GetScale();
	auto configIrisScale = (config.irisScale != 0.f) ? config.irisScale : 1.f;
	auto scale = static_cast<float>((1.0 / (eyeball.irisScale / configIrisScale)) + config.eyeSize) * vScale;
	for(uint8_t i = 0; i < 3; ++i) {
		if(scale[i] > 0.0)
			scale[i] = 1.0 / scale[i];
	}

	auto org = state.origin;

	auto u = state.right * -scale;
	state.irisProjectionU.x = u.x;
	state.irisProjectionU.y = u.y;
	state.irisProjectionU.z = u.z;
	state.irisProjectionU.w = -uvec::dot(org, u);

	auto v = state.up * -scale;
	state.irisProjectionV.x = v.x;
	state.irisProjectionV.y = v.y;
	state.irisProjectionV.z = v.z;
	state.irisProjectionV.w = -uvec::dot(org, v);

	if(g_debugPrint) {
		Con::COUT << "View target: " << viewTarget << Con::endl;

		Con::COUT << "Dilation: " << config.dilation << Con::endl;
		Con::COUT << "Eye move: " << config.eyeMove << Con::endl;
		Con::COUT << "Eye shift: " << config.eyeShift << Con::endl;
		Con::COUT << "Eye size: " << config.eyeSize << Con::endl;
		Con::COUT << "Jitter: " << config.jitter << Con::endl;

		Con::COUT << "Forward: " << state.forward << Con::endl;
		Con::COUT << "Iris projection u: " << state.irisProjectionU << Con::endl;
		Con::COUT << "Iris projection v: " << state.irisProjectionV << Con::endl;
		Con::COUT << "Origin: " << state.origin << Con::endl;
		Con::COUT << "Right: " << state.right << Con::endl;
		Con::COUT << "Up: " << state.up << Con::endl;
	}
}
