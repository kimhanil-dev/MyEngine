#include "Math.h"

FVector Mul(const FVector& v, const Matrix4x4& m)
{
	FVector result;

	result.X = v.X * m.m11 + v.Y * m.m21 + v.Z * m.m31 + m.m41;
	result.Y = v.X * m.m12 + v.Y * m.m22 + v.Z * m.m32 + m.m42;
	result.Z = v.X * m.m13 + v.Y * m.m23 + v.Z * m.m33 + m.m43;

	return result;
}

#define Matrix(i, j, m1, m2)  m1.m##i##1 * m2.m##1##j + m1.m##i##2 * m2.m##2##j + m1.m##i##3 * m2.m##3##j + m1.m##i##4 * m2.m##4##j
Matrix4x4 Mul(const Matrix4x4& m1, const Matrix4x4& m2)
{
	Matrix4x4 result;

	result.m11 = Matrix(1, 1, m1, m2);
	result.m12 = Matrix(1, 2, m1, m2);
	result.m13 = Matrix(1, 3, m1, m2);
	result.m14 = Matrix(1, 4, m1, m2);
	result.m21 = Matrix(2, 1, m1, m2);
	result.m22 = Matrix(2, 2, m1, m2);
	result.m23 = Matrix(2, 3, m1, m2);
	result.m24 = Matrix(2, 4, m1, m2);
	result.m31 = Matrix(3, 1, m1, m2);
	result.m32 = Matrix(3, 2, m1, m2);
	result.m33 = Matrix(3, 3, m1, m2);
	result.m34 = Matrix(3, 4, m1, m2);
	result.m41 = Matrix(4, 1, m1, m2);
	result.m42 = Matrix(4, 2, m1, m2);
	result.m43 = Matrix(4, 3, m1, m2);
	result.m44 = Matrix(4, 4, m1, m2);

	return result;
}

