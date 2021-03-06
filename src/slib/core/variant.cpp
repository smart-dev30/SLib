/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "../../../inc/slib/core/variant.h"

#include "../../../inc/slib/core/string_buffer.h"


#define PTR_VAR(TYPE, x) (reinterpret_cast<TYPE*>(&x))
#define REF_VAR(TYPE, x) (*PTR_VAR(TYPE, x))

namespace slib
{

	int Compare<Variant>::operator()(const Variant& a, const Variant& b) const
	{
		return Compare<sl_uint64>()(a._value, b._value);
	}

	const char _VariantMap_ClassID[] = "VariantMap";

	const char _VariantList_ClassID[] = "VariantList";

	const char _VariantMapList_ClassID[] = "VariantMapList";


	const _Variant_Const _Variant_Null = {0, VariantType::Null, 0};


	SLIB_INLINE void _Variant_copy(VariantType src_type, sl_uint64 src_value, sl_uint64& dst_value)
	{
		switch (src_type) {
			case VariantType::String8:
				new PTR_VAR(String, dst_value) String(REF_VAR(String, src_value));
				break;
			case VariantType::String16:
				new PTR_VAR(String16, dst_value) String16(REF_VAR(String16, src_value));
				break;
			case VariantType::Object:
			case VariantType::Weak:
				new PTR_VAR(Ref<Referable>, dst_value) Ref<Referable>(REF_VAR(Ref<Referable>, src_value));
				break;
			default:
				dst_value = src_value;
				break;
		}
	}

	SLIB_INLINE void _Variant_free(VariantType type, sl_uint64 value)
	{
		switch (type)
		{
			case VariantType::String8:
				REF_VAR(String, value).String::~String();
				break;
			case VariantType::String16:
				REF_VAR(String16, value).String16::~String16();
				break;
			case VariantType::Object:
			case VariantType::Weak:
				REF_VAR(Ref<Referable>, value).Ref<Referable>::~Ref();
				break;
			default:
				break;
		}
	}

	void Variant::_free(VariantType type, sl_uint64 value)
	{
		_Variant_free(type, value);
	}

	SLIB_INLINE void Atomic<Variant>::_retain(VariantType& type, sl_uint64& value) const
	{
		if ((void*)(this) == (void*)(&_Variant_Null)) {
			type = VariantType::Null;
			value = 0;
		} else {
			SpinLocker lock(&m_lock);
			type = _type;
			_Variant_copy(_type, _value, value);
		}
	}

	SLIB_INLINE void Atomic<Variant>::_replace(VariantType type, sl_uint64 value)
	{
		VariantType typeOld;
		sl_uint64 valueOld;
		{
			SpinLocker lock(&m_lock);
			typeOld = _type;
			valueOld = _value;
			_type = type;
			_value = value;
		}
		_Variant_free(typeOld, valueOld);
	}


	Variant::Variant(Variant&& other)
	{
		_type = other._type;
		_value = other._value;
		other._type = VariantType::Null;
	}

	Variant::Variant(const Variant& other)
	{
		_type = other._type;
		_Variant_copy(_type, other._value, _value);
	}

	Variant::Variant(AtomicVariant&& _other)
	{
		Variant& other = REF_VAR(Variant, _other);
		_type = other._type;
		_value = other._value;
		other._type = VariantType::Null;
	}

	Variant::Variant(const AtomicVariant& other)
	{
		other._retain(_type, _value);
	}

	Variant::~Variant()
	{
		_Variant_free(_type, _value);
	}

	Variant::Variant(sl_null_t): _value(0), _type(VariantType::Null)
	{
	}
	
	Variant::Variant(char value)
	{
		_type = VariantType::Int32;
		REF_VAR(sl_int32, _value) = value;
	}
	
	Variant::Variant(unsigned char value)
	{
		_type = VariantType::Uint32;
		REF_VAR(sl_uint32, _value) = value;
	}

	Variant::Variant(short value)
	{
		_type = VariantType::Int32;
		REF_VAR(sl_int32, _value) = value;
	}

	Variant::Variant(unsigned short value)
	{
		_type = VariantType::Uint32;
		REF_VAR(sl_uint32, _value) = value;
	}
	
	Variant::Variant(int value)
	{
		_type = VariantType::Int32;
		REF_VAR(sl_int32, _value) = (sl_int32)value;
	}
	
	Variant::Variant(unsigned int value)
	{
		_type = VariantType::Uint32;
		REF_VAR(sl_uint32, _value) = (sl_uint32)value;
	}
	
	Variant::Variant(long value)
	{
		_type = VariantType::Int32;
		REF_VAR(sl_int32, _value) = (sl_int32)value;
	}
	
	Variant::Variant(unsigned long value)
	{
		_type = VariantType::Uint32;
		REF_VAR(sl_uint32, _value) = (sl_uint32)value;
	}

	Variant::Variant(sl_int64 value)
	{
		_type = VariantType::Int64;
		REF_VAR(sl_int64, _value) = value;
	}

	Variant::Variant(sl_uint64 value)
	{
		_type = VariantType::Uint64;
		REF_VAR(sl_uint64, _value) = value;
	}

	Variant::Variant(float value)
	{
		_type = VariantType::Float;
		REF_VAR(float, _value) = value;
	}

	Variant::Variant(double value)
	{
		_type = VariantType::Double;
		REF_VAR(double, _value) = value;
	}

	Variant::Variant(sl_bool value)
	{
		_type = VariantType::Boolean;
		REF_VAR(sl_bool, _value) = value;
	}

	Variant::Variant(const String& value)
	{
		if (value.isNotNull()) {
			_type = VariantType::String8;
			new PTR_VAR(String, _value) String(value);
		} else {
			_type = VariantType::Null;
		}
	}

	Variant::Variant(const String16& value)
	{
		if (value.isNotNull()) {
			_type = VariantType::String16;
			new PTR_VAR(String16, _value) String16(value);
		} else {
			_type = VariantType::Null;
		}
	}

	Variant::Variant(const AtomicString& s)
	{
		String value(s);
		if (value.isNotNull()) {
			_type = VariantType::String8;
			new PTR_VAR(String, _value) String(value);
		} else {
			_type = VariantType::Null;
		}
	}

	Variant::Variant(const AtomicString16& s)
	{
		String16 value(s);
		if (value.isNotNull()) {
			_type = VariantType::String16;
			new PTR_VAR(String16, _value) String16(value);
		} else {
			_type = VariantType::Null;
		}
	}

	Variant::Variant(const sl_char8* sz8)
	{
		if (sz8) {
			_type = VariantType::Sz8;
			REF_VAR(const sl_char8*, _value) = sz8;
		} else {
			_type = VariantType::Null;
		}
	}

	Variant::Variant(const sl_char16* sz16)
	{
		if (sz16) {
			_type = VariantType::Sz16;
			REF_VAR(const sl_char16*, _value) = sz16;
		} else {
			_type = VariantType::Null;
		}
	}

	Variant::Variant(const Time& value)
	{
		_type = VariantType::Time;
		REF_VAR(Time, _value) = value;
	}

	Variant::Variant(const void* ptr)
	{
		if (ptr) {
			_type = VariantType::Pointer;
			REF_VAR(const void*, _value) = ptr;
		} else {
			_type = VariantType::Null;
		}
	}

	Variant::Variant(const Memory& mem)
	{
		if (mem.isNotNull()) {
			_type = VariantType::Object;
			new PTR_VAR(Memory, _value) Memory(mem);
		} else {
			_type = VariantType::Null;
		}
	}

	Variant::Variant(const AtomicMemory& _mem)
	{
		Memory mem(_mem);
		if (mem.isNotNull()) {
			_type = VariantType::Object;
			new PTR_VAR(Memory, _value) Memory(mem);
		} else {
			_type = VariantType::Null;
		}
	}
	
	Variant::Variant(const VariantList& list)
	{
		if (list.isNotNull()) {
			_type = VariantType::Object;
			new PTR_VAR(VariantList, _value) VariantList(list);
		} else {
			_type = VariantType::Null;
		}
	}
	
	Variant::Variant(const AtomicVariantList& list)
	{
		if (list.isNotNull()) {
			_type = VariantType::Object;
			new PTR_VAR(VariantList, _value) VariantList(list);
		} else {
			_type = VariantType::Null;
		}
	}
	
	Variant::Variant(const VariantMap& list)
	{
		if (list.isNotNull()) {
			_type = VariantType::Object;
			new PTR_VAR(VariantMap, _value) VariantMap(list);
		} else {
			_type = VariantType::Null;
		}
	}
	
	Variant::Variant(const AtomicVariantMap& list)
	{
		if (list.isNotNull()) {
			_type = VariantType::Object;
			new PTR_VAR(VariantMap, _value) VariantMap(list);
		} else {
			_type = VariantType::Null;
		}
	}
	
	Variant::Variant(const VariantMapList& list)
	{
		if (list.isNotNull()) {
			_type = VariantType::Object;
			new PTR_VAR(VariantMapList, _value) VariantMapList(list);
		} else {
			_type = VariantType::Null;
		}
	}
	
	Variant::Variant(const AtomicVariantMapList& list)
	{
		if (list.isNotNull()) {
			_type = VariantType::Object;
			new PTR_VAR(VariantMapList, _value) VariantMapList(list);
		} else {
			_type = VariantType::Null;
		}
	}
		
	Variant Variant::createList()
	{
		return List<Variant>::create();
	}
	
	Variant Variant::createMap()
	{
		return Map<String, Variant>::createHash();
	}
	
	Variant Variant::createListMap()
	{
		return Map<String, Variant>::createList();
	}
	
	Variant Variant::createTreeMap()
	{
		return Map<String, Variant>::createTree();
	}
	
	Variant Variant::createHashMap()
	{
		return Map<String, Variant>::createHash();
	}
	
	Variant Variant::createMapList()
	{
		return List< Map<String, Variant> >::create();
	}
	
	Variant Variant::fromInt32(sl_int32 value)
	{
		return value;
	}

	Variant Variant::fromUint32(sl_uint32 value)
	{
		return value;
	}

	Variant Variant::fromInt64(sl_int64 value)
	{
		return value;
	}

	Variant Variant::fromUint64(sl_uint64 value)
	{
		return value;
	}

	Variant Variant::fromFloat(float value)
	{
		return value;
	}

	Variant Variant::fromDouble(double value)
	{
		return value;
	}

	Variant Variant::fromBoolean(sl_bool value)
	{
		return value;
	}

	Variant Variant::fromString(const String& value)
	{
		return value;
	}

	Variant Variant::fromString16(const String16& value)
	{
		return value;
	}

	Variant Variant::fromSz8(const sl_char8* value)
	{
		return value;
	}

	Variant Variant::fromSz16(const sl_char16* value)
	{
		return value;
	}

	Variant Variant::fromTime(const Time& value)
	{
		return value;
	}

	Variant Variant::fromPointer(const void* value)
	{
		return value;
	}

