#include "../../../inc/slib/core/definition.h"

#ifdef SLIB_PLATFORM_USE_JNI

#include "../../../inc/slib/core/java.h"
#include "../../../inc/slib/core/log.h"

#define JNIVERSION JNI_VERSION_1_4

SLIB_NAMESPACE_BEGIN

static void _logJniError(const String& text)
{
	SLIB_LOG_ERROR("SLIB_JNI", text);
}

JavaVM* _g_jvm_shared = sl_null;
typedef Map<String, JniClass> JniClassMap;
SLIB_SAFE_STATIC_REF(JniClassMap, _g_jni_classes);

SLIB_THREAD JNIEnv* _gt_jniCurrent = sl_null;

SLIB_SAFE_STATIC_REF(List<_JniSingletonClass*>, _g_jni_singleton_classes);
SLIB_SAFE_STATIC_REF(List<_JniSingletonMethod*>, _g_jni_singleton_methods);
SLIB_SAFE_STATIC_REF(List<_JniSingletonStaticMethod*>, _g_jni_singleton_static_methods);
SLIB_SAFE_STATIC_REF(List<_JniSingletonField*>, _g_jni_singleton_fields);
SLIB_SAFE_STATIC_REF(List<_JniSingletonStaticField*>, _g_jni_singleton_static_fields);
SLIB_SAFE_STATIC_REF(List<_JniNativeMethod*>, _g_jni_native_methods);

void Jni::initialize(JavaVM* jvm)
{
	static sl_bool flagInit = sl_false;
	if (! flagInit) {
		flagInit = sl_true;
		Jni::setSharedJVM(jvm);

		// singleton classes
		{
			ListLocker< _JniSingletonClass* > list(_g_jni_singleton_classes);
			for (sl_size i = 0; i < list.count(); i++) {
				_JniSingletonClass* obj = list[i];
				obj->cls = Jni::getClass(obj->name);
			}
		}
		// singleton methods
		{
			ListLocker< _JniSingletonMethod* > list(_g_jni_singleton_methods);
			for (sl_size i = 0; i < list.count(); i++) {
				_JniSingletonMethod* obj = list[i];
				JniClass cls = obj->gcls->cls;
				if (cls.isNotNull()) {
					obj->cls = cls;
					obj->id = cls.getMethodID(obj->name, obj->sig);
				}
			}
		}
		// singleton static methods
		{
			ListLocker< _JniSingletonStaticMethod* > list(_g_jni_singleton_static_methods);
			for (sl_size i = 0; i < list.count(); i++) {
				_JniSingletonStaticMethod* obj = list[i];
				JniClass cls = obj->gcls->cls;
				if (cls.isNotNull()) {
					obj->cls = cls;
					obj->id = cls.getStaticMethodID(obj->name, obj->sig);
				}
			}
		}
		// singleton fields
		{
			ListLocker< _JniSingletonField* > list(_g_jni_singleton_fields);
			for (sl_size i = 0; i < list.count(); i++) {
				_JniSingletonField* obj = list[i];
				JniClass cls = obj->gcls->cls;
				if (cls.isNotNull()) {
					obj->cls = cls;
					obj->id = cls.getFieldID(obj->name, obj->sig);
				}
			}
		}
		// singleton static fields
		{
			ListLocker< _JniSingletonStaticField* > list(_g_jni_singleton_static_fields);
			for (sl_size i = 0; i < list.count(); i++) {
				_JniSingletonStaticField* obj = list[i];
				JniClass cls = obj->gcls->cls;
				if (cls.isNotNull()) {
					obj->cls = cls;
					obj->id = cls.getStaticFieldID(obj->name, obj->sig);
				}
			}
		}
		// native methods
		{
			ListLocker< _JniNativeMethod* > list(_g_jni_native_methods);
			for (sl_size i = 0; i < list.count(); i++) {
				_JniNativeMethod* obj = list[i];
				JniClass cls = obj->gcls->cls;
				if (cls.isNotNull()) {
					if (!cls.registerNative(obj->name, obj->sig, obj->fn)) {
						_logJniError(String("Failed to register JNI native function - ") + obj->name + ":" + obj->sig);
					}
				}
			}
		}
	}
}

void Jni::setSharedJVM(JavaVM *jvm)
{
	if (_g_jvm_shared != jvm) {
		_g_jvm_shared = jvm;
	}
}

JavaVM* Jni::getSharedJVM()
{
	return _g_jvm_shared;
}

JNIEnv* Jni::getCurrent()
{
	JNIEnv *env = _gt_jniCurrent;
	if (! env) {
		env = Jni::attachThread();
	}
	return env;
}

void Jni::setCurrent(JNIEnv* jni)
{
	_gt_jniCurrent = jni;
}

JNIEnv* Jni::attachThread(JavaVM* jvm)
{
	if (! jvm) {
		jvm = Jni::getSharedJVM();
	}
	JNIEnv *env = sl_null;
	if (jvm) {
		if (jvm->GetEnv((void**) &env, JNIVERSION) != JNI_OK) {
#if defined(SLIB_PLATFORM_IS_ANDROID)
			jint res = jvm->AttachCurrentThread(&env, sl_null);
#else
			jint res = jvm->AttachCurrentThread((void**)&env, sl_null);
#endif
			if ((res < 0) || !env) {
				_logJniError("Failed to attach thread");
			}
			Jni::setCurrent(env);
		}
	}
	if (env) {
		Jni::setCurrent(env);
	}
	return env;
}

void Jni::detachThread(JavaVM* jvm)
{
	if (! jvm) {
		jvm = Jni::getSharedJVM();
	}
	if (jvm) {
		jvm->DetachCurrentThread();
	}
}

