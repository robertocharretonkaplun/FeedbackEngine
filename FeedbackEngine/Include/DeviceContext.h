/*
 * MIT License
 *
 * Copyright (c) 2025 Roberto Charreton
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * In addition, any project or software that uses this library or class must include
 * the following acknowledgment in the credits:
 *
 * "This project uses software developed by Roberto Charreton and Attribute Overload."
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/
#pragma once
#include "PreRequisites.h"

class 
DeviceContext {
public:
	DeviceContext() = default;
	~DeviceContext() = default;

	void
	init();
	
	void 
	update();
	
	void 
	render();
	
	void 
	destroy();

	void 
	RSSetViewports(unsigned int NumViewports,const D3D11_VIEWPORT *pViewports);

	void 
	PSSetShaderResources(unsigned int StartSlot,
											 unsigned int NumViews,
											 ID3D11ShaderResourceView* const* ppShaderResourceViews);

	void 
	IASetInputLayout(ID3D11InputLayout* pInputLayout);

	void
	VSSetShader(ID3D11VertexShader* pVertexShader, 
							ID3D11ClassInstance* const* ppClassInstances, 
							unsigned int NumClassInstances);
	void 
  PSSetShader(ID3D11PixelShader *pPixelShader,
              ID3D11ClassInstance *const *ppClassInstances,
              unsigned int NumClassInstances);

	void 
  UpdateSubresource(ID3D11Resource* pDstResource, 
                    unsigned int DstSubresource,
                    const D3D11_BOX* pDstBox,
                    const void* pSrcData,
                    unsigned int SrcRowPitch,
                    unsigned int SrcDepthPitch);

  void 
  IASetVertexBuffers(unsigned int StartSlot,
                     unsigned int NumBuffers,
                     ID3D11Buffer *const *ppVertexBuffers,
                     const unsigned int*pStrides,
                     const unsigned int*pOffsets);

  void 
  IASetIndexBuffer(ID3D11Buffer *pIndexBuffer,
                   DXGI_FORMAT Format,
                   unsigned int Offset);

	void
  PSSetSamplers(unsigned int StartSlot,
                unsigned int NumSamplers,
                ID3D11SamplerState* const* ppSamplers);
	
	void 
	RSSetState(ID3D11RasterizerState* pRasterizerState);

	void 
	OMSetBlendState(ID3D11BlendState* pBlendState,
									const float BlendFactor[4],
									unsigned int SampleMask);

	void 
	OMSetRenderTargets(unsigned int NumViews, 
										 ID3D11RenderTargetView* const* ppRenderTargetViews, 
										 ID3D11DepthStencilView* pDepthStencilView);

	void
	IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY Topology);

	void
	ClearRenderTargetView(ID3D11RenderTargetView* pRenderTargetView, 
												const float ColorRGBA[4]);

	void
	ClearDepthStencilView(ID3D11DepthStencilView* pDepthStencilView, 
												unsigned int ClearFlags, 
												float Depth, 
												UINT8 Stencil);

	void
	VSSetConstantBuffers(unsigned int StartSlot, 
											 unsigned int NumBuffers, 
											 ID3D11Buffer* const* ppConstantBuffers);

	void
	PSSetConstantBuffers(unsigned int StartSlot, 
											 unsigned int NumBuffers, 
											 ID3D11Buffer* const* ppConstantBuffers);
	void
	DrawIndexed(unsigned int IndexCount, 
							unsigned int StartIndexLocation, 
							int BaseVertexLocation);
private:

public:
	ID3D11DeviceContext* m_deviceContext = nullptr;
};