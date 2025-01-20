#include "stdafx.h"
#include "Game.h"
#include "ThrowOnFail.h"
#include "Mesh/Importer.h"
#include "Mesh/Importer.h"
#include "Flex/MeshBuilder.h"
#include "DirectXTex.h"
#include "UtfConverter.h"
#include "DDSTextureLoader.h"
#include <assimp/importer.hpp>      // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postProcess.h> // Post processing flags
using namespace Egg::Math;

Game::Game(com_ptr<ID3D11Device> device) : Egg::App(device)
{
}

Game::~Game(void)
{
}

HRESULT Game::createResources() {
	using namespace Egg::Flex;

	try{
	// create camera
	firstPersonCam = Egg::Cam::FirstPerson::create();

	// create binder for input layout creation
	inputBinder = Egg::Mesh::InputBinder::create(device);

	// load giraffe geometry
	Assimp::Importer importer;
	const aiScene* assScene = importer.ReadFile( App::getSystemEnvironment().resolveMediaPath("tiggerfilled.obj"),
		aiProcess_Triangulate | aiProcess_GenSmoothNormals);
	MeshBuilderP meshBuilder = 
		MeshBuilder::create(assScene->mMeshes[0]);
	MeshBuilder::AdjacencyDesc adjDesc;
	adjDesc.epsilon = 0.01;
	adjDesc.transcendCreases = true;
	meshBuilder->generateAdjacency(adjDesc, MeshBuilder::AdjacencyAlgorithmVector());
	Egg::Mesh::GeometryP geometry = meshBuilder->commitToDevice(device.Get());
	//Egg::Mesh::GeometryP geometry = Egg::Mesh::Importer::fromAiMesh(device, assScene->mMeshes[0]);

	// load compiled shaders
	com_ptr<ID3DBlob> vertexShaderByteCode = loadShaderCode("vsTrafo.cso");
	Egg::Mesh::ShaderP vertexShader = Egg::Mesh::Shader::create(device, vertexShaderByteCode);
	com_ptr<ID3DBlob> geometryShaderByteCode = loadShaderCode("gsAdjacencyTest.cso");
	Egg::Mesh::ShaderP geometryShader = Egg::Mesh::Shader::create(device, geometryShaderByteCode);
	com_ptr<ID3DBlob> pixelShaderByteCode = loadShaderCode("psEnvMapped.cso");
	Egg::Mesh::ShaderP pixelShader = Egg::Mesh::Shader::create(device, pixelShaderByteCode);

	// create material using shaders
	Egg::Mesh::MaterialP material = Egg::Mesh::Material::create();
	material->setShader(Egg::Mesh::ShaderStageFlag::Vertex, vertexShader);
	material->setShader(Egg::Mesh::ShaderStageFlag::Geometry, geometryShader);
	material->setShader(Egg::Mesh::ShaderStageFlag::Pixel, pixelShader);

	// create mesh from geometry and material
	com_ptr<ID3D11InputLayout> inputLayout = inputBinder->getCompatibleInputLayout(vertexShaderByteCode, geometry);
	shadedMesh = Egg::Mesh::Shaded::create(geometry, material, inputLayout);

	// create constant buffers (per object, per frame) -- we could use shader reflection to obtain buffer size (and variable layout) from the shader code, but this is simpler
	D3D11_BUFFER_DESC constantBufferDesc;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof(Egg::Math::float4x4) * 4;
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.MiscFlags = 0;
	constantBufferDesc.StructureByteStride = 0;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	Egg::ThrowOnFail("Failed to create per object constant buffer.", __FILE__, __LINE__) ^
		device->CreateBuffer(&constantBufferDesc, nullptr, perObjectConstantBuffer.GetAddressOf());

	constantBufferDesc.ByteWidth = sizeof(Egg::Math::float4) * 1;
	Egg::ThrowOnFail("Failed to create per frame constant buffer.", __FILE__, __LINE__) ^
		device->CreateBuffer(&constantBufferDesc, nullptr, perFrameConstantBuffer.GetAddressOf());

	// bind the constant buffer resources to the cbuffer declarations in the HLSL shader code
	material->setConstantBuffer("perObject", perObjectConstantBuffer);
	material->setConstantBuffer("perFrame", perFrameConstantBuffer);

	// load texture from file
	std::wstring file = Egg::UtfConverter::utf8to16( App::getSystemEnvironment()
		.resolveMediaPath("tigger.png") );

	DirectX::ScratchImage si;
	DirectX::ScratchImage simipmap;
	DirectX::TexMetadata metadata;
	Egg::ThrowOnFail("Failed to load texture.", __FILE__, __LINE__) ^
		DirectX::LoadFromWICFile(file.c_str(), 0, &metadata, si);

	// compute mipmaps
	Egg::ThrowOnFail("Failed generating mipmaps.", __FILE__, __LINE__) ^
		DirectX::GenerateMipMaps(si.GetImage(0, 0, 0), 1, metadata, (DWORD)DirectX::TEX_FILTER_DEFAULT, 0, simipmap);

	// create texture resource
	com_ptr<ID3D11Resource> resource;
	Egg::ThrowOnFail("Could not create 2D texture.", __FILE__, __LINE__) ^
		DirectX::CreateTextureEx(
			device.Get(),
			simipmap.GetImages(), simipmap.GetImageCount(), metadata,
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_SHADER_RESOURCE,
			0,
			0,
			false, resource.GetAddressOf());
	resource.As<ID3D11Texture2D>(&texture);

	// create shader resource view
	D3D11_TEXTURE2D_DESC tdesc;
	texture->GetDesc(&tdesc);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Format = tdesc.Format;
	desc.Texture2D.MipLevels = tdesc.MipLevels;
	desc.Texture2D.MostDetailedMip = 0;

	com_ptr<ID3D11ShaderResourceView> tsrv;
	Egg::ThrowOnFail("Could not create SRV.", __FILE__, __LINE__) ^
		device->CreateShaderResourceView(texture.Get(), &desc, tsrv.GetAddressOf());

	// load enviroment texture and create shader resource view
	com_ptr<ID3D11Resource> envTexture;
	com_ptr<ID3D11ShaderResourceView> envSrv;

	std::wstring envfile = Egg::UtfConverter::utf8to16(App::getSystemEnvironment()
		.resolveMediaPath("cloudynoon.dds"));
	Egg::ThrowOnFail("Could not create cube map texture and/or SRV.", __FILE__, __LINE__) ^
		DirectX::CreateDDSTextureFromFile(device.Get(),
			envfile.c_str(),
			envTexture.GetAddressOf(), envSrv.GetAddressOf());
	
	// bind both textures to the Texture2d and TextureCube declarations in the shader code
	material->setShaderResource("envTexture", envSrv);
	material->setShaderResource("kd", tsrv);

	// create a sampler state with default settings
	com_ptr<ID3D11SamplerState> ss;
	CD3D11_SAMPLER_DESC ssDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
	Egg::ThrowOnFail("Could not create SS.", __FILE__, __LINE__) ^
		device->CreateSamplerState(&ssDesc, ss.GetAddressOf());

	// bind the sampler state to the SamplerState declaration in the shader code
	material->setSamplerState("ss", ss);


	// create background geometry
	Egg::Mesh::GeometryP backgroundGeometry = Egg::Mesh::Indexed::createQuad(device);

	// load shaders
	com_ptr<ID3DBlob> backgroundVertexShaderByteCode = loadShaderCode("vsQuad.cso");
	Egg::Mesh::ShaderP backgroundVertexShader = Egg::Mesh::Shader::create(device, backgroundVertexShaderByteCode);
	com_ptr<ID3DBlob> backgroundPixelShaderByteCode = loadShaderCode("psBackground.cso");
	Egg::Mesh::ShaderP backgroundPixelShader = Egg::Mesh::Shader::create(device, backgroundPixelShaderByteCode);

	// create material
	Egg::Mesh::MaterialP backgroundMaterial = Egg::Mesh::Material::create();
	backgroundMaterial->setShader(Egg::Mesh::ShaderStageFlag::Vertex, backgroundVertexShader);
	backgroundMaterial->setShader(Egg::Mesh::ShaderStageFlag::Pixel, backgroundPixelShader);
	backgroundMaterial->setSamplerState("ss", ss);

	// bind material shader resources and constant buffers
	backgroundMaterial->setShaderResource("envTexture", envSrv);
	backgroundMaterial->setConstantBuffer("perObject", perObjectConstantBuffer);
    //backgroundMaterial->setConstantBuffer("perFrame", perFrameConstantBuffer);

	// create mesh
	com_ptr<ID3D11InputLayout> backgroundInputLayout = inputBinder->getCompatibleInputLayout(backgroundVertexShaderByteCode, backgroundGeometry);
	backgroundMesh = Egg::Mesh::Shaded::create(backgroundGeometry, backgroundMaterial, backgroundInputLayout);


	// create show geometry
	Egg::Mesh::GeometryP showGeometry = Egg::Mesh::Indexed::createQuad(device);

	// load shaders
	com_ptr<ID3DBlob> showVertexShaderByteCode = loadShaderCode("vsQuad.cso");
	Egg::Mesh::ShaderP showVertexShader = Egg::Mesh::Shader::create(device, showVertexShaderByteCode);
	com_ptr<ID3DBlob> showPixelShaderByteCode = loadShaderCode("psShow.cso");
	Egg::Mesh::ShaderP showPixelShader = Egg::Mesh::Shader::create(device, showPixelShaderByteCode);

	// create material
	showMaterial = Egg::Mesh::Material::create();
	showMaterial->setShader(Egg::Mesh::ShaderStageFlag::Vertex, showVertexShader);
	showMaterial->setShader(Egg::Mesh::ShaderStageFlag::Pixel, showPixelShader);
	showMaterial->setSamplerState("ss", ss);

	// bind material shader resources
	showMaterial->setShaderResource("showTexture", sampleSrv);

	// create mesh
	com_ptr<ID3D11InputLayout> showInputLayout = inputBinder->getCompatibleInputLayout(showVertexShaderByteCode, showGeometry);
	showMesh = Egg::Mesh::Shaded::create(showGeometry, showMaterial, showInputLayout);

	// create resources //////////////////////////////////////////////////

	CD3D11_TEXTURE2D_DESC sampleTextureDesc(
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		128, 128);
	sampleTextureDesc.BindFlags =
		D3D11_BIND_SHADER_RESOURCE |
		D3D11_BIND_RENDER_TARGET;
	sampleTextureDesc.MipLevels = 1;

	device->CreateTexture2D(
		&sampleTextureDesc,
		NULL,
		sampleTexture.GetAddressOf());



	CD3D11_RENDER_TARGET_VIEW_DESC sampleRtvDesc;
	sampleRtvDesc.Format =
		DXGI_FORMAT_R32G32B32A32_FLOAT;
	sampleRtvDesc.ViewDimension =
		D3D11_RTV_DIMENSION_TEXTURE2D;
	sampleRtvDesc.Texture2D.MipSlice = 0;

	device->CreateRenderTargetView(
		sampleTexture.Get(),
		&sampleRtvDesc,
		sampleRtv.GetAddressOf());




	CD3D11_SHADER_RESOURCE_VIEW_DESC sampleSrvDesc;
	sampleSrvDesc.Format =
		DXGI_FORMAT_R32G32B32A32_FLOAT;
	sampleSrvDesc.ViewDimension =
		D3D11_SRV_DIMENSION_TEXTURE2D;
	sampleSrvDesc.Texture2D.MipLevels = 1;
	sampleSrvDesc.Texture2D.MostDetailedMip = 0;

	device->CreateShaderResourceView(
		sampleTexture.Get(),
		&sampleSrvDesc,
		sampleSrv.GetAddressOf());


	CD3D11_TEXTURE2D_DESC sampleDepthTextureDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		128, 128);
	sampleDepthTextureDesc.BindFlags =
		D3D11_BIND_DEPTH_STENCIL;
	sampleDepthTextureDesc.MipLevels = 1;
	HRESULT hr = device->CreateTexture2D(
		&sampleDepthTextureDesc,
		NULL,
		sampleDepthTexture.GetAddressOf());


	CD3D11_DEPTH_STENCIL_VIEW_DESC sampleDsvDesc;
	sampleDsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	sampleDsvDesc.Flags = 0;
	sampleDsvDesc.ViewDimension =
		D3D11_DSV_DIMENSION_TEXTURE2D;
	sampleDsvDesc.Texture2D.MipSlice = 0;

	device->CreateDepthStencilView(
		sampleDepthTexture.Get(),
		&sampleDsvDesc,
		sampleDsv.GetAddressOf());


	//////////////////////////////////////////////////////////////////////

	CD3D11_BUFFER_DESC
		reduceBuffer1Desc(sizeof(float) * 128, D3D11_BIND_UNORDERED_ACCESS);
	device->CreateBuffer(&reduceBuffer1Desc, NULL, reduceBuffer1.GetAddressOf());

	CD3D11_UNORDERED_ACCESS_VIEW_DESC
		reduceUav1Desc(D3D11_UAV_DIMENSION_BUFFER,
			DXGI_FORMAT_R32_FLOAT, 0, 128);
	device->CreateUnorderedAccessView(
		reduceBuffer1.Get(), &reduceUav1Desc, reduceUav1.GetAddressOf());

	CD3D11_BUFFER_DESC
		reduceBuffer2Desc(sizeof(float) * 1, D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE);
	device->CreateBuffer(&reduceBuffer2Desc, NULL, reduceBuffer2.GetAddressOf());

	CD3D11_UNORDERED_ACCESS_VIEW_DESC
		reduceUav2Desc(D3D11_UAV_DIMENSION_BUFFER,
			DXGI_FORMAT_R32_FLOAT, 0, 1);
	device->CreateUnorderedAccessView(
		reduceBuffer2.Get(), &reduceUav2Desc, reduceUav2.GetAddressOf());

	CD3D11_SHADER_RESOURCE_VIEW_DESC
		reduceSrv2Desc(
			D3D11_SRV_DIMENSION_BUFFER,
			DXGI_FORMAT_R32_FLOAT, 0, 1);
	device->CreateShaderResourceView(
		reduceBuffer2.Get(), &reduceSrv2Desc, reduceSrv2.GetAddressOf());


	com_ptr<ID3DBlob> csReduceHorizontalByteCode = loadShaderCode("csReduceHorizontal.cso");
	csReduceHorizontal = Egg::Mesh::Shader::create(device, csReduceHorizontalByteCode);
	com_ptr<ID3DBlob> csReduceVerticalByteCode = loadShaderCode("csReduceVertical.cso");
	csReduceVertical = Egg::Mesh::Shader::create(device, csReduceVerticalByteCode);

	}
	catch (Egg::HrException hr) {
		MessageBoxA(nullptr, (hr.errorMessage).c_str(), "Error!", MB_OK);
		exit(-1);
	}
	return S_OK;
}

