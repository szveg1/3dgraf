#pragma once
#include "App/App.h"
#include "Mesh/InputBinder.h"
#include "Mesh/Shaded.h"
#include "Cam/FirstPerson.h"

GG_SUBCLASS(Game, Egg::App)
	Egg::Mesh::InputBinderP inputBinder;	//< Input layout manager.
	Egg::Mesh::ShadedP shadedMesh;			//< Giraffe mesh.
	Egg::Mesh::ShadedP backgroundMesh;		//< Background quad mesh.
	Egg::Mesh::ShadedP showMesh;
	Egg::Mesh::MaterialP showMaterial;

	Egg::Cam::FirstPersonP firstPersonCam;	//< Camera.
	com_ptr<ID3D11Buffer> perObjectConstantBuffer;	//< Constant buffer for transformation matrices.
	com_ptr<ID3D11Buffer> perFrameConstantBuffer;	//< Constant buffer for trnasformation matrices.
	com_ptr<ID3D11Texture2D> texture;				//< Giraffe pattern texture.

	Microsoft::WRL::ComPtr<ID3D11Texture2D> sampleTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sampleSrv;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> sampleRtv;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> sampleDepthTexture;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> sampleDsv;

	Microsoft::WRL::ComPtr<ID3D11Buffer> reduceBuffer1;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> reduceUav1;
	Microsoft::WRL::ComPtr<ID3D11Buffer> reduceBuffer2;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> reduceUav2;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> reduceSrv2;

	Egg::Mesh::ShaderP csReduceHorizontal;
	Egg::Mesh::ShaderP csReduceVertical;


public:
	Game(com_ptr<ID3D11Device> device);				//< Constructor.
	~Game(void);													//< Destructor.

	HRESULT createResources();		//< Event handler invoked on device creation. Create all resources here.
	HRESULT releaseResources();		//< Event handler invoked on device destruction. Release all resources here.

	HRESULT createSwapChainResources();
	HRESULT releaseSwapChainResources();


	void render(com_ptr<ID3D11DeviceContext> context);	//< Event handler for frame render. Draw here.
	void animate(float dt, float t);									//< Event handler for frame move.
	bool processMessage(HWND hWnd,										//< Event handler for Windows messages. Handle keypresses here.
		UINT uMsg, WPARAM wParam, LPARAM lParam);

GG_ENDCLASS
