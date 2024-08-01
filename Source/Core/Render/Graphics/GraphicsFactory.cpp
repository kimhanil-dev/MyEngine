#include "pch.h"

#include "GraphicsFactory.h"
#include "Graphics.h"

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
	/*case Renderer::Software:
	{
		static GraphicsSW sw;
		return &sw;
	}
	break;*/
	default:
		break;
	}

	return nullptr;
}
