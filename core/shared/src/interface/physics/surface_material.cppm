// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:physics.surface_material;

export import :ai.nav_system;
export import :physics.liquid;
export import :physics.surface_type_manager;

export {
	namespace pragma::physics {
		class IEnvironment;
		class IMaterial;

		class DLLNETWORK SurfaceMaterial {
		public:
			struct AudioInfo {
				// These should correspond to the values specified in "c_game_audio.cpp"
				float lowFreqAbsorption = 0.10f;
				float midFreqAbsorption = 0.20f;
				float highFreqAbsorption = 0.30f;
				float scattering = 0.05f;
				float lowFreqTransmission = 0.100f;
				float midFreqTransmission = 0.050f;
				float highFreqTransmission = 0.030f;
			};
			struct PBRInfo {
				float metalness = 0.f;
				float roughness = 0.5f;
				struct {
					Color color = colors::White;
					Vector3 scatterColor = {0.f, 0.f, 0.f};
					Vector3 radiusMM {};
					float factor = 0.f;
				} subsurface;
			};
		public:
			SurfaceMaterial(IEnvironment &env, const std::string &identifier, UInt idx, IMaterial &physMat);
			SurfaceMaterial(const SurfaceMaterial &other);
			void Reset();
			void Load(udm::LinkedPropertyWrapper &prop);

			const std::string &GetIdentifier() const;
			UInt GetIndex() const;
			void SetFriction(Float friction);
			float GetStaticFriction() const;
			void SetStaticFriction(float friction);
			float GetDynamicFriction() const;
			void SetDynamicFriction(float friction);
			Float GetRestitution() const;

			const std::optional<float> &GetIOR() const;
			void SetIOR(float ior);
			void ClearIOR();

			IMaterial &GetPhysicsMaterial() const;
			SurfaceType *GetSurfaceType() const;
			void SetSurfaceType(const std::string &surfaceType);

			void SetRestitution(Float restitution);
			const std::string &GetFootstepType() const;
			void SetFootstepType(const std::string &footstep);
			void SetSoftImpactSound(const std::string &snd);
			const std::string &GetSoftImpactSound() const;
			void SetHardImpactSound(const std::string &snd);
			const std::string &GetHardImpactSound() const;
			void SetBulletImpactSound(const std::string &snd);
			const std::string &GetBulletImpactSound() const;
			void SetImpactParticleEffect(const std::string &particle);
			const std::string &GetImpactParticleEffect() const;

			void SetNavigationFlags(nav::PolyFlags flags);
			nav::PolyFlags GetNavigationFlags() const;

			void SetDensity(float density);
			float GetDensity() const;
			void SetLinearDragCoefficient(float coefficient);
			float GetLinearDragCoefficient() const;
			void SetTorqueDragCoefficient(float coefficient);
			float GetTorqueDragCoefficient() const;
			void SetWaveStiffness(float stiffness);
			float GetWaveStiffness() const;
			void SetWavePropagation(float propagation);
			float GetWavePropagation() const;

			const AudioInfo &GetAudioInfo() const;
			void SetAudioInfo(const AudioInfo &info);
			const PBRInfo &GetPBRInfo() const;
			PBRInfo &GetPBRInfo();
			void SetPBRInfo(const PBRInfo &pbrInfo);
			void SetAudioLowFrequencyAbsorption(float absp);
			float GetAudioLowFrequencyAbsorption() const;
			void SetAudioMidFrequencyAbsorption(float absp);
			float GetAudioMidFrequencyAbsorption() const;
			void SetAudioHighFrequencyAbsorption(float absp);
			float GetAudioHighFrequencyAbsorption() const;
			void SetAudioScattering(float scattering);
			float GetAudioScattering() const;
			void SetAudioLowFrequencyTransmission(float transmission);
			float GetAudioLowFrequencyTransmission() const;
			void SetAudioMidFrequencyTransmission(float transmission);
			float GetAudioMidFrequencyTransmission() const;
			void SetAudioHighFrequencyTransmission(float transmission);
			float GetAudioHighFrequencyTransmission() const;
		protected:
			IEnvironment &m_physEnv;
			UInt m_index;
			std::string m_identifier;
			std::string m_footstepType;
			std::string m_softImpactSound;
			std::string m_hardImpactSound;
			std::string m_bulletImpactSound;
			std::string m_impactParticle;
			std::optional<float> m_ior {};
			nav::PolyFlags m_navigationFlags;
			std::unique_ptr<PhysLiquid> m_liquidInfo = nullptr;
			std::shared_ptr<IMaterial> m_physMaterial = nullptr;
			mutable util::TWeakSharedHandle<SurfaceType> m_surfaceType = {};
			AudioInfo m_audioInfo = {};
			PBRInfo m_pbrInfo = {};
			PhysLiquid &InitializeLiquid();
		};

		class DLLNETWORK SurfaceMaterialManager {
		public:
			SurfaceMaterialManager(IEnvironment &env);
			bool Load(const std::string &path);
			SurfaceMaterial &Create(const std::string &identifier, Float staticFriction, Float dynamicFriction, Float restitution);
			SurfaceMaterial &Create(const std::string &identifier, Float friction = 0.5f, Float restitution = 0.5f);
			// The returned pointer is NOT guaranteed to stay alive; Don't store it.
			SurfaceMaterial *GetMaterial(const std::string &id);
			std::vector<SurfaceMaterial> &GetMaterials();
		protected:
			std::vector<SurfaceMaterial> m_materials; // These have to be objects (Not pointers) to uphold the requirements for the btTriangleIndexVertexMaterialArray constructor.
			IEnvironment &m_physEnv;
		};
	}
	DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::physics::SurfaceMaterial &surfaceMaterial);
};
