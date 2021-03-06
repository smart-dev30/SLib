/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHECKHEADER_SLIB_RENDER_BASE
#define CHECKHEADER_SLIB_RENDER_BASE

#include "definition.h"

#include "../core/object.h"
#include "../core/list.h"

#define SLIB_MAX_RENDER_ENGINE_COUNT_PER_OBJECT 4

namespace slib
{

	class RenderEngine;
	class RenderBaseObject;
	
	class SLIB_EXPORT RenderBaseObjectInstance : public Object
	{
		SLIB_DECLARE_OBJECT
		
	protected:
		RenderBaseObjectInstance();
		
		~RenderBaseObjectInstance();
		
	public:
		void link(const Ref<RenderEngine>& engine, const Ref<RenderBaseObject>& object);
		
		Ref<RenderBaseObject> getObject();
		
		Ref<RenderEngine> getEngine();
		
	protected:
		virtual void onUpdate(RenderBaseObject* object);
		
	public:
		// should be called by only engine internally
		void _update(RenderBaseObject* object);
		
		sl_bool _isUpdated();
		
	protected:
		AtomicWeakRef<RenderBaseObject> m_object;
		AtomicWeakRef<RenderEngine> m_engine;
		
		sl_bool m_flagUpdated;
		
		friend class RenderBaseObject;
		
	};
	
	class SLIB_EXPORT RenderBaseObject : public Object
	{
		SLIB_DECLARE_OBJECT
		
	protected:
		RenderBaseObject();
		
		~RenderBaseObject();
		
	public:
		void addInstance(const Ref<RenderBaseObjectInstance>& instance);
		
		void removeInstance(const Ref<RenderBaseObjectInstance>& instance);
		
		void removeAllInstances();
		
		Ref<RenderBaseObjectInstance> getInstance(RenderEngine* engine);
		
	protected:
		AtomicRef<RenderBaseObjectInstance> m_instances[SLIB_MAX_RENDER_ENGINE_COUNT_PER_OBJECT];
		
	};

}

#endif
