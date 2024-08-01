#pragma once
#include "Core/Math/Vector.h"

struct FMatrix4x4
{
	union
	{
		float m[16];
		struct
		{
			float m11, m12, m13, m14,
				m21, m22, m23, m24,
				m31, m32, m33, m34,
				m41, m42, m43, m44;
		};
	};

	
	void Identity();
	FMatrix4x4 operator*(const FMatrix4x4& m2);
	FVector operator*(const FVector& v);
};

namespace Matrix
{
	FMatrix4x4 MakeTranslationMatrix(const FVector& v);
	FMatrix4x4 MakeRotationMatrix(const FVector& r);
}