	Variant Variant::fromMemory(const Memory& mem)
	{
		return mem;
	}

	Variant Variant::fromVariantList(const VariantList& value)
	{
		return value;
	}

	Variant Variant::fromVariantMap(const VariantMap& value)
	{
		return value;
	}
	
	Variant Variant::fromVariantMapList(const VariantMapList& value)
	{
		return value;
	}


	Variant& Variant::operator=(Variant&& other)
	{
		if (this != &other) {
			_Variant_free(_type, _value);
			_type = other._type;
			_value = other._value;
			other._type = VariantType::Null;
		}
		return *this;
	}

	Variant& Variant::operator=(const Variant& other)
	{
		if (this != &other) {
			_Variant_free(_type, _value);
			_type = other._type;
			_Variant_copy(_type, other._value, _value);
		}
		return *this;
	}

	Variant& Variant::operator=(AtomicVariant&& other)
	{
		if ((void*)this != (void*)(&other)) {
			_Variant_free(_type, _value);
			_type = other._type;
			_value = other._value;
			other._type = VariantType::Null;
		}
		return *this;
	}

	Variant& Variant::operator=(const AtomicVariant& other)
	{
		_Variant_free(_type, _value);
		other._retain(_type, _value);
		return *this;
	}

	Variant& Variant::operator=(sl_null_t)
	{
		setNull();
		return *this;
	}
	
	Variant Variant::operator[](sl_size indexForVariantList) const
	{
		return getElement(indexForVariantList);
	}

	Variant Variant::operator[](const String& keyForVariantMap) const
	{
		return getItem(keyForVariantMap);
	}

	void Variant::setNull()
	{
		if (_type != VariantType::Null) {
			_Variant_free(_type, _value);
			_type = VariantType::Null;
		}
	}

	sl_bool Variant::isInt32() const
	{
		return _type == VariantType::Int32;
	}

	sl_int32 Variant::getInt32(sl_int32 def) const
	{
		switch (_type) {
			case VariantType::Int32:
				return (sl_int32)(REF_VAR(sl_int32 const, _value));
			case VariantType::Uint32:
				return (sl_int32)(REF_VAR(sl_uint32 const, _value));
			case VariantType::Int64:
				return (sl_int32)(REF_VAR(sl_int64 const, _value));
			case VariantType::Uint64:
				return (sl_int32)(REF_VAR(sl_uint64 const, _value));
			case VariantType::Float:
				return (sl_int32)(REF_VAR(float const, _value));
			case VariantType::Double:
				return (sl_int32)(REF_VAR(double const, _value));
			case VariantType::Boolean:
				return (REF_VAR(sl_bool const, _value)) ? 1 : 0;
			case VariantType::String8:
				return REF_VAR(String const, _value).parseInt32(10, def);
			case VariantType::String16:
				return REF_VAR(String16 const, _value).parseInt32(10, def);
			case VariantType::Pointer:
				return (sl_int32)(REF_VAR(const sl_size, _value));
			case VariantType::Sz8:
			{
				sl_int32 ret;
				const sl_char8* str = REF_VAR(sl_char8 const* const , _value);
				sl_reg pos = String::parseInt32(10, &ret, str);
				if (pos != SLIB_PARSE_ERROR && str[pos] == 0) {
					return ret;
				}
				break;
			}
			case VariantType::Sz16:
			{
				sl_int32 ret;
				const sl_char16* str = REF_VAR(sl_char16 const* const , _value);
				sl_reg pos = String16::parseInt32(10, &ret, str);
				if (pos != SLIB_PARSE_ERROR && str[pos] == 0) {
					return ret;
				}
				break;
			}
			default:
				break;
		}
		return def;
	}

	void Variant::setInt32(sl_int32 value)
	{
		_Variant_free(_type, _value);
		_type = VariantType::Int32;
		REF_VAR(sl_int32, _value) = value;
	}

	sl_bool Variant::isUint32() const
	{
		return _type == VariantType::Uint32;
	}

	sl_uint32 Variant::getUint32(sl_uint32 def) const
	{
		switch (_type) {
			case VariantType::Int32:
				return (sl_uint32)(REF_VAR(sl_int32 const, _value));
			case VariantType::Uint32:
				return (sl_uint32)(REF_VAR(sl_uint32 const, _value));
			case VariantType::Int64:
				return (sl_uint32)(REF_VAR(sl_int64 const, _value));
			case VariantType::Uint64:
				return (sl_uint32)(REF_VAR(sl_uint64 const, _value));
			case VariantType::Float:
				return (sl_uint32)(REF_VAR(float const, _value));
			case VariantType::Double:
				return (sl_uint32)(REF_VAR(double const, _value));
			case VariantType::Boolean:
				return (REF_VAR(sl_bool const, _value)) ? 1 : 0;
			case VariantType::String8:
				return REF_VAR(String const, _value).parseUint32(10, def);
			case VariantType::String16:
				return REF_VAR(String16 const, _value).parseUint32(10, def);
			case VariantType::Pointer:
				return (sl_uint32)(REF_VAR(sl_size const, _value));
			case VariantType::Sz8:
			{
				sl_uint32 ret;
				const sl_char8* str = REF_VAR(sl_char8 const* const, _value);
				sl_reg pos = String::parseUint32(10, &ret, str);
				if (pos != SLIB_PARSE_ERROR && str[pos] == 0) {
					return ret;
				}
				break;
			}
			case VariantType::Sz16:
			{
				sl_uint32 ret;
				const sl_char16* str = REF_VAR(sl_char16 const* const, _value);
				sl_reg pos = String16::parseUint32(10, &ret, str);
				if (pos != SLIB_PARSE_ERROR && str[pos] == 0) {
					return ret;
				}
				break;
			}
			default:
				break;
		}
		return def;
	}

	void Variant::setUint32(sl_uint32 value)
	{
		_Variant_free(_type, _value);
		_type = VariantType::Uint32;
		REF_VAR(sl_uint32, _value) = value;
	}

	sl_bool Variant::isInt64() const
	{
		return _type == VariantType::Int64;
	}

	sl_int64 Variant::getInt64(sl_int64 def) const
	{
		switch (_type) {
			case VariantType::Int32:
				return (sl_int64)(REF_VAR(sl_int32 const, _value));
			case VariantType::Uint32:
				return (sl_int64)(REF_VAR(sl_uint32 const, _value));
			case VariantType::Int64:
				return (sl_int64)(REF_VAR(sl_int64 const, _value));
			case VariantType::Uint64:
				return (sl_int64)(REF_VAR(sl_uint64 const, _value));
			case VariantType::Float:
				return (sl_int64)(REF_VAR(float const, _value));
			case VariantType::Double:
				return (sl_int64)(REF_VAR(double const, _value));
			case VariantType::Boolean:
				return (REF_VAR(sl_bool const, _value)) ? 1 : 0;
			case VariantType::String8:
				return (REF_VAR(String const, _value)).parseInt64(10, def);
			case VariantType::String16:
				return (REF_VAR(String16 const, _value)).parseInt64(10, def);
			case VariantType::Pointer:
				return (sl_int64)(REF_VAR(sl_size const, _value));
			case VariantType::Sz8:
			{
				sl_int64 ret;
				const sl_char8* str = REF_VAR(sl_char8 const* const, _value);
				sl_reg pos = String::parseInt64(10, &ret, str);
				if (pos != SLIB_PARSE_ERROR && str[pos] == 0) {
					return ret;
				}
				break;
			}
			case VariantType::Sz16:
			{
				sl_int64 ret;
				const sl_char16* str = REF_VAR(sl_char16 const* const, _value);
				sl_reg pos = String16::parseInt64(10, &ret, str);
				if (pos != SLIB_PARSE_ERROR && str[pos] == 0) {
					return ret;
				}
				break;
			}
			default:
				break;
		}
		return def;
	}

	void Variant::setInt64(sl_int64 value)
	{
		_Variant_free(_type, _value);
		_type = VariantType::Int64;
		REF_VAR(sl_int64, _value) = value;
	}

	sl_bool Variant::isUint64() const
	{
		return _type == VariantType::Uint64;
	}

	sl_uint64 Variant::getUint64(sl_uint64 def) const
	{
		switch (_type) {
			case VariantType::Int32:
				return (sl_uint64)(REF_VAR(sl_int32 const, _value));
			case VariantType::Uint32:
				return (sl_uint64)(REF_VAR(sl_uint32 const, _value));
			case VariantType::Int64:
				return (sl_uint64)(REF_VAR(sl_int64 const, _value));
			case VariantType::Uint64:
				return (sl_uint64)(REF_VAR(sl_uint64 const, _value));
			case VariantType::Float:
				return (sl_uint64)(REF_VAR(float const, _value));
			case VariantType::Double:
				return (sl_uint64)(REF_VAR(double const, _value));
			case VariantType::Boolean:
				return (REF_VAR(sl_bool const, _value)) ? 1 : 0;
			case VariantType::String8:
				return (REF_VAR(String const, _value)).parseUint64(10, def);
			case VariantType::String16:
				return (REF_VAR(String16 const, _value)).parseUint64(10, def);
			case VariantType::Pointer:
				return (sl_uint64)(REF_VAR(sl_size const, _value));
			case VariantType::Sz8:
			{
				sl_uint64 ret;
				const sl_char8* str = REF_VAR(sl_char8 const* const, _value);
				sl_reg pos = String::parseUint64(10, &ret, str);
				if (pos != SLIB_PARSE_ERROR && str[pos] == 0) {
					return ret;
				}
				break;
			}
			case VariantType::Sz16:
			{
				sl_uint64 ret;
				const sl_char16* str = REF_VAR(sl_char16 const* const, _value);
				sl_reg pos = String16::parseUint64(10, &ret, str);
				if (pos != SLIB_PARSE_ERROR && str[pos] == 0) {
					return ret;
				}
				break;
			}
			default:
				break;
		}
		return def;
	}

	void Variant::setUint64(sl_uint64 value)
	{
		_Variant_free(_type, _value);
		_type = VariantType::Uint64;
		REF_VAR(sl_uint64, _value) = value;
	}

	sl_bool Variant::isInteger() const
	{
		return _type == VariantType::Int32 || _type == VariantType::Uint32 || _type == VariantType::Int64 || _type == VariantType::Uint64;
	}

	sl_bool Variant::isSignedInteger() const
	{
		return _type == VariantType::Int32 || _type == VariantType::Int64;
	}

	sl_bool Variant::isUnsignedInteger() const
	{
		return _type == VariantType::Uint32 || _type == VariantType::Uint64;
	}

	sl_bool Variant::isFloat() const
	{
		return _type == VariantType::Float;
	}

