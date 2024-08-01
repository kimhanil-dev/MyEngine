#include "pch.h"
#include "Matrix.h"

#include <math.h>

void FMatrix4x4::Identity()
{
	ZeroMemory(this, sizeof(FMatrix4x4));

	this->m11 = 1.0f;
	this->m22 = 1.0f;
	this->m33 = 1.0f;
	this->m44 = 1.0f;
}

#define Matrix(i, j, m1, m2)  m1.m##i##1 * m2.m##1##j + m1.m##i##2 * m2.m##2##j + m1.m##i##3 * m2.m##3##j + m1.m##i##4 * m2.m##4##j
FMatrix4x4 FMatrix4x4::operator*(const FMatrix4x4& m2)
{
	FMatrix4x4 result;

	result.m11 = Matrix(1, 1, (*this), m2);
	result.m12 = Matrix(1, 2, (*this), m2);
	result.m13 = Matrix(1, 3, (*this), m2);
	result.m14 = Matrix(1, 4, (*this), m2);
	result.m21 = Matrix(2, 1, (*this), m2);
	result.m22 = Matrix(2, 2, (*this), m2);
	result.m23 = Matrix(2, 3, (*this), m2);
	result.m24 = Matrix(2, 4, (*this), m2);
	result.m31 = Matrix(3, 1, (*this), m2);
	result.m32 = Matrix(3, 2, (*this), m2);
	result.m33 = Matrix(3, 3, (*this), m2);
	result.m34 = Matrix(3, 4, (*this), m2);
	result.m41 = Matrix(4, 1, (*this), m2);
	result.m42 = Matrix(4, 2, (*this), m2);
	result.m43 = Matrix(4, 3, (*this), m2);
	result.m44 = Matrix(4, 4, (*this), m2);

	return result;
}

FVector FMatrix4x4::operator*(const FVector& v)
{
	FVector result;

	result.X = v.X * m11 + v.Y * m21 + v.Z * m31 + m41;
	result.Y = v.X * m12 + v.Y * m22 + v.Z * m32 + m42;
	result.Z = v.X * m13 + v.Y * m23 + v.Z * m33 + m43;

	return result;
}

FMatrix4x4 Matrix::MakeTranslationMatrix(const FVector& v)
{
	FMatrix4x4 result;
	result.Identity();
	result.m41 = v.X;
	result.m42 = v.Y;
	result.m43 = v.Z;

	return result;
}

#define Radian(degree) degree * 3.1415926258 / 180
FMatrix4x4 Matrix::MakeRotationMatrix(const FVector& r)
{
	FMatrix4x4 result;
	result.Identity();

	// rot z
	result.m11 = cosf(Radian(r.Z));
	result.m12 = -sinf(Radian(r.Z));
	result.m22 = cos(Radian(r.Z));
	result.m21 = sinf(Radian(r.Z));

	// rot x
	result.m33 = cosf(Radian(-r.X));
	result.m32 = -sinf(Radian(-r.X));
	result.m22 = cos(Radian(r.X));
	result.m23 = sinf(Radian(r.X));


	return result;
}
