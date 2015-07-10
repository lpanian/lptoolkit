#pragma once
#ifndef INCLUDED_toolkit_kdtree_HH
#define INCLUDED_toolkit_kdtree_HH

//#include "dynary.hh"
#include <vector>
#include "aabb.hh"

namespace lptk
{

// T must be a vector type
template<class T>
class KdTree
{
public:
	KdTree();
	void Init(const T* points, uint32_t numPoints, uint32_t bucketSize);

	int Nearest(const T& p) const;

	uint32_t GetNumPoints() const ;
	const T& GetPoint(uint32_t idx) const;
	T& GetPoint(uint32_t idx) ;
private:
	////////////////////////////////////////	
	struct Node {
		// type of node
		int m_axis;
			
		typename T::baseType m_split;
		int m_children[2];

		// point list
		uint32_t m_offset;
		uint32_t m_count;
	};

	////////////////////////////////////////	
	uint32_t Build(typename BoxType<T>::type& bounds, uint32_t offset, uint32_t len);
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
	uint32_t m_bucketSize;
};

}

#include "kdtree.inl"

#endif

