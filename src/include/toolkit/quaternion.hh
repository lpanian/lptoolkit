#pragma once
#ifndef INCLUDED_toolkit_quaternion_HH
#define INCLUDED_toolkit_quaternion_HH

#include <cmath>
#include <ostream>

#include "vec.hh"
#include "matrix.hh"
#include "mathcommon.hh"

namespace lptk
{
    template<class T>
    class quaternion
    {
    public:
        using baseType = T;
        static constexpr auto Dims = 4;
        static inline auto dims() { return Dims; }

        T a, b, c, r;

        enum NoRotationType { no_rotation };

        inline quaternion() {}
        inline quaternion(NoRotationType)
            : a(0), b(0), c(0), r(1) {}
        inline quaternion(T a, T b, T c, T r) :
            a(a), b(b), c(c), r(r) {}
        inline quaternion(const vec3<T>& v, T r) :
            a(v.x), b(v.y), c(v.z), r(r) { }
        inline quaternion(const quaternion& q) : a(q.a), b(q.b), c(q.c), r(q.r) { }
        quaternion(const mat44<T>& m);
        quaternion(const mat33<T>& m);

        inline bool AllNumeric() const { return !isnan(a) && !isnan(b) && !isnan(c) && !isnan(r); }

        inline quaternion& operator=(const quaternion& q) {
            if (this != &q) {
                a = q.a;
                b = q.b;
                c = q.c;
                r = q.r;
            }
            return *this;
        }

        inline void Set(T qa, T qb, T qc, T qr) {
            a = qa;
            b = qb;
            c = qc;
            r = qr;
        }

        inline quaternion operator-() const {
            return quaternion(-a, -b, -c, -r);
        }

        inline quaternion& operator+=(const quaternion& o) {
            r += o.r;
            a += o.a;
            b += o.b;
            c += o.c;
            return *this;
        }


        inline quaternion& operator-=(const quaternion& o) {
            r -= o.r;
            a -= o.a;
            b -= o.b;
            c -= o.c;
            return *this;
        }

        inline void Conjugate() {
            a = -a;
            b = -b;
            c = -c;
        }

        inline quaternion& operator*=(T f) {
            r *= f;
            a *= f;
            b *= f;
            c *= f;
            return *this;
        }

        inline quaternion operator/=(T f) {
            T inv_f = T(1) / f;
            *this *= inv_f;
            return *this;
        }

        mat44<T> ToMatrix44() const;
        mat33<T> ToMatrix33() const;

        inline bool operator==(const quaternion& o) const {
            return r == o.r &&
                a == o.a &&
                b == o.b &&
                c == o.c;
        }

        inline bool operator!=(const quaternion& o) const {
            return !(*this == o);
        }

        T operator[](unsigned i) const {
            ASSERT(i < 4);
            return (&a)[i];
        }

        T& operator[](unsigned i) {
            ASSERT(i < 4);
            return (&a)[i];
        }

    };

    template<class T>
    inline quaternion<T> Conjugate(const quaternion<T>&  o) {
        return quaternion<T>(-o.a, -o.b, -o.c, o.r);
    }

    template<class T>
    inline T MagnitudeSq(const quaternion<T>&  v) {
        return v.r*v.r + v.a*v.a + v.b*v.b + v.c*v.c;
    }

    template<class T>
    inline T Magnitude(const quaternion<T>& v) {
        return sqrt(MagnitudeSq(v));
    }

    template<class T>
    inline quaternion<T> operator+(const quaternion<T>& l, const quaternion<T>& r) {
        return quaternion<T>(l.a + r.a,
            l.b + r.b,
            l.c + r.c,
            l.r + r.r);
    }

    template<class T>
    inline quaternion<T> operator-(const quaternion<T>& l, const quaternion<T>& r) {
        return quaternion<T>(l.a - r.a,
            l.b - r.b,
            l.c - r.c,
            l.r - r.r);
    }

