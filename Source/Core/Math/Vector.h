#pragma once

#include <cmath>

template <typename T>
class Vector
{
public:
	float X = 0.0f, Y = 0.0f, Z = 0.0f;

	Vector(){}
	Vector(float x, float y, float z) :X(x), Y(y), Z(z) {}
	Vector(float num) :X(num), Y(num), Z(num) {}

	Vector<T> operator + (const Vector<T>& v) const { return { X + v.X, Y + v.Y, Z + v.Z }; }
	Vector<T> operator - (const Vector<T>& v) const { return { X - v.X, Y - v.Y, Z - v.Z }; }
	Vector<T> operator * (const Vector<T>& v) const { return { X * v.X, Y * v.Y, Z * v.Z }; }
	Vector<T> operator * (const float f) const { return { X * f, Y * f, Z * f }; }
	Vector<T> operator / (const float f) const { return { X / f, Y / f, Z / f }; }
	Vector<T>& operator += (const Vector<T>& v) { X += v.X, Y += v.Y, Z += v.Z; return *this; }
	Vector<T>& operator -= (const Vector<T>& v) { X -= v.X, Y -= v.Y, Z -= v.Z; return *this; }
	Vector<T>& operator *= (const Vector<T>& v) { X *= v.X, Y *= v.Y, Z *= v.Z; return *this; }
	Vector<T> operator  - () const { return { -X,-Y,-Z }; }

	Vector<T> Cross(const Vector<T>& v) const { return { Y * v.Z - Z * v.Y, Z * v.X - X * v.Z, X * v.Y - Y * v.X }; }
	float Dot(const Vector<T>& v) const { return { X * v.X + Y * v.Y + Z * v.Z }; }
	float Length2() const { return X * X + Y * Y; }
	float Length() const { return sqrt(X * X + Y * Y); }
	Vector<T>& Normalize()
	{
		float invNor = 1 / Length();

		X *= invNor;
		Y *= invNor;
		Z *= invNor;

		return *this;
	}

};

typedef Vector<float> FVector;
