#include "GraphicsSW.h"

#include <cassert>

#include "Core/Object/Object.h"
#include "Utill/frame.h"

static constexpr float PI = 3.14159265359f;
#define Radian(Degree) Degree * (PI / 180.0f)


#pragma region Forward Declaration

// line drawings
	// using GDI
void DrawLines(HDC, std::vector<FVector>);
void DrawLine(HDC, int, int, int, int);

// using DDA
void DrawLines(uint* pixelBuffer, uint width, uint height, const std::vector<Vertex>& vertices, const FVector& rotate, const FVector& transform);

// draw vertex none transformation
void DrawLines(uint* pixelBuffer, uint width, uint height, const std::vector<FVector>& vectors);
void DrawLine(uint* pixelBuffer, uint width, uint height, const FVector& start, const FVector& end);

// fill triangles
	// using Slope
void FillTriangle(uint* pixelBuffer, const uint width, const uint height, const std::vector<Vertex>& triangle);
// using scan line, interpolation
void FillTriangleScanLine(uint* pixelBuffer, const uint width, const uint height, std::vector<Vertex> vertices, const FVector& location, const FVector& rotation);

// transformation
void TransformMesh(std::vector<FVector>& v, const FVector& rotate, const FVector& transform);
void Rotate(FVector& v, const FVector& rotate);
void Translate(FVector& v, const FVector& deltaLocation);


//// vertex base
void TransformMesh(std::vector<Vertex>& v, const FVector& rotate, const FVector& transform);
void FillTriangleScanLine(uint* pixelBuffer, const uint width, const uint height, std::vector<Vertex> vertices);
void FillTriangles(uint* pixelBuffer, uint width, uint height, const std::vector<Vertex>& vertices, const int* indices, const uint indexCount);
void DrawLines(uint* pixelBuffer, uint width, uint height, const std::vector<Vertex>& vertices, const int* indices, const uint indexCount);
////

// �ӽ�
void Draw_Indexed(uint* pixelBuffer, uint width, uint height, const Object* mCamera, const Object* object);
bool IsBackfacePolygon(std::vector<FVector> polygon)
{
	FVector mCameraForward = FVector(0.0f, 0.0f, 1.0f);
	FVector normal = ((polygon[1] - polygon[0]).Cross(polygon[2] - polygon[1])).Normalize();

	return (mCameraForward.Dot(normal) > 0);
}

#pragma endregion Forward Definition


#pragma region Rendring Algorithms

float CalculateSlope(const FVector& startV, const FVector& endV)
{
	if (startV.Y == endV.Y)
		return 0.0f;

	float x = (endV.X - startV.X);
	float y = (endV.Y - startV.Y);

	// y���� ���� x���� ���� �ϱ� ������, y / x�� ���� x / y�� �����մϴ�.
	return x / y;
}

void FillTriangle(uint* pixelBuffer, const uint width, const uint height, const std::vector<FVector>& triangle)
{
	FVector top, middle, bottom, startPos;
	int t = 0, m = 1, b = 2, temp = 0;
	if (triangle[t].Y > triangle[m].Y)
	{
		temp = t;
		t = m;
		m = temp;
	}

	if (triangle[t].Y > triangle[b].Y)
	{
		temp = t;
		t = b;
		b = temp;
	}

	if (triangle[m].Y > triangle[b].Y)
	{
		temp = m;
		m = b;
		b = temp;
	}

	top = triangle[t];
	middle = triangle[m];
	bottom = triangle[b];

	float lSlope = 0.0f, rSlope = 0.0f;
	lSlope = CalculateSlope(top, middle);
	rSlope = CalculateSlope(top, bottom);

	float startX = 0.0f, endX = 0.0f;
	int h = static_cast<int>(roundf(middle.Y - top.Y));
	for (int y = 0; y <= h; ++y)
	{
		startX = lSlope * y;
		endX = rSlope * y;

		if (startX > endX)
		{
			float temp = startX;
			startX = endX;
			endX = temp;
		}

		for (int x = (int)startX; x <= (int)endX; ++x)
		{
			pixelBuffer[(int)(y + top.Y) * width + x + (int)top.X] = 0xff0000;
		}
	}

	lSlope = -CalculateSlope(middle, bottom);
	rSlope = -rSlope;

	h = static_cast<int>(roundf(bottom.Y - middle.Y));
	for (int y = 0; y <= h; ++y)
	{
		startX = lSlope * y;
		endX = rSlope * y;

		if (startX > endX)
		{
			float temp = startX;
			startX = endX;
			endX = temp;
		}

		for (int x = (int)startX; x <= (int)endX; ++x)
		{
			pixelBuffer[(int)(bottom.Y - y) * width + x + (int)bottom.X] = 0xff0000;
		}
	}
}

