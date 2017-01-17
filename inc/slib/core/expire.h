#ifndef CHECKHEADER_SLIB_CORE_EXPIRE
#define CHECKHEADER_SLIB_CORE_EXPIRE

#include "definition.h"

#include "object.h"
#include "map.h"
#include "dispatch.h"

SLIB_NAMESPACE_BEGIN

template <class KT, class VT>
class SLIB_EXPORT ExpiringMap : public Object
{
protected:
	Map<KT, VT> m_mapCurrent;
	Map<KT, VT> m_mapBackup;
	
	sl_uint32 m_duration;
	
	Ref<Timer> m_timer;
	WeakRef<Dispatcher> m_dispatcher;

public:
	ExpiringMap();
	
	~ExpiringMap();
	
public:
	sl_uint32 getExpiringMilliseconds() const;

	void setupTimer(sl_uint32 expiring_duration_ms, const Ref<Dispatcher>& _dispatcher);

	void clearTimer();
	
	void setupTimer(sl_uint32 period_ms);

	sl_bool get(const KT& key, VT* _out = sl_null, sl_bool flagUpdateLifetime = sl_true);
	
	VT getValue(const KT& key, const VT& def, sl_bool flagUpdateLifetime = sl_true);
	
	sl_bool put(const KT& key, const VT& value);
	
	void remove(const KT& key);
	
	void removeAll();
	
	sl_bool containsKey(const KT& key);
	
protected:
	void _update();
	
	void _release();

};

SLIB_NAMESPACE_END


SLIB_NAMESPACE_BEGIN

template <class KT, class VT>
ExpiringMap<KT, VT>::ExpiringMap()
{
	m_mapCurrent.initHash();
	m_mapBackup.initHash();
	
	m_duration = 0;
}

template <class KT, class VT>
ExpiringMap<KT, VT>::~ExpiringMap()
{
	_release();
}

template <class KT, class VT>
sl_uint32 ExpiringMap<KT, VT>::getExpiringMilliseconds() const
{
	return m_duration;
}

template <class KT, class VT>
void ExpiringMap<KT, VT>::setupTimer(sl_uint32 expiring_duration_ms, const Ref<Dispatcher>& _dispatcher)
{
	ObjectLocker lock(this);
	clearTimer();
	Ref<Dispatcher> dispatcher = _dispatcher;
	if (dispatcher.isNull()) {
		dispatcher = Dispatcher::getDefault();
		if (dispatcher.isNull()) {
			return;
		}
	}
	if (expiring_duration_ms > 0) {
		m_duration = expiring_duration_ms;
		m_dispatcher = dispatcher;
		typedef ExpiringMap<KT, VT> _ExpiringMap;
		m_timer = dispatcher->addTimer(SLIB_FUNCTION_CLASS(_ExpiringMap, _update, this), expiring_duration_ms);
	}
}

template <class KT, class VT>
void ExpiringMap<KT, VT>::clearTimer()
{
	ObjectLocker lock(this);
	Ref<Timer> timer = m_timer;
	if (timer.isNotNull()) {
		Ref<Dispatcher> dispatcher = m_dispatcher;
		if (dispatcher.isNotNull()) {
			dispatcher->removeTimer(timer);
		}
	}
	m_timer.setNull();
	m_dispatcher.setNull();
}

template <class KT, class VT>
void ExpiringMap<KT, VT>::setupTimer(sl_uint32 period_ms)
{
	setupTimer(period_ms, Ref<Dispatcher>::null());
}

template <class KT, class VT>
sl_bool ExpiringMap<KT, VT>::get(const KT& key, VT* _out, sl_bool flagUpdateLifetime)
{
	ObjectLocker lock(this);
	VT* p = m_mapCurrent.getItemPointer(key);
	if (p) {
		if (_out) {
			*_out = *p;
		}
		return sl_true;
	}
	p = m_mapBackup.getItemPointer(key);
	if (p) {
		if (_out) {
			*_out = *p;
		}
		if (flagUpdateLifetime) {
			m_mapCurrent.put_NoLock(key, *p);
			m_mapBackup.remove_NoLock(key);
		}
		return sl_true;
	}
	return sl_false;
}

template <class KT, class VT>
VT ExpiringMap<KT, VT>::getValue(const KT& key, const VT& def, sl_bool flagUpdateLifetime)
{
	ObjectLocker lock(this);
	VT* p = m_mapCurrent.getItemPointer(key);
	if (p) {
		return *p;
	}
	p = m_mapBackup.getItemPointer(key);
	if (p) {
		if (flagUpdateLifetime) {
			m_mapCurrent.put_NoLock(key, *p);
			m_mapBackup.remove_NoLock(key);
		}
		return *p;
	}
	return def;
}

template <class KT, class VT>
sl_bool ExpiringMap<KT, VT>::put(const KT& key, const VT& value)
{
	ObjectLocker lock(this);
	m_mapBackup.remove_NoLock(key);
	return m_mapCurrent.put_NoLock(key, value);
}

template <class KT, class VT>
void ExpiringMap<KT, VT>::remove(const KT& key)
{
	ObjectLocker lock(this);
	m_mapCurrent.remove_NoLock(key);
	m_mapBackup.remove_NoLock(key);
}

template <class KT, class VT>
void ExpiringMap<KT, VT>::removeAll()
{
	ObjectLocker lock(this);
	m_mapCurrent.removeAll_NoLock();
	m_mapBackup.removeAll_NoLock();
}

template <class KT, class VT>
sl_bool ExpiringMap<KT, VT>::containsKey(const KT& key)
{
	ObjectLocker lock(this);
	if (m_mapCurrent.containsKey_NoLock(key)) {
		return sl_true;
	}
	if (m_mapBackup.containsKey_NoLock(key)) {
		return sl_true;
	}
	return sl_false;
}

template <class KT, class VT>
void ExpiringMap<KT, VT>::_update()
{
	ObjectLocker lock(this);
	m_mapBackup = m_mapCurrent;
	m_mapCurrent.initHash();
}

template <class KT, class VT>
void ExpiringMap<KT, VT>::_release()
{
	ObjectLocker lock(this);
	if (m_timer.isNotNull()) {
		m_timer->stopAndWait();
	}
	clearTimer();
}

SLIB_NAMESPACE_END

#endif
