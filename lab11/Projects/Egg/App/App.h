#pragma once

#include "SystemEnvironment.h"

namespace Egg {
	GG_CLASS(App)
	protected:
		com_ptr<ID3D11Device> device;
		com_ptr<IDXGISwapChain1> swapChain;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc;

		com_ptr<ID3D11Texture2D> backBuffer;
		com_ptr<ID3D11RenderTargetView> defaultRenderTargetView;
		com_ptr<ID3D11Texture2D> depthStencil;
		com_ptr<ID3D11DepthStencilView> defaultDepthStencilView;

		SystemEnvironment systemEnvironment;

		App(com_ptr<ID3D11Device> device):
			device(device) 
			{}
	public:
		void setSwapChain(com_ptr<IDXGISwapChain1> swapChain) {
			this->swapChain = swapChain;
			swapChain->GetDesc1(&swapChainDesc);
		}
		virtual ~App(){}

		SystemEnvironment& getSystemEnvironment(){return systemEnvironment;}
		com_ptr<ID3DBlob> loadShaderCode(const std::string& shaderFilename);

		virtual HRESULT createResources()
			{return S_OK;}
		virtual HRESULT createSwapChainResources();
		virtual HRESULT releaseResources() { return S_OK; }
		virtual HRESULT releaseSwapChainResources();
		virtual void animate(float dt, float t){}
		virtual bool processMessage( HWND hWnd, 
			UINT uMsg, WPARAM wParam, LPARAM lParam)
			{return false;}
		virtual void render(
			com_ptr<ID3D11DeviceContext> context){}
	GG_ENDCLASS
} // namespace Egg
