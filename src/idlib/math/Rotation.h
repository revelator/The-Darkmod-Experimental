/*****************************************************************************
                    The Dark Mod GPL Source Code

 This file is part of the The Dark Mod Source Code, originally based
 on the Doom 3 GPL Source Code as published in 2011.

 The Dark Mod Source Code is free software: you can redistribute it
 and/or modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation, either version 3 of the License,
 or (at your option) any later version. For details, see LICENSE.TXT.

 Project: The Dark Mod (http://www.thedarkmod.com/)

 $Revision$ (Revision of last commit)
 $Date$ (Date of last commit)
 $Author$ (Author of last commit)

******************************************************************************/

#ifndef __MATH_ROTATION_H__
#define __MATH_ROTATION_H__

/*
===============================================================================

	Describes a complete rotation in degrees about an abritray axis.
	A local rotation matrix is stored for fast rotation of multiple points.

===============================================================================
*/

class idAngles;
class idQuat;
class idMat3;

class idRotation {
	friend class idAngles;
	friend class idQuat;
	friend class idMat3;

public:
						idRotation( void );
						idRotation( const idVec3 &rotationOrigin, const idVec3 &rotationVec, const float rotationAngle );

	void				Set( const idVec3 &rotationOrigin, const idVec3 &rotationVec, const float rotationAngle );
	void				SetOrigin( const idVec3 &rotationOrigin );
	void				SetVec( const idVec3 &rotationVec );					// has to be normalized
	void				SetVec( const float x, const float y, const float z );	// has to be normalized
	void				SetAngle( const float rotationAngle );
	void				Scale( const float s );
	void				ReCalculateMatrix( void );
	const idVec3 &		GetOrigin( void ) const;
	const idVec3 &		GetVec( void ) const;
	float				GetAngle( void ) const;

	idRotation			operator-() const;										// flips rotation
	idRotation			operator*( const float s ) const;						// scale rotation
	idRotation			operator/( const float s ) const;						// scale rotation
	idRotation &		operator*=( const float s );							// scale rotation
	idRotation &		operator/=( const float s );							// scale rotation
	idVec3				operator*( const idVec3 &v ) const;						// rotate vector

	friend idRotation	operator*( const float s, const idRotation &r );		// scale rotation
	friend idVec3		operator*( const idVec3 &v, const idRotation &r );		// rotate vector
	friend idVec3 &		operator*=( idVec3 &v, const idRotation &r );			// rotate vector

	idAngles			ToAngles( void ) const;
	idQuat				ToQuat( void ) const;
	const idMat3 &		ToMat3( void ) const;
	idMat4				ToMat4( void ) const;
	idVec3				ToAngularVelocity( void ) const;

	void				RotatePoint( idVec3 &point ) const;

	void				Normalize180( void );
	void				Normalize360( void );

private:
	idVec3				origin;			// origin of rotation
	idVec3				vec;			// normalized vector to rotate around
	float				angle;			// angle of rotation in degrees
	mutable idMat3		axis;			// rotation axis
	mutable bool		axisValid;		// true if rotation axis is valid
};

ID_FORCE_INLINE idRotation::idRotation( void ) {
}

ID_FORCE_INLINE idRotation::idRotation( const idVec3 &rotationOrigin, const idVec3 &rotationVec, const float rotationAngle ) {
	origin = rotationOrigin;
	vec = rotationVec;
	angle = rotationAngle;
	axisValid = false;
}

ID_FORCE_INLINE void idRotation::Set( const idVec3 &rotationOrigin, const idVec3 &rotationVec, const float rotationAngle ) {
	origin = rotationOrigin;
	vec = rotationVec;
	angle = rotationAngle;
	axisValid = false;
}

ID_FORCE_INLINE void idRotation::SetOrigin( const idVec3 &rotationOrigin ) {
	origin = rotationOrigin;
}

ID_FORCE_INLINE void idRotation::SetVec( const idVec3 &rotationVec ) {
	vec = rotationVec;
	axisValid = false;
}

ID_FORCE_INLINE void idRotation::SetVec( float x, float y, float z ) {
	vec[0] = x;
	vec[1] = y;
	vec[2] = z;
	axisValid = false;
}

ID_FORCE_INLINE void idRotation::SetAngle( const float rotationAngle ) {
	angle = rotationAngle;
	axisValid = false;
}

ID_FORCE_INLINE void idRotation::Scale( const float s ) {
	angle *= s;
	axisValid = false;
}

ID_FORCE_INLINE void idRotation::ReCalculateMatrix( void ) {
	axisValid = false;
	ToMat3();
}

ID_FORCE_INLINE const idVec3 &idRotation::GetOrigin( void ) const {
	return origin;
}

ID_FORCE_INLINE const idVec3 &idRotation::GetVec( void ) const  {
	return vec;
}

ID_FORCE_INLINE float idRotation::GetAngle( void ) const  {
	return angle;
}

ID_FORCE_INLINE idRotation idRotation::operator-() const {
	return idRotation( origin, vec, -angle );
}

ID_FORCE_INLINE idRotation idRotation::operator*( const float s ) const {
	return idRotation( origin, vec, angle * s );
}

ID_FORCE_INLINE idRotation idRotation::operator/( const float s ) const {
	assert( s != 0.0f );
	return idRotation( origin, vec, angle / s );
}

ID_FORCE_INLINE idRotation &idRotation::operator*=( const float s ) {
	angle *= s;
	axisValid = false;
	return *this;
}

ID_FORCE_INLINE idRotation &idRotation::operator/=( const float s ) {
	assert( s != 0.0f );
	angle /= s;
	axisValid = false;
	return *this;
}

ID_FORCE_INLINE idVec3 idRotation::operator*( const idVec3 &v ) const {
	if ( !axisValid ) {
		ToMat3();
	}
	return ((v - origin) * axis + origin);
}

ID_FORCE_INLINE idRotation operator*( const float s, const idRotation &r ) {
	return r * s;
}

ID_FORCE_INLINE idVec3 operator*( const idVec3 &v, const idRotation &r ) {
	return r * v;
}

ID_FORCE_INLINE idVec3 &operator*=( idVec3 &v, const idRotation &r ) {
	v = r * v;
	return v;
}

ID_FORCE_INLINE void idRotation::RotatePoint( idVec3 &point ) const {
	if ( !axisValid ) {
		ToMat3();
	}
	point = ((point - origin) * axis + origin);
}

#endif /* !__MATH_ROTATION_H__ */