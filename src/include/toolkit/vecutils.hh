#pragma once
#ifndef INCLUDED_toolkit_vecutils_HH
#define INCLUDED_toolkit_vecutils_HH

#include <tuple>
#include "vec.hh"

template<class T>
inline vec3<T> RotateAround(const vec3<T>& v, const vec3<T>& axis, T angle)
{
	T c = Cos(angle);
	T s = Sin(angle);
	T one_minus_c = T(1) - c;
	T axis_dot_v = Dot(axis, v);
	T k = one_minus_c * axis_dot_v;
	vec3<T> axis_cross_v = Cross(axis, v);
	return vec3<T>(
	 c * v.x + k * axis.x + s * axis_cross_v.x,
	 c * v.y + k * axis.y + s * axis_cross_v.y,
	 c * v.z + k * axis.z + s * axis_cross_v.z);
}

// return a side and up vector to form an orthogonal basis perpendicular to dir.
// if dir has no length then the results are undefined.
template<class T>
inline std::tuple<vec3<T>, vec3<T> > CreateArbitraryBasis(const vec3<T>& dir)
{
	const T dirVals[3] = {Abs(dir.x), Abs(dir.y), Abs(dir.z)};
	int smallest = 0;
	for(int i = 1; i < 3; ++i)
		if(dirVals[smallest] > dirVals[i])
			smallest = i;
	T minAxisVals[3] = {T(0),T(0),T(0)};
	minAxisVals[smallest] = T(1);
	const vec3<T> minAxis = vec3<T>(minAxisVals[0], minAxisVals[1], minAxisVals[2]);

	const vec3<T> up = Cross(minAxis, dir);
	const vec3<T> side = Cross(up, dir);

	const vec3<T> expectedDir = Cross(side, up);
	if(Dot(expectedDir, dir) < T(0))
		return std::make_tuple(-Normalize(side), -Normalize(up));
	else
		return std::make_tuple(Normalize(side), Normalize(up));
}

#endif
