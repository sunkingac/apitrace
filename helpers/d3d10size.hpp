/**************************************************************************
 *
 * Copyright 2012 Jose Fonseca
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * AUTHORS,
 * AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **************************************************************************/


/*
 * Auxiliary functions to compute the size of array/blob arguments.
 */

#ifndef _D3D10SIZE_HPP_
#define _D3D10SIZE_HPP_


/* We purposedly don't include any D3D header, so that this header can be used
 * with all D3D versions. */

#include <assert.h>

#include <algorithm>


static size_t
_calcDataSize(DXGI_FORMAT Format, UINT Width, UINT Height, UINT RowPitch, UINT Depth = 1, UINT DepthPitch = 0) {
    if (Width == 0 || Height == 0 || Depth == 0) {
        return 0;
    }

    switch (Format) {
    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
        Width  = (Width  + 3) / 4;
        Height = (Height + 3) / 4;
        break;

    case DXGI_FORMAT_R8G8_B8G8_UNORM:
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
        Width = (Width + 1) / 2;
        break;

    case DXGI_FORMAT_UNKNOWN:
        return 0;

    default:
        break;
    }

    /* FIXME */
    (void)Width;

    size_t size = Height * RowPitch;

    if (Depth > 1) {
        size += (Depth - 1) * DepthPitch;
    }

    return size;
}

static size_t
_calcMipDataSize(UINT MipLevel, DXGI_FORMAT Format, UINT Width, UINT Height, UINT RowPitch, UINT Depth = 1, UINT DepthPitch = 0) {
    if (Width == 0 || Height == 0 || Depth == 0) {
        return 0;
    }

    Width  = std::max(Width  >> MipLevel, UINT(1));
    Height = std::max(Height >> MipLevel, UINT(1));
    Depth  = std::max(Depth  >> MipLevel, UINT(1));

    return _calcDataSize(Format, Width, Height, RowPitch, Depth, DepthPitch);
}


inline UINT
_getNumMipLevels(UINT Width, UINT Height = 1, UINT Depth = 1) {
    UINT MipLevels = 0;
    do {
        ++MipLevels;
        Width  >>= 1;
        Height >>= 1;
        Depth  >>= 1;
    } while (Width || Height || Depth);
    return MipLevels;
}

inline UINT
_getNumMipLevels(const D3D10_BUFFER_DESC *pDesc) {
    return 1;
}

inline UINT
_getNumMipLevels(const D3D10_TEXTURE1D_DESC *pDesc) {
    return pDesc->MipLevels != 0 ? pDesc->MipLevels : _getNumMipLevels(pDesc->Width);
}

inline UINT
_getNumMipLevels(const D3D10_TEXTURE2D_DESC *pDesc) {
    return pDesc->MipLevels != 0 ? pDesc->MipLevels : _getNumMipLevels(pDesc->Width, pDesc->Height);
}

inline UINT
_getNumMipLevels(const D3D10_TEXTURE3D_DESC *pDesc) {
    return pDesc->MipLevels != 0 ? pDesc->MipLevels : _getNumMipLevels(pDesc->Width, pDesc->Height, pDesc->Depth);
}

inline UINT
_getNumSubResources(const D3D10_BUFFER_DESC *pDesc) {
    return 1;
}

inline UINT
_getNumSubResources(const D3D10_TEXTURE1D_DESC *pDesc) {
    return _getNumMipLevels(pDesc) * pDesc->ArraySize;
}

inline UINT
_getNumSubResources(const D3D10_TEXTURE2D_DESC *pDesc) {
    return _getNumMipLevels(pDesc) * pDesc->ArraySize;
}

inline UINT
_getNumSubResources(const D3D10_TEXTURE3D_DESC *pDesc) {
    return _getNumMipLevels(pDesc);
}

static inline size_t
_calcSubresourceSize(const D3D10_BUFFER_DESC *pDesc, UINT Subresource, UINT RowPitch = 0, UINT SlicePitch = 0) {
    return pDesc->ByteWidth;
}

