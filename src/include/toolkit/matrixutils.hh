#pragma once
#ifndef INCLUDED_toolkit_matrixutils_HH
#define INCLUDED_toolkit_matrixutils_HH

#include "matrix.hh"

////////////////////////////////////////////////////////////////////////////////
// utilities

template<class T>
vec3<T> TransformVec(const mat44<T>& m, const vec3<T>& v);
template<class T>
vec3<T> TransformPoint(const mat44<T>& m, const vec3<T>& v);
template<class T>
void TransformFloat4(T* out, const mat44<T>& m, const T *in);

// ortho projection specifically for the front end. Basically y is inversedin a weird way.
template<class T>
mat44<T> Compute2DProj(T w, T h, T znear, T zfar);
// normal ortho projection
template<class T>
mat44<T> ComputeOrthoProj(T w, T h, T znear, T zfar);
template<class T>
mat44<T> Compute3DProj(T degfov, T aspect, T znear, T zfar);
template<class T>
mat44<T> ComputeFrustumProj(T left, T right, T bottom, T top, T znear, T zfar);
template<class T>
mat44<T> ComputeDirShadowView(const vec3<T>& focus, const vec3<T> &dir, T distance);

template<class T>
mat44<T> RotateAround(const vec3<T>& axis, T rads);
template<class T>
mat44<T> TransformInverse(const mat44<T>& m);
template<class T>
mat44<T> Transpose(const mat44<T>& m);
template<class T>
mat44<T> MakeTranslation(T tx, T ty, T tz);
template<class T>
mat44<T> MakeTranslation(const vec3<T>& t);
template<class T>
mat44<T> MakeScale(const vec3<T>& v) ;
template<class T>
mat44<T> MakeScale(T sx, T sy, T sz);
template<class T>
mat44<T> MatFromFrame(const vec3<T>& xaxis, const vec3<T>& yaxis, const vec3<T>& zaxis, const vec3<T>& trans);
template<class T>
mat44<T> MakeCoordinateScale(T scale, T add);

template<class T>
inline mat44<T> TransposeOfInverse(const mat44<T>& m) {
	return Transpose(TransformInverse(m));
}

#include "matrixutils.inl"

#endif
