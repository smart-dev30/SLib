/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHECKHEADER_SLIB_MATH_DETAIL_SPHERE
#define CHECKHEADER_SLIB_MATH_DETAIL_SPHERE

#include "../sphere.h"

namespace slib
{
	
	template <class T>
	template <class O>
	SLIB_INLINE SphereT<T>::SphereT(const SphereT<O>& other) : center(other.center), radius((T)(other.radius))
	{
	}
	
	template <class T>
	template <class O>
	SLIB_INLINE SphereT<T>& SphereT<T>::operator=(const SphereT<O>& other)
	{
		center = other.center;
		radius = (T)(other.radius);
		return *this;
	}

}

#endif
