#pragma once
#ifndef INCLUDED_toolkit_color_HH
#define INCLUDED_toolkit_color_HH

class MemSerializer;

#include "mathcommon.hh"

////////////////////////////////////////////////////////////////////////////////
class ColorRGBA;
class Color
{
public:
	Color() {}
	explicit Color(float f) : r(f), g(f), b(f), a(f) {}
	Color(float r_, float g_, float b_) : r(r_), g(g_), b(b_), a(1.f) {}
	Color(float r_, float g_, float b_, float a_) : r(r_), g(g_), b(b_), a(a_) {}
	explicit Color(const ColorRGBA& c);
	bool operator==(const Color& r) const;
	bool operator!=(const Color& r) const;
	Color operator+(const Color& o) const;
	Color& operator+=(const Color& o) ;
	Color operator*(float f) const;
	Color operator/(float f) const;
	void operator/=(float f);

	void SerializeTo(MemSerializer&) const;
	float r, g, b, a;
} ;

inline Color Pow(const Color& c, float f)
{
	return Color(powf(c.r, f), powf(c.g, f), powf(c.b, f), powf(c.a, f));
}

inline Color Min(const Color& l, const Color& r) {
	return Color(Min(l.r, r.r),
		Min(l.g, r.g),
		Min(l.b, r.b),
		Min(l.a, r.a));
}

inline Color Max(const Color& l, const Color& r) {
	return Color(Max(l.r, r.r),
		Max(l.g, r.g),
		Max(l.b, r.b),
		Max(l.a, r.a));
}

inline bool Color::operator==(const Color& o) const {
	return r == o.r && g == o.g && b == o.b && a == o.a;
}

inline bool Color::operator!=(const Color& o) const {
	return r != o.r || g != o.g || b != o.b || a != o.a;
}

inline Color Color::operator+(const Color& o) const
{
	return Color(r + o.r, g + o.g, b + o.b, a + o.a);
}

inline Color& Color::operator+=(const Color& o) 
{
	r += o.r;
	g += o.g;
	b += o.b;
	a += o.a;
	return *this;
}

inline Color Color::operator*(float f) const
{
	return Color(r * f, g * f, b * f, a * f);
}

inline Color operator*(float f, const Color& c) 
{
	return Color(c.r * f, c.g * f, c.b * f, c.a * f);
}

inline Color Color::operator/(float f) const
{
	float inv_f = 1.f/ f;
	return inv_f * (*this);
}

inline void Color::operator/=(float f)
{
	float inv_f = 1.f/ f;
	r *= inv_f;
	g *= inv_f;
	b *= inv_f;
	a *= inv_f;
}


////////////////////////////////////////////////////////////////////////////////
class ColorRGBA
{
public:
	ColorRGBA() {}

	explicit ColorRGBA(uint8_t f) : r(f), g(f), b(f), a(f) {}
	ColorRGBA(uint8_t r_, uint8_t g_, uint8_t b_) : r(r_), g(g_), b(b_), a(255) {}
	ColorRGBA(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_) : r(r_), g(g_), b(b_), a(a_) {}
	explicit ColorRGBA(const Color& c) ;

	bool operator==(const ColorRGBA& r) const;
	bool operator!=(const ColorRGBA& r) const;
	ColorRGBA operator+(const ColorRGBA& o) const;
	ColorRGBA& operator+=(const ColorRGBA& o) ;

	void SerializeTo(MemSerializer&) const;
	uint8_t r, g, b, a;
};

inline ColorRGBA Min(const ColorRGBA& l, const ColorRGBA& r) {
	return ColorRGBA(Min(l.r, r.r),
		Min(l.g, r.g),
		Min(l.b, r.b),
		Min(l.a, r.a));
}

inline ColorRGBA Max(const ColorRGBA& l, const ColorRGBA& r) {
	return ColorRGBA(Max(l.r, r.r),
		Max(l.g, r.g),
		Max(l.b, r.b),
		Max(l.a, r.a));
}

inline bool ColorRGBA::operator==(const ColorRGBA& o) const {
	return r == o.r && g == o.g && b == o.b && a == o.a;
}

inline bool ColorRGBA::operator!=(const ColorRGBA& o) const {
	return r != o.r || g != o.g || b != o.b || a != o.a;
}

inline ColorRGBA ColorRGBA::operator+(const ColorRGBA& o) const
{
	return ColorRGBA(r + o.r, g + o.g, b + o.b, a + o.a);
}

inline ColorRGBA& ColorRGBA::operator+=(const ColorRGBA& o) 
{
	r += o.r;
	g += o.g;
	b += o.b;
	a += o.a;
	return *this;
}


#endif
