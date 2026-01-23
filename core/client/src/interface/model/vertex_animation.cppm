// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:model.vertex_animation;

export import pragma.shared;

export namespace pragma::animation {
	class DLLCLIENT CVertexAnimation : public VertexAnimation {
	  public:
		static std::shared_ptr<CVertexAnimation> Create();
		static std::shared_ptr<CVertexAnimation> Create(const CVertexAnimation &other);
		static std::shared_ptr<CVertexAnimation> Create(const std::string &name);

		virtual std::shared_ptr<VertexAnimation> Copy() const override;

		void UpdateBuffer();
	  protected:
		CVertexAnimation();
		CVertexAnimation(const CVertexAnimation &other);
		CVertexAnimation(const std::string &name);
	};
}
