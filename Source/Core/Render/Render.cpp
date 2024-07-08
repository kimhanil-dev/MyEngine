#include "Render.h"

#include <cassert>

#include "Core/Object/Object.h"

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

// 임시
void Draw_Indexed(uint* pixelBuffer, uint width, uint height, const Object* camera, const Object* object);

#pragma endregion Forward Definition

#pragma region Interface
static bool gIsInited = false;

// windows
static HWND		ghWnd			= NULL;
static HDC		ghdc			= NULL;
static HDC		ghMemDC			= NULL;
static HBITMAP	ghRenderTarget	= NULL;

// render states
static uint gWidth = 0;
static uint gHeight = 0;
static uint *gPixelBuffer = nullptr;

void Render::Init(HWND hWnd, const uint width, const uint height)
{
	if (!gIsInited)
	{
		if (hWnd == NULL)
		{
			assert(false);
		}

		hWnd = hWnd;
	}
	else
	{
		Release();
	}

	gWidth = width;
	gHeight = height;

	// window's draw tools
	ghdc = GetDC(hWnd);
	ghMemDC = CreateCompatibleDC(ghdc);
	ghRenderTarget = CreateCompatibleBitmap(ghdc, gWidth, gHeight);

	// render buffer
	gPixelBuffer = new uint[width * height];
	ZeroMemory(gPixelBuffer, sizeof(uint) * width * height);

	gIsInited = true;
}

void Render::Render(const Object* camera, const std::vector<Object*> objects)
{
	if (!gIsInited)
	{
		assert(false);
	}

	// clear pixel buffer
	ZeroMemory(gPixelBuffer,sizeof(uint) * gWidth * gHeight);

	HBITMAP oldBitmap = (HBITMAP)SelectObject(ghMemDC, ghRenderTarget);
	{
		for (int i = 0; i < objects.size(); ++i)
		{
			if (objects[i]->mID == 1)
			{
				FillTriangleScanLine(gPixelBuffer, gWidth, gHeight, objects[i]->mVertices, objects[i]->mOrigin, objects[i]->mRotate);
				DrawLines(gPixelBuffer, gWidth, gHeight, objects[i]->mVertices, objects[i]->mRotate, objects[i]->mOrigin);
			}
			else if (objects[i]->mID == 3)
			{
				Draw_Indexed(gPixelBuffer,gWidth,gHeight,camera,objects[i]);
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

					// transform cube to camera local coord
					FVector rotate = objects[i]->mRotate - camera->mRotate;
					FVector origin = objects[i]->mOrigin - camera->mOrigin;
					std::vector<Vertex> v = std::vector<Vertex>(objects[i]->mMesh->VertexCount);
					for (int j = 0; j < objects[i]->mMesh->VertexCount; ++j)
					{
						v[j] = objects[i]->mMesh->Vertices[j];
					}

					// local transform
					TransformMesh(v, objects[i]->mRotate, 0.0f);

					// world transform
					TransformMesh(v, 0.0f, origin);

					// 종횡비
					float screenRatio = (float)gHeight / gWidth;

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

						// raster space
						x *= gWidth;
						y *= gHeight;

						vertex.Position = FVector(x, y, 0.0f);
					}


					FillTriangles(gPixelBuffer,gWidth,gHeight,v,objects[i]->mMesh->Indices,objects[i]->mMesh->IndexCount);
					DrawLines(gPixelBuffer, gWidth, gHeight, v, objects[i]->mMesh->Indices, objects[i]->mMesh->IndexCount);
				}
				// frustom test end
			}
		}

		SetBitmapBits(ghRenderTarget, gWidth* gHeight* 4, gPixelBuffer);

		BitBlt(ghdc, 0, 0, gWidth, gHeight, ghMemDC, 0, 0, SRCCOPY);
	}
	SelectObject(ghMemDC, oldBitmap);
}

void Render::Release()
{
	if (gIsInited)
	{
		delete[] gPixelBuffer;
		gPixelBuffer = nullptr;

		DeleteObject(ghRenderTarget);
		ghRenderTarget = NULL;

		ReleaseDC(ghWnd, ghMemDC);
		ghMemDC = NULL;
	}
}
#pragma endregion Interface

