#pragma once
#ifndef INCLUDED_toolkit_kdtree_INL
#define INCLUDED_toolkit_kdtree_INL

#include <algorithm>
#include <iostream>

namespace lptk
{

namespace detail 
{
	// ensure all points less than the pivot are on the left side, and all points
	// right of the pivot are on the right side. Points equal to the pivot can
	// end up on either side.
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
void KdTree<T>::Init(const T* points, uint32_t numPoints, uint32_t bucketSize)
{
	m_points.clear();
	m_points.reserve(numPoints);
	m_points.insert(m_points.begin(), points, points + numPoints);
	m_bucketSize = Max(bucketSize, uint32_t(2));
	for(auto i = uint32_t(0); i < numPoints; ++i)
		m_bounds.Extend(points[i]);

	auto box = m_bounds;
	Build(box, 0, static_cast<uint32_t>(m_points.size()));
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
uint32_t KdTree<T>::Build(typename BoxType<T>::type& bounds, uint32_t offset, uint32_t len)
{
	const auto idx = m_nodes.size();
	m_nodes.emplace_back();

	m_nodes[idx].m_split = typename T::baseType(0);
	m_nodes[idx].m_offset = offset;
	m_nodes[idx].m_count = len;
	m_nodes[idx].m_children[0] = -1;
	m_nodes[idx].m_children[1] = -1;

	if(len <= m_bucketSize)
	{
		m_nodes[idx].m_axis = 3 ;
		return static_cast<uint32_t>(idx);
	}

	const int axis = bounds.MajorAxis();
	detail::Select(&m_points[offset], 0, len - 1, len / 2, axis);
	const int splitIdx = len/2;
	const typename T::baseType split = m_points[offset + splitIdx][axis];

	m_nodes[idx].m_axis = static_cast<char>(axis);
	m_nodes[idx].m_split = split;

	typename T::baseType save = bounds.m_max[axis];
	bounds.m_max[axis] = split;
	const int leftChild = Build(bounds, offset, splitIdx);
	m_nodes[idx].m_children[0] = leftChild;
	bounds.m_max[axis] = save;
	
	save = bounds.m_min[axis];
	bounds.m_min[axis] = split;
	const int rightChild = Build(bounds, offset + splitIdx, len - splitIdx);
	m_nodes[idx].m_children[1] = rightChild;
	bounds.m_min[axis] = save;

	return static_cast<uint32_t>(idx);
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
const T& KdTree<T>::GetPoint(uint32_t idx) const
{
	return m_points[idx];
}

template<class T>
T& KdTree<T>::GetPoint(uint32_t idx) 
{
	return m_points[idx];
}

template<class T>
inline uint32_t KdTree<T>::GetNumPoints() const {
	return static_cast<uint32_t>(m_points.size());
}

}

#endif