// y���� from, to�� �̷���� ���������� ��ġ �� (0 ~ 1)
float EdgeLinearRatio(const FVector& from, const FVector& to, const float y)
{
	return (y - from.Y) / (to.Y - from.Y);
}

FVector ColorInterpolation(const FVector& fromColor, const FVector& toColor, const float ratio)
{
	return fromColor + (toColor - fromColor) * ratio;
}

uint ColorToUINT(const FVector& color)
{
	return (int)(color.X * 255) << 16 | (int)(color.Y * 255) << 8 | (int)(color.Z * 255);
}

float getScanlinePosition(const FVector& from, const FVector& to, const float y)
{
	if ((int)from.Y == (int)to.Y)
		return from.X;

	return from.X + (to.X - from.X) * (y - from.Y) / (to.Y - from.Y);
}

void FillTriangleScanLine(uint* pixelBuffer, const uint width, const uint height, std::vector<Vertex> vertices, const FVector& location, const FVector& rotation)
{
	for (int i = 0; i < 3; ++i)
	{
		Rotate(vertices[i].Position, rotation);
		vertices[i].Position += location;
	}

	Vertex top, middle, bottom, startPos;
	int t = 0, m = 1, b = 2, temp = 0;
	if (vertices[t].Position.Y > vertices[m].Position.Y)
	{
		temp = t;
		t = m;
		m = temp;
	}

	if (vertices[t].Position.Y > vertices[b].Position.Y)
	{
		temp = t;
		t = b;
		b = temp;
	}

	if (vertices[m].Position.Y > vertices[b].Position.Y)
	{
		temp = m;
		m = b;
		b = temp;
	}

	top = vertices[t];
	middle = vertices[m];
	bottom = vertices[b];

	FVector startColor, endColor;
	int pixelPos = 0;
	float startX = 0.0f, endX = 0.0f;
	for (int y = static_cast<int>(top.Position.Y); y < static_cast<int>(bottom.Position.Y); ++y)
	{
		if (y < middle.Position.Y)
		{
			startX = getScanlinePosition(top.Position, middle.Position, static_cast<float>(y));
			endX = getScanlinePosition(top.Position, bottom.Position, static_cast<float>(y));

			startColor = ColorInterpolation(top.Color, middle.Color, EdgeLinearRatio(top.Position, middle.Position, static_cast<float>(y)));
			endColor = ColorInterpolation(top.Color, bottom.Color, EdgeLinearRatio(top.Position, bottom.Position, static_cast<float>(y)));
		}
		else
		{
			startX = getScanlinePosition(middle.Position, bottom.Position, static_cast<float>(y));
			endX = getScanlinePosition(top.Position, bottom.Position, static_cast<float>(y));

			startColor = ColorInterpolation(middle.Color, bottom.Color, EdgeLinearRatio(middle.Position, bottom.Position, static_cast<float>(y)));
			endColor = ColorInterpolation(top.Color, bottom.Color, EdgeLinearRatio(top.Position, bottom.Position, static_cast<float>(y)));
		}

		if (startX > endX)
		{
			float temp = startX;
			startX = endX;
			endX = temp;

			FVector colorTemp = startColor;
			startColor = endColor;
			endColor = colorTemp;
		}

		pixelPos = y * width;
		for (int x = static_cast<int>(startX); x < static_cast<int>(endX); ++x)
		{
			FVector color = ColorInterpolation(startColor, endColor, (x - startX) / (endX - startX));
			pixelBuffer[pixelPos + x] = ColorToUINT(color);
		}
	}
}