JniClass Jni::findClass(const char* className)
{
	JniClass ret;
	JNIEnv *env = getCurrent();
	if (env) {
		JniLocal<jclass> cls = env->FindClass(className);
		if (Jni::checkException()) {
			_logJniError(String("Exception occurred while finding class: ") + className);
			Jni::printException();
			Jni::clearException();
		} else {
			if (cls.isNotNull()) {
				ret = JniClass::from(cls);
			}
		}
	}
	return ret;
}

JniClass Jni::getClass(const String& _className)
{
	String className = _className;
	JniClass ret;
	if (_g_jni_classes.get(className, &ret)) {
		return ret;
	}
	ret = Jni::findClass(className.getBuf());
	if (ret.isNotNull()) {
		Jni::registerClass(className, ret);
	}
	return ret;
}

void Jni::registerClass(const String& className, jclass cls)
{
	_g_jni_classes.put(className, cls);
}

void Jni::unregisterClass(const String& className)
{
	_g_jni_classes.remove(className);
}

sl_bool Jni::isSameObject(jobject ref1, jobject ref2)
{
	JNIEnv* env = Jni::getCurrent();
	if (env) {
		return env->IsSameObject(ref1, ref2) != 0;
	}
	return sl_false;
}

jobjectRefType Jni::getRefType(jobject obj)
{
	JNIEnv* env = Jni::getCurrent();
	if (env) {
		return env->GetObjectRefType(obj);
	}
	return JNIInvalidRefType;
}

sl_bool Jni::isInvalidRef(jobject obj)
{
	return Jni::getRefType(obj) == JNIInvalidRefType;
}

sl_bool Jni::isLocalRef(jobject obj)
{
	return Jni::getRefType(obj) == JNILocalRefType;
}

jobject Jni::newLocalRef(jobject obj)
{
	JNIEnv* env = Jni::getCurrent();
	if (env) {
		return env->NewLocalRef(obj);
	}
	return 0;
}

void Jni::deleteLocalRef(jobject obj)
{
	JNIEnv* env = Jni::getCurrent();
	if (env) {
		jobjectRefType type = env->GetObjectRefType(obj);
		if (type == JNILocalRefType) {
			env->DeleteLocalRef(obj);
		}
	}
}

sl_bool Jni::isGlobalRef(jobject obj)
{
	return Jni::getRefType(obj) == JNIGlobalRefType;
}

jobject Jni::newGlobalRef(jobject obj)
{
	JNIEnv* env = Jni::getCurrent();
	if (env) {
		return env->NewGlobalRef(obj);
	}
	return 0;
}

void Jni::deleteGlobalRef(jobject obj)
{
	JNIEnv* env = Jni::getCurrent();
	if (env) {
		jobjectRefType type = env->GetObjectRefType(obj);
		if (type == JNIGlobalRefType) {
			env->DeleteGlobalRef(obj);
		}
	}
}

sl_bool Jni::isWeakRef(jobject obj)
{
	return Jni::getRefType(obj) == JNIWeakGlobalRefType;
}

jobject Jni::newWeakRef(jobject obj)
{
	JNIEnv* env = Jni::getCurrent();
	if (env) {
		return env->NewWeakGlobalRef(obj);
	}
	return 0;
}

void Jni::deleteWeakRef(jobject obj)
{
	JNIEnv* env = Jni::getCurrent();
	if (env) {
		jobjectRefType type = env->GetObjectRefType(obj);
		if (type == JNIWeakGlobalRefType) {
			env->DeleteWeakGlobalRef(obj);
		}
	}
}

jstring Jni::getJniString(const String& _str)
{
	jstring ret = sl_null;
	String8 str = _str;
	if (str.isNotNull()) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			ret = env->NewStringUTF(str.getBuf());
		}
	}
	return ret;
}

jstring Jni::getJniString16(const String16& _str)
{
	jstring ret = sl_null;
	String16 str = _str;
	if (str.isNotNull()) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			ret = env->NewString((jchar*)(str.getBuf()), str.getLength());
		}
	}
	return ret;
}

String Jni::getString(jstring str)
{
	String ret;
	if (str) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			sl_uint32 len = (sl_uint32)(env->GetStringUTFLength(str));
			const char* sz = env->GetStringUTFChars(str, sl_null);
			if (sz) {
				ret = String::fromUtf8(sz, len);
				env->ReleaseStringUTFChars(str, sz);
			}
		}
	}
	return ret;
}

String16 Jni::getString16(jstring str)
{
	String16 ret;
	if (str) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			sl_uint32 len = (sl_uint32)(env->GetStringLength(str));
			const jchar* sz = env->GetStringChars(str, sl_null);
			if (sz) {
				ret = String16::fromUtf16(sz, len);
				env->ReleaseStringChars(str, sz);
			}
		}
	}
	return ret;
}

sl_uint32 Jni::getArrayLength(jarray array)
{
	sl_uint32 ret = 0;
	if (array) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			ret = env->GetArrayLength(array);
		}
	}
	return ret;
}
jobjectArray Jni::newObjectArray(jclass clsElement, sl_uint32 length)
{
	jobjectArray ret = sl_null;
	if (clsElement) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			ret = env->NewObjectArray(length, clsElement, sl_null);
		}
	}
	return ret;
}
jobject Jni::getObjectArrayElement(jobjectArray array, sl_uint32 index)
{
	jobject ret = sl_null;
	if (array) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			ret = env->GetObjectArrayElement(array, index);
		}
	}
	return ret;
}
void Jni::setObjectArrayElement(jobjectArray array, sl_uint32 index, jobject value)
{
	if (array) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			env->SetObjectArrayElement(array, index, value);
		}
	}
}

