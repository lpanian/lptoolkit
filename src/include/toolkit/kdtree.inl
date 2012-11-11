#pragma once
#ifndef INCLUDED_kdtree_INL
#define INCLUDED_kdtree_INL

#include <algorithm>
#include <iostream>

namespace detail 
{
	template<class T>
	void Sort2(T* p, int axis)
	{
		if(p[0][axis] > p[1][axis])
			std::swap(p[0], p[1]);
	}

	template<class T>
	void Sort3(T* p, int axis)
	{
		Sort2(&p[0], axis);
		Sort2(&p[1], axis);
		Sort2(&p[0], axis);
	}

	template<class T>
	int Median3(const T* p, int aIdx, int bIdx, int cIdx, int axis)
	{
		const typename T::baseType a = p[aIdx][axis];
		const typename T::baseType b = p[bIdx][axis];
		const typename T::baseType c = p[cIdx][axis];

		if(a < b)
		{
			// a b c
			if(b < c) 
				return bIdx;
			// c a b
			else if(c < a)
				return aIdx;
			// a c b
			else 
				return cIdx;
		} 
		else // b <= a
		{
			if(a < c)
				// b a c
				return aIdx;
			else if(c < b)
				// c b a
				return bIdx;
			else
				// b c a
				return cIdx;
		}
	}

	// ensure all points lesss than the pivot are on the left side, and all points
	// right of the pivot are on the right side
	template<class T>
	inline int Partition(T* points, int left, int right, int pivot, int axis)
	{
		const typename T::baseType pivotValue = points[pivot][axis];
		std::swap(points[pivot], points[right]);
		int cur = left;
		for(int i = left; i < right; ++i)
		{
			if(points[i][axis] <= pivotValue)
				std::swap(points[i], points[cur++]);
		}
		std::swap(points[cur], points[right]);
		return cur;
	}
	
	// Return the 'kth' element if the points listed were sorted
	template<class T>
	inline void Select(T* points, int left, int right, int k, int axis)
	{
		while(left < right)
		{
			const int pivotIndex = Partition(points, left, right, left + (right - left)/2, axis);
			const int toPivot = pivotIndex - left;

			if(toPivot == k)
				return ;
			else if(k < toPivot)
			{
				right = pivotIndex - 1;
			}
			else // k > toPivot
			{
				left = pivotIndex + 1;
				k = k - (toPivot + 1);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
template<class T>
KdTree<T>::KdTree()
	: m_points()
	, m_nodes()
	, m_bounds()
	, m_bucketSize(0)
{}

template<class T>
void KdTree<T>::Init(const T* points, int numPoints, int bucketSize)
{
	m_points.clear();
	m_points.reserve(numPoints);
	m_points.insert(m_points.begin(), points, points + numPoints);
	m_bucketSize = Max(bucketSize, 2);
	for(int i = 0; i < numPoints; ++i)
		m_bounds.Extend(points[i]);

	auto box = m_bounds;
	Build(box, 0, m_points.size());
//	Validate(0);
}

template<class T>
void KdTree<T>::Validate(int idx)
{
	const Node* node = &m_nodes[idx];

	const int axis = node->m_axis;
	if(axis == 3)
		return;

	const typename T::baseType split = node->m_split;
	if(node->m_children[0] >= 0) 
	{
		const Node* child = &m_nodes[node->m_children[0]];
		for(int i = child->m_offset; i < child->m_offset + child->m_count; ++i)
		{
			if(m_points[i][axis] > split) {
				std::cerr << "Node " << idx << ": Point " << m_points[i] << " incorrectly on right side of axis " << axis << " split: " << split << std::endl;
			}
		}
	}
	
	if(node->m_children[1] >= 0) 
	{
		const Node* child = &m_nodes[node->m_children[1]];
		for(int i = child->m_offset; i < child->m_offset + child->m_count; ++i)
		{
			if(m_points[i][axis] < split) {
				std::cerr << "Node " << idx << ": Point " << m_points[i] << " incorrectly on left side of axis " << axis << " split: " << split << std::endl;
			}
		}
	}
}

template<class T>
int KdTree<T>::Build(typename BoxType<T>::type& bounds, int offset, int len)
{
	int idx = m_nodes.size();
	m_nodes.emplace_back();

	m_nodes[idx].m_split = typename T::baseType(0);
	m_nodes[idx].m_offset = offset;
	m_nodes[idx].m_count = len;
	m_nodes[idx].m_children[0] = -1;
	m_nodes[idx].m_children[1] = -1;

	if(len <= m_bucketSize)
	{
		m_nodes[idx].m_axis = 3 ;
		return idx;
	}

	const int axis = bounds.MajorAxis();
	detail::Select(&m_points[offset], 0, len - 1, len / 2, axis);
	const int splitIdx = len/2;
	const typename T::baseType split = m_points[offset + splitIdx][axis];

	m_nodes[idx].m_axis = static_cast<char>(axis);
	m_nodes[idx].m_split = split;

	typename T::baseType save = bounds.m_max[axis];
	bounds.m_max[axis] = split;
	m_nodes[idx].m_children[0] = Build(bounds, offset, splitIdx);
	bounds.m_max[axis] = save;
	
	save = bounds.m_min[axis];
	bounds.m_min[axis] = split;
	m_nodes[idx].m_children[1] = Build(bounds, offset + splitIdx, len - splitIdx);
	bounds.m_min[axis] = save;

	return idx;
}

template<class T>
int KdTree<T>::Nearest(const T& p) const
{
	T nearPt = p;
	int bestIdx = -1;
	typename T::baseType bestR2 = std::numeric_limits<typename T::baseType>::max();
	NearestBody(
		0,
		p,
		&nearPt,
		&bestIdx,
		&bestR2);
	return bestIdx;
}

template<class T>
void KdTree<T>::NearestBody(
	int nodeIdx,
	const T& p,
	T* nearPt, 
	int* bestIdx,
	typename T::baseType* bestR2
	) const
{
	const Node* node = &m_nodes[nodeIdx];
	if(node->m_axis == 3)
	{
		int curIdx = *bestIdx;
		typename T::baseType curR2 = *bestR2;
		for(int i = node->m_offset, c = node->m_offset + node->m_count;
			i < c; ++i)
		{
			const float d2 = LengthSq(m_points[i] - p);
			if(d2 > 0.f && d2 < curR2)
			{
				curR2 = d2;
				curIdx = i;
			}

		}
		*bestIdx = curIdx;
		*bestR2 = curR2;
		return;
	}

	const int axis = node->m_axis;
	const typename T::baseType split = node->m_split;
	const int first = p[axis] > split;

	NearestBody(node->m_children[first], p, nearPt, bestIdx, bestR2);

	const float save = (*nearPt)[axis];
	(*nearPt)[axis] = split;
	if( LengthSq(*nearPt - p) < *bestR2 )
		NearestBody(node->m_children[first ^ 1], p, nearPt, bestIdx, bestR2);
	(*nearPt)[axis] = save;
}

template<class T>
const T& KdTree<T>::GetPoint(int idx) const
{
	return m_points[idx];
}

template<class T>
T& KdTree<T>::GetPoint(int idx) 
{
	return m_points[idx];
}

template<class T>
inline int KdTree<T>::GetNumPoints() const {
	return m_points.size();
}
#endif