void FillTriangleScanLine(uint* pixelBuffer, const uint width, const uint height, std::vector<Vertex> vertices)
{
	Vertex top, middle, bottom, startPos;
	int t = 0, m = 1, b = 2, temp = 0;
	if (vertices[t].Position.Y > vertices[m].Position.Y)
	{
		temp = t;
		t = m;
		m = temp;
	}

	if (vertices[t].Position.Y > vertices[b].Position.Y)
	{
		temp = t;
		t = b;
		b = temp;
	}

	if (vertices[m].Position.Y > vertices[b].Position.Y)
	{
		temp = m;
		m = b;
		b = temp;
	}

	top = vertices[t];
	middle = vertices[m];
	bottom = vertices[b];

	FVector startColor, endColor;
	int pixelPos = 0;
	float startX = 0.0f, endX = 0.0f;
	for (float y = top.Position.Y; y < bottom.Position.Y; ++y)
	{
		if (y < middle.Position.Y)
		{
			startX = getScanlinePosition(top.Position, middle.Position, y);
			endX = getScanlinePosition(top.Position, bottom.Position, y);

			startColor = ColorInterpolation(top.Color, middle.Color, EdgeLinearRatio(top.Position, middle.Position, y));
			endColor = ColorInterpolation(top.Color, bottom.Color, EdgeLinearRatio(top.Position, bottom.Position, y));
		}
		else
		{
			startX = getScanlinePosition(middle.Position, bottom.Position, y);
			endX = getScanlinePosition(top.Position, bottom.Position, y);

			startColor = ColorInterpolation(middle.Color, bottom.Color, EdgeLinearRatio(middle.Position, bottom.Position, y));
			endColor = ColorInterpolation(top.Color, bottom.Color, EdgeLinearRatio(top.Position, bottom.Position, y));
		}

		if (startX > endX)
		{
			float temp = startX;
			startX = endX;
			endX = temp;

			FVector colorTemp = startColor;
			startColor = endColor;
			endColor = colorTemp;
		}

		if (startX < 0)
		{
			int t = 0;
		}

		pixelPos = (int)y * width;
		for (int x = static_cast<int>(startX); x < static_cast<int>(endX); ++x)
		{
			FVector color = ColorInterpolation(startColor, endColor, (x - startX) / (endX - startX));
			pixelBuffer[pixelPos + x] = ColorToUINT(color);
		}
	}
}


void DrawLines(HDC hdc, std::vector<FVector> lines)
{
	int count = (int)(lines.size() - 1);
	for (int i = 0; i < count; ++i)
	{
		DrawLine(hdc, (int)lines[i].X, (int)lines[i].Y, (int)lines[i + 1].X, (int)lines[i + 1].Y);
	}
}

void DrawLines(uint* pixelBuffer, uint width, uint height, const std::vector<Vertex>& vertices, const FVector& rotate, const FVector& transform)
{
	std::vector<FVector> v = std::vector<FVector>(vertices.size());
	for (int i = 0; i < vertices.size(); ++i)
	{
		v[i] = vertices[i].Position;
	}

	TransformMesh(v, rotate, transform);

	int count = (int)(vertices.size() - 1);
	for (int i = 0; i < count; ++i)
	{
		DrawLine(pixelBuffer, width, height, v[i], v[i + 1]);
	}
}

void DrawLines(uint* pixelBuffer, uint width, uint height, const std::vector<FVector>& vectors)
{
	int count = (int)(vectors.size() - 1);
	for (int i = 0; i < count; ++i)
	{
		DrawLine(pixelBuffer, width, height, vectors[i], vectors[i + 1]);
	}
}

// using GDI
void DrawLine(HDC hdc, int x1, int y1, int x2, int y2)
{
	MoveToEx(hdc, x1, y1, nullptr);
	LineTo(hdc, x2, y2);
}