jobjectArray Jni::newStringArray(sl_uint32 length)
{
	SLIB_STATIC_STRING(cls, "java/lang/String");
	return Jni::newObjectArray(Jni::getClass(cls), length);
}
String Jni::getStringArrayElement(jobjectArray array, sl_uint32 index)
{
	String ret;
	JniLocal<jstring> v = (jstring)(Jni::getObjectArrayElement(array, index));
	if (v.isNotNull()) {
		ret = Jni::getString(v);
	}
	return ret;
}
String16 Jni::getString16ArrayElement(jobjectArray array, sl_uint32 index)
{
	String16 ret;
	JniLocal<jstring> v = (jstring)(Jni::getObjectArrayElement(array, index));
	if (v.isNotNull()) {
		ret = Jni::getString16(v);
	}
	return ret;
}
void Jni::setStringArrayElement(jobjectArray array, sl_uint32 index, const String& value)
{
	JniLocal<jstring> v = Jni::getJniString(value);
	Jni::setObjectArrayElement(array, index, v);
}
void Jni::setString16ArrayElement(jobjectArray array, sl_uint32 index, const String16& value)
{
	JniLocal<jstring> v = Jni::getJniString16(value);
	Jni::setObjectArrayElement(array, index, v);
}

#define DEFINE_JNI_ARRAY(TYPE, NAME) \
TYPE##Array Jni::new##NAME##Array(sl_uint32 length) \
{ \
	TYPE##Array ret = sl_null; \
	JNIEnv* env = Jni::getCurrent(); \
	if (env) { \
		ret = env->New##NAME##Array(length); \
	} \
	return ret; \
} \
TYPE* Jni::get##NAME##ArrayElements(TYPE##Array array, jboolean* isCopy) \
{ \
	TYPE* ret = sl_null; \
	if (array) { \
		JNIEnv* env = Jni::getCurrent(); \
		if (env) { \
			ret = env->Get##NAME##ArrayElements(array, isCopy); \
		} \
	} \
	return ret; \
} \
void Jni::release##NAME##ArrayElements(TYPE##Array array, TYPE* buf, jint mode) \
{ \
	if (array && buf) { \
		JNIEnv* env = Jni::getCurrent(); \
		if (env) { \
			env->Release##NAME##ArrayElements(array, buf, mode); \
		} \
	} \
} \
void Jni::get##NAME##ArrayRegion(TYPE##Array array, sl_uint32 index, sl_uint32 len, TYPE* buf) \
{ \
	if (array && buf) { \
		JNIEnv* env = Jni::getCurrent(); \
		if (env) { \
			env->Get##NAME##ArrayRegion(array, index, len, buf); \
		} \
	} \
} \
void Jni::set##NAME##ArrayRegion(TYPE##Array array, sl_uint32 index, sl_uint32 len, TYPE* buf) \
{ \
	if (array && buf) { \
		JNIEnv* env = Jni::getCurrent(); \
		if (env) { \
			env->Set##NAME##ArrayRegion(array, index, len, buf); \
		} \
	} \
} \

DEFINE_JNI_ARRAY(jboolean, Boolean)
DEFINE_JNI_ARRAY(jbyte, Byte)
DEFINE_JNI_ARRAY(jchar, Char)
DEFINE_JNI_ARRAY(jshort, Short)
DEFINE_JNI_ARRAY(jint, Int)
DEFINE_JNI_ARRAY(jlong, Long)
DEFINE_JNI_ARRAY(jfloat, Float)
DEFINE_JNI_ARRAY(jdouble, Double)

jobject Jni::newDirectByteBuffer(void* address, sl_size capacity)
{
	JNIEnv* env = Jni::getCurrent();
	if (env && address && capacity) {
		return env->NewDirectByteBuffer(address, capacity);
	}
	return sl_null;
}

void* Jni::getDirectBufferAddress(jobject buf)
{
	JNIEnv* env = Jni::getCurrent();
	if (env && buf) {
		return env->GetDirectBufferAddress(buf);
	}
	return sl_null;
}

sl_size Jni::getDirectBufferCapacity(jobject buf)
{
	JNIEnv* env = Jni::getCurrent();
	if (env && buf) {
		return (sl_size)(env->GetDirectBufferCapacity(buf));
	}
	return 0;
}

sl_bool Jni::checkException()
{
	JNIEnv* env = Jni::getCurrent();
	if (env) {
		return env->ExceptionCheck() != 0;
	}
	return sl_false;
}

void Jni::clearException()
{
	JNIEnv* env = Jni::getCurrent();
	if (env) {
		env->ExceptionClear();
	}
}

void Jni::printException()
{
	JNIEnv* env = Jni::getCurrent();
	if (env) {
		env->ExceptionDescribe();
	}
}


SLIB_JNI_BEGIN_CLASS(_JniInputStream, "java/io/InputStream")
	SLIB_JNI_METHOD(read, "read", "([B)I");
	SLIB_JNI_METHOD(close, "close", "()V");
SLIB_JNI_END_CLASS

sl_int32 Jni::readFromInputStream(jobject stream, jbyteArray array)
{
	if (stream && array) {
		sl_int32 n = _JniInputStream::read.callInt(stream, array);
		if (n < 0) {
			n = 0;
		}
		if (Jni::checkException()) {
			n = -1;
			Jni::printException();
			Jni::clearException();
		}
		return n;
	}
	return -1;
}

void Jni::closeInputStream(jobject stream)
{
	if (stream) {
		_JniInputStream::close.call(stream);
		if (Jni::checkException()) {
			Jni::printException();
			Jni::clearException();
		}
	}
}

/************************************************
	JniGlobal
**************************************************/
Ref<_JniGlobal> _JniGlobal::from(jobject obj)
{
	Ref<_JniGlobal> ret;
	if (obj) {
		jobject jglobal = Jni::newGlobalRef(obj);
		if (jglobal) {
			ret = new _JniGlobal();
			if (ret.isNotNull()) {
				ret->object = jglobal;
				return ret;
			}
			Jni::deleteGlobalRef(jglobal);
		}
	}
	return ret;
}

_JniGlobal::~_JniGlobal()
{
	Jni::deleteGlobalRef(object);
}