    template<class T>
    inline quaternion<T> operator*(const quaternion<T>&  l, const quaternion<T>& r) {
        T tr = l.r;
        T ta = l.a;
        T tb = l.b;
        T tc = l.c;
        T nr = tr * r.r - ta * r.a - tb * r.b - tc * r.c;
        T na = tr * r.a + ta * r.r + tb * r.c - tc * r.b;
        T nb = tr * r.b + tb * r.r + tc * r.a - ta * r.c;
        T nc = tr * r.c + tc * r.r + ta * r.b - tb * r.a;
        return quaternion<T>(na, nb, nc, nr);
    }

    template<class T>
    inline quaternion<T> operator*(const quaternion<T>&  o, T f) {
        return quaternion<T>(o.a*f, o.b*f, o.c*f, o.r*f);
    }

    template<class T>
    inline quaternion<T> operator*(T f, const quaternion<T>&  o) {
        return quaternion<T>(o.a*f, o.b*f, o.c*f, o.r*f);
    }

    template<class T>
    inline quaternion<T> operator/(const quaternion<T>&  lhs, T f) {
        return lhs * (T(1) / f);
    }

    template<class T>
    inline quaternion<T> operator/(const quaternion<T>&  lhs, const quaternion<T>& rhs) {
        T div = MagnitudeSq(rhs);
        quaternion<T> q = lhs * (T(2) * rhs.r) - lhs * rhs;
        return q / div;
    }

    template<class T>
    inline quaternion<T> Inverse(const quaternion<T>& v) {
        return Conjugate(v) / MagnitudeSq(v);
    }

    template<class T>
    inline quaternion<T> Normalize(const quaternion<T>& q) {
        return q / Magnitude(q);
    }
    
    template<class T>
    inline quaternion<T> NormalizeSafe(const quaternion<T>& q, const quaternion<T>& def = quaternion<T>::no_rotation) {
        const auto m = Magnitude(q);
        if (m > 0)
            return q / m;
        else
            return def;
    }

    template<class T>
    quaternion<T> MakeRotation(T radians, const vec3<T>& vec);

    template<class T>
    quaternion<T> MakeRotation(const vec3<T>& fromVec, const vec3<T>& toVec);

    template<class T>
    void GetAxisAngle(const quaternion<T>& q, vec3<T>& axis, T &rotation);

    // assumes q is unit length - otherwise conjugate should be inverse
    template<class T>
    inline vec3<T> Rotate(const vec3<T>& p, const quaternion<T>& q) {
        quaternion<T> result = q * quaternion<T>(p, 0) * Conjugate(q);
        return vec3<T>(result.a, result.b, result.c);
    }

    template<class T>
    quaternion<T> Slerp(
        const quaternion<T>&  left,
        const quaternion<T>&  right,
        T param);

    template<class T>
    inline T Dot(const quaternion<T>& lhs, const quaternion<T>& rhs)
    {
        return lhs.a * rhs.a + lhs.b * rhs.b + lhs.c * rhs.c + lhs.r * rhs.r;
    }

    template<class T>
    inline bool Equal(const quaternion<T>& lhs, const quaternion<T>& rhs, T eps = T(1e-3))
    {
        return Equal(lhs.r, rhs.r, eps) &&
            Equal(lhs.a, rhs.a, eps) &&
            Equal(lhs.b, rhs.b, eps) &&
            Equal(lhs.c, rhs.c, eps);
    }

    template<class T>
    inline std::ostream& operator<<(std::ostream& s, const quaternion<T>& q) {
        s << '<' << q.a << ',' << q.b << ',' << q.c << ',' << q.r << '>';
        return s;
    }

    typedef quaternion<float> quatf;
    typedef quaternion<double> quatd;

    template<class To, class From>
    inline quaternion<To> ConvertQuat(const quaternion<From>& v) {
        return quaternion<To>{To(v[0]), To(v[1]), To(v[2]), To(v[3])};
    }
}

#include "quaternion.inl"


#endif