static inline size_t
_calcSubresourceSize(const D3D10_TEXTURE1D_DESC *pDesc, UINT Subresource, UINT RowPitch = 0, UINT SlicePitch = 0) {
    UINT MipLevel = Subresource % _getNumMipLevels(pDesc);
    return _calcMipDataSize(MipLevel, pDesc->Format, pDesc->Width, 1, RowPitch, 1, SlicePitch);
}

static inline size_t
_calcSubresourceSize(const D3D10_TEXTURE2D_DESC *pDesc, UINT Subresource, UINT RowPitch, UINT SlicePitch = 0) {
    UINT MipLevel = Subresource % _getNumMipLevels(pDesc);
    return _calcMipDataSize(MipLevel, pDesc->Format, pDesc->Width, pDesc->Height, RowPitch, 1, SlicePitch);
}

static inline size_t
_calcSubresourceSize(const D3D10_TEXTURE3D_DESC *pDesc, UINT Subresource, UINT RowPitch, UINT SlicePitch) {
    UINT MipLevel = Subresource;
    return _calcMipDataSize(MipLevel, pDesc->Format, pDesc->Width, pDesc->Height, RowPitch, pDesc->Depth, SlicePitch);
}

static inline void
_getMapInfo(ID3D10Buffer *pResource, D3D10_MAP MapType, UINT MapFlags, void * * ppData,
            void * & pMappedData, size_t & MappedSize) {
    pMappedData = 0;
    MappedSize = 0;

    if (MapType == D3D10_MAP_READ) {
        return;
    }

    D3D10_BUFFER_DESC Desc;
    pResource->GetDesc(&Desc);

    pMappedData = *ppData;
    MappedSize = Desc.ByteWidth;
}

static inline void
_getMapInfo(ID3D10Texture1D *pResource, UINT Subresource, D3D10_MAP MapType, UINT MapFlags, void * * ppData,
            void * & pMappedData, size_t & MappedSize) {
    pMappedData = 0;
    MappedSize = 0;

    if (MapType == D3D10_MAP_READ) {
        return;
    }

    D3D10_TEXTURE1D_DESC Desc;
    pResource->GetDesc(&Desc);

    pMappedData = *ppData;
    MappedSize = _calcSubresourceSize(&Desc, Subresource);
}

static inline void
_getMapInfo(ID3D10Texture2D *pResource, UINT Subresource, D3D10_MAP MapType, UINT MapFlags, D3D10_MAPPED_TEXTURE2D * pMappedTex2D,
            void * & pMappedData, size_t & MappedSize) {
    pMappedData = 0;
    MappedSize = 0;

    if (MapType == D3D10_MAP_READ) {
        return;
    }

    D3D10_TEXTURE2D_DESC Desc;
    pResource->GetDesc(&Desc);

    pMappedData = pMappedTex2D->pData;
    MappedSize = _calcSubresourceSize(&Desc, Subresource, pMappedTex2D->RowPitch);
}

static inline void
_getMapInfo(ID3D10Texture3D *pResource, UINT Subresource, D3D10_MAP MapType, UINT MapFlags, D3D10_MAPPED_TEXTURE3D * pMappedTex3D,
            void * & pMappedData, size_t & MappedSize) {
    pMappedData = 0;
    MappedSize = 0;

    if (MapType == D3D10_MAP_READ) {
        return;
    }

    D3D10_TEXTURE3D_DESC Desc;
    pResource->GetDesc(&Desc);

    pMappedData = pMappedTex3D->pData;
    MappedSize = _calcSubresourceSize(&Desc, Subresource, pMappedTex3D->RowPitch, pMappedTex3D->DepthPitch);
}


static inline void
_getMapInfo(IDXGISurface *pResource, DXGI_MAPPED_RECT * pLockedRect, UINT MapFlags,
            void * & pMappedData, size_t & MappedSize) {
    pMappedData = 0;
    MappedSize = 0;

    if (!(MapFlags & DXGI_MAP_WRITE)) {
        return;
    }

    DXGI_SURFACE_DESC Desc;
    HRESULT hr = pResource->GetDesc(&Desc);
    if (FAILED(hr)) {
        return;
    }

    pMappedData = pLockedRect->pBits;
    MappedSize = _calcDataSize(Desc.Format, Desc.Width, Desc.Height, pLockedRect->Pitch);
}


#endif /* _D3D10SIZE_HPP_ */