/************************************************
	JniClass
**************************************************/
sl_bool JniClass::isInstanceOf(jobject obj)
{
	jclass cls = get();
	JNIEnv* env = Jni::getCurrent();
	if (env) {
		return env->IsInstanceOf(obj, cls) != 0;
	}
	return sl_false;
}

jmethodID JniClass::getMethodID(const char* name, const char* sig)
{
	jmethodID ret = sl_null;
	jclass cls = get();
	JNIEnv* env = Jni::getCurrent();
	if (env) {
		ret = env->GetMethodID(cls, name, sig);
		if (Jni::checkException()) {
			Jni::printException();
			Jni::clearException();
		}
		return ret;
	}
	return 0;
}

jmethodID JniClass::getStaticMethodID(const char* name, const char* sig)
{
	jmethodID ret = sl_null;
	jclass cls = get();
	JNIEnv* env = Jni::getCurrent();
	if (env) {
		ret = env->GetStaticMethodID(cls, name, sig);
		if (Jni::checkException()) {
			Jni::printException();
			Jni::clearException();
		}
	}
	return ret;
}

jfieldID JniClass::getFieldID(const char* name, const char* sig)
{
	jfieldID ret = sl_null;
	jclass cls = get();
	JNIEnv* env = Jni::getCurrent();
	if (env) {
		ret = env->GetFieldID(cls, name, sig);
		if (Jni::checkException()) {
			Jni::printException();
			Jni::clearException();
		}
	}
	return ret;
}

jfieldID JniClass::getStaticFieldID(const char* name, const char* sig)
{
	jfieldID ret = sl_null;
	jclass cls = get();
	JNIEnv* env = Jni::getCurrent();
	if (env) {
		ret = env->GetStaticFieldID(cls, name, sig);
		if (Jni::checkException()) {
			Jni::printException();
			Jni::clearException();
		}
	}
	return ret;
}

#define DEFINE_JNI_CALL_METHOD(TYPE, NAME) \
TYPE JniClass::call##NAME##Method(jmethodID method, jobject _this, ...) \
{ \
	va_list args; \
	va_start(args, _this); \
	TYPE ret = 0; \
	if (method && _this) { \
		JNIEnv* env = Jni::getCurrent(); \
		if (env) { \
			ret = env->Call##NAME##MethodV(_this, method, args); \
		} \
	} \
	va_end(args); \
	return ret; \
} \
TYPE JniClass::call##NAME##Method(const char* name, const char* sig, jobject _this, ...) \
{ \
	va_list args; \
	va_start(args, _this); \
	TYPE ret = 0; \
	if (_this) { \
		jmethodID method = getMethodID(name, sig); \
		if (method) { \
			JNIEnv* env = Jni::getCurrent(); \
			if (env) { \
				ret = env->Call##NAME##MethodV(_this, method, args); \
			} \
		} else { \
			_logJniError(String("Failed to get method id - ") + name + " " + sig); \
		} \
	} \
	va_end(args); \
	return ret; \
} \
TYPE JniClass::callStatic##NAME##Method(jmethodID method, ...) \
{ \
	va_list args; \
	va_start(args, method); \
	TYPE ret = 0; \
	if (method) { \
		jclass cls = get(); \
		JNIEnv* env = Jni::getCurrent(); \
		if (env) { \
			ret = env->CallStatic##NAME##MethodV(cls, method, args); \
		} \
	} \
	va_end(args); \
	return ret; \
} \
TYPE JniClass::callStatic##NAME##Method(const char* name, const char* sig, ...) \
{ \
	va_list args; \
	va_start(args, sig); \
	TYPE ret = 0; \
	jclass cls = get(); \
	jmethodID method = getStaticMethodID(name, sig); \
	if (method) { \
		JNIEnv* env = Jni::getCurrent(); \
		if (env) { \
			ret = env->CallStatic##NAME##MethodV(cls, method, args); \
		} \
	} else { \
		_logJniError(String("Failed to get static method id - ") + name + " " + sig); \
	} \
	va_end(args); \
	return ret; \
} \
TYPE _JniSingletonMethod::call##NAME(jobject _this, ...) \
{ \
	va_list args; \
	va_start(args, _this); \
	TYPE ret = 0; \
	if (cls && id && _this) { \
		JNIEnv* env = Jni::getCurrent(); \
		if (env) { \
			ret = env->Call##NAME##MethodV(_this, id, args); \
			if (Jni::checkException()) { \
				Jni::printException(); \
				Jni::clearException(); \
			} \
		} \
	} \
	return ret;\
} \
TYPE _JniSingletonStaticMethod::call##NAME(jobject _null, ...) \
{ \
	va_list args; \
	va_start(args, _null); \
	TYPE ret = 0; \
	if (cls && id) { \
		JNIEnv* env = Jni::getCurrent(); \
		if (env) { \
			ret = env->CallStatic##NAME##MethodV(cls, id, args); \
			if (Jni::checkException()) { \
				Jni::printException(); \
				Jni::clearException(); \
			} \
		} \
	} \
	return ret;\
}

DEFINE_JNI_CALL_METHOD(jobject, Object)
DEFINE_JNI_CALL_METHOD(jboolean, Boolean)
DEFINE_JNI_CALL_METHOD(jbyte, Byte)
DEFINE_JNI_CALL_METHOD(jchar, Char)
DEFINE_JNI_CALL_METHOD(jshort, Short)
DEFINE_JNI_CALL_METHOD(jint, Int)
DEFINE_JNI_CALL_METHOD(jlong, Long)
DEFINE_JNI_CALL_METHOD(jfloat, Float)
DEFINE_JNI_CALL_METHOD(jdouble, Double)

jobject JniClass::newObject(jmethodID method, ...)
{
	va_list args;
	va_start(args, method);
	jobject ret = sl_null;
	if (method) {
		jclass cls = get();
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			ret = env->NewObjectV(cls, method, args);
		}
	}
	va_end(args);
	return ret;
}