// using DDA
void DrawLine(uint* pixelBuffer, uint width, uint height, const FVector& start, const FVector& end)
{
	float dx = end.X - start.X;
	float dy = end.Y - start.Y;
	float step = 0;

	if (abs(dx) > abs(dy))
		step = abs(dx);
	else
		step = abs(dy);

	if (dx != 0)
		dx /= step;
	else
		dx = 0.0f;

	if (dy != 0)
		dy /= step;
	else
		dy = 0.0f;

	float x = start.X;
	float y = start.Y;
	for (int i = 0; i < step; ++i)
	{
		*(pixelBuffer + (int)x + (int)y * width) = 0x0000ff;
		x += dx;
		y += dy;

		if (x < 0 || x >= width)
			break;
		if (y < 0 || y >= height)
			break;
	}
}
#pragma endregion Rendring Algorithms


#pragma region Transformation
void TransformMesh(std::vector<FVector>& v, const FVector& rotate, const FVector& transform)
{
	for (int i = 0; i < v.size(); ++i)
	{
		Rotate(v[i], rotate);
		Translate(v[i], transform);
	}
}

void TransformMesh(std::vector<Vertex>& v, const FVector& rotate, const FVector& transform)
{
	for (int i = 0; i < v.size(); ++i)
	{
		Rotate(v[i].Position, rotate);
		Translate(v[i].Position, transform);
	}
}

// vector transform
void Rotate(FVector& v, const FVector& rotate)
{

	// ȸ�� ���� ����
	// x2 = cos(a + b), y2 = sin(a + b)
	// cos(a + b) = cosa * cosb - sina * sinb = x1 * cosb - y1 * sinb
	// sin(a + b) = sina * cosb + cosa * sinb = y1 * cosb + x1 * sinb 

	// X�� ȸ��
	FVector xRotVec = v;
	float rX = Radian(rotate.X);
	xRotVec.Y = v.Y * cos(rX) - v.Z * sin(rX);
	xRotVec.Z = v.Z * cos(rX) + v.Y * sin(rX);

	// Y�� ȸ��
	FVector xyRotVec = xRotVec;
	float rY = Radian(rotate.Y);
	xyRotVec.X = xRotVec.X * cos(rY) + xRotVec.Z * sin(rY);
	xyRotVec.Z = xRotVec.Z * cos(rY) - xRotVec.X * sin(rY);

	// Z�� ȸ��
	FVector xyzRotVec = xyRotVec;
	float rZ = Radian(rotate.Z);
	xyzRotVec.X = xyRotVec.X * cos(rZ) - xyRotVec.Y * sin(rZ);
	xyzRotVec.Y = xyRotVec.Y * cos(rZ) + xyRotVec.X * sin(rZ);

	v = xyzRotVec;
}

void Translate(FVector& v, const FVector& deltaLocaiton)
{
	v += deltaLocaiton;
}


void FillTriangles(uint* pixelBuffer, uint width, uint height, const std::vector<Vertex>& vertices, const int* indices, const uint indexCount)
{
	for (UINT i = 0; i < indexCount; i += 3)
	{
		if (IsBackfacePolygon({ vertices[indices[i]].Position, vertices[indices[i + 1]].Position,vertices[indices[i + 2]].Position }))
			continue;

		FillTriangleScanLine(pixelBuffer, width, height, { vertices[indices[i]],vertices[indices[i + 1]],vertices[indices[i + 2]] });
	}
}

void DrawLines(uint* pixelBuffer, uint width, uint height, const std::vector<FVector>& vertices, const int* indices, const uint indexCount)
{
	for (UINT i = 0; i < indexCount; i += 3)
	{
		DrawLine(pixelBuffer, width, height, vertices[indices[i]], vertices[indices[i + 1]]);
		DrawLine(pixelBuffer, width, height, vertices[indices[i + 1]], vertices[indices[i + 2]]);
		DrawLine(pixelBuffer, width, height, vertices[indices[i + 2]], vertices[indices[i]]);
	}
}

void DrawLines(uint* pixelBuffer, uint width, uint height, const std::vector<Vertex>& vertices, const int* indices, const uint indexCount)
{
	for (UINT i = 0; i < indexCount; i += 3)
	{
		if (IsBackfacePolygon({ vertices[indices[i]].Position, vertices[indices[i + 1]].Position,vertices[indices[i + 2]].Position }))
			continue;

		DrawLine(pixelBuffer, width, height, vertices[indices[i]].Position, vertices[indices[i + 1]].Position);
		DrawLine(pixelBuffer, width, height, vertices[indices[i + 1]].Position, vertices[indices[i + 2]].Position);
		DrawLine(pixelBuffer, width, height, vertices[indices[i + 2]].Position, vertices[indices[i]].Position);
	}
}

