#include "IGraphics.h"

#include "Graphics.h"
#include "GraphicsSW.h"


IGraphics* GetRenderer(Renderer renderer)
{
	switch (renderer)
	{
	case Renderer::DriectX:
	{
		static Graphics dx;
		return &dx;
	}
		break;
	case Renderer::Software:
	{
		static GraphicsSW sw;
		return &sw;
	}
		break;
	default:
		break;
	}

	return nullptr;
}