jobject JniClass::newObject(const char* sig, ...)
{
	va_list args;
	va_start(args, sig);
	jobject ret = sl_null;
	jclass cls = get();
	jmethodID method = getMethodID("<init>", sig);
	if (method) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			ret = env->NewObjectV(cls, method, args);
		}
	} else {
		_logJniError(String("Failed to get constructor id - <init> ") + sig);
	}
	va_end(args);
	return ret;
}

jobject JniClass::newObject()
{
	return newObject("()V");
}

jobject _JniSingletonMethod::newObject(jobject _null, ...)
{
	va_list args;
	va_start(args, _null);
	jobject ret = sl_null;
	if (cls && id) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			ret = env->NewObjectV(cls, id, args);
			if (Jni::checkException()) {
				Jni::printException();
				Jni::clearException();
			}
		}
	}
	va_end(args);
	return ret;
}

void JniClass::callVoidMethod(jmethodID method, jobject _this, ...)
{
	va_list args;
	va_start(args, _this);
	if (method && _this) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			env->CallVoidMethodV(_this, method, args);
		}
	}
	va_end(args);
}

void JniClass::callVoidMethod(const char* name, const char* sig, jobject _this, ...)
{
	va_list args;
	va_start(args, _this);
	if (_this) {
		jmethodID method = getMethodID(name, sig);
		if (method) {
			JNIEnv* env = Jni::getCurrent();
			if (env) {
				env->CallVoidMethodV(_this, method, args);
			}
		} else {
			_logJniError(String("Failed to get method id - ") + name + " " + sig);
		}
	}
	va_end(args);
}

void _JniSingletonMethod::call(jobject _this, ...)
{
	va_list args;
	va_start(args, _this);
	if (cls && id && _this) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			env->CallVoidMethodV(_this, id, args);
			if (Jni::checkException()) {
				Jni::printException();
				Jni::clearException();
			}
		}
	}
	va_end(args);
}

void JniClass::callStaticVoidMethod(jmethodID method, ...)
{
	va_list args;
	va_start(args, method);
	if (method) {
		jclass cls = get();
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			env->CallStaticVoidMethodV(cls, method, args);
		}
	}
	va_end(args);
}

void JniClass::callStaticVoidMethod(const char* name, const char* sig, ...)
{
	va_list args;
	va_start(args, sig);
	jclass cls = get();
	jmethodID method = getStaticMethodID(name, sig);
	if (method) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			env->CallStaticVoidMethodV(cls, method, args);
		}
	} else {
		_logJniError(String("Failed to get static method id - ") + name + " " + sig);
	}
	va_end(args);
}

void _JniSingletonStaticMethod::call(jobject _null, ...)
{
	va_list args;
	va_start(args, _null);
	if (cls && id) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			env->CallStaticVoidMethodV(cls, id, args);
			if (Jni::checkException()) {
				Jni::printException();
				Jni::clearException();
			}
		}
	}
	va_end(args);
}

String JniClass::callStringMethod(jmethodID method, jobject _this, ...)
{
	va_list args;
	va_start(args, _this);
	String ret;
	if (method && _this) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			JniLocal<jstring> str = (jstring)(env->CallObjectMethodV(_this, method, args));
			if (str.isNotNull()) {
				ret = Jni::getString(str);
			}
		}
	}
	va_end(args);
	return ret;
}

String16 JniClass::callString16Method(jmethodID method, jobject _this, ...)
{
	va_list args;
	va_start(args, _this);
	String16 ret;
	if (method && _this) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			JniLocal<jstring> str = (jstring)(env->CallObjectMethodV(_this, method, args));
			if (str.isNotNull()) {
				ret = Jni::getString16(str);
			}
		}
	}
	va_end(args);
	return ret;
}

String JniClass::callStringMethod(const char* name, const char* sig, jobject _this, ...)
{
	va_list args;
	va_start(args, _this);
	String ret;
	if (_this) {
		jmethodID method = getMethodID(name, sig);
		if (method) {
			JNIEnv* env = Jni::getCurrent();
			if (env) {
				JniLocal<jstring> str = (jstring)(env->CallObjectMethodV(_this, method, args));
				if (str.isNotNull()) {
					ret = Jni::getString(str);
				}
			}
		} else {
			_logJniError(String("Failed to get method id - ") + name + " " + sig);
		}
	}
	va_end(args);
	return ret;
}

String16 JniClass::callString16Method(const char* name, const char* sig, jobject _this, ...)
{
	va_list args;
	va_start(args, _this);
	String16 ret;
	if (_this) {
		jmethodID method = getMethodID(name, sig);
		if (method) {
			JNIEnv* env = Jni::getCurrent();
			if (env) {
				JniLocal<jstring> str = (jstring)(env->CallObjectMethodV(_this, method, args));
				if (str.isNotNull()) {
					ret = Jni::getString16(str);
				}
			}
		} else {
			_logJniError(String("Failed to get method id - ") + name + " " + sig);
		}
	}
	va_end(args);
	return ret;
}

String _JniSingletonMethod::callString(jobject _this, ...)
{
	va_list args;
	va_start(args, _this);
	String ret;
	if (cls && id && _this) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			JniLocal<jstring> str = (jstring)(env->CallObjectMethodV(_this, id, args));
			if (Jni::checkException()) {
				Jni::printException();
				Jni::clearException();
			} else {
				if (str.isNotNull()) {
					ret = Jni::getString(str);
				}
			}
		}
	}
	va_end(args);
	return ret;
}

String16 _JniSingletonMethod::callString16(jobject _this, ...)
{
	va_list args;
	va_start(args, _this);
	String16 ret;
	if (cls && id && _this) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			JniLocal<jstring> str = (jstring)(env->CallObjectMethodV(_this, id, args));
			if (Jni::checkException()) {
				Jni::printException();
				Jni::clearException();
			} else {
				if (str.isNotNull()) {
					ret = Jni::getString16(str);
				}
			}
		}
	}
	va_end(args);
	return ret;
}