	float Variant::getFloat(float def) const
	{
		switch (_type) {
			case VariantType::Int32:
				return (float)(REF_VAR(sl_int32 const, _value));
			case VariantType::Uint32:
				return (float)(REF_VAR(sl_uint32 const, _value));
			case VariantType::Int64:
				return (float)(REF_VAR(sl_int64 const, _value));
			case VariantType::Uint64:
				return (float)(REF_VAR(sl_uint64 const, _value));
			case VariantType::Float:
				return (float)(REF_VAR(float const, _value));
			case VariantType::Double:
				return (float)(REF_VAR(double const, _value));
			case VariantType::String8:
				return (REF_VAR(String const, _value)).parseFloat(def);
			case VariantType::String16:
				return (REF_VAR(String16 const, _value)).parseFloat(def);
			case VariantType::Sz8:
			{
				float ret;
				const sl_char8* str = REF_VAR(sl_char8 const* const, _value);
				sl_reg pos = String::parseFloat(&ret, str);
				if (pos != SLIB_PARSE_ERROR && str[pos] == 0) {
					return ret;
				}
				break;
			}
			case VariantType::Sz16:
			{
				float ret;
				const sl_char16* str = REF_VAR(sl_char16 const* const, _value);
				sl_reg pos = String16::parseFloat(&ret, str);
				if (pos != SLIB_PARSE_ERROR && str[pos] == 0) {
					return ret;
				}
				break;
			}
			default:
				break;
		}
		return def;
	}

	void Variant::setFloat(float value)
	{
		_Variant_free(_type, _value);
		_type = VariantType::Float;
		REF_VAR(float, _value) = value;
	}

	sl_bool Variant::isDouble() const
	{
		return _type == VariantType::Double;
	}

	double Variant::getDouble(double def) const
	{
		switch (_type) {
			case VariantType::Int32:
				return (double)(REF_VAR(sl_int32 const, _value));
			case VariantType::Uint32:
				return (double)(REF_VAR(sl_uint32 const, _value));
			case VariantType::Int64:
				return (double)(REF_VAR(sl_int64 const, _value));
			case VariantType::Uint64:
				return (double)(REF_VAR(sl_uint64 const, _value));
			case VariantType::Float:
				return (double)(REF_VAR(float const, _value));
			case VariantType::Double:
				return (double)(REF_VAR(double const, _value));
			case VariantType::String8:
				return (REF_VAR(String const, _value)).parseDouble(def);
			case VariantType::String16:
				return (REF_VAR(String16 const, _value)).parseDouble(def);
			case VariantType::Sz8:
			{
				double ret;
				const sl_char8* str = REF_VAR(sl_char8 const* const, _value);
				sl_reg pos = String::parseDouble(&ret, str);
				if (pos != SLIB_PARSE_ERROR && str[pos] == 0) {
					return ret;
				}
				break;
			}
			case VariantType::Sz16:
			{
				double ret;
				const sl_char16* str = REF_VAR(sl_char16 const* const, _value);
				sl_reg pos = String16::parseDouble(&ret, str);
				if (pos != SLIB_PARSE_ERROR && str[pos] == 0) {
					return ret;
				}
				break;
			}
			default:
				break;
		}
		return def;
	}

	void Variant::setDouble(double value)
	{
		_Variant_free(_type, _value);
		_type = VariantType::Double;
		REF_VAR(double, _value) = value;
	}

	sl_bool Variant::isNumber() const
	{
		return isInteger() || _type == VariantType::Float || _type == VariantType::Double;
	}

	sl_bool Variant::isBoolean() const
	{
		return _type == VariantType::Boolean;
	}

	sl_bool Variant::getBoolean(sl_bool def) const
	{
		switch (_type) {
			case VariantType::Int32:
			{
				sl_int32 n = REF_VAR(sl_int32 const, _value);
				if (n != 0) {
					return sl_true;
				} else {
					return sl_false;
				}
			}
			case VariantType::Uint32:
			{
				sl_uint32 n = REF_VAR(sl_uint32 const, _value);
				if (n != 0) {
					return sl_true;
				} else {
					return sl_false;
				}
			}
			case VariantType::Int64:
			{
				sl_int64 n = REF_VAR(sl_int64 const, _value);
				if (n != 0) {
					return sl_true;
				} else {
					return sl_false;
				}
			}
			case VariantType::Uint64:
			{
				sl_uint64 n = REF_VAR(sl_uint64 const, _value);
				if (n != 0) {
					return sl_true;
				} else {
					return sl_false;
				}
			}
			case VariantType::Boolean:
				return REF_VAR(sl_bool const, _value);
			case VariantType::String8:
				return (REF_VAR(String const, _value)).parseBoolean(def);
			case VariantType::String16:
				return (REF_VAR(String16 const, _value)).parseBoolean(def);
			case VariantType::Sz8:
			{
				sl_bool ret;
				const sl_char8* str = REF_VAR(sl_char8 const* const, _value);
				sl_reg pos = String::parseBoolean(&ret, str);
				if (pos != SLIB_PARSE_ERROR && str[pos] == 0) {
					return ret;
				}
				break;
			}
			case VariantType::Sz16:
			{
				sl_bool ret;
				const sl_char16* str = REF_VAR(sl_char16 const* const, _value);
				sl_reg pos = String16::parseBoolean(&ret, str);
				if (pos != SLIB_PARSE_ERROR && str[pos] == 0) {
					return ret;
				}
				break;
			}
			default:
				break;
		}
		return def;
	}

	void Variant::setBoolean(sl_bool value)
	{
		_Variant_free(_type, _value);
		_type = VariantType::Boolean;
		REF_VAR(sl_bool, _value) = value;
	}

	sl_bool Variant::isString() const
	{
		return _type == VariantType::String8 || _type == VariantType::String16 || _type == VariantType::Sz8 || _type == VariantType::Sz16;
	}

	sl_bool Variant::isString8() const
	{
		return _type == VariantType::String8;
	}

	sl_bool Variant::isString16() const
	{
		return _type == VariantType::String16;
	}

	sl_bool Variant::isSz8() const
	{
		return _type == VariantType::Sz8;
	}

	sl_bool Variant::isSz16() const
	{
		return _type == VariantType::Sz16;
	}

	String Variant::getString(const String& def) const
	{
		switch (_type) {
			case VariantType::Int32:
				return String::fromInt32(REF_VAR(sl_int32 const, _value));
			case VariantType::Uint32:
				return String::fromUint32(REF_VAR(sl_uint32 const, _value));
			case VariantType::Int64:
				return String::fromInt64(REF_VAR(sl_int64 const, _value));
			case VariantType::Uint64:
				return String::fromUint64(REF_VAR(sl_uint64 const, _value));
			case VariantType::Float:
				return String::fromFloat(REF_VAR(float const, _value));
			case VariantType::Double:
				return String::fromDouble(REF_VAR(double const, _value));
			case VariantType::Boolean:
				if (REF_VAR(sl_bool const, _value)) {
					SLIB_STATIC_STRING_BY_ARRAY(ret, 't', 'r', 'u', 'e');
					return ret;
				} else {
					SLIB_STATIC_STRING_BY_ARRAY(ret, 'f', 'a', 'l', 's', 'e');
					return ret;
				}
			case VariantType::Time:
				return REF_VAR(Time const, _value).toString();
			case VariantType::String8:
				return REF_VAR(String const, _value);
			case VariantType::String16:
				return REF_VAR(String16 const, _value);
			case VariantType::Sz8:
				return REF_VAR(sl_char8 const* const, _value);
			case VariantType::Sz16:
				return REF_VAR(sl_char16 const* const, _value);
			case VariantType::Pointer:
				return "#" + String::fromPointerValue(REF_VAR(void const* const, _value));
			default:
				break;
		}
		return def;
	}

	String Variant::getString() const
	{
		return getString(String::null());
	}

	String16 Variant::getString16(const String16& def) const
	{
		switch (_type) {
			case VariantType::Int32:
				return String16::fromInt32(REF_VAR(sl_int32 const, _value));
			case VariantType::Uint32:
				return String16::fromUint32(REF_VAR(sl_uint32 const, _value));
			case VariantType::Int64:
				return String16::fromInt64(REF_VAR(sl_int64 const, _value));
			case VariantType::Uint64:
				return String16::fromUint64(REF_VAR(sl_uint64 const, _value));
			case VariantType::Float:
				return String16::fromFloat(REF_VAR(float const, _value));
			case VariantType::Double:
				return String16::fromDouble(REF_VAR(double const, _value));
			case VariantType::Boolean:
				if (REF_VAR(sl_bool const, _value)) {
					SLIB_STATIC_STRING16_BY_ARRAY(ret, 't', 'r', 'u', 'e');
					return ret;
				} else {
					SLIB_STATIC_STRING16_BY_ARRAY(ret, 'f', 'a', 'l', 's', 'e');
					return ret;
				}
			case VariantType::Time:
				return REF_VAR(Time const, _value).toString();
			case VariantType::String8:
				return REF_VAR(String const, _value);
			case VariantType::String16:
				return REF_VAR(String16 const, _value);
			case VariantType::Sz8:
				return REF_VAR(sl_char8 const* const, _value);
			case VariantType::Sz16:
				return REF_VAR(sl_char16 const* const, _value);
			case VariantType::Pointer:
				return "#" + String16::fromPointerValue(REF_VAR(void const* const, _value));
			default:
				break;
		}
		return def;
	}

	String16 Variant::getString16() const
	{
		return getString16(String16::null());
	}

	const sl_char8* Variant::getSz8(const sl_char8* def) const
	{
		switch (_type) {
			case VariantType::Boolean:
				if (REF_VAR(sl_bool const, _value)) {
					return "true";
				} else {
					return "false";
				}
			case VariantType::String8:
				return REF_VAR(String const, _value).getData();
			case VariantType::Sz8:
				return REF_VAR(sl_char8 const* const, _value);
			default:
				break;
		}
		return def;
	}

	const sl_char16* Variant::getSz16(const sl_char16* def) const
	{
		switch (_type) {
			case VariantType::Boolean:
				if (REF_VAR(sl_bool const, _value)) {
					static const sl_char16 _s[] = {'t', 'r', 'u', 'e', 0};
					return _s;
				} else {
					static const sl_char16 _s[] = {'f', 'a', 'l', 's', 'e', 0};
					return _s;
				}
			case VariantType::String16:
				return REF_VAR(String16 const, _value).getData();
			case VariantType::Sz16:
				return REF_VAR(sl_char16 const* const, _value);
			default:
				break;
		}
		return def;
	}

	void Variant::setString(const String& value)
	{
		_Variant_free(_type, _value);
		if (value.isNotNull()) {
			_type = VariantType::String8;
			new PTR_VAR(String, _value) String(value);
		} else {
			_type = VariantType::Null;
		}
	}

	void Variant::setString(const String16& value)
	{
		_Variant_free(_type, _value);
		if (value.isNotNull()) {
			_type = VariantType::String16;
			new PTR_VAR(String16, _value) String16(value);
		} else {
			_type = VariantType::Null;
		}
	}

