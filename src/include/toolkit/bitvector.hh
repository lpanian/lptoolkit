#pragma once
#ifndef INCLUDED_toolkit_bitvector_HH
#define INCLUDED_toolkit_bitvector_HH

#include "dynary.hh"
#include <cstdint>

class BitVector
{
public:
	BitVector(int initialSize = 0, bool initialVal = false);
	BitVector(BitVector&& o);
	BitVector(const BitVector& o);
	BitVector& operator=(const BitVector& o);
	BitVector& operator=(BitVector&& o);

	void set(int index, bool value);
	bool get(int index) const;

	int size() const { return m_numBits; }
	void resize(int newSize, bool value = false);

	void push_back(bool value);

	bool operator[](int index) const;
	void swap(BitVector& other);
private:
	int m_numBits;
	DynAry<uint8_t> m_bytes;
};

#endif