String JniClass::callStaticStringMethod(jmethodID method, ...)
{
	va_list args;
	va_start(args, method);
	String ret;
	if (method) {
		jclass cls = get();
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			JniLocal<jstring> str = (jstring)(env->CallStaticObjectMethodV(cls, method, args));
			if (str.isNotNull()) {
				ret = Jni::getString(str);
			}
		}
	}
	va_end(args);
	return ret;
}

String16 JniClass::callStaticString16Method(jmethodID method, ...)
{
	va_list args;
	va_start(args, method);
	String16 ret;
	if (method) {
		jclass cls = get();
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			JniLocal<jstring> str = (jstring)(env->CallStaticObjectMethodV(cls, method, args));
			if (str.isNotNull()) {
				ret = Jni::getString16(str);
			}
		}
	}
	va_end(args);
	return ret;
}

String JniClass::callStaticStringMethod(const char* name, const char* sig, ...)
{
	va_list args;
	va_start(args, sig);
	String ret;
	jclass cls = get();
	jmethodID method = getStaticMethodID(name, sig);
	if (method) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			JniLocal<jstring> str = (jstring)(env->CallStaticObjectMethodV(cls, method, args));
			if (str.isNotNull()) {
				ret = Jni::getString(str);
			}
		}
	} else {
		_logJniError(String("Failed to get static method id - ") + name + " " + sig);
	}
	va_end(args);
	return ret;
}

String16 JniClass::callStaticString16Method(const char* name, const char* sig, ...)
{
	va_list args;
	va_start(args, sig);
	String16 ret;
	jclass cls = get();
	jmethodID method = getStaticMethodID(name, sig);
	if (method) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			JniLocal<jstring> str = (jstring)(env->CallStaticObjectMethodV(cls, method, args));
			if (str.isNotNull()) {
				ret = Jni::getString16(str);
			}
		}
	} else {
		_logJniError(String("Failed to get static method id - ") + name + " " + sig);
	}
	va_end(args);
	return ret;
}

String _JniSingletonStaticMethod::callString(jobject _null, ...)
{
	va_list args;
	va_start(args, _null);
	String ret;
	if (cls && id) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			JniLocal<jstring> str = (jstring)(env->CallStaticObjectMethodV(cls, id, args));
			if (Jni::checkException()) {
				Jni::printException();
				Jni::clearException();
			} else {
				if (str.isNotNull()) {
					ret = Jni::getString(str);
				}
			}
		}
	}
	va_end(args);
	return ret;
}

String16 _JniSingletonStaticMethod::callString16(jobject _null, ...)
{
	va_list args;
	va_start(args, _null);
	String16 ret;
	if (cls && id) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			JniLocal<jstring> str = (jstring)(env->CallStaticObjectMethodV(cls, id, args));
			if (Jni::checkException()) {
				Jni::printException();
				Jni::clearException();
			} else {
				if (str.isNotNull()) {
					ret = Jni::getString16(str);
				}
			}
		}
	}
	va_end(args);
	return ret;
}

#define DEFINE_JNI_FIELD(TYPE, NAME) \
TYPE JniClass::get##NAME##Field(jfieldID field, jobject _this) \
{ \
	TYPE ret = 0; \
	if (field && _this) { \
		JNIEnv* env = Jni::getCurrent(); \
		if (env) { \
			ret = env->Get##NAME##Field(_this, field); \
		} \
	} \
	return ret; \
} \
TYPE JniClass::get##NAME##Field(const char* name, const char* sig, jobject _this) \
{ \
	TYPE ret = 0; \
	if (_this) { \
		jfieldID field = getFieldID(name, sig); \
		if (field) { \
			JNIEnv* env = Jni::getCurrent(); \
			if (env) { \
				ret = env->Get##NAME##Field(_this, field); \
			} \
		} else { \
			_logJniError(String("Failed to get field id - ") + name + " " + sig); \
		} \
	} \
	return ret; \
} \
TYPE JniClass::getStatic##NAME##Field(jfieldID field) \
{ \
	TYPE ret = 0; \
	if (field) { \
		jclass cls = get(); \
		JNIEnv* env = Jni::getCurrent(); \
		if (env) { \
			ret = env->GetStatic##NAME##Field(cls, field); \
		} \
	} \
	return ret; \
} \
TYPE JniClass::getStatic##NAME##Field(const char* name, const char* sig) \
{ \
	TYPE ret = 0; \
	jclass cls = get(); \
	jfieldID field = getStaticFieldID(name, sig); \
	if (field) { \
		JNIEnv* env = Jni::getCurrent(); \
		if (env) { \
			ret = env->GetStatic##NAME##Field(cls, field); \
		} \
	} else { \
		_logJniError(String("Failed to get static field id - ") + name + " " + sig); \
	} \
	return ret; \
} \
void JniClass::set##NAME##Field(jfieldID field, jobject _this, TYPE value) \
{ \
	if (field && _this) { \
		JNIEnv* env = Jni::getCurrent(); \
		if (env) { \
			env->Set##NAME##Field(_this, field, value); \
		} \
	} \
} \
void JniClass::set##NAME##Field(const char* name, const char* sig, jobject _this, TYPE value) \
{ \
	if (_this) { \
		jfieldID field = getFieldID(name, sig); \
		if (field) { \
			JNIEnv* env = Jni::getCurrent(); \
			if (env) { \
				env->Set##NAME##Field(_this, field, value); \
			} \
		} else { \
			_logJniError(String("Failed to get field id - ") + name + " " + sig); \
		} \
	} \
} \
void JniClass::setStatic##NAME##Field(jfieldID field, TYPE value) \
{ \
	TYPE ret = sl_null; \
	if (field) { \
		jclass cls = get(); \
		JNIEnv* env = Jni::getCurrent(); \
		if (env) { \
			env->SetStatic##NAME##Field(cls, field, value); \
		} \
	} \
} \
void JniClass::setStatic##NAME##Field(const char* name, const char* sig, TYPE value) \
{ \
	TYPE ret = sl_null; \
	jclass cls = get(); \
	jfieldID field = getStaticFieldID(name, sig); \
	if (field) { \
		JNIEnv* env = Jni::getCurrent(); \
		if (env) { \
			env->SetStatic##NAME##Field(cls, field, value); \
		} \
	} else { \
		_logJniError(String("Failed to get static field id - ") + name + " " + sig); \
	} \
} \
TYPE _JniSingletonField::get##NAME(jobject _this) \
{ \
	TYPE ret = 0; \
	if (cls && id && _this) { \
		JNIEnv* env = Jni::getCurrent(); \
		if (env) { \
			ret = env->Get##NAME##Field(_this, id); \
			if (Jni::checkException()) { \
				Jni::printException(); \
				Jni::clearException(); \
			} \
		} \
	} \
	return ret; \
} \
void _JniSingletonField::set##NAME(jobject _this, TYPE value) \
{ \
	if (cls && id && _this) { \
		JNIEnv* env = Jni::getCurrent(); \
		if (env) { \
			env->Set##NAME##Field(_this, id, value); \
			if (Jni::checkException()) { \
				Jni::printException(); \
				Jni::clearException(); \
			} \
		} \
	} \
} \
TYPE _JniSingletonStaticField::get##NAME(jobject _null) \
{ \
	TYPE ret = 0; \
	if (cls && id) { \
		JNIEnv* env = Jni::getCurrent(); \
		if (env) { \
			ret = env->GetStatic##NAME##Field(cls, id); \
			if (Jni::checkException()) { \
				Jni::printException(); \
				Jni::clearException(); \
			} \
		} \
	} \
	return ret; \
} \
void _JniSingletonStaticField::set##NAME(jobject _null, TYPE value) \
{ \
	if (cls && id) { \
		JNIEnv* env = Jni::getCurrent(); \
		if (env) { \
			env->SetStatic##NAME##Field(cls, id, value); \
			if (Jni::checkException()) { \
				Jni::printException(); \
				Jni::clearException(); \
			} \
		} \
	} \
}

