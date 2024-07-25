#pragma once

enum class Renderer : unsigned int
{
	DriectX,
	Software,
	// add renderer here

	// ...
	Count,
};

class IGraphics;
IGraphics* GetRenderer(Renderer renderer);