#pragma region Rendring Algorithms

float CalculateSlope(const FVector& startV, const FVector& endV)
{
	if (startV.Y == endV.Y)
		return 0.0f;

	float x = (endV.X - startV.X);
	float y = (endV.Y - startV.Y);

	// y값을 통해 x값을 얻어야 하기 때문에, y / x의 식을 x / y로 변형합니다.
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

// y값의 from, to로 이루어진 직선에서의 위치 값 (0 ~ 1)
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

	// 회전 공식 유도
	// x2 = cos(a + b), y2 = sin(a + b)
	// cos(a + b) = cosa * cosb - sina * sinb = x1 * cosb - y1 * sinb
	// sin(a + b) = sina * cosb + cosa * sinb = y1 * cosb + x1 * sinb 

	// X축 회전
	FVector xRotVec = v;
	float rX = Radian(rotate.X);
	xRotVec.Y = v.Y * cos(rX) - v.Z * sin(rX);
	xRotVec.Z = v.Z * cos(rX) + v.Y * sin(rX);

	// Y축 회전
	FVector xyRotVec = xRotVec;
	float rY = Radian(rotate.Y);
	xyRotVec.X = xRotVec.X * cos(rY) + xRotVec.Z * sin(rY);
	xyRotVec.Z = xRotVec.Z * cos(rY) - xRotVec.X * sin(rY);

	// Z축 회전
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
	for (int i = 0; i < indexCount; i+=3)
	{
		FillTriangleScanLine(pixelBuffer, width, height,{vertices[indices[i]],vertices[indices[i+1]],vertices[indices[i+2]]});
	}
}

void DrawLines(uint* pixelBuffer, uint width, uint height, const std::vector<FVector>& vertices, const int* indices, const uint indexCount)
{
	for (int i = 0; i < indexCount; i += 3)
	{
		DrawLine(pixelBuffer, width, height, vertices[indices[i]], vertices[indices[i + 1]]);
		DrawLine(pixelBuffer, width, height, vertices[indices[i + 1]], vertices[indices[i + 2]]);
		DrawLine(pixelBuffer, width, height, vertices[indices[i + 2]], vertices[indices[i]]);
	}
}

void DrawLines(uint* pixelBuffer, uint width, uint height, const std::vector<Vertex>& vertices, const int* indices, const uint indexCount)
{
	for (int i = 0; i < indexCount; i += 3)
	{
		DrawLine(pixelBuffer, width, height, vertices[indices[i]].Position, vertices[indices[i + 1]].Position);
		DrawLine(pixelBuffer, width, height, vertices[indices[i + 1]].Position, vertices[indices[i + 2]].Position);
		DrawLine(pixelBuffer, width, height, vertices[indices[i + 2]].Position, vertices[indices[i]].Position);
	}
}

void Draw_Indexed(uint* pixelBuffer, uint width, uint height, const Object* camera, const Object* object)
{
	float x = 0.0f, y = 0.0f;
	float z = 0.0f, nearZ = 1.0f;
	float planeX = 0.0f, planeY = 0.0f;
	float screenX = 0.0f, screenY = 0.0f;
	planeX = z / x * nearZ;
	planeY = z / y * nearZ;

	// rotate local

	// transform cube to camera local coord
	FVector rotate = object->mRotate - camera->mRotate;
	FVector origin = object->mOrigin - camera->mOrigin;
	std::vector<FVector> v = std::vector<FVector>(object->mMesh->VertexCount);

	for (int j = 0; j < v.size(); ++j)
	{
		v[j] = object->mMesh->Vertices[j].Position;
	}

	// local transform
	TransformMesh(v, object->mRotate, 0.0f);

	// world transform
	TransformMesh(v, 0.0f, origin);

	// 종횡비
	float screenRatio = (float)gHeight / gWidth;

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
		x *= gWidth;
		y *= gHeight;

		vertex = FVector(x, y, 0.0f);
	}

	DrawLines(pixelBuffer,width,height,v,object->mMesh->Indices,object->mMesh->IndexCount);
}


#pragma endregion Transformation