	void Variant::setString(const AtomicString& s)
	{
		_Variant_free(_type, _value);
		String value(s);
		if (value.isNotNull()) {
			_type = VariantType::String8;
			new PTR_VAR(String, _value) String(value);
		} else {
			_type = VariantType::Null;
		}
	}

	void Variant::setString(const AtomicString16& s)
	{
		_Variant_free(_type, _value);
		String value(s);
		if (value.isNotNull()) {
			_type = VariantType::String16;
			new PTR_VAR(String16, _value) String16(value);
		} else {
			_type = VariantType::Null;
		}
	}

	void Variant::setString(const sl_char8* value)
	{
		_Variant_free(_type, _value);
		if (value) {
			_type = VariantType::Sz8;
			REF_VAR(const sl_char8*, _value) = value;
		} else {
			_type = VariantType::Null;
		}
	}

	void Variant::setString(const sl_char16* value)
	{
		_Variant_free(_type, _value);
		if (value) {
			_type = VariantType::Sz16;
			REF_VAR(const sl_char16*, _value) = value;
		} else {
			_type = VariantType::Null;
		}
	}

	sl_bool Variant::isTime() const
	{
		return _type == VariantType::Time;
	}

	Time Variant::getTime(const Time& def) const
	{
		switch (_type) {
			case VariantType::Time:
				return REF_VAR(Time const, _value);
			case VariantType::String8:
				return Time(REF_VAR(String const, _value));
			case VariantType::String16:
				return Time(REF_VAR(String16 const, _value));
			case VariantType::Sz8:
				return Time(REF_VAR(sl_char8 const* const, _value));
			case VariantType::Sz16:
				return Time(REF_VAR(sl_char16 const* const, _value));
			default:
				break;
		}
		return def;
	}

	Time Variant::getTime() const
	{
		return getTime(Time::zero());
	}

	void Variant::setTime(const Time& value)
	{
		_Variant_free(_type, _value);
		_type = VariantType::Time;
		REF_VAR(Time, _value) = value;
	}

	sl_bool Variant::isPointer() const
	{
		return _type == VariantType::Pointer || _type == VariantType::Sz8 || _type == VariantType::Sz16;
	}

	void* Variant::getPointer(const void* def) const
	{
		if (_type == VariantType::Pointer || _type == VariantType::Sz8 || _type == VariantType::Sz16) {
			return REF_VAR(void* const, _value);
		}
		return (void*)def;
	}

	void Variant::setPointer(const void *ptr)
	{
		_Variant_free(_type, _value);
		if (ptr) {
			_type = VariantType::Pointer;
			REF_VAR(const void*, _value) = ptr;
		} else {
			_type = VariantType::Null;
		}
	}

	sl_bool Variant::isObject() const
	{
		return _type == VariantType::Object || _type == VariantType::Weak;
	}

	sl_bool Variant::isWeak() const
	{
		return _type == VariantType::Weak;
	}

	Ref<Referable> Variant::getObject() const
	{
		if (_type == VariantType::Object) {
			return REF_VAR(Ref<Referable> const, _value);
		} else if (_type == VariantType::Weak) {
			return REF_VAR(WeakRef<Referable> const, _value);
		}
		return sl_null;
	}

	sl_object_type Variant::getObjectType() const
	{
		Ref<Referable> obj(getObject());
		if (obj.isNotNull()) {
			return obj->getObjectType();
		}
		return 0;
	}

	sl_bool Variant::isObjectNotNull() const
	{
		if (_type == VariantType::Object || _type == VariantType::Weak) {
			return REF_VAR(Ref<Referable> const, _value).isNotNull();
		}
		return sl_false;
	}

	sl_bool Variant::isObjectNull() const
	{
		if (_type == VariantType::Object || _type == VariantType::Weak) {
			return REF_VAR(Ref<Referable> const, _value).isNull();
		}
		return sl_true;
	}

	sl_bool Variant::isMemory() const
	{
		return IsInstanceOf<CMemory>(getObject());
	}

	Memory Variant::getMemory() const
	{
		Ref<Referable> obj(getObject());
		if (CMemory* p = CastInstance<CMemory>(obj._ptr)) {
			return p;
		}
		return sl_null;
	}

	void Variant::setMemory(const Memory& mem)
	{
		_Variant_free(_type, _value);
		if (mem.isNotNull()) {
			_type = VariantType::Object;
			new PTR_VAR(Memory, _value) Memory(mem);
		} else {
			_type = VariantType::Null;
		}
	}

	sl_bool Variant::isVariantList() const
	{
		return IsInstanceOf< CList<Variant> >(getObject());
	}

	VariantList Variant::getVariantList() const
	{
		Ref<Referable> obj(getObject());
		if (CList<Variant>* p = CastInstance< CList<Variant> >(obj._ptr)) {
			return p;
		}
		return sl_null;
	}
	
	void Variant::setVariantList(const VariantList& list)
	{
		_Variant_free(_type, _value);
		if (list.isNotNull()) {
			_type = VariantType::Object;
			new PTR_VAR(VariantList, _value) VariantList(list);
		} else {
			_type = VariantType::Null;
		}
	}

	sl_bool Variant::isVariantMap() const
	{
		return IsInstanceOf< IMap<String, Variant> >(getObject());
	}

	VariantMap Variant::getVariantMap() const
	{
		Ref<Referable> obj(getObject());
		if (IMap<String, Variant>* p = CastInstance< IMap<String, Variant> >(obj._ptr)) {
			return p;
		}
		return sl_null;
	}
	
	void Variant::setVariantMap(const VariantMap& map)
	{
		_Variant_free(_type, _value);
		if (map.isNotNull()) {
			_type = VariantType::Object;
			new PTR_VAR(VariantMap, _value) VariantMap(map);
		} else {
			_type = VariantType::Null;
		}
	}

	sl_bool Variant::isVariantMapList() const
	{
		return IsInstanceOf< CList< Map<String, Variant> > >(getObject());
	}

	VariantMapList Variant::getVariantMapList() const
	{
		Ref<Referable> obj(getObject());
		if (CList< Map<String, Variant> >* p = CastInstance< CList< Map<String, Variant> > >(obj._ptr)) {
			return p;
		}
		return sl_null;
	}
	
	void Variant::setVariantMapList(const VariantMapList& list)
	{
		_Variant_free(_type, _value);
		if (list.isNotNull()) {
			_type = VariantType::Object;
			new PTR_VAR(VariantMapList, _value) VariantMapList(list);
		} else {
			_type = VariantType::Null;
		}
	}

	sl_size Variant::getElementsCount() const
	{
		Ref<Referable> obj(getObject());
		if (CList<Variant>* p1 = CastInstance< CList<Variant> >(obj._ptr)) {
			return p1->getCount();
		} else if (CList< Map<String, Variant> >* p2 = CastInstance< CList< Map<String, Variant> > >(obj._ptr)) {
			return p2->getCount();
		}
		return 0;
	}

	Variant Variant::getElement(sl_size index) const
	{
		Ref<Referable> obj(getObject());
		if (CList<Variant>* p1 = CastInstance< CList<Variant> >(obj._ptr)) {
			return p1->getValueAt(index);
		} else if (CList< Map<String, Variant> >* p2 = CastInstance< CList< Map<String, Variant> > >(obj._ptr)) {
			return p2->getValueAt(index);
		}
		return sl_null;
	}

	sl_bool Variant::setElement(sl_size index, const Variant& value)
	{
		Ref<Referable> obj(getObject());
		if (CList<Variant>* p = CastInstance< CList<Variant> >(obj._ptr)) {
			return p->setAt(index, value);
		}
		return sl_false;
	}

	sl_bool Variant::addElement(const Variant& value)
	{
		Ref<Referable> obj(getObject());
		if (CList<Variant>* p = CastInstance< CList<Variant> >(obj._ptr)) {
			return p->add(value);
		}
		return sl_false;
	}

	Variant Variant::getItem(const String& key) const
	{
		Ref<Referable> obj(getObject());
		if (IMap<String, Variant>* p = CastInstance< IMap<String, Variant> >(obj._ptr)) {
			return p->getValue(key);
		}
		return sl_null;
	}

	sl_bool Variant::putItem(const String& key, const Variant& value)
	{
		Ref<Referable> obj(getObject());
		if (IMap<String, Variant>* p = CastInstance< IMap<String, Variant> >(obj._ptr)) {
			return p->put(key, value);
		}
		return sl_false;
	}


	static sl_bool _Variant_getVariantMapJsonString(StringBuffer& ret, const Map<String, Variant>& map);
	static sl_bool _Variant_getVariantMapListJsonString(StringBuffer& ret, const List< Map<String, Variant> >& list);
	static sl_bool _Variant_getVariantListJsonString(StringBuffer& ret, const List<Variant>& list)
	{
		ListLocker<Variant> l(list);
		sl_size n = l.count;
		Variant* lb = l.data;
		
		if (!(ret.addStatic("[", 1))) {
			return sl_false;
		}
		for (sl_size i = 0; i < n; i++) {
			Variant& v = lb[i];
			if (i) {
				if (!(ret.addStatic(", ", 2))) {
					return sl_false;
				}
			}
			if (v.isObject()) {
				Ref<Referable> obj(v.getObject());
				if (obj.isNotNull()) {
					if (CList<Variant>* p1 = CastInstance< CList<Variant> >(obj._ptr)) {
						if (!_Variant_getVariantListJsonString(ret, p1)) {
							return sl_false;
						}
					} else if (IMap<String, Variant>* p2 = CastInstance< IMap<String, Variant> >(obj._ptr)) {
						if (!_Variant_getVariantMapJsonString(ret, p2)) {
							return sl_false;
						}
					} else if (CList< Map<String, Variant> >* p3 = CastInstance< CList< Map<String, Variant> > >(obj._ptr)) {
						if (!_Variant_getVariantMapListJsonString(ret, p3)) {
							return sl_false;
						}
					}
				}
			} else {
				String valueText = v.toJsonString();
				if (!(ret.add(valueText))) {
					return sl_false;
				}
			}
		}
		if (!(ret.addStatic("]", 1))) {
			return sl_false;
		}
		return sl_true;
	}

	static sl_bool _Variant_getVariantMapListJsonString(StringBuffer& ret, const List< Map<String, Variant> >& list)
	{
		ListLocker< Map<String, Variant> > l(list);
		sl_size n = l.count;
		Map<String, Variant>* lb = l.data;
		
		if (!(ret.addStatic("[", 1))) {
			return sl_false;
		}
		for (sl_size i = 0; i < n; i++) {
			Map<String, Variant>& v = lb[i];
			if (i) {
				if (!(ret.addStatic(", ", 2))) {
					return sl_false;
				}
			}
			if (!_Variant_getVariantMapJsonString(ret, v)) {
				return sl_false;
			}
		}
		if (!(ret.addStatic("]", 1))) {
			return sl_false;
		}
		return sl_true;
	}

