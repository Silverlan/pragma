// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "util_enum_flags.hpp"

export module pragma.client:gui.transformable;

export import :gui.snap_area;

export namespace pragma::gui::types {
	class DLLCLIENT WITransformable : public WIBase {
	  public:
		enum class StateFlags : uint8_t { None = 0u, Draggable = 1u, Resizable = Draggable << 1u, Dragging = Resizable << 1u, Resizing = Dragging << 1u, ResizeRatioLocked = Resizing << 1u, WasDragged = ResizeRatioLocked << 1u };
		WITransformable();
		virtual ~WITransformable() override;
		virtual void Initialize() override;
		using WIBase::SetSize;
		virtual void SetSize(int x, int y) override;
		using WIBase::SetPos;
		virtual void SetPos(int x, int y) override;
		virtual void SetParent(WIBase *base, std::optional<uint32_t> childIndex = {}) override;
		virtual void SetZPos(int zpos) override;
		virtual void SetVisible(bool b) override;
		WIBase *GetDragArea() const;
		void SetDraggable(bool b);
		void SetResizable(bool b);
		bool IsDraggable();
		bool IsResizable();
		virtual void Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd) override;
		virtual util::EventReply MouseCallback(platform::MouseButton button, platform::KeyState state, platform::Modifier mods) override;
		virtual void OnCursorMoved(int x, int y) override;
		void SetMinWidth(int w);
		void SetMinHeight(int h);
		void SetMinSize(int w, int h);
		void SetMinSize(Vector2i size);
		int GetMinWidth();
		int GetMinHeight();
		const Vector2i &GetMinSize() const;
		void SetMaxWidth(int w);
		void SetMaxHeight(int h);
		void SetMaxSize(int w, int h);
		void SetMaxSize(Vector2i size);
		int GetMaxWidth();
		int GetMaxHeight();
		const Vector2i &GetMaxSize() const;
		void Close();
		void SnapToTarget(WIBase &el);

		void SetResizeRatioLocked(bool bLocked);
		bool IsResizeRatioLocked() const;

		bool IsBeingDragged() const;
		bool IsBeingResized() const;

		void SetDragBounds(const Vector2i &min, const Vector2i &max);
		std::pair<Vector2i, Vector2i> GetDragBounds() const;

		void AddSnapTarget(WISnapArea &target);
		void SetRemoveOnClose(bool remove);

		void StartDrag();
		void EndDrag();
	  protected:
		virtual void DoUpdate() override;
		enum class ResizeMode { none = -1, ew = 1, we = 2, ns = 3, sn = 4, nwse = 5, nesw = 6, senw = 7, swne = 8 };
		WIHandle m_hMoveRect = {};
		WIHandle m_hResizeRect = {};
		StateFlags m_stateFlags = StateFlags::None;
		ResizeMode m_resizeMode = ResizeMode::none;
		WIHandle m_snapGhost = {};
		Vector2i m_resizeLastPos = {};
		Vector2i m_dragCursorOffset = {};
		bool m_bRemoveOnClose = true;
		std::vector<WIHandle> m_snapTargets = {};
		void InitializeSnapTargetGhost(WISnapArea &snapArea);
		void DestroySnapTargetGhost();
		void SetResizeMode(ResizeMode mode);
		void StartResizing();
		void EndResizing();
		ResizeMode InvertResizeAxis(ResizeMode mode, bool bXAxis, bool bYAxis) const;
		Vector2i m_minSize = {};
		Vector2i m_maxSize = {-1, -1};
		float m_resizeRatio = 0.f;

		Vector2i m_minDrag = {std::numeric_limits<int32_t>::lowest(), std::numeric_limits<int32_t>::lowest()};
		Vector2i m_maxDrag = {std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max()};

		virtual void OnVisibilityChanged(bool bVisible) override;
		Vector2i GetConfinedMousePos();
		void OnTitleBarMouseEvent(platform::MouseButton button, platform::KeyState state, platform::Modifier mods);
		void OnCloseButtonPressed();
		void UpdateResizeRect();
		void UpdateResizeRectPos();
	};
	using namespace pragma::math::scoped_enum::bitwise;
};

export {
	REGISTER_ENUM_FLAGS(pragma::gui::types::WITransformable::StateFlags)
}
