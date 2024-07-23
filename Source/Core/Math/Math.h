#pragma once

#include "Matrix.h"
#include "Vector.h"

// mul
FVector Mul(const FVector& v, const Matrix4x4& m);
Matrix4x4 Mul(const Matrix4x4& m1, const Matrix4x4& m2);