	static sl_bool _Variant_getVariantMapJsonString(StringBuffer& ret, const Map<String, Variant>& map)
	{
		Iterator< Pair<String, Variant> > iterator(map.toIterator());
		if (!(ret.addStatic("{", 1))) {
			return sl_false;
		}
		sl_bool flagFirst = sl_true;
		Pair<String, Variant> pair;
		while (iterator.next(&pair)) {
			Variant& v = pair.value;
			if (!flagFirst) {
				if (!(ret.addStatic(", ", 2))) {
					return sl_false;
				}
			}
			if (!(ret.add(ParseUtil::applyBackslashEscapes(pair.key)))) {
				return sl_false;
			}
			if (!(ret.addStatic(": ", 2))) {
				return sl_false;
			}
			if (v.isObject()) {
				Ref<Referable> obj(v.getObject());
				if (obj.isNotNull()) {
					if (CList<Variant>* p1 = CastInstance< CList<Variant> >(obj._ptr)) {
						if (!_Variant_getVariantListJsonString(ret, p1)) {
							return sl_false;
						}
					} else if (IMap<String, Variant>* p2 = CastInstance< IMap<String, Variant> >(obj._ptr)) {
						if (!_Variant_getVariantMapJsonString(ret, p2)) {
							return sl_false;
						}
					} else if (CList< Map<String, Variant> >* p3 = CastInstance< CList< Map<String, Variant> > >(obj._ptr)) {
						if (!_Variant_getVariantMapListJsonString(ret, p3)) {
							return sl_false;
						}
					}
				}
			} else {
				String valueText = v.toJsonString();
				if (!(ret.add(valueText))) {
					return sl_false;
				}
			}
			flagFirst = sl_false;
		}
		if (!(ret.addStatic("}", 1))) {
			return sl_false;
		}
		return sl_true;
	}

	String Variant::toString() const
	{
		switch (_type) {
			case VariantType::Null:
				return String::null();
			case VariantType::Int32:
			case VariantType::Uint32:
			case VariantType::Int64:
			case VariantType::Uint64:
			case VariantType::Float:
			case VariantType::Double:
			case VariantType::Boolean:
			case VariantType::String8:
			case VariantType::String16:
			case VariantType::Sz8:
			case VariantType::Sz16:
			case VariantType::Time:
			case VariantType::Pointer:
				return getString();
			case VariantType::Object:
			case VariantType::Weak:
				{
					Ref<Referable> obj(getObject());
					if (obj.isNotNull()) {
						if (CList<Variant>* p1 = CastInstance< CList<Variant> >(obj._ptr)) {
							StringBuffer ret;
							if (!_Variant_getVariantListJsonString(ret, p1)) {
								return "<json-error>";
							}
							return ret.merge();
						} else if (IMap<String, Variant>* p2 = CastInstance< IMap<String, Variant> >(obj._ptr)) {
							StringBuffer ret;
							if (!_Variant_getVariantMapJsonString(ret, p2)) {
								return "<json-error>";
							}
							return ret.merge();
						} else if (CList< Map<String, Variant> >* p3 = CastInstance< CList< Map<String, Variant> > >(obj._ptr)) {
							StringBuffer ret;
							if (!_Variant_getVariantMapListJsonString(ret, p3)) {
								return "<json-error>";
							}
							return ret.merge();
						} else {
							return String::format("<object:%s>", obj->getObjectType());
						}
					} else {
						return "<object:null>";
					}
				}
			default:
				return "<error-type>";
		}
	}

	String Variant::toJsonString() const
	{
		SLIB_STATIC_STRING(strNull, "null")
		switch (_type) {
			case VariantType::Null:
				{
					return strNull;
				}
			case VariantType::Int32:
			case VariantType::Uint32:
			case VariantType::Int64:
			case VariantType::Uint64:
			case VariantType::Float:
			case VariantType::Double:
			case VariantType::Boolean:
				return getString();
			case VariantType::Time:
			case VariantType::String8:
			case VariantType::Sz8:
				return ParseUtil::applyBackslashEscapes(getString());
			case VariantType::String16:
			case VariantType::Sz16:
				return ParseUtil::applyBackslashEscapes(getString16());
			case VariantType::Object:
			case VariantType::Weak:
				{
					Ref<Referable> obj(getObject());
					if (obj.isNotNull()) {
						if (CList<Variant>* p1 = CastInstance< CList<Variant> >(obj._ptr)) {
							StringBuffer ret;
							if (!_Variant_getVariantListJsonString(ret, p1)) {
								return strNull;
							}
							return ret.merge();
						} else if (IMap<String, Variant>* p2 = CastInstance< IMap<String, Variant> >(obj._ptr)) {
							StringBuffer ret;
							if (!_Variant_getVariantMapJsonString(ret, p2)) {
								return strNull;
							}
							return ret.merge();
						} else if (CList< Map<String, Variant> >* p3 = CastInstance< CList< Map<String, Variant> > >(obj._ptr)) {
							StringBuffer ret;
							if (!_Variant_getVariantMapListJsonString(ret, p3)) {
								return strNull;
							}
							return ret.merge();
						} else {
							return strNull;
						}
					} else {
						return strNull;
					}
				}
				return strNull;
			default:
				return strNull;
		}
	}
	
	void Variant::get(Variant& _out) const
	{
		_out = *this;
	}
	
	void Variant::set(const Variant& _in)
	{
		*this = _in;
	}

	void Variant::get(AtomicVariant& _out) const
	{
		_out = *this;
	}

	void Variant::set(const AtomicVariant& _in)
	{
		*this = _in;
	}
	
	void Variant::get(char& _out) const
	{
		_out = (char)(getInt32());
	}
	
	void Variant::get(char& _out, char def) const
	{
		_out = (char)(getInt32((sl_int32)def));
	}
	
	void Variant::set(char _in)
	{
		setInt32((sl_int32)_in);
	}
	
	void Variant::get(unsigned char& _out) const
	{
		_out = (unsigned char)(getUint32());
	}
	
	void Variant::get(unsigned char& _out, unsigned char def) const
	{
		_out = (unsigned char)(getUint32((sl_uint32)def));
	}
	
	void Variant::set(unsigned char _in)
	{
		setUint32((sl_uint32)_in);
	}
	
	void Variant::get(short& _out) const
	{
		_out = (short)(getInt32());
	}
	
	void Variant::get(short& _out, short def) const
	{
		_out = (short)(getInt32((sl_int32)def));
	}
	
	void Variant::set(short _in)
	{
		setInt32((sl_int32)_in);
	}
	
	void Variant::get(unsigned short& _out) const
	{
		_out = (unsigned short)(getUint32());
	}
	
	void Variant::get(unsigned short& _out, unsigned short def) const
	{
		_out = (unsigned short)(getUint32((sl_uint32)def));
	}
	
	void Variant::set(unsigned short _in)
	{
		setUint32((sl_uint32)_in);
	}
	
	void Variant::get(int& _out) const
	{
		_out = (int)(getInt32());
	}
	
	void Variant::get(int& _out, int def) const
	{
		_out = (int)(getInt32((sl_int32)def));
	}
	
	void Variant::set(int _in)
	{
		setInt32((sl_int32)_in);
	}
	
	void Variant::get(unsigned int& _out) const
	{
		_out = (unsigned int)(getUint32());
	}
	
	void Variant::get(unsigned int& _out, unsigned int def) const
	{
		_out = (unsigned int)(getUint32((sl_uint32)def));
	}
	
	void Variant::set(unsigned int _in)
	{
		setUint32((sl_uint32)_in);
	}
	
	void Variant::get(long& _out) const
	{
		_out = (long)(getInt32());
	}
	
	void Variant::get(long& _out, long def) const
	{
		_out = (long)(getInt32((sl_int32)def));
	}
	
	void Variant::set(long _in)
	{
		setInt32((sl_int32)_in);
	}
	
	void Variant::get(unsigned long& _out) const
	{
		_out = (unsigned long)(getUint32());
	}
	
	void Variant::get(unsigned long& _out, unsigned long def) const
	{
		_out = (unsigned long)(getUint32((sl_uint32)def));
	}
	
	void Variant::set(unsigned long _in)
	{
		setUint32((sl_uint32)_in);
	}
	
	void Variant::get(sl_int64& _out) const
	{
		_out = getInt64();
	}
	
	void Variant::get(sl_int64& _out, sl_int64 def) const
	{
		_out = getInt64(def);
	}
	
	void Variant::set(sl_int64 _in)
	{
		setInt64(_in);
	}
	
	void Variant::get(sl_uint64& _out) const
	{
		_out = getUint64();
	}
	
	void Variant::get(sl_uint64& _out, sl_uint64 def) const
	{
		_out = getUint64(def);
	}
	
	void Variant::set(sl_uint64 _in)
	{
		setUint64(_in);
	}
	
	void Variant::get(float& _out) const
	{
		_out = getFloat();
	}
	
	void Variant::get(float& _out, float def) const
	{
		_out = getFloat(def);
	}
	
	void Variant::set(float _in)
	{
		setFloat(_in);
	}
	
	void Variant::get(double& _out) const
	{
		_out = getDouble();
	}
	
	void Variant::get(double& _out, double def) const
	{
		_out = getDouble(def);
	}
	
	void Variant::set(double _in)
	{
		setDouble(_in);
	}
	
	void Variant::get(bool& _out) const
	{
		_out = getBoolean();
	}
	
	void Variant::get(bool& _out, bool def) const
	{
		_out = getBoolean(def);
	}
	
	void Variant::set(bool _in)
	{
		setBoolean(_in);
	}
	
	void Variant::get(String& _out) const
	{
		_out = getString();
	}
	
	void Variant::get(String& _out, const String& def) const
	{
		_out = getString(def);
	}
	
	void Variant::set(const String& _in)
	{
		setString(_in);
	}
	
	void Variant::get(AtomicString& _out) const
	{
		_out = getString();
	}
	
	void Variant::get(AtomicString& _out, const String& def) const
	{
		_out = getString(def);
	}
	
	void Variant::set(const AtomicString& _in)
	{
		setString(_in);
	}

	void Variant::get(String16& _out) const
	{
		_out = getString16();
	}
	
	void Variant::get(String16& _out, const String16& def) const
	{
		_out = getString16(def);
	}
	
	void Variant::set(const String16& _in)
	{
		setString(_in);
	}
	
	void Variant::get(AtomicString16& _out) const
	{
		_out = getString16();
	}
	
	void Variant::get(AtomicString16& _out, const String16& def) const
	{
		_out = getString16(def);
	}
	
	void Variant::set(const AtomicString16& _in)
	{
		setString(_in);
	}
	
	void Variant::set(const sl_char8* _in)
	{
		setString(_in);
	}
	
	void Variant::set(const sl_char16* _in)
	{
		setString(_in);
	}
	
	void Variant::get(Time& _out) const
	{
		_out = getTime();
	}
	
	void Variant::get(Time& _out, const Time& def) const
	{
		_out = getTime(def);
	}
	