void Draw_Indexed(uint* pixelBuffer, uint width, uint height, const Object* mCamera, const Object* object)
{
	float x = 0.0f, y = 0.0f;
	float z = 0.0f, nearZ = 1.0f;
	float planeX = 0.0f, planeY = 0.0f;
	float screenX = 0.0f, screenY = 0.0f;
	planeX = z / x * nearZ;
	planeY = z / y * nearZ;

	// rotate local

	// transform cube to mCamera local coord
	FVector rotate = object->mRotate - mCamera->mRotate;
	FVector origin = object->mOrigin - mCamera->mOrigin;
	std::vector<FVector> v = std::vector<FVector>(object->mMesh->VertexCount);

	for (int j = 0; j < v.size(); ++j)
	{
		v[j] = object->mMesh->Vertices[j].Position;
	}

	// local transform
	TransformMesh(v, object->mRotate, 0.0f);

	// world transform
	TransformMesh(v, 0.0f, origin);

	// ��Ⱦ��
	float screenRatio = (float)width / height;

	static float maxX = 0.0f, maxY = 0.0f;
	FVector planePos;
	for (auto& vertex : v)
	{
		x = vertex.X;
		y = vertex.Y;
		z = vertex.Z;

		// screen space
		x = x / z * screenRatio;
		y = y / z;

		// NDC (normalized device coordinates)
		x = (1 + x) * 0.5f;
		y = (1 + y) * 0.5f;

		// raster space
		x *= width;
		y *= height;

		vertex = FVector(x, y, 0.0f);
	}

	DrawLines(pixelBuffer, width, height, v, object->mMesh->Indices, object->mMesh->IndexCount);
}


#pragma endregion Transformation


HRESULT GraphicsSW::Init(const HWND& hWnd)
{
	assert(hWnd != NULL);

	mWnd = hWnd;

	RECT rc;
	GetWindowRect(mWnd, &rc);
	mWindowWidth = rc.right - rc.left;
	mWindowHeight = rc.bottom - rc.top;

	// window's draw tools
	mDC = GetDC(mWnd);
	mMemDC = CreateCompatibleDC(mDC);
	mRenderTarget = CreateCompatibleBitmap(mDC, mWindowWidth, mWindowHeight);

	// render buffer
	mPixelBuffer = new uint[mWindowWidth * mWindowHeight];
	ZeroMemory(mPixelBuffer, sizeof(uint) * mWindowWidth * mWindowHeight);


	FVector rightVector = { 1.0f,0.0f,0.0f };
	FVector upVector = { 0.0f,1.0f,0.0f };

	//init frustom planes
	mFrustomPlanes[0] = { FVector(0.0f,sin(Radian(45.0f)),cos(Radian(45.0f))).Cross(rightVector)}; // top
	mFrustomPlanes[1] = { rightVector.Cross({0.0f,sin(Radian(-45.0f)),cos(Radian(-45.0f))}) }; // bottom
	mFrustomPlanes[2] = { upVector.Cross(FVector(sin(Radian(45.0f)),0.0f,cos(Radian(45.0f))))}; // right
	mFrustomPlanes[3] = { FVector(sin(Radian(-45.0f)),0.0f,cos(Radian(-45.0f))).Cross(upVector) }; // left

	return S_OK;
}

