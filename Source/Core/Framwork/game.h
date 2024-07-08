#pragma once

#include "Window/framework.h"
#include "Core/Types.h"

void Init(HWND hWnd, const uint width, const uint height);
bool Update();
void UpdateInput(uint key);
void Release();