	void Variant::set(const Time& _in)
	{
		setTime(_in);
	}
	
	void Variant::get(void const* & _out) const
	{
		_out = getPointer();
	}
	
	void Variant::get(void const* & _out, const void* def) const
	{
		_out = getPointer(def);
	}
	
	void Variant::set(const void* _in)
	{
		setPointer(_in);
	}
	
	void Variant::get(Memory& _out) const
	{
		_out = getMemory();
	}
	
	void Variant::set(const Memory& _in)
	{
		setMemory(_in);
	}
	
	void Variant::get(AtomicMemory& _out) const
	{
		_out = getMemory();
	}
	
	void Variant::set(const AtomicMemory& _in)
	{
		setMemory(_in);
	}
	
	void Variant::get(VariantList& _out) const
	{
		_out = getVariantList();
	}
	
	void Variant::set(const VariantList& _in)
	{
		setVariantList(_in);
	}
	
	void Variant::get(AtomicVariantList& _out) const
	{
		_out = getVariantList();
	}
	
	void Variant::set(const AtomicVariantList& _in)
	{
		setVariantList(_in);
	}
	
	void Variant::get(VariantMap& _out) const
	{
		_out = getVariantMap();
	}
	
	void Variant::set(const VariantMap& _in)
	{
		setVariantMap(_in);
	}
	
	void Variant::get(AtomicVariantMap& _out) const
	{
		_out = getVariantMap();
	}
	
	void Variant::set(const AtomicVariantMap& _in)
	{
		setVariantMap(_in);
	}
	
	void Variant::get(VariantMapList& _out) const
	{
		_out = getVariantMapList();
	}
	
	void Variant::set(const VariantMapList& _in)
	{
		setVariantMapList(_in);
	}
	
	void Variant::get(AtomicVariantMapList& _out) const
	{
		_out = getVariantMapList();
	}
	
	void Variant::set(const AtomicVariantMapList& _in)
	{
		setVariantMapList(_in);
	}

	
	Atomic<Variant>::Atomic(AtomicVariant&& other)
	{
		_type = other._type;
		_value = other._value;
		other._type = VariantType::Null;
	}

	Atomic<Variant>::Atomic(const AtomicVariant& other)
	{
		other._retain(_type, _value);
	}

	Atomic<Variant>::Atomic(Variant&& other)
	{
		_type = other._type;
		_value = other._value;
		other._type = VariantType::Null;
	}

	Atomic<Variant>::Atomic(const Variant& other)
	{
		_type = other._type;
		_Variant_copy(_type, other._value, _value);
	}

	Atomic<Variant>::~Atomic()
	{
		_Variant_free(_type, _value);
	}

	Atomic<Variant>::Atomic(sl_null_t): _value(0), _type(VariantType::Null)
	{
	}
	
	Atomic<Variant>::Atomic(char value)
	{
		_type = VariantType::Int32;
		REF_VAR(sl_int32, _value) = value;
	}
	
	Atomic<Variant>::Atomic(unsigned char value)
	{
		_type = VariantType::Uint32;
		REF_VAR(sl_uint32, _value) = value;
	}

	Atomic<Variant>::Atomic(short value)
	{
		_type = VariantType::Int32;
		REF_VAR(sl_int32, _value) = value;
	}

	Atomic<Variant>::Atomic(unsigned short value)
	{
		_type = VariantType::Uint32;
		REF_VAR(sl_uint32, _value) = value;
	}
	
	Atomic<Variant>::Atomic(int value)
	{
		_type = VariantType::Int32;
		REF_VAR(sl_int32, _value) = (sl_int32)value;
	}
	
	Atomic<Variant>::Atomic(unsigned int value)
	{
		_type = VariantType::Uint32;
		REF_VAR(sl_uint32, _value) = (sl_uint32)value;
	}
	
	Atomic<Variant>::Atomic(long value)
	{
		_type = VariantType::Int32;
		REF_VAR(sl_int32, _value) = (sl_int32)value;
	}
	
	Atomic<Variant>::Atomic(unsigned long value)
	{
		_type = VariantType::Uint32;
		REF_VAR(sl_uint32, _value) = (sl_uint32)value;
	}

	Atomic<Variant>::Atomic(sl_int64 value)
	{
		_type = VariantType::Int64;
		REF_VAR(sl_int64, _value) = value;
	}

	Atomic<Variant>::Atomic(sl_uint64 value)
	{
		_type = VariantType::Uint64;
		REF_VAR(sl_uint64, _value) = value;
	}

	Atomic<Variant>::Atomic(float value)
	{
		_type = VariantType::Float;
		REF_VAR(float, _value) = value;
	}

	Atomic<Variant>::Atomic(double value)
	{
		_type = VariantType::Double;
		REF_VAR(double, _value) = value;
	}

	Atomic<Variant>::Atomic(sl_bool value)
	{
		_type = VariantType::Boolean;
		REF_VAR(sl_bool, _value) = value;
	}

	Atomic<Variant>::Atomic(const String& value)
	{
		if (value.isNotNull()) {
			_type = VariantType::String8;
			new PTR_VAR(String, _value) String(value);
		} else {
			_type = VariantType::Null;
		}
	}

	Atomic<Variant>::Atomic(const String16& value)
	{
		if (value.isNotNull()) {
			_type = VariantType::String16;
			new PTR_VAR(String16, _value) String16(value);
		} else {
			_type = VariantType::Null;
		}
	}

	Atomic<Variant>::Atomic(const AtomicString& s)
	{
		String value(s);
		if (value.isNotNull()) {
			_type = VariantType::String8;
			new PTR_VAR(String, _value) String(value);
		} else {
			_type = VariantType::Null;
		}
	}

	Atomic<Variant>::Atomic(const AtomicString16& s)
	{
		String16 value(s);
		if (value.isNotNull()) {
			_type = VariantType::String16;
			new PTR_VAR(String16, _value) String16(value);
		} else {
			_type = VariantType::Null;
		}
	}

	Atomic<Variant>::Atomic(const sl_char8* value)
	{
		if (value) {
			_type = VariantType::Sz8;
			REF_VAR(const sl_char8*, _value) = value;
		} else {
			_type = VariantType::Null;
		}
	}

	Atomic<Variant>::Atomic(const sl_char16* value)
	{
		if (value) {
			_type = VariantType::Sz16;
			REF_VAR(const sl_char16*, _value) = value;
		} else {
			_type = VariantType::Null;
		}
	}

	Atomic<Variant>::Atomic(const Time& value)
	{
		_type = VariantType::Time;
		REF_VAR(Time, _value) = value;
	}

	Atomic<Variant>::Atomic(const void* ptr)
	{
		if (ptr) {
			_type = VariantType::Pointer;
			REF_VAR(const void*, _value) = ptr;
		} else {
			_type = VariantType::Null;
		}
	}

	Atomic<Variant>::Atomic(const Memory& mem)
	{
		if (mem.isNotNull()) {
			_type = VariantType::Object;
			new PTR_VAR(Memory, _value) Memory(mem);
		} else {
			_type = VariantType::Null;
		}
	}

	Atomic<Variant>::Atomic(const AtomicMemory& _mem)
	{
		Memory mem(_mem);
		if (mem.isNotNull()) {
			_type = VariantType::Object;
			new PTR_VAR(Memory, _value) Memory(mem);
		} else {
			_type = VariantType::Null;
		}
	}
	
	Atomic<Variant>::Atomic(const VariantList& list)
	{
		if (list.isNotNull()) {
			_type = VariantType::Object;
			new PTR_VAR(VariantList, _value) VariantList(list);
		} else {
			_type = VariantType::Null;
		}
	}
	
	Atomic<Variant>::Atomic(const AtomicVariantList& list)
	{
		if (list.isNotNull()) {
			_type = VariantType::Object;
			new PTR_VAR(VariantList, _value) VariantList(list);
		} else {
			_type = VariantType::Null;
		}
	}
	
	Atomic<Variant>::Atomic(const VariantMap& list)
	{
		if (list.isNotNull()) {
			_type = VariantType::Object;
			new PTR_VAR(VariantMap, _value) VariantMap(list);
		} else {
			_type = VariantType::Null;
		}
	}
	
	Atomic<Variant>::Atomic(const AtomicVariantMap& list)
	{
		if (list.isNotNull()) {
			_type = VariantType::Object;
			new PTR_VAR(VariantMap, _value) VariantMap(list);
		} else {
			_type = VariantType::Null;
		}
	}
	
	Atomic<Variant>::Atomic(const VariantMapList& list)
	{
		if (list.isNotNull()) {
			_type = VariantType::Object;
			new PTR_VAR(VariantMapList, _value) VariantMapList(list);
		} else {
			_type = VariantType::Null;
		}
	}
	
	Atomic<Variant>::Atomic(const AtomicVariantMapList& list)
	{
		if (list.isNotNull()) {
			_type = VariantType::Object;
			new PTR_VAR(VariantMapList, _value) VariantMapList(list);
		} else {
			_type = VariantType::Null;
		}
	}
	
	AtomicVariant& Atomic<Variant>::operator=(AtomicVariant&& other)
	{
		if (this != &other) {
			_replace(other._type, other._value);
			other._type = VariantType::Null;
		}
		return *this;
	}

	AtomicVariant& Atomic<Variant>::operator=(const AtomicVariant& other)
	{
		if (this != &other) {
			VariantType type;
			sl_uint64 value;
			other._retain(type, value);
			_replace(type, value);
		}
		return *this;
	}

	AtomicVariant& Atomic<Variant>::operator=(Variant&& other)
	{
		if ((void*)this != (void*)(&other)) {
			_replace(other._type, other._value);
			other._type = VariantType::Null;
		}
		return *this;
	}

	AtomicVariant& Atomic<Variant>::operator=(const Variant& other)
	{
		VariantType type = other._type;
		sl_uint64 value;
		_Variant_copy(type, other._value, value);
		_replace(type, value);
		return *this;
	}

	AtomicVariant& Atomic<Variant>::operator=(sl_null_t)
	{
		setNull();
		return *this;
	}
	
	Variant Atomic<Variant>::operator[](sl_size indexForVariantList) const
	{
		return getElement(indexForVariantList);
	}

	Variant Atomic<Variant>::operator[](const String& keyForVariantMap) const
	{
		return getItem(keyForVariantMap);
	}

	void Atomic<Variant>::setNull()
	{
		if (_type != VariantType::Null) {
			_replace(VariantType::Null, 0);
		}
	}

	sl_bool Atomic<Variant>::isInt32() const
	{
		return _type == VariantType::Int32;
	}

	sl_int32 Atomic<Variant>::getInt32(sl_int32 def) const
	{
		Variant var(*this);
		return var.getInt32(def);
	}

	void Atomic<Variant>::setInt32(sl_int32 value)
	{
		sl_int64 v;
		REF_VAR(sl_int32, v) = value;
		_replace(VariantType::Int32, v);
	}

