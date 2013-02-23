#include "toolkit/color.hh"
#include "toolkit/memstream.hh"
#include "toolkit/serialize.hh"

static const float g_inv255 = 1.f/255.f;
Color::Color(const ColorRGBA& c)
	: r(float(c.r))
	, g(float(c.g))
	, b(float(c.b))
	, a(float(c.a)) 
{
	r *= g_inv255; g *= g_inv255; b *= g_inv255; a *= g_inv255;
}

ColorRGBA::ColorRGBA(const Color& c) 
	: r(uint8_t(Clamp(int(c.r * 255.f), 0, 255)))
	, g(uint8_t(Clamp(int(c.g * 255.f), 0, 255)))
	, b(uint8_t(Clamp(int(c.b * 255.f), 0, 255)))
	, a(uint8_t(Clamp(int(c.a * 255.f), 0, 255))) 
{}

void Color::SerializeTo(MemSerializer& serializer) const
{
	serializer.PutColor(*this);
}

void ColorRGBA::SerializeTo(MemSerializer& serializer) const
{
	serializer.PutColorRGBA(*this);
}


