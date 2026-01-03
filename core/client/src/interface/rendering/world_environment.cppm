// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.world_environment;

export import pragma.shared;

export namespace pragma::rendering {
	class DLLCLIENT WorldEnvironment : public std::enable_shared_from_this<WorldEnvironment> {
	  public:
		static std::shared_ptr<WorldEnvironment> Create();

		class DLLCLIENT Fog {
		  public:
			Fog();

			void SetColor(const Color &col);
			const Color &GetColor() const;
			const std::shared_ptr<util::ColorProperty> &GetColorProperty() const;

			void SetStart(float start);
			float GetStart() const;
			const std::shared_ptr<util::FloatProperty> &GetStartProperty() const;

			void SetEnd(float end);
			float GetEnd() const;
			const std::shared_ptr<util::FloatProperty> &GetEndProperty() const;

			void SetMaxDensity(float density);
			float GetMaxDensity() const;
			const std::shared_ptr<util::FloatProperty> &GetMaxDensityProperty() const;

			void SetType(util::FogType type);
			util::FogType GetType() const;
			const std::shared_ptr<util::UInt8Property> &GetTypeProperty() const;

			void SetEnabled(bool bEnabled);
			bool IsEnabled() const;
			const std::shared_ptr<util::BoolProperty> &GetEnabledProperty() const;

			Vector2 GetRange() const;
			void SetRange(const Vector2 &range);
			void SetRange(float start, float end);
			float GetFarDistance() const;
		  private:
			std::shared_ptr<util::ColorProperty> m_color;
			std::shared_ptr<util::FloatProperty> m_start;
			std::shared_ptr<util::FloatProperty> m_end;
			std::shared_ptr<util::FloatProperty> m_maxDensity;
			std::shared_ptr<util::UInt8Property> m_type;
			std::shared_ptr<util::BoolProperty> m_bEnabled;
		};

		void SetShaderQuality(int32_t quality);
		int32_t GetShaderQuality() const;
		const std::shared_ptr<util::Int32Property> &GetShaderQualityProperty() const;

		void SetUnlit(bool b);
		bool IsUnlit() const;
		const std::shared_ptr<util::BoolProperty> &GetUnlitProperty() const;

		void SetShadowResolution(uint32_t resolution);
		uint32_t GetShadowResolution() const;
		const std::shared_ptr<util::UInt32Property> &GetShadowResolutionProperty() const;

		Fog &GetFogSettings();
	  protected:
		WorldEnvironment();
		std::shared_ptr<util::Int32Property> m_shaderQuality;
		std::shared_ptr<util::BoolProperty> m_bUnlit;
		std::shared_ptr<util::UInt32Property> m_shadowResolution;
		Fog m_fog = {};
	};
}
