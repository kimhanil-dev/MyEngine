#pragma once

#include <math.h>

class FVector
{
public:
	union
	{
		float v[3]{};
		struct
		{
			float X, Y, Z;
		};
	};

	FVector() :v{0.0f,0.0f,0.0f} {}
	FVector(float x, float y, float z) :v{x,y,z} {}
	FVector(float num) :v{ num ,num, num } {}

	FVector operator + (const FVector& v) const { return { X + v.X, Y + v.Y, Z + v.Z }; }
	FVector operator - (const FVector& v) const { return { X - v.X, Y - v.Y, Z - v.Z }; }
	FVector operator * (const FVector& v) const { return { X * v.X, Y * v.Y, Z * v.Z }; }
	FVector operator * (const float f) const { return { X * f, Y * f, Z * f }; }
	FVector operator / (const float f) const { return { X / f, Y / f, Z / f }; }
	FVector& operator += (const FVector& v) { X += v.X, Y += v.Y, Z += v.Z; return *this; }
	FVector& operator -= (const FVector& v) { X -= v.X, Y -= v.Y, Z -= v.Z; return *this; }
	FVector& operator *= (const FVector& v) { X *= v.X, Y *= v.Y, Z *= v.Z; return *this; }
	FVector& operator *= (const float& f) { X *= f, Y *= f, Z *= f; return *this; }
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

