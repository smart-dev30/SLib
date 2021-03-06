/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHECKHEADER_SLIB_UI_RENDER_VIEW
#define CHECKHEADER_SLIB_UI_RENDER_VIEW

#include "definition.h"

#include "view.h"

#include "../render/engine.h"

namespace slib
{

	class SLIB_EXPORT RenderView : public View
	{
		SLIB_DECLARE_OBJECT
		
	public:
		RenderView();
		
		~RenderView();
		
	public:
		RedrawMode getRedrawMode();
		
		void setRedrawMode(RedrawMode mode);
		
		sl_bool isDispatchingEventsToRenderingThread();
		
		void setDispatchingEventsToRenderingThread(sl_bool flag);
		
		void requestRender();
		
		// override
		void invalidate();
		
		// override
		void invalidate(const UIRect& rect);
		
		void renderViewContent(RenderEngine* engine);
		
		// override
		Size measureText(const String& text, const Ref<Font>& font, sl_bool flagMultiLine = sl_false);
		
		// override
		Ref<AnimationLoop> getAnimationLoop();
		
		// override
		sl_bool isDrawingThread();
		
		// override
		void dispatchToDrawingThread(const Function<void()>& callback, sl_uint32 delayMillis = 0);
		
		// override
		void runOnDrawingThread(const Function<void()>& callback);
		
		// override
		Ref<Dispatcher> getDispatcher();

		sl_bool isDebugTextVisible();
		
		void setDebugTextVisible(sl_bool flagVisible);
		
		sl_bool isDebugTextVisibleOnRelease();
		
		void setDebugTextVisibleOnRelease(sl_bool flagVisible);
		
	public:
		SLIB_PROPERTY(RenderEngineType, PreferredEngineType)
		
		SLIB_PROPERTY(AtomicFunction<void(RenderView*, RenderEngine*)>, OnFrame)

	protected:
		virtual void onFrame(RenderEngine* engine);
		
		// override
		void onAttach();
		
		// override
		void onDrawBackground(Canvas* canvas);
		
	public:
		// override
		Ref<ViewInstance> createNativeWidget(ViewInstance* _parent);
		
		virtual void dispatchFrame(RenderEngine* engine);
		
		// override
		void dispatchMouseEvent(UIEvent* ev);
		
		// override
		void dispatchTouchEvent(UIEvent* ev);
		
		// override
		void dispatchMouseWheelEvent(UIEvent* ev);
		
		// override
		void dispatchKeyEvent(UIEvent* ev);
		
		// override
		void dispatchSetCursor(UIEvent* ev);
		
		// override
		void dispatchSwipe(GestureEvent* ev);

	private:
		void _dispatchMouseEvent(const Ref<UIEvent>& ev);
		
		void _dispatchTouchEvent(const Ref<UIEvent>& ev);
		
		void _dispatchMouseWheelEvent(const Ref<UIEvent>& ev);
		
		void _dispatchKeyEvent(const Ref<UIEvent>& ev);
		
		void _dispatchSetCursor(const Ref<UIEvent>& ev);
		
		void _dispatchSwipe(const Ref<GestureEvent>& ev);
		
		void _setRedrawMode_NW(RedrawMode mode);

		void _requestRender_NW();
		
	protected:
		RedrawMode m_redrawMode;
		sl_bool m_flagDispatchEventsToRenderingThread;
		
		Ref<AnimationLoop> m_animationLoop;
		sl_uint64 m_lastRenderingThreadId;
		Queue< Function<void()> > m_queuePostedCallbacks;

		sl_bool m_flagDebugTextVisible;
		sl_bool m_flagDebugTextVisibleOnRelease;
		
	};

}

#endif