DEFINE_JNI_FIELD(jobject, Object)
DEFINE_JNI_FIELD(jboolean, Boolean)
DEFINE_JNI_FIELD(jbyte, Byte)
DEFINE_JNI_FIELD(jchar, Char)
DEFINE_JNI_FIELD(jshort, Short)
DEFINE_JNI_FIELD(jint, Int)
DEFINE_JNI_FIELD(jlong, Long)
DEFINE_JNI_FIELD(jfloat, Float)
DEFINE_JNI_FIELD(jdouble, Double)

String JniClass::getStringField(jfieldID field, jobject _this)
{
	String ret;
	JniLocal<jstring> str = (jstring)(getObjectField(field, _this));
	if (str.isNotNull()) {
		ret = Jni::getString(str);
	}
	return ret;
}

String16 JniClass::getString16Field(jfieldID field, jobject _this)
{
	String16 ret;
	JniLocal<jstring> str = (jstring)(getObjectField(field, _this));
	if (str.isNotNull()) {
		ret = Jni::getString16(str);
	}
	return ret;
}

String JniClass::getStringField(const char* name, const char* sig, jobject _this)
{
	String ret;
	JniLocal<jstring> str = (jstring)(getObjectField(name, sig, _this));
	if (str.isNotNull()) {
		ret = Jni::getString(str);
	}
	return ret;
}

String16 JniClass::getString16Field(const char* name, const char* sig, jobject _this)
{
	String16 ret;
	JniLocal<jstring> str = (jstring)(getObjectField(name, sig, _this));
	if (str.isNotNull()) {
		ret = Jni::getString16(str);
	}
	return ret;
}

String JniClass::getStaticStringField(jfieldID field)
{
	String ret;
	JniLocal<jstring> str = (jstring)(getStaticObjectField(field));
	if (str.isNotNull()) {
		ret = Jni::getString(str);
	}
	return ret;
}

String16 JniClass::getStaticString16Field(jfieldID field)
{
	String16 ret;
	JniLocal<jstring> str = (jstring)(getStaticObjectField(field));
	if (str.isNotNull()) {
		ret = Jni::getString16(str);
	}
	return ret;
}

String JniClass::getStaticStringField(const char* name, const char* sig)
{
	String ret;
	JniLocal<jstring> str = (jstring)(getStaticObjectField(name, sig));
	if (str.isNotNull()) {
		ret = Jni::getString(str);
	}
	return ret;
}

String16 JniClass::getStaticString16Field(const char* name, const char* sig)
{
	String16 ret;
	JniLocal<jstring> str = (jstring)(getStaticObjectField(name, sig));
	if (str.isNotNull()) {
		ret = Jni::getString16(str);
	}
	return ret;
}

void JniClass::setStringField(jfieldID field, jobject _this, const String& value)
{
	JniLocal<jstring> str = Jni::getJniString(value);
	setObjectField(field, _this, str);
}

void JniClass::setString16Field(jfieldID field, jobject _this, const String16& value)
{
	JniLocal<jstring> str = Jni::getJniString16(value);
	setObjectField(field, _this, str);
}

void JniClass::setStringField(const char* name, const char* sig, jobject _this, const String& value)
{
	JniLocal<jstring> str = Jni::getJniString(value);
	setObjectField(name, sig, _this, str);
}

void JniClass::setString16Field(const char* name, const char* sig, jobject _this, const String16& value)
{
	JniLocal<jstring> str = Jni::getJniString16(value);
	setObjectField(name, sig, _this, str);
}

void JniClass::setStaticStringField(jfieldID field, const String& value)
{
	JniLocal<jstring> str = Jni::getJniString(value);
	setStaticObjectField(field, str);
}

