#pragma once
#ifndef INCLUDED_toolkit_bitvector_HH
#define INCLUDED_toolkit_bitvector_HH

#include "dynary.hh"
#include <cstdint>

namespace lptk {

class BitVector
{
public:
	BitVector(size_t initialSize = 0, bool initialVal = false);
	BitVector(BitVector&& o);
	BitVector(const BitVector& o);
	BitVector& operator=(const BitVector& o);
	BitVector& operator=(BitVector&& o);

	void set(size_t index, bool value);
	bool get(size_t index) const;

	size_t size() const { return m_numBits; }
	void resize(size_t newSize, bool value = false);

	void push_back(bool value);

	bool operator[](size_t index) const;
	void swap(BitVector& other);
private:
	size_t m_numBits;
	DynAry<uint8_t> m_bytes;
};

}

#endif

