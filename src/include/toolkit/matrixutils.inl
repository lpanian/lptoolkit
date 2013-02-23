#pragma once
#ifndef INCLUDED_toolkit_matrixutils_INL
#define INCLUDED_toolkit_matrixutils_INL

template<class T>
vec3<T> TransformVec(const mat44<T>& m, const vec3<T>& v)
{
	T x,y,z;

	x = v.x * m.m[0];
	y = v.x * m.m[1];
	z = v.x * m.m[2];

	x += v.y * m.m[4];
	y += v.y * m.m[5];
	z += v.y * m.m[6];
	
	x += v.z * m.m[8];
	y += v.z * m.m[9];
	z += v.z * m.m[10];

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

	T inv_w = T(1)/w;
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
mat44<T> Transpose(const mat44<T>& m)
{
	mat44<T> dest;
	for(int c = 0; c < 4; ++c)
	{
		int c4 = c*4;
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
mat44<T> MakeScale(const vec3<T>& v) { return MakeScale(v.x, v.y, v.z); }
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

#endif
