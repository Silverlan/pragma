// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.particle;

export import pragma.math;

export namespace pragma::pts {
#pragma warning(push)
#pragma warning(disable : 4251)
	class DLLCLIENT CParticleSystemBaseKeyValues {
	  public:
		CParticleSystemBaseKeyValues() = default;
		void RecordKeyValues(const std::unordered_map<std::string, std::string> &values);
		const std::unordered_map<std::string, std::string> *GetKeyValues() const;
		bool IsRecordingKeyValues() const;
	  private:
		std::unique_ptr<std::unordered_map<std::string, std::string>> m_keyValues;
	};

	class DLLCLIENT CParticle {
	  public:
		enum class FieldId : uint8_t {
			Pos = 0,
			Rot,
			RotYaw,
			Origin,
			Velocity,
			AngularVelocity,
			Radius,
			Length,
			Life,
			Color,
			Alpha,
			Sequence,
			CreationTime,

			Count,
			Invalid = Count
		};
		static const char *field_id_to_name(FieldId id);
		static FieldId name_to_field_id(const std::string &fieldName);

		CParticle();
		~CParticle();

		void PopulateInitialValues();
		void SetPrevPos(const Vector3 &prevPos);

		uint32_t GetIndex() const;
		void SetIndex(uint32_t idx);
		bool IsAlive() const;
		void SetAlive(bool b);
		bool ShouldDraw() const;
		float GetRotation() const;
		void SetRotation(float rotation);
		float GetRotationYaw() const;
		void SetRotationYaw(float yaw);
		const Vector3 &GetPosition() const;
		const Vector3 &GetVelocity() const;
		const Vector3 &GetAngularVelocity() const;
		float GetLife() const;
		void SetLife(float life);
		float GetTimeAlive() const;
		void SetTimeAlive(float alive);
		float GetTimeCreated() const;
		void SetTimeCreated(float time);
		void SetColor(const Vector4 &col);
		void SetColor(const Color &col);
		void SetPosition(const Vector3 &pos);
		void SetVelocity(const Vector3 &vel);
		void SetAngularVelocity(const Vector3 &vel);
		const Vector4 &GetColor() const;
		Vector4 &GetColor();
		float GetRadius() const;
		float GetLength() const;
		float GetExtent() const;
		void SetRadius(float radius);
		void SetLength(float length);
		void SetCameraDistance(float dist);
		float GetCameraDistance() const;
		void Reset(float t);
		void SetFrameOffset(float offset);
		float GetFrameOffset() const;
		void SetSequence(uint32_t sequence);
		uint32_t GetSequence() const;

		const Vector3 &GetPrevPos() const;
		float GetInitialRadius() const;
		float GetInitialLength() const;
		float GetInitialRotation() const;
		float GetInitialLife() const;
		const Vector4 &GetInitialColor() const;
		float GetInitialFrameOffset() const;

		void Die();
		void Resurrect();
		bool IsDying() const;
		// Returns the time at which the particle has died
		float GetDeathTime() const;
		// Returns the total lifespan of the particle up to the point where it will die
		float GetLifeSpan() const;

		const Quat &GetWorldRotation() const;
		void SetWorldRotation(const Quat &rot);
		const Vector3 &GetOrigin() const;
		void SetOrigin(const Vector3 &origin);

		void SetField(FieldId fieldId, float value);
		void SetField(FieldId fieldId, const Vector4 &value);
		bool GetField(FieldId fieldId, float &outValue) const;
		bool GetField(FieldId fieldId, Vector4 &outValue) const;

		uint32_t GetSeed() const;
		// Generates a random int, but gurantees to always return the same int for a specific particle and a specific seed
		template<typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
		T PseudoRandomInt(T min, T max, uint32_t seed = 0u) const
		{
			return PseudoRandomInt(std::uniform_int_distribution<T>(min, max), seed);
		}
		template<typename T>
		T PseudoRandomInt(const std::uniform_int_distribution<T> &dis, uint32_t seed = 0u) const
		{
			m_mt.seed(m_seed + seed);
			return const_cast<std::uniform_int_distribution<T> &>(dis)(m_mt);
		}
		// Generates a random real, but gurantees to always return the same real for a specific particle and a specific seed
		template<typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
		T PseudoRandomReal(T min, T max, uint32_t seed = 0u) const
		{
			return PseudoRandomReal(std::uniform_real_distribution<T>(min, max), seed);
		}
		template<typename T>
		T PseudoRandomReal(const std::uniform_real_distribution<T> &dis, uint32_t seed = 0u) const
		{
			m_mt.seed(m_seed + seed);
			return const_cast<std::uniform_real_distribution<T> &>(dis)(m_mt);
		}

		template<typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
		T PseudoRandomRealExp(T min, T max, float exp, uint32_t seed = 0u) const
		{
			auto v = PseudoRandomReal(std::uniform_real_distribution<T>(min, max), seed) - min;
			if(exp != 1.f)
				v = powf(v, exp);
			return v + min;
		}
	  private:
		Vector3 m_pos = {};
		Quat m_rot = uquat::identity(); // Only used by model renderer and physics
		                                // Optional relative origin, which is rotated by the particle's rotation, and added to its position before
		                                // being written to the render buffer (= Position modifier)
		Vector3 m_origin = {};
		Vector3 m_velocity = {};
		Vector3 m_angularVelocity = {};
		float m_radius = 0.f;
		float m_length = 0.f;
		bool m_bHasLength = false;
		float m_tLife = 0.f;
		float m_tAlive = 0.f;
		float m_tCreated = 0.f;
		float m_rotation = 0.f;
		float m_rotationYaw = 0.f;
		Vector4 m_color = {};
		float m_camDist = 0.f;
		bool m_bIsAlive = false;
		float m_frameOffset = 0.f;
		uint32_t m_index = 0u;
		uint32_t m_sequence;
		uint32_t m_seed = 0u;
		mutable std::mt19937 m_mt;

		bool m_bDying = false;
		float m_tDeath = 0.f;

		Vector3 m_prevPos = {};

		// Initial values after the initializers have run
		float m_initialRadius = 0.f;
		float m_initialLength = 0.f;
		float m_initialRotation = 0.f;
		float m_initialRotationYaw = 0.f;
		float m_initialLife = 0.f;
		Vector4 m_initialColor = {};
		float m_initialFrameOffset = 0.f;
	};
#pragma warning(pop)

	inline bool operator<(const CParticle &a, const CParticle &b) { return a.GetCameraDistance() > b.GetCameraDistance(); }
};