	sl_bool Atomic<Variant>::isUint32() const
	{
		return _type == VariantType::Uint32;
	}

	sl_uint32 Atomic<Variant>::getUint32(sl_uint32 def) const
	{
		Variant var(*this);
		return var.getUint32(def);
	}

	void Atomic<Variant>::setUint32(sl_uint32 value)
	{
		sl_int64 v;
		REF_VAR(sl_uint32, v) = value;
		_replace(VariantType::Uint32, v);
	}

	sl_bool Atomic<Variant>::isInt64() const
	{
		return _type == VariantType::Int64;
	}

	sl_int64 Atomic<Variant>::getInt64(sl_int64 def) const
	{
		Variant var(*this);
		return var.getInt64(def);
	}

	void Atomic<Variant>::setInt64(sl_int64 value)
	{
		_replace(VariantType::Int64, value);
	}

	sl_bool Atomic<Variant>::isUint64() const
	{
		return _type == VariantType::Uint64;
	}

	sl_uint64 Atomic<Variant>::getUint64(sl_uint64 def) const
	{
		Variant var(*this);
		return var.getUint64(def);
	}

	void Atomic<Variant>::setUint64(sl_uint64 value)
	{
		_replace(VariantType::Uint64, value);
	}

	sl_bool Atomic<Variant>::isInteger() const
	{
		return _type == VariantType::Int32 || _type == VariantType::Uint32 || _type == VariantType::Int64 || _type == VariantType::Uint64;
	}

	sl_bool Atomic<Variant>::isSignedInteger() const
	{
		return _type == VariantType::Int32 || _type == VariantType::Int64;
	}

	sl_bool Atomic<Variant>::isUnsignedInteger() const
	{
		return _type == VariantType::Uint32 || _type == VariantType::Uint64;
	}

	sl_bool Atomic<Variant>::isFloat() const
	{
		return _type == VariantType::Float;
	}

	float Atomic<Variant>::getFloat(float def) const
	{
		Variant var(*this);
		return var.getFloat(def);
	}

	void Atomic<Variant>::setFloat(float value)
	{
		sl_int64 v;
		REF_VAR(float, v) = value;
		_replace(VariantType::Float, v);
	}

	sl_bool Atomic<Variant>::isDouble() const
	{
		return _type == VariantType::Double;
	}

	double Atomic<Variant>::getDouble(double def) const
	{
		Variant var(*this);
		return var.getDouble(def);
	}

	void Atomic<Variant>::setDouble(double value)
	{
		sl_int64 v;
		REF_VAR(double, v) = value;
		_replace(VariantType::Double, v);
	}

	sl_bool Atomic<Variant>::isNumber() const
	{
		return isInteger() || _type == VariantType::Float || _type == VariantType::Double;
	}

	sl_bool Atomic<Variant>::isBoolean() const
	{
		return _type == VariantType::Boolean;
	}

	sl_bool Atomic<Variant>::getBoolean(sl_bool def) const
	{
		Variant var(*this);
		return var.getBoolean(def);
	}

	void Atomic<Variant>::setBoolean(sl_bool value)
	{
		sl_int64 v;
		REF_VAR(sl_bool, v) = value;
		_replace(VariantType::Boolean, v);
	}

	sl_bool Atomic<Variant>::isString() const
	{
		return _type == VariantType::String8 || _type == VariantType::String16 || _type == VariantType::Sz8 || _type == VariantType::Sz16;
	}

	sl_bool Atomic<Variant>::isString8() const
	{
		return _type == VariantType::String8;
	}

	sl_bool Atomic<Variant>::isString16() const
	{
		return _type == VariantType::String16;
	}

	sl_bool Atomic<Variant>::isSz8() const
	{
		return _type == VariantType::Sz8;
	}

	sl_bool Atomic<Variant>::isSz16() const
	{
		return _type == VariantType::Sz16;
	}

	String Atomic<Variant>::getString(const String& def) const
	{
		Variant var(*this);
		return var.getString(def);
	}

	String Atomic<Variant>::getString() const
	{
		Variant var(*this);
		return var.getString();
	}

	String16 Atomic<Variant>::getString16(const String16& def) const
	{
		Variant var(*this);
		return var.getString16(def);
	}

	String16 Atomic<Variant>::getString16() const
	{
		Variant var(*this);
		return var.getString16();
	}

	const sl_char8* Atomic<Variant>::getSz8(const sl_char8* def) const
	{
		Variant var(*this);
		return var.getSz8(def);
	}

	const sl_char16* Atomic<Variant>::getSz16(const sl_char16* def) const
	{
		Variant var(*this);
		return var.getSz16(def);
	}

	void Atomic<Variant>::setString(const String& value)
	{
		if (value.isNotNull()) {
			sl_int64 v;
			new PTR_VAR(String, v) String(value);
			_replace(VariantType::String8, v);
		} else {
			_replace(VariantType::Null, 0);
		}
	}

	void Atomic<Variant>::setString(const String16& value)
	{
		if (value.isNotNull()) {
			sl_int64 v;
			new PTR_VAR(String16, v) String16(value);
			_replace(VariantType::String16, v);
		} else {
			_replace(VariantType::Null, 0);
		}
	}

	void Atomic<Variant>::setString(const AtomicString& s)
	{
		String value(s);
		if (value.isNotNull()) {
			sl_int64 v;
			new PTR_VAR(String, v) String(value);
			_replace(VariantType::String8, v);
		} else {
			_replace(VariantType::Null, 0);
		}
	}

	void Atomic<Variant>::setString(const AtomicString16& s)
	{
		String16 value(s);
		if (value.isNotNull()) {
			sl_int64 v;
			new PTR_VAR(String16*, v) String16(value);
			_replace(VariantType::String16, v);
		} else {
			_replace(VariantType::Null, 0);
		}
	}

	void Atomic<Variant>::setString(const sl_char8* value)
	{
		if (value) {
			sl_int64 v;
			REF_VAR(const sl_char8*, v) = value;
			_replace(VariantType::Sz8, v);
		} else {
			_replace(VariantType::Null, 0);
		}
	}

	void Atomic<Variant>::setString(const sl_char16* value)
	{
		if (value) {
			sl_int64 v;
			REF_VAR(const sl_char16*, v) = value;
			_replace(VariantType::Sz16, v);
		} else {
			_replace(VariantType::Null, 0);
		}
	}

	sl_bool Atomic<Variant>::isTime() const
	{
		return _type == VariantType::Time;
	}

	Time Atomic<Variant>::getTime(Time def) const
	{
		Variant var(*this);
		return var.getTime(def);
	}

	Time Atomic<Variant>::getTime() const
	{
		Variant var(*this);
		return var.getTime();
	}

	void Atomic<Variant>::setTime(const Time& value)
	{
		sl_int64 v;
		REF_VAR(Time, v) = value;
		_replace(VariantType::Time, v);
	}

	sl_bool Atomic<Variant>::isPointer() const
	{
		return _type == VariantType::Pointer || _type == VariantType::Sz8 || _type == VariantType::Sz16;
	}

	void* Atomic<Variant>::getPointer(const void* def) const
	{
		Variant var(*this);
		return var.getPointer(def);
	}

	void Atomic<Variant>::setPointer(const void *ptr)
	{
		if (ptr) {
			sl_int64 v;
			REF_VAR(const void*, v) = ptr;
			_replace(VariantType::Pointer, v);
		} else {
			_replace(VariantType::Null, 0);
		}
	}

	sl_bool Atomic<Variant>::isObject() const
	{
		return _type == VariantType::Object;
	}

	sl_bool Atomic<Variant>::isWeak() const
	{
		return _type == VariantType::Weak;
	}

	Ref<Referable> Atomic<Variant>::getObject() const
	{
		Variant var(*this);
		return var.getObject();
	}

	sl_bool Atomic<Variant>::isObjectNotNull() const
	{
		Variant var(*this);
		return var.isObjectNotNull();
	}

	sl_bool Atomic<Variant>::isObjectNull() const
	{
		Variant var(*this);
		return var.isObjectNull();
	}

	sl_object_type Atomic<Variant>::getObjectType() const
	{
		Variant var(*this);
		return var.getObjectType();
	}

	sl_bool Atomic<Variant>::isMemory() const
	{
		Variant var(*this);
		return var.isMemory();
	}

	Memory Atomic<Variant>::getMemory() const
	{
		Variant var(*this);
		return var.getMemory();
	}

	void Atomic<Variant>::setMemory(const Memory& mem)
	{
		if (mem.isNotNull()) {
			sl_int64 v;
			new PTR_VAR(Memory, v) Memory(mem);
			_replace(VariantType::Object, v);
		} else {
			_replace(VariantType::Null, 0);
		}
	}

	sl_bool Atomic<Variant>::isVariantList() const
	{
		Variant var(*this);
		return var.isVariantList();
	}

	VariantList Atomic<Variant>::getVariantList() const
	{
		Variant var(*this);
		return var.getVariantList();
	}
	
	void Atomic<Variant>::setVariantList(const VariantList& list)
	{
		if (list.isNotNull()) {
			sl_int64 v;
			new PTR_VAR(VariantList, v) VariantList(list);
			_replace(VariantType::Object, v);
		} else {
			_replace(VariantType::Null, 0);
		}
	}
	
	sl_bool Atomic<Variant>::isVariantMap() const
	{
		Variant var(*this);
		return var.isVariantMap();
	}

	VariantMap Atomic<Variant>::getVariantMap() const
	{
		Variant var(*this);
		return var.getVariantMap();
	}
	
	void Atomic<Variant>::setVariantMap(const VariantMap& map)
	{
		if (map.isNotNull()) {
			sl_int64 v;
			new PTR_VAR(VariantMap, v) VariantMap(map);
			_replace(VariantType::Object, v);
		} else {
			_replace(VariantType::Null, 0);
		}
	}

	sl_bool Atomic<Variant>::isVariantMapList() const
	{
		Variant var(*this);
		return var.isVariantMap();
	}

	VariantMapList Atomic<Variant>::getVariantMapList() const
	{
		Variant var(*this);
		return var.getVariantMapList();
	}
	
	void Atomic<Variant>::setVariantMapList(const VariantMapList& list)
	{
		if (list.isNotNull()) {
			sl_int64 v;
			new PTR_VAR(VariantMapList, v) VariantMapList(list);
			_replace(VariantType::Object, v);
		} else {
			_replace(VariantType::Null, 0);
		}
	}

	sl_size Atomic<Variant>::getElementsCount() const
	{
		Variant var(*this);
		return var.getElementsCount();
	}

	Variant Atomic<Variant>::getElement(sl_size index) const
	{
		Variant var(*this);
		return var.getElement(index);
	}

	sl_bool Atomic<Variant>::setElement(sl_size index, const Variant& value)
	{
		Variant var(*this);
		return var.setElement(index, value);
	}

	sl_bool Atomic<Variant>::addElement(const Variant& value)
	{
		Variant var(*this);
		return var.addElement(value);
	}

