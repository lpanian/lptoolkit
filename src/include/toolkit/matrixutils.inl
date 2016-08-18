#pragma once
#ifndef INCLUDED_toolkit_matrixutils_INL
#define INCLUDED_toolkit_matrixutils_INL

#include "mathcommon.hh"
#include "quaternion.hh"

namespace lptk
{

template<class T>
vec3<T> TransformVec(const mat33<T>& m, const vec3<T>& v)
{
    T x, y, z;

    x = v.x * m.m[0];
    y = v.x * m.m[1];
    z = v.x * m.m[2];

    x += v.y * m.m[3];
    y += v.y * m.m[4];
    z += v.y * m.m[5];

    x += v.z * m.m[6];
    y += v.z * m.m[7];
    z += v.z * m.m[8];

    return lptk::vec3<T>(x, y, z);
}

template<class T>
vec3<T> TransformPoint(const mat33<T>& m, const vec3<T>& v)
{
    T x, y, z;

    x = v.x * m.m[0];
    y = v.x * m.m[1];
    z = v.x * m.m[2];

    x += v.y * m.m[3];
    y += v.y * m.m[4];
    z += v.y * m.m[5];

    x += v.z * m.m[6];
    y += v.z * m.m[7];
    z += v.z * m.m[8];

    return lptk::vec3<T>(x, y, z);
}

template<class T>
vec3<T> TransformVec(const mat44<T>& m, const vec3<T>& v)
{
    T x,y,z,w;

    x = v.x * m.m[0];
    y = v.x * m.m[1];
    z = v.x * m.m[2];
    w = v.x * m.m[3];

    x += v.y * m.m[4];
    y += v.y * m.m[5];
    z += v.y * m.m[6];
    w += v.y * m.m[7];

    x += v.z * m.m[8];
    y += v.z * m.m[9];
    z += v.z * m.m[10];
    w += v.z * m.m[11];

    w += m.m[15];

    const T inv_w = T(1) / w;
    x *= inv_w;
    y *= inv_w;
    z *= inv_w;
    return vec3<T>(x,y,z);
}

template<class T>
vec3<T> TransformPoint(const mat44<T>& m, const vec3<T>& v)
{
    T x,y,z,w;

    x = v.x * m.m[0];
    y = v.x * m.m[1];
    z = v.x * m.m[2];
    w = v.x * m.m[3];

    x += v.y * m.m[4];
    y += v.y * m.m[5];
    z += v.y * m.m[6];
    w += v.y * m.m[7];

    x += v.z * m.m[8];
    y += v.z * m.m[9];
    z += v.z * m.m[10];
    w += v.z * m.m[11];

    x += m.m[12];
    y += m.m[13];
    z += m.m[14];
    w += m.m[15];

    const T inv_w = T(1)/w;
    x*=inv_w;
    y*=inv_w;
    z*=inv_w;
    return vec3<T>(x,y,z);
}

template<class T>
void TransformFloat4(T* out, const mat44<T>& m, const T *in)
{
    T x,y,z,w;
    x = in[0] * m.m[0];
    y = in[0] * m.m[1];
    z = in[0] * m.m[2];
    w = in[0] * m.m[3];

    x += in[1] * m.m[4];
    y += in[1] * m.m[5];
    z += in[1] * m.m[6];
    w += in[1] * m.m[7];

    x += in[2] * m.m[8];
    y += in[2] * m.m[9];
    z += in[2] * m.m[10];
    w += in[2] * m.m[11];

    x += in[3] * m.m[12];
    y += in[3] * m.m[13];
    z += in[3] * m.m[14];
    w += in[3] * m.m[15];

    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = w;
}

template<class T>
mat44<T> Compute2DProj(T w, T h, T znear, T zfar)
{
    mat44<T> result;
    const T zlen = zfar - znear;
    bzero(result.m, sizeof(T)*16);
    result.m[0] = T(2) / w;
    result.m[5] = T(-2) / h;
    result.m[10] = T(-2) / (zlen);
    result.m[12] = T(-1);
    result.m[13] = T(1);
    result.m[14] = T(1) + T(2) * znear / zlen;
    result.m[15] = T(1);
    return result;
}

template<class T>
mat44<T> ComputeOrthoProj(T w, T h, T znear, T zfar)
{
    mat44<T> result;
    T zlen = zfar - znear;
    T inv_zlen = T(1) / zlen;
    bzero(result.m, sizeof(T)*16);
    result.m[0] = T(2) / w;
    result.m[5] = T(2) / h;
    result.m[10] = T(-2) * (inv_zlen);
    result.m[14] = - (zfar + znear) * inv_zlen;
    result.m[15] = T(1);
    return result;
}

template<class T>
mat44<T> Compute3DProj(T degfov, T aspect, T znear, T zfar)
{
    const T fov2 = T(0.5) * degfov * T(kDegToRadD);
    const T top = tan(fov2) * znear;
    const T bottom = -top;
    const T right = aspect * top;
    const T left = aspect * bottom;

    return ComputeFrustumProj(left, right, bottom, top, znear, zfar);
}

template<class T>
mat44<T> ComputeFrustumProj(T left, T right, T bottom, T top, T znear, T zfar)
{
    mat44<T> result;
    const T inv_r_minus_l = T(1) / (right - left);
    const T inv_t_minus_b = T(1) / (top - bottom);
    const T inv_f_minus_n = T(1) / (zfar - znear);

    result.m[0] = T(2) * znear * inv_r_minus_l;
    result.m[1] = T(0);
    result.m[2] = T(0);
    result.m[3] = T(0);

    result.m[4] = T(0);
    result.m[5] = T(2) * znear * inv_t_minus_b;
    result.m[6] = T(0);
    result.m[7] = T(0);

    result.m[8] = (right + left) * inv_r_minus_l;
    result.m[9] = (top + bottom) * inv_t_minus_b;
    result.m[10] = -(zfar + znear) * inv_f_minus_n;
    result.m[11] = -T(1);

    result.m[12] = T(0);
    result.m[13] = T(0);
    result.m[14] = -T(2) * znear * zfar * inv_f_minus_n;
    result.m[15] = T(0);
    return result;
}

template<class T>
mat44<T> RotateAround(const vec3<T>& axis, T rads)
{
    mat44<T> result;
    T c = cos(rads);
    T one_minus_c = T(1)-c;
    T s = sin(rads);
    T ax = axis.x;
    T ay = axis.y;
    T az = axis.z;

    T c1xy = one_minus_c * ax * ay;
    T c1xz = one_minus_c * ax * az;
    T c1yz = one_minus_c * ay * az;

    T sax = s * ax;
    T say = s * ay;
    T saz = s * az;

    result.m[0] = c + one_minus_c * ax * ax;
    result.m[1] = c1xy + saz;
    result.m[2] = c1xz - say;
    result.m[3] = T(0);

    result.m[4] = c1xy - saz;
    result.m[5] = c + one_minus_c * ay * ay;
    result.m[6] = c1yz + sax;
    result.m[7] = T(0);

    result.m[8] = c1xz + say;
    result.m[9] = c1yz - sax;
    result.m[10] = c + one_minus_c * az * az;
    result.m[11] = T(0);

    result.m[12] = T(0);
    result.m[13] = T(0);
    result.m[14] = T(0);
    result.m[15] = T(1);
    return result;
}

template<class T>
mat44<T> TransformInverse(const mat44<T>& m)
{
    mat44<T> dest;
    T isx = T(1)/sqrtf(m.m[0]*m.m[0] + m.m[1]*m.m[1] + m.m[2]*m.m[2]);
    T isy = T(1)/sqrtf(m.m[4]*m.m[4] + m.m[5]*m.m[5] + m.m[6]*m.m[6]);
    T isz = T(1)/sqrtf(m.m[8]*m.m[8] + m.m[9]*m.m[9] + m.m[10]*m.m[10]);
    T itx = -m.m[12];
    T ity = -m.m[13];
    T itz = -m.m[14];

    T isx2 = isx*isx;
    T isy2 = isy*isy;
    T isz2 = isz*isz;

    dest.m[0] = isx2 * m.m[0];
    dest.m[4] = isx2 * m.m[1];
    dest.m[8] = isx2 * m.m[2];

    dest.m[1] = isy2 * m.m[4];
    dest.m[5] = isy2 * m.m[5];
    dest.m[9] = isy2 * m.m[6];

    dest.m[2] = isz2 * m.m[8];
    dest.m[6] = isz2 * m.m[9];
    dest.m[10] = isz2 * m.m[10];

    dest.m[3] = T(0);
    dest.m[7] = T(0);
    dest.m[11] = T(0);

    T tx = dest.m[0] * itx;
    T ty = dest.m[1] * itx;
    T tz = dest.m[2] * itx;
    tx += dest.m[4] * ity;
    ty += dest.m[5] * ity;
    tz += dest.m[6] * ity;
    tx += dest.m[8] * itz; 
    ty += dest.m[9] * itz;
    tz += dest.m[10] * itz;

    dest.m[12] = tx;
    dest.m[13] = ty;
    dest.m[14] = tz;
    dest.m[15] = T(1);
    return dest;
}

template<class T>
mat33<T> Transpose(const mat33<T>& m)
{
    mat33<T> dest;
    for (int c = 0; c < 3; ++c)
    {
        const int c3 = c * 3;
        for (int r = 0; r < 3; ++r)
            dest.m[r * 3 + c] = m.m[c3 + r];
    }

    return dest;
}

template<class T>
mat44<T> Transpose(const mat44<T>& m)
{
    mat44<T> dest;
    for(int c = 0; c < 4; ++c)
    {
        const int c4 = c*4;
        for(int r = 0; r < 4; ++r)
            dest.m[r*4 + c] = m.m[c4 + r];
    }
    return dest;
}

template<class T>
mat44<T> MakeTranslation(T tx, T ty, T tz)
{
    return mat44<T>( 
            T(1), T(0), T(0), T(0),
            T(0), T(1), T(0), T(0),
            T(0), T(0), T(1), T(0),
            tx, ty, tz, T(1));
}

template<class T>
mat44<T> MakeTranslation(const vec3<T>& t)
{
    return mat44<T>( 
            T(1), T(0), T(0), T(0),
            T(0), T(1), T(0), T(0),
            T(0), T(0), T(1), T(0),
            t.x, t.y, t.z, T(1));
}

template<class T>
inline mat44<T> MakeScale(const vec3<T>& v) { return MakeScale(v.x, v.y, v.z); }

template<class T>
mat44<T> MakeScale(T sx, T sy, T sz)
{
    return mat44<T>(
            sx, T(0), T(0), T(0),
            T(0), sy, T(0), T(0),
            T(0), T(0), sz, T(0),
            T(0), T(0), T(0), T(1)
            );
}

template<class T>
mat44<T> MakeShear(T shXY, T shXZ, T shYZ)
{
    return mat44<T>(
        T(1), T(0), T(0), T(0),
        shXY, T(1), T(0), T(0),
        shXZ, shYZ, T(1), T(0),
        T(0), T(0), T(0), T(1));
}

template<class T>
inline mat44<T> MakeShear(const vec3<T>& v) { return MakeShear(v.x, v.y, v.z); }

template<class T>
mat33<T> MakeM33RotationX(T rad)
{
    const auto cost = lptk::Cos(rad);
    const auto sint = lptk::Sin(rad);

    mat33<T> result;
    result.m[0] = T(1);
    result.m[1] = T(0);
    result.m[2] = T(0);

    result.m[3] = T(0);
    result.m[4] = cost;
    result.m[5] = sint;
    
    result.m[6] = T(0);
    result.m[7] = -sint;
    result.m[8] = cost;

    return result;
}

template<class T>
mat44<T> MakeM44RotationX(T rad)
{
    const auto cost = lptk::Cos(rad);
    const auto sint = lptk::Sin(rad);

    mat44<T> result;
    result.m[0] = T(1);
    result.m[1] = T(0);
    result.m[2] = T(0);
    result.m[3] = T(0);

    result.m[4] = T(0);
    result.m[5] = cost;
    result.m[6] = sint;
    result.m[7] = T(0);
    
    result.m[8] = T(0);
    result.m[9] = -sint;
    result.m[10] = cost;
    result.m[11] = T(0);
    
    result.m[12] = T(0);
    result.m[13] = T(0);
    result.m[14] = T(0);
    result.m[15] = T(1);

    return result;
}


template<class T>
mat33<T> MakeM33RotationY(T rad)
{
    const auto cost = lptk::Cos(rad);
    const auto sint = lptk::Sin(rad);

    mat33<T> result;
    result.m[0] = cost;
    result.m[1] = T(0);
    result.m[2] = -sint;

    result.m[3] = T(0);
    result.m[4] = T(1);
    result.m[5] = T(0);
    
    result.m[6] = sint;
    result.m[7] = T(0);
    result.m[8] = cost;

    return result;
}

template<class T>
mat44<T> MakeM44RotationY(T rad)
{
    const auto cost = lptk::Cos(rad);
    const auto sint = lptk::Sin(rad);

    mat44<T> result;
    result.m[0] = cost;
    result.m[1] = T(0);
    result.m[2] = -sint;
    result.m[3] = T(0);

    result.m[4] = T(0);
    result.m[5] = T(1);
    result.m[6] = T(0);
    result.m[7] = T(0);
    
    result.m[8] = sint;
    result.m[9] = T(0);
    result.m[10] = cost;
    result.m[11] = T(0);
    
    result.m[12] = T(0);
    result.m[13] = T(0);
    result.m[14] = T(0);
    result.m[15] = T(1);

    return result;
}

template<class T>
mat33<T> MakeM33RotationZ(T rad)
{
    const auto cost = lptk::Cos(rad);
    const auto sint = lptk::Sin(rad);

    mat33<T> result;
    result.m[0] = cost;
    result.m[1] = sint;
    result.m[2] = T(0);

    result.m[3] = -sint;
    result.m[4] = cost;
    result.m[5] = T(0);
    
    result.m[6] = T(0);
    result.m[7] = T(0);
    result.m[8] = T(1);

    return result;
}


template<class T>
mat44<T> MakeM44RotationZ(T rad)
{
    const auto cost = lptk::Cos(rad);
    const auto sint = lptk::Sin(rad);

    mat44<T> result;
    result.m[0] = cost;
    result.m[1] = sint;
    result.m[2] = T(0);
    result.m[3] = T(0);

    result.m[4] = -sint;
    result.m[5] = cost;
    result.m[6] = T(0);
    result.m[7] = T(0);
    
    result.m[8] = T(0);
    result.m[9] = T(0);
    result.m[10] = T(1);
    result.m[11] = T(0);
    
    result.m[12] = T(0);
    result.m[13] = T(0);
    result.m[14] = T(0);
    result.m[15] = T(1);

    return result;
}

template<class T>
mat44<T> MatFromFrame(const vec3<T>& xaxis, const vec3<T>& yaxis, const vec3<T>& zaxis, const vec3<T>& trans)
{
    mat44<T> result;
    result.m[0] = xaxis.x;
    result.m[1] = xaxis.y;
    result.m[2] = xaxis.z;
    result.m[3] = T(0);

    result.m[4] = yaxis.x;
    result.m[5] = yaxis.y;
    result.m[6] = yaxis.z;
    result.m[7] = T(0);

    result.m[8] = zaxis.x;
    result.m[9] = zaxis.y;
    result.m[10] = zaxis.z;
    result.m[11] = T(0);

    result.m[12] = trans.x;
    result.m[13] = trans.y;
    result.m[14] = trans.z;
    result.m[15] = T(1);
    return result;
}

template<class T>
mat44<T> ComputeDirShadowView(const vec3<T>& focus, const vec3<T> &dir, T distance)
{
    vec3<T> a, b, c = Normalize(dir);
    vec3<T> pos = focus + distance * dir;
    // compute an orthogonal basis with a to the right, b up, dir back.
    // TODO: fix this for extreme angles
    static const vec3<T> kZAxis = vec3<T>(0,0,1);

    a = Normalize(Cross(kZAxis, dir));
    b = Normalize(Cross(dir, a)); // should already be normalized.

    mat44<T> result;
    result.m[0] = a.x;
    result.m[1] = b.x;
    result.m[2] = c.x;
    result.m[3] = T(0);

    result.m[4] = a.y;
    result.m[5] = b.y;
    result.m[6] = c.y;
    result.m[7] = T(0);

    result.m[8] = a.z;
    result.m[9] = b.z;
    result.m[10] = c.z;
    result.m[11] = T(0);

    result.m[12] = -Dot(a,pos);
    result.m[13] = -Dot(b,pos);
    result.m[14] = -Dot(c,pos);
    result.m[15] = T(1);
    return result;
}	

template<class T>
mat44<T> MakeCoordinateScale(T scale, T add)
{
    return mat44<T>(
            scale, T(0), T(0), T(0),
            T(0), scale, T(0), T(0),
            T(0), T(0), scale, T(0),
            add, add, add, T(1));
}

template<class T>
T Cofactor(const mat44<T>& m, int i, int j)
{
    ASSERT(i >= 0 && i < 4);
    ASSERT(j >= 0 && j < 4);
    static const int sel0[] = { 1, 0, 0, 0 };
    static const int sel1[] = { 2, 2, 1, 1 };
    static const int sel2[] = { 3, 3, 3, 2 };

    const int col0 = sel0[i] * 4;
    const int row0 = sel0[j];
    const int col1 = sel1[i] * 4;
    const int row1 = sel1[j];
    const int col2 = sel2[i] * 4;
    const int row2 = sel2[j];

    // [ a b c ]
    // [ d e f ] = a(ei - fh) - d(bi - ch) + g(bf - ce) = aei - afh - dbi + dch + gbf - gce
    // [ g h i ]  =  (aei + dch + gbf) - (afh + dbi + gce)

    const T aei = m.m[col0 + row0] * m.m[col1 + row1] * m.m[col2 + row2];
    const T dch = m.m[col0 + row1] * m.m[col2 + row0] * m.m[col1 + row2];
    const T gbf = m.m[col0 + row2] * m.m[col1 + row0] * m.m[col2 + row1];

    const T afh = m.m[col0 + row0] * m.m[col2 + row1] * m.m[col1 + row2];
    const T dbi = m.m[col0 + row1] * m.m[col1 + row0] * m.m[col2 + row2];
    const T gce = m.m[col0 + row2] * m.m[col2 + row0] * m.m[col1 + row1];

    const T det3x3 = (aei + dch + gbf) - (afh + dbi + gce);

    CONSTEXPR_CONST unsigned int shift = 8 * sizeof(T) - 1;
    const unsigned int signmask = ((i + j) & 1) << shift;

    union {
        T f;
        int i;
    } cofactor;

    cofactor.f = det3x3;
    cofactor.i ^= signmask;

    return cofactor.f;
}

template<class T>
mat44<T> Cofactor(const mat44<T>& m) 
{
    mat44<T> result;
    for(int j = 0, jOff = 0; j < 4; ++j, jOff+=4)
    {
        for(int i = 0; i < 4; ++i) 
        {
            result.m[i + jOff] = Cofactor(m, j, i);
        }
    }
    return result;
}

template<class T> 
T Det44(const mat44<T>& m)
{
    T result = 0.f;
    result += m.m[0] * Cofactor(m, 0, 0);
    result += m.m[1] * Cofactor(m, 0, 1);
    result += m.m[2] * Cofactor(m, 0, 2);
    result += m.m[3] * Cofactor(m, 0, 3);
    return result;
}

template<class T>
T Det33(const mat44<T>& m)
{
    return m.m[0] * (m.m[5] * m.m[10] - m.m[9] * m.m[6])
        -  m.m[4] * (m.m[1] * m.m[10] - m.m[9] * m.m[2])
        +  m.m[8] * (m.m[1] * m.m[6] - m.m[5] * m.m[2]);
}

template<class T>
mat44<T> GeneralInverse(const mat44<T>& m)
{
    const T invdet = T(1) / Det44(m);
    const mat44<T> cof = Cofactor(m);
    const mat44<T> adj = Transpose(cof);
    return invdet * adj; 
}

//template<class T>
//mat44<T> GeneralInverse(const mat44<T>& m)
//{
//    mat44<T> result = m;
//    int indexCol[4] = {0};
//    int indexRow[4] = {0};
//    int ipiv[4] = {0};
//    int irow = 0, icol = 0;
//    for(int i = 0; i < 4; ++i)
//    {
//        T biggest = T(0);
//        for(int j = 0; j < 4; ++j) 
//        {
//            if(ipiv[j] != 1)
//            {
//                for(int k = 0; k < 4; ++k)
//                {
//                    if(ipiv[k]) 
//                        continue;
//                    const int offset = j + k*4;
//                    if(Abs(result[offset]) > biggest) 
//                    {
//                        biggest = result[offset];
//                        irow = j;
//                        icol = k;
//                    }
//                }
//            }
//        }
//
//        ASSERT(irow >= 0);
//        ASSERT(icol >= 0);
//        ++ipiv[icol];
//
//        if(irow != icol) 
//        {
//            for(int c = 0; c < 4; ++c) 
//            {
//                std::swap(result[icol + 4*c], result[irow + 4*c]);
//            }
//        }
//        indexRow[i] = irow;
//        indexCol[i] = icol;
//
//        if(result[icol + icol*4] == T(0)) 
//        {
//            return mat44<T>();
//            // how to report this error?
//        }
//
//        const int pivotOffset = icol + icol*4;
//        const T pivotInv = T(1) / result[pivotOffset];
//        result[pivotOffset] = T(1);
//        for(int c = 0; c < 4; ++c) 
//        {
//            result[icol + 4*c] *= pivotInv;
//        }
//
//        for(int c = 0; c < 4; ++c) 
//        {
//            if(c == icol)
//                continue;
//
//            const T dum = result[c + icol*4];
//            result[c + icol*4] = T(0);
//            for(int j = 0; j < 4; ++j)
//            {
//                result[c + 4*j] -= result[icol + 4*j] * dum;
//            }
//        }
//    }
//
//    for(int i = 3; i >= 0; --i) 
//    {
//        if(indexRow[i] != indexCol[i]) 
//        {
//            for(int k = 0; k < 4; ++k)
//            {
//                std::swap(result[k + 4*indexRow[i]],
//                    result[k + 4*indexCol[i]]); 
//            }
//        }
//    }
//    
//    return result;
//}

template<class T>
mat44<T> MatLookAt_CameraSpace(const vec3<T>& eyePos, const vec3<T>& xAxis,
    const vec3<T>& yAxis,
    const vec3<T>& zAxis)
{
    mat44<T> result;
    result.m[0] = xAxis.x;
    result.m[4] = xAxis.y;
    result.m[8] = xAxis.z;
    result.m[12] = Dot(xAxis, -eyePos);
    
    result.m[1] = yAxis.x;
    result.m[5] = yAxis.y;
    result.m[9] = yAxis.z;
    result.m[13] = Dot(yAxis, -eyePos);
    
    result.m[2] = zAxis.x;
    result.m[6] = zAxis.y;
    result.m[10] = zAxis.z;
    result.m[14] = Dot(zAxis, -eyePos);

    result.m[3] = result.m[7] = result.m[11] = 0.f;
    result.m[15] = 1.f;
    return result;
}

template<class T>
mat44<T> MatLookAt_WorldSpace(const vec3<T>& eyePos, const vec3<T>& xAxis,
    const vec3<T>& yAxis,
    const vec3<T>& zAxis)
{
    mat44<T> result;
    result.m[0] = xAxis.x;
    result.m[1] = xAxis.y;
    result.m[2] = xAxis.z;
    result.m[3] = 0.f;
    
    result.m[4] = yAxis.x;
    result.m[5] = yAxis.y;
    result.m[6] = yAxis.z;
    result.m[7] = 0.f;
    
    result.m[8] = zAxis.x;
    result.m[9] = zAxis.y;
    result.m[10] = zAxis.z;
    result.m[11] = 0.f;
    
    result.m[12] = eyePos.x;
    result.m[13] = eyePos.y;
    result.m[14] = eyePos.z;
    result.m[15] = 1.f;
    
    return result;
}

template<class T>
mat44<T> MatLookAt_CameraSpace(const vec3<T>& eyePos, const vec3<T>& target, const vec3<T>& up)
{
    const vec3<T> zAxis = Normalize(eyePos - target); // -(target - eyePos)
    const vec3<T> xAxis = Normalize(Cross(up, zAxis));
    const vec3<T> yAxis = Cross(zAxis, xAxis);

    return MatLookAt_CameraSpace(eyePos, xAxis, yAxis, zAxis);
}

template<class T>
mat44<T> MatLookAt_CameraSpace(const vec3<T>& eyePos, const vec3<T>& axis, float angle)
{
    const auto zAxis = Rotate(lptk::v3f{ 0,0,1 }, lptk::MakeRotation(angle, axis));
    const auto yAxis = axis;
    const auto xAxis = Cross(yAxis, zAxis);

    return MatLookAt_CameraSpace(eyePos, xAxis, yAxis, zAxis);
}

template<class T>
mat44<T> MatLookAt_WorldSpace(const vec3<T>& eyePos, const vec3<T>& target, const vec3<T>& up)
{
    const vec3<T> zAxis = Normalize(eyePos - target); // -(target - eyePos)
    const vec3<T> xAxis = Normalize(Cross(up, zAxis));
    const vec3<T> yAxis = Cross(zAxis, xAxis);

    return MatLookAt_WorldSpace(eyePos, xAxis, yAxis, zAxis);
}

template<class T>
mat44<T> MatLookAt_WorldSpace(const vec3<T>& eyePos, const vec3<T>& axis, float angle)
{
    const auto zAxis = Rotate(lptk::v3f{ 0,0,1 }, lptk::MakeRotation(angle, axis)) ;
    const auto yAxis = axis;
    const auto xAxis = Cross(yAxis, zAxis);

    return MatLookAt_WorldSpace(eyePos, xAxis, yAxis, zAxis);
}

template<class T>
void RemovePerspectivePartition(const mat44<T>& m, mat44<T>& result)
{
    result = m;
    result.m[3] = 0.f;
    result.m[7] = 0.f;
    result.m[11] = 0.f;
    result.m[15] = 1.f;
}

template<class T>
void ExtractScaleAndShear(const mat44<T>& m, mat44<T>& result, vec3<T>& scale, vec3<T>& shear)
{
    // col0 = scaleX * rot0
    // col1 = scaleY * (shearXY * rot0 + rot1)
    // col2 = scaleZ * (shearXZ * rot0 + shearYZ * rot1 + rot2)

    vec3<T> col[3] = {
        { m.m[0], m.m[1], m.m[2] },
        { m.m[4], m.m[5], m.m[6] },
        { m.m[8], m.m[9], m.m[10] },
    };

    // save X scale and remove it from the working column.
    result = m;
    scale.x = Length(col[0]);
    const T invScaleX = T(1) / scale.x;
    col[0] *= invScaleX;

    // Dot(rot0, scaleY * shearXY * rot0) + Dot(rot0, scaleY * rot1)
    // = scaleY * shearXY
    const T shearXYbyScaleY = Dot(col[0], col[1]);

    // remove the shearing from the column
    col[1] -= shearXYbyScaleY * col[0];

    // save Y scale and remove it from the second column
    scale.y = Length(col[1]);
    const T invScaleY = T(1) / scale.y;
    col[1] *= invScaleY;

    // adjust shear by the extra scaling factor
    shear[0] = shearXYbyScaleY * invScaleY;

    const T shearXZByScaleZ = Dot(col[0], col[2]);
    const T shearYZByScaleZ = Dot(col[1], col[2]);

    col[2] -= shearXZByScaleZ * col[0];
    col[2] -= shearYZByScaleZ * col[1];

    scale.z = Length(col[2]);
    const T invScaleZ = T(1) / scale.z;
    col[2] *= invScaleZ;

    shear[1] = shearXZByScaleZ * invScaleZ;
    shear[2] = shearYZByScaleZ * invScaleZ;

   
    result.m[0] = col[0][0];
    result.m[1] = col[0][1];
    result.m[2] = col[0][2];
    result.m[3] = T(0);
    
    result.m[4] = col[1][0];
    result.m[5] = col[1][1];
    result.m[6] = col[1][2];
    result.m[7] = T(0);
    
    result.m[8] = col[2][0];
    result.m[9] = col[2][1];
    result.m[10] = col[2][2];
    result.m[11] = T(0);

    result.m[12] = T(0);
    result.m[13] = T(0);
    result.m[14] = T(0);
    result.m[15] = T(1);

    if(Det33(result) < T(0)) {
        result *= T(-1);
        scale *= T(-1);
    }
}

template<class T>
void ExtractSHRT(const mat44<T>& m, vec3<T>& scale, vec3<T>& shear, quaternion<T>& rot, vec3<T>& translate)
{
    mat44<T> rotm;
    ExtractScaleAndShear(m, rotm, scale, shear);

    translate.x = m.m[12];
    translate.y = m.m[13];
    translate.z = m.m[14];

    rot = quaternion<T>(rotm);
}
   

// NOTE: currently this method of decomposing doesn't work for some rotation
// matrices (ie: around z=1). The problem is that the inv(trans(M)) of a
// rotation around Z gives a matrix that zeroes out the important components,
// and normalizes the errors to 1 after many iterations.
template<class T>
void MatrixDecompose(const mat44<T>& mat, vec3<T>* trans, quaternion<T>* rot, mat44<T>* scale)
{
    static CONSTEXPR_CONST int kMaxNumIters = 100;
    ASSERT(trans && rot && scale);
    // extract the last column as translation.
    trans->x = mat.m[12];
    trans->y = mat.m[13];
    trans->z = mat.m[14];

    mat44<T> m = mat;
    m.m[14] = m.m[13] = m.m[12] = T(0);
    m.m[15] = T(1);

    T normError = T(0);
    int count = 0;
    mat44<T> curM = m;
    do {
        mat44<T> MinvTrans = GeneralInverse(Transpose(curM));
        mat44<T> nextM = T(0.5) * (curM + MinvTrans);

        normError = NormDiff(nextM, curM);
        curM = nextM;
    } while(++count < kMaxNumIters && normError > T(1e-4));
    *rot = quaternion<T>(curM);
    *scale = GeneralInverse(curM) * m;
}

}

#endif
