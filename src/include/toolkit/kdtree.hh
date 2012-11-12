#pragma once
#ifndef INCLUDED_kdtree_hh
#define INCLUDED_kdtree_hh

#include <vector>
#include "aabb.hh"

// T must be a vector type
template<class T>
class KdTree
{
public:
	KdTree();
	void Init(const T* points, int numPoints, int bucketSize);

	int Nearest(const T& p) const;

	int GetNumPoints() const ;
	const T& GetPoint(int idx) const;
	T& GetPoint(int idx) ;
private:
	////////////////////////////////////////	
	struct Node {
		// type of node
		int m_axis;
			
		typename T::baseType m_split;
		int m_children[2];

		// point list
		int m_offset;
		int m_count;
	};

	////////////////////////////////////////	
	int Build(typename BoxType<T>::type& bounds, int offset, int len);
	void Validate(int idx);

	void NearestBody(
		int nodeIdx,
		const T& p,
		T* nearPt, 
		int* bestIdx,
		typename T::baseType* bestR2
		) const;

	////////////////////////////////////////	
	std::vector<T> m_points;
	std::vector<Node> m_nodes;
	typename BoxType<T>::type m_bounds;
	int m_bucketSize;
};

#include "kdtree.inl"

#endif

