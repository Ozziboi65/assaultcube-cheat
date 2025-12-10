#pragma once
#include <d3d11.h>

// Load image from file and create DirectX texture
// Returns texture pointer (ID3D11ShaderResourceView*) or nullptr on failure
bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height);
