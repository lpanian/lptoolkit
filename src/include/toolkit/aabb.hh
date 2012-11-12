#pragma once
#ifndef INCLUDED_aabb_HH
#define INCLUDED_aabb_HH

#include <cfloat>
#include "vec.hh"

template< class T >
class Box3
{
public:
	vec3<T> m_min;
	vec3<T> m_max;
	Box3() : m_min(FLT_MAX), m_max(FLT_MIN) {}

	inline void Extend(const vec3<T>& v) 
	{
		m_min = Min(m_min, v);
		m_max = Max(m_max, v);
	}

	int MajorAxis() const;
};

template<class Vec>
struct BoxType { };
template<class T> 
struct BoxType<vec3<T> > { typedef Box3<T> type; };

typedef Box3<float> Box3f;
typedef Box3<double> Box3d;

template<class T>
inline int Box3<T>::MajorAxis() const
{
	float bestDist = m_max[0] - m_min[0];
	int best = 0;
	for(int i = 1; i < 3; ++i)
	{
		const float dist = m_max[i] - m_min[i];
		if (dist > bestDist)
		{
			bestDist = dist;
			best = i;
		}
	}
	return best;
}

#endif