void GraphicsSW::Render()
{
	// clear pixel buffer
	ZeroMemory(mPixelBuffer, sizeof(uint) * mWindowWidth * mWindowHeight);

	HBITMAP oldBitmap = (HBITMAP)SelectObject(mMemDC, mRenderTarget);
	{
		for (int i = 0; i < mObjects.size(); ++i)
		{
			if (mObjects[i]->mID == 1)
			{
				FillTriangleScanLine(mPixelBuffer, mWindowWidth, mWindowHeight, mObjects[i]->mVertices, mObjects[i]->mOrigin, mObjects[i]->mRotate);
				DrawLines(mPixelBuffer, mWindowWidth, mWindowHeight, mObjects[i]->mVertices, mObjects[i]->mRotate, mObjects[i]->mOrigin);
			}
			else if (mObjects[i]->mID == 3)
			{
				Draw_Indexed(mPixelBuffer, mWindowWidth, mWindowHeight, mCamera, mObjects[i]);
			}
			else
			{
				// frustom test
				{
					float x = 0.0f, y = 0.0f;
					float z = 0.0f, nearZ = 1.0f;
					float planeX = 0.0f, planeY = 0.0f;
					float screenX = 0.0f, screenY = 0.0f;
					planeX = z / x * nearZ;
					planeY = z / y * nearZ;

					// rotate local

					// transform cube to mCamera local coord
					FVector rotate = mObjects[i]->mRotate - mCamera->mRotate;
					FVector origin = mObjects[i]->mOrigin - mCamera->mOrigin;
					std::vector<Vertex> v = std::vector<Vertex>(mObjects[i]->mMesh->VertexCount);
					for (UINT j = 0; j < mObjects[i]->mMesh->VertexCount; ++j)
					{
						v[j] = mObjects[i]->mMesh->Vertices[j];
					}

					// local transform
					TransformMesh(v, mObjects[i]->mRotate, 0.0f);

					// world transform
					TransformMesh(v, 0.0f, origin);

					// check frustom
					bool bIsOutFrustom = false;


					/*for (const auto& _v : v)
					{
						for (UINT j = 0; j < 4; ++j)
						{
							if (CalculatePointPlaneRelation(mFrustomPlanes[i], _v.Position) != PlaneRelation::PP_Back)
							{
								bIsOutFrustom = true;
								break;
							}
						}

						if(bIsOutFrustom)
							break;
					}
					if(bIsOutFrustom)
						continue;*/

					// ��Ⱦ��
					float screenRatio = (float)mWindowHeight / mWindowWidth;

					static float maxX = 0.0f, maxY = 0.0f;
					FVector planePos;
					for (auto& vertex : v)
					{
						x = vertex.Position.X;
						y = vertex.Position.Y;
						z = vertex.Position.Z;

						// screen space
						x = x / z * screenRatio;
						y = y / z;

						// NDC (normalized device coordinates)
						x = (1 + x) * 0.5f;
						y = (1 + y) * 0.5f;

						// 
						x = x < 0 ? 0 : x > 1 ? 1 : x;
						y = y < 0 ? 0 : y > 1 ? 1 : y;


						// raster space
						x *= mWindowWidth;
						y *= mWindowHeight;

						vertex.Position = FVector(x, y, 0.0f);
					}

					if (bIsOutFrustom)
						continue;


					FillTriangles(mPixelBuffer, mWindowWidth, mWindowHeight, v, mObjects[i]->mMesh->Indices, mObjects[i]->mMesh->IndexCount);
					DrawLines(mPixelBuffer, mWindowWidth, mWindowHeight, v, mObjects[i]->mMesh->Indices, mObjects[i]->mMesh->IndexCount);
				}
				// frustom test end
			}
		}

		SetBitmapBits(mRenderTarget, mWindowWidth * mWindowHeight * 4, mPixelBuffer);

		RECT rt;
		rt.left = 10;
		rt.top = 10;
		rt.right = 500;
		rt.bottom = 500;

		wchar_t deltaTimeStr[50];
		swprintf_s(deltaTimeStr, 50, L"fps : %d", GetFPS());
		DrawText(mMemDC, deltaTimeStr, -1, &rt, DT_LEFT);

		BitBlt(mDC, 0, 0, mWindowWidth, mWindowHeight, mMemDC, 0, 0, SRCCOPY);
	}
	SelectObject(mMemDC, oldBitmap);

}

void GraphicsSW::Release()
{
	delete[] mPixelBuffer;
	mPixelBuffer = nullptr;

	DeleteObject(mRenderTarget);
	mRenderTarget = NULL;

	ReleaseDC(mWnd, mMemDC);
	mMemDC = NULL;
}

void GraphicsSW::AddObject(Object* object)
{
	mObjects.push_back(object);
}

void GraphicsSW::SetCamera(Object* object)
{
	mCamera = object;
}
