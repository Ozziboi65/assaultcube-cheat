#include "image_loader.h"
#include "overlay.h"

#define STB_IMAGE_IMPLEMENTATION
#include "ect/stb_image.h"

bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height) {
    // Load image with stb_image
    int width, height, channels;
    unsigned char* image_data = stbi_load(filename, &width, &height, &channels, 4); // Force RGBA
    if (!image_data) return false;

    // Create texture description
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    // Create texture from image data
    ID3D11Texture2D* pTexture = nullptr;
    D3D11_SUBRESOURCE_DATA subResource = {};
    subResource.pSysMem = image_data;
    subResource.SysMemPitch = width * 4;
    subResource.SysMemSlicePitch = 0;

    HRESULT hr = g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);
    stbi_image_free(image_data); // Free stb_image data
    
    if (FAILED(hr) || !pTexture) return false;

    // Create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    hr = g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
    pTexture->Release();

    if (FAILED(hr)) return false;

    *out_width = width;
    *out_height = height;
    return true;
}