void JniClass::setStaticString16Field(jfieldID field, const String16& value)
{
	JniLocal<jstring> str = Jni::getJniString16(value);
	setStaticObjectField(field, str);
}

void JniClass::setStaticStringField(const char* name, const char* sig, const String& value)
{
	JniLocal<jstring> str = Jni::getJniString(value);
	setStaticObjectField(name, sig, str);
}

void JniClass::setStaticString16Field(const char* name, const char* sig, const String16& value)
{
	JniLocal<jstring> str = Jni::getJniString16(value);
	setStaticObjectField(name, sig, str);
}

String _JniSingletonField::getString(jobject _this)
{
	String ret;
	if (cls && id) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			JniLocal<jstring> str = (jstring)(env->GetObjectField(_this, id));
			if (Jni::checkException()) {
				Jni::printException();
				Jni::clearException();
			} else {
				if (str.isNotNull()) {
					ret = Jni::getString(str);
				}
			}
		}
	}
	return ret;
}

String16 _JniSingletonField::getString16(jobject _this)
{
	String16 ret;
	if (cls && id) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			JniLocal<jstring> str = (jstring)(env->GetObjectField(_this, id));
			if (Jni::checkException()) {
				Jni::printException();
				Jni::clearException();
			} else {
				if (str.isNotNull()) {
					ret = Jni::getString16(str);
				}
			}
		}
	}
	return ret;
}

String _JniSingletonStaticField::getString(jobject _null)
{
	String ret;
	if (cls && id) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			JniLocal<jstring> str = (jstring)(env->GetStaticObjectField(cls, id));
			if (Jni::checkException()) {
				Jni::printException();
				Jni::clearException();
			} else {
				if (str.isNotNull()) {
					ret = Jni::getString(str);
				}
			}
		}
	}
	return ret;
}

String16 _JniSingletonStaticField::getString16(jobject _null)
{
	String16 ret;
	if (cls && id) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			JniLocal<jstring> str = (jstring)(env->GetStaticObjectField(cls, id));
			if (Jni::checkException()) {
				Jni::printException();
				Jni::clearException();
			} else {
				if (str.isNotNull()) {
					ret = Jni::getString16(str);
				}
			}
		}
	}
	return ret;
}

void _JniSingletonField::setString(jobject _this, const String& value)
{
	if (cls && id && _this) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			JniLocal<jstring> str = Jni::getJniString(value);
			env->SetObjectField(_this, id, str);
			if (Jni::checkException()) {
				Jni::printException();
				Jni::clearException();
			}
		}
	}
}

void _JniSingletonField::setString16(jobject _this, const String16& value)
{
	if (cls && id && _this) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			JniLocal<jstring> str = Jni::getJniString16(value);
			env->SetObjectField(_this, id, str);
			if (Jni::checkException()) {
				Jni::printException();
				Jni::clearException();
			}
		}
	}
}

void _JniSingletonStaticField::setString(jobject _null, const String& value)
{
	if (cls && id) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			JniLocal<jstring> str = Jni::getJniString(value);
			env->SetObjectField(cls, id, str);
			if (Jni::checkException()) {
				Jni::printException();
				Jni::clearException();
			}
		}
	}
}

void _JniSingletonStaticField::setString16(jobject _null, const String16& value)
{
	if (cls && id) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			JniLocal<jstring> str = Jni::getJniString16(value);
			env->SetObjectField(cls, id, str);
			if (Jni::checkException()) {
				Jni::printException();
				Jni::clearException();
			}
		}
	}
}

JniClass JniClass::getClassOfObject(jobject obj)
{
	JniClass ret;
	if (obj) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			JniLocal<jclass> cls = env->GetObjectClass(obj);
			ret = JniClass::from(cls);
		}
	}
	return ret;
}

sl_bool JniClass::registerNative(const char* name, const char* sig, const void* fn)
{
	jclass cls = get();
	if (cls) {
		JNIEnv* env = Jni::getCurrent();
		if (env) {
			JNINativeMethod method;
			method.name = (char*)name;
			method.signature = (char*)sig;
			method.fnPtr = (void*)fn;

			return 0 == env->RegisterNatives(cls, &method, 1);
		}
	}
	return sl_false;
}

_JniSingletonClass::_JniSingletonClass(const char* name)
{
	this->name = name;
	_g_jni_singleton_classes.add(this);
}

_JniSingletonMethod::_JniSingletonMethod(_JniSingletonClass* gcls, const char* name, const char* sig)
{
	this->gcls = gcls;
	this->name = name;
	this->sig = sig;
	this->cls = sl_null;
	this->id = sl_null;
	_g_jni_singleton_methods.add(this);
}

_JniSingletonStaticMethod::_JniSingletonStaticMethod(_JniSingletonClass* gcls, const char* name, const char* sig)
{
	this->gcls = gcls;
	this->name = name;
	this->sig = sig;
	this->cls = sl_null;
	this->id = sl_null;
	_g_jni_singleton_static_methods.add(this);
}

_JniSingletonField::_JniSingletonField(_JniSingletonClass* gcls, const char* name, const char* sig)
{
	this->gcls = gcls;
	this->name = name;
	this->sig = sig;
	this->cls = sl_null;
	this->id = sl_null;
	_g_jni_singleton_fields.add(this);
}

_JniSingletonStaticField::_JniSingletonStaticField(_JniSingletonClass* gcls, const char* name, const char* sig)
{
	this->gcls = gcls;
	this->name = name;
	this->sig = sig;
	this->cls = sl_null;
	this->id = sl_null;
	_g_jni_singleton_static_fields.add(this);
}

_JniNativeMethod::_JniNativeMethod(_JniSingletonClass* gcls, const char* name, const char* sig, const void* fn)
{
	this->gcls = gcls;
	this->name = name;
	this->sig = sig;
	this->fn = fn;
	_g_jni_native_methods.add(this);
}
SLIB_NAMESPACE_END

#endif