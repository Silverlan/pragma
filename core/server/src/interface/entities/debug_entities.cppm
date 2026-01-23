// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.debug;

import :entities.base;
import :entities.components.entity;

export {
	class DLLSERVER EntDebugText : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};

	////////////////

	class DLLSERVER EntDebugPoint : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};

	////////////////

	class DLLSERVER EntDebugLine : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};

	////////////////

	class DLLSERVER EntDebugBox : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};

	////////////////

	class DLLSERVER EntDebugSphere : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};

	////////////////

	class DLLSERVER EntDebugCone : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};

	////////////////

	class DLLSERVER EntDebugCylinder : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};

	////////////////

	class DLLSERVER EntDebugPlane : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