HRESULT Game::releaseResources()
{
	return Egg::App::releaseResources();
	sampleTexture.Reset();
	sampleSrv.Reset();
	sampleRtv.Reset();
	sampleDepthTexture.Reset();
	sampleDsv.Reset();
	reduceBuffer1.Reset();
	reduceUav1.Reset();
	reduceBuffer2.Reset();
	reduceUav2.Reset();
	reduceSrv2.Reset();
}

HRESULT Game::createSwapChainResources() {
	firstPersonCam->setAspect((float)swapChainDesc.Width / swapChainDesc.Height);
	return __super::createSwapChainResources();
}
HRESULT Game::releaseSwapChainResources() {
	return __super::releaseSwapChainResources();
}

void Game::render(com_ptr<ID3D11DeviceContext> context)
{
	using namespace Egg::Math;

	context->OMSetRenderTargets(1, defaultRenderTargetView.GetAddressOf(), defaultDepthStencilView.Get());

	float clearColor[4] =
	{ 0.9f, 0.7f, 0.1f, 0.0f };
	context->ClearRenderTargetView(defaultRenderTargetView.Get(), clearColor);
	context->ClearDepthStencilView(defaultDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0, 0);

	// Update per object data. The memory layout has to be match the cbuffer declaration in perObject.hlsli
	float4x4 perObjectMatrices[4];
	perObjectMatrices[0] = float4x4::identity; // modelMatrix
	perObjectMatrices[1] = float4x4::identity; // modelMatrixInverse;
	perObjectMatrices[2] = (firstPersonCam->getViewMatrix() * firstPersonCam->getProjMatrix()); // modelViewProjMatrix;
	perObjectMatrices[3] = firstPersonCam->getRayDirMatrix(); // rayDirMatrix
	context->UpdateSubresource(perObjectConstantBuffer.Get(), 0, nullptr, perObjectMatrices, 0, 0);

	// Update per frame data. The memory layout has to be match the cbuffer declaration in perFrame.hlsli
	float4 perFrameVectors[1];
	perFrameVectors[0] = firstPersonCam->getEyePosition().xyz1;
	context->UpdateSubresource(perFrameConstantBuffer.Get(), 0, nullptr, perFrameVectors, 0, 0);

	context->OMSetRenderTargets(
		1, sampleRtv.GetAddressOf(), sampleDsv.Get());
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Height = 128;
	vp.Width = 128;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	context->RSSetViewports(1, &vp);

	context->ClearRenderTargetView(sampleRtv.Get(), clearColor);
	context->ClearDepthStencilView(sampleDsv.Get(), D3D11_CLEAR_DEPTH, 1.0, 0);

	// draw scene
	backgroundMesh->draw(context);
	shadedMesh->draw(context);

	context->OMSetRenderTargets(
		1,
		defaultRenderTargetView.GetAddressOf(),
		defaultDepthStencilView.Get());
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Height = swapChainDesc.Height;
	vp.Width = swapChainDesc.Width;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	context->RSSetViewports(1, &vp);

	uint zeros[2] = { 0, 0 };
	ID3D11UnorderedAccessView* nulls[2] = { nullptr, nullptr };

	context->CSSetShader(csReduceHorizontal->as<ID3D11ComputeShader>().Get(), nullptr, 0);
	context->CSSetUnorderedAccessViews(0, 1, reduceUav1.GetAddressOf(), zeros);
	context->CSSetShaderResources(0, 1, sampleSrv.GetAddressOf());
	context->Dispatch(128, 1, 1);

	context->CSSetShader(csReduceVertical->as<ID3D11ComputeShader>().Get(), nullptr, 0);
	context->CSSetUnorderedAccessViews(0, 1, reduceUav1.GetAddressOf(), zeros);
	context->CSSetUnorderedAccessViews(1, 1, reduceUav2.GetAddressOf(), zeros);
	context->Dispatch(1, 1, 1);

	context->CSSetUnorderedAccessViews(0, 2, nulls, zeros);

	showMaterial->setShaderResource("showTexture", sampleSrv);
	showMaterial->setShaderResource("toneData", reduceSrv2);
	showMesh->draw(context);

	context->ClearState();
}

void Game::animate(float dt, float t)
{
	if (!firstPersonCam) return;
	firstPersonCam->animate(dt);

}

bool Game::processMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!firstPersonCam) return false;
	firstPersonCam->processMessage(hWnd, uMsg, wParam, lParam);
	return false;
}

