#pragma once
#ifndef INCLUDED_toolkit_aabb_HH
#define INCLUDED_toolkit_aabb_HH

#include <cfloat>
#include "vec.hh"

////////////////////////////////////////////////////////////////////////////////
template< class T >
class Box2
{
public:
	vec2<T> m_min;
	vec2<T> m_max;
	Box2() : m_min(FLT_MAX), m_max(FLT_MIN) {}

	inline void Extend(const vec2<T>& v) 
	{
		m_min = Min(m_min, v);
		m_max = Max(m_max, v);
	}

	int MajorAxis() const;
};

template<class T>
inline int Box2<T>::MajorAxis() const
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

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
template<class Vec>
struct BoxType { };
template<class T> 
struct BoxType<vec2<T> > { typedef Box2<T> type; };
template<class T> 
struct BoxType<vec3<T> > { typedef Box3<T> type; };

typedef Box2<int> Box2i;
typedef Box2<float> Box2f;
typedef Box2<double> Box2d;
typedef Box3<int> Box3i;
typedef Box3<float> Box3f;
typedef Box3<double> Box3d;


#endif
