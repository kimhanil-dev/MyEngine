#pragma once

#include <math.h>

class FVector
{
public:
	float X = 0.0f, Y = 0.0f, Z = 0.0f;

	FVector(){}
	FVector(float x, float y, float z) :X(x), Y(y), Z(z) {}
	FVector(float num) :X(num), Y(num), Z(num) {}

	FVector operator + (const FVector& v) const { return { X + v.X, Y + v.Y, Z + v.Z }; }
	FVector operator - (const FVector& v) const { return { X - v.X, Y - v.Y, Z - v.Z }; }
	FVector operator * (const FVector& v) const { return { X * v.X, Y * v.Y, Z * v.Z }; }
	FVector operator * (const float f) const { return { X * f, Y * f, Z * f }; }
	FVector operator / (const float f) const { return { X / f, Y / f, Z / f }; }
	FVector& operator += (const FVector& v) { X += v.X, Y += v.Y, Z += v.Z; return *this; }
	FVector& operator -= (const FVector& v) { X -= v.X, Y -= v.Y, Z -= v.Z; return *this; }
	FVector& operator *= (const FVector& v) { X *= v.X, Y *= v.Y, Z *= v.Z; return *this; }
	FVector operator  - () const { return { -X,-Y,-Z }; }

	FVector Cross(const FVector& v) const { return { Y * v.Z - Z * v.Y, Z * v.X - X * v.Z, X * v.Y - Y * v.X }; }
	float Dot(const FVector& v) const { return { X * v.X + Y * v.Y + Z * v.Z }; }
	float Length2() const { return X * X + Y * Y + Z * Z; }
	float Length() const { return sqrtf(X * X + Y * Y + Z * Z); }
	FVector& Normalize()
	{
		float invNor = 1 / Length();

		X *= invNor;
		Y *= invNor;
		Z *= invNor;

		return *this;
	}

};

