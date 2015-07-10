#pragma once
#ifndef INCLUDED_toolkit_veccommon_HH
#define INCLUDED_toolkit_veccommon_HH

#include "toolkit/vec.hh"
#include "toolkit/mathcommon.hh"

namespace lptk
{
    inline float SphericalTheta(const lptk::v3f& v)
    {
	return lptk::Acos(lptk::Clamp(v.z, -1.f, 1.f));
    }

    inline float SphericalPhi(const lptk::v3f& v)
    {
	const float phi = lptk::Atan2(v.y, v.x);
	if(phi < 0.f)
	    return phi + 2.f * kPi;
	return phi;
    }
}

#endif