	Variant Atomic<Variant>::getItem(const String& key) const
	{
		Variant var(*this);
		return var.getItem(key);
	}

	sl_bool Atomic<Variant>::putItem(const String& key, const Variant& value)
	{
		Variant var(*this);
		return var.putItem(key, value);
	}

	String Atomic<Variant>::toString() const
	{
		Variant var(*this);
		return var.toString();
	}

	String Atomic<Variant>::toJsonString() const
	{
		Variant var(*this);
		return var.toJsonString();
	}
	
	void Atomic<Variant>::get(Variant& _out) const
	{
		_out = *this;
	}
	
	void Atomic<Variant>::set(const Variant& _in)
	{
		*this = _in;
	}
	
	void Atomic<Variant>::get(AtomicVariant& _out) const
	{
		_out = *this;
	}
	
	void Atomic<Variant>::set(const AtomicVariant& _in)
	{
		*this = _in;
	}
	
	void Atomic<Variant>::get(char& _out) const
	{
		_out = (char)(getInt32());
	}
	
	void Atomic<Variant>::get(char& _out, char def) const
	{
		_out = (char)(getInt32((sl_int32)def));
	}
	
	void Atomic<Variant>::set(char _in)
	{
		setInt32((sl_int32)_in);
	}
	
	void Atomic<Variant>::get(unsigned char& _out) const
	{
		_out = (unsigned char)(getUint32());
	}
	
	void Atomic<Variant>::get(unsigned char& _out, unsigned char def) const
	{
		_out = (unsigned char)(getUint32((sl_uint32)def));
	}
	
	void Atomic<Variant>::set(unsigned char _in)
	{
		setUint32((sl_uint32)_in);
	}
	
	void Atomic<Variant>::get(short& _out) const
	{
		_out = (short)(getInt32());
	}
	
	void Atomic<Variant>::get(short& _out, short def) const
	{
		_out = (short)(getInt32((sl_int32)def));
	}
	
	void Atomic<Variant>::set(short _in)
	{
		setInt32((sl_int32)_in);
	}
	
	void Atomic<Variant>::get(unsigned short& _out) const
	{
		_out = (unsigned short)(getUint32());
	}
	
	void Atomic<Variant>::get(unsigned short& _out, unsigned short def) const
	{
		_out = (unsigned short)(getUint32((sl_uint32)def));
	}
	
	void Atomic<Variant>::set(unsigned short _in)
	{
		setUint32((sl_uint32)_in);
	}
	
	void Atomic<Variant>::get(int& _out) const
	{
		_out = (int)(getInt32());
	}
	
	void Atomic<Variant>::get(int& _out, int def) const
	{
		_out = (int)(getInt32((sl_int32)def));
	}
	
	void Atomic<Variant>::set(int _in)
	{
		setInt32((sl_int32)_in);
	}
	
	void Atomic<Variant>::get(unsigned int& _out) const
	{
		_out = (unsigned int)(getUint32());
	}
	
	void Atomic<Variant>::get(unsigned int& _out, unsigned int def) const
	{
		_out = (unsigned int)(getUint32((sl_uint32)def));
	}
	
	void Atomic<Variant>::set(unsigned int _in)
	{
		setUint32((sl_uint32)_in);
	}
	
	void Atomic<Variant>::get(long& _out) const
	{
		_out = (long)(getInt32());
	}
	
	void Atomic<Variant>::get(long& _out, long def) const
	{
		_out = (long)(getInt32((sl_int32)def));
	}
	
	void Atomic<Variant>::set(long _in)
	{
		setInt32((sl_int32)_in);
	}
	
	void Atomic<Variant>::get(unsigned long& _out) const
	{
		_out = (unsigned long)(getUint32());
	}
	
	void Atomic<Variant>::get(unsigned long& _out, unsigned long def) const
	{
		_out = (unsigned long)(getUint32((sl_uint32)def));
	}
	
	void Atomic<Variant>::set(unsigned long _in)
	{
		setUint32((sl_uint32)_in);
	}
	
	void Atomic<Variant>::get(sl_int64& _out) const
	{
		_out = getInt64();
	}
	
	void Atomic<Variant>::get(sl_int64& _out, sl_int64 def) const
	{
		_out = getInt64(def);
	}
	
	void Atomic<Variant>::set(sl_int64 _in)
	{
		setInt64(_in);
	}
	
	void Atomic<Variant>::get(sl_uint64& _out) const
	{
		_out = getUint64();
	}
	
	void Atomic<Variant>::get(sl_uint64& _out, sl_uint64 def) const
	{
		_out = getUint64(def);
	}
	
	void Atomic<Variant>::set(sl_uint64 _in)
	{
		setUint64(_in);
	}
	
	void Atomic<Variant>::get(float& _out) const
	{
		_out = getFloat();
	}
	
	void Atomic<Variant>::get(float& _out, float def) const
	{
		_out = getFloat(def);
	}
	
	void Atomic<Variant>::set(float _in)
	{
		setFloat(_in);
	}
	
	void Atomic<Variant>::get(double& _out) const
	{
		_out = getDouble();
	}
	
	void Atomic<Variant>::get(double& _out, double def) const
	{
		_out = getDouble(def);
	}
	
	void Atomic<Variant>::set(double _in)
	{
		setDouble(_in);
	}
	
	void Atomic<Variant>::get(bool& _out) const
	{
		_out = getBoolean();
	}
	
	void Atomic<Variant>::get(bool& _out, bool def) const
	{
		_out = getBoolean(def);
	}
	
	void Atomic<Variant>::set(bool _in)
	{
		setBoolean(_in);
	}
	
	void Atomic<Variant>::get(String& _out) const
	{
		_out = getString();
	}
	
	void Atomic<Variant>::get(String& _out, const String& def) const
	{
		_out = getString(def);
	}
	
	void Atomic<Variant>::set(const String& _in)
	{
		setString(_in);
	}
	
	void Atomic<Variant>::get(AtomicString& _out) const
	{
		_out = getString();
	}
	
	void Atomic<Variant>::get(AtomicString& _out, const String& def) const
	{
		_out = getString(def);
	}
	
	void Atomic<Variant>::set(const AtomicString& _in)
	{
		setString(_in);
	}
	
	void Atomic<Variant>::get(String16& _out) const
	{
		_out = getString16();
	}
	
	void Atomic<Variant>::get(String16& _out, const String16& def) const
	{
		_out = getString16(def);
	}
	
	void Atomic<Variant>::set(const String16& _in)
	{
		setString(_in);
	}
	
	void Atomic<Variant>::get(AtomicString16& _out) const
	{
		_out = getString16();
	}
	
	void Atomic<Variant>::get(AtomicString16& _out, const String16& def) const
	{
		_out = getString16(def);
	}
	
	void Atomic<Variant>::set(const AtomicString16& _in)
	{
		setString(_in);
	}
	
	void Atomic<Variant>::set(const sl_char8* _in)
	{
		setString(_in);
	}
	
	void Atomic<Variant>::set(const sl_char16* _in)
	{
		setString(_in);
	}
	
	void Atomic<Variant>::get(Time& _out) const
	{
		_out = getTime();
	}
	
	void Atomic<Variant>::get(Time& _out, const Time& def) const
	{
		_out = getTime(def);
	}
	
	void Atomic<Variant>::set(const Time& _in)
	{
		setTime(_in);
	}
	
	void Atomic<Variant>::get(void const* & _out) const
	{
		_out = getPointer();
	}
	
	void Atomic<Variant>::get(void const* & _out, const void* def) const
	{
		_out = getPointer(def);
	}
	
	void Atomic<Variant>::set(const void* _in)
	{
		setPointer(_in);
	}
	
	void Atomic<Variant>::get(Memory& _out) const
	{
		_out = getMemory();
	}
	
	void Atomic<Variant>::set(const Memory& _in)
	{
		setMemory(_in);
	}
	
	void Atomic<Variant>::get(AtomicMemory& _out) const
	{
		_out = getMemory();
	}
	
	void Atomic<Variant>::set(const AtomicMemory& _in)
	{
		setMemory(_in);
	}
	
	void Atomic<Variant>::get(VariantList& _out) const
	{
		_out = getVariantList();
	}
	
	void Atomic<Variant>::set(const VariantList& _in)
	{
		setVariantList(_in);
	}
	
	void Atomic<Variant>::get(AtomicVariantList& _out) const
	{
		_out = getVariantList();
	}
	
	void Atomic<Variant>::set(const AtomicVariantList& _in)
	{
		setVariantList(_in);
	}
	
	void Atomic<Variant>::get(VariantMap& _out) const
	{
		_out = getVariantMap();
	}
		
	void Atomic<Variant>::set(const VariantMap& _in)
	{
		setVariantMap(_in);
	}
	
	void Atomic<Variant>::get(AtomicVariantMap& _out) const
	{
		_out = getVariantMap();
	}
	
	void Atomic<Variant>::set(const AtomicVariantMap& _in)
	{
		setVariantMap(_in);
	}
	
	void Atomic<Variant>::get(VariantMapList& _out) const
	{
		_out = getVariantMapList();
	}
	
	void Atomic<Variant>::set(const VariantMapList& _in)
	{
		setVariantMapList(_in);
	}
	
	void Atomic<Variant>::get(AtomicVariantMapList& _out) const
	{
		_out = getVariantMapList();
	}
	
	void Atomic<Variant>::set(const AtomicVariantMapList& _in)
	{
		setVariantMapList(_in);
	}
	

	sl_bool operator==(const Variant& v1, const Variant& v2)
	{
		VariantType type = v1._type;
		if (type == v2._type) {
			if (v1._value == v2._value) {
				return sl_true;
			}
			switch (type) {
				case VariantType::Null:
					return sl_true;
				case VariantType::Int32:
				case VariantType::Uint32:
					return REF_VAR(sl_int32 const, v1._value) == REF_VAR(sl_int32 const, v2._value);
				case VariantType::Float:
					return REF_VAR(float const, v1._value) == REF_VAR(float const, v2._value);
				case VariantType::Double:
					return REF_VAR(double const, v1._value) == REF_VAR(double const, v2._value);
				case VariantType::Boolean:
					return REF_VAR(sl_bool const, v1._value) == REF_VAR(sl_bool const, v2._value);
				case VariantType::Pointer:
				case VariantType::Sz8:
				case VariantType::Sz16:
				case VariantType::Object:
					return REF_VAR(void const* const, v1._value) == REF_VAR(void const* const, v2._value);
				case VariantType::String8:
					return REF_VAR(String const, v1._value) == REF_VAR(String const, v2._value);
				case VariantType::String16:
					return REF_VAR(String16 const, v1._value) == REF_VAR(String16 const, v2._value);
				default:
					break;
			}
		}
		return sl_false;
	}

	sl_bool operator!=(const Variant& v1, const Variant& v2)
	{
		return !(v1 == v2);
	}

}
