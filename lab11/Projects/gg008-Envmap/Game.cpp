#include "stdafx.h"
#include "Game.h"
#include "ThrowOnFail.h"
#include "Mesh/Importer.h"
#include "Mesh/Importer.h"
#include "DirectXTex.h"
#include "UtfConverter.h"
#include "DDSTextureLoader.h"
#include <assimp/importer.hpp>      // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postProcess.h> // Post processing flags
using namespace Egg::Math;

Game::Game(com_ptr<ID3D11Device> device) : Egg::App(device)
{
	// LABTODO: inital particles
	for (uint i = 0; i < 40; i++) {
		particles.push_back(Particle());
	}
}

Game::~Game(void)
{
}

HRESULT Game::createResources() {
	using namespace Microsoft::WRL;

	try{
	// create camera
	firstPersonCam = Egg::Cam::FirstPerson::create();
	firstPersonCam->setView(float3(0, 0, 200), float3(0, 0, -1));
	firstPersonCam->setProj(1.2, 1, 1, 1000);
	firstPersonCam->setSpeed(50);

	// create binder for input layout creation
	inputBinder = Egg::Mesh::InputBinder::create(device);

	// load giraffe geometry
	Assimp::Importer importer;
	const aiScene* assScene = importer.ReadFile( App::getSystemEnvironment().resolveMediaPath("giraffe.obj"), 0);
	Egg::Mesh::GeometryP geometry = Egg::Mesh::Importer::fromAiMesh(device, assScene->mMeshes[0]);

	// load compiled shaders
	com_ptr<ID3DBlob> vertexShaderByteCode = loadShaderCode("vsTrafo.cso");
	Egg::Mesh::ShaderP vertexShader = Egg::Mesh::Shader::create(device, vertexShaderByteCode);
	com_ptr<ID3DBlob> pixelShaderByteCode = loadShaderCode("psEnvMapped.cso");
	Egg::Mesh::ShaderP pixelShader = Egg::Mesh::Shader::create(device, pixelShaderByteCode);

	// create material using shaders
	Egg::Mesh::MaterialP material = Egg::Mesh::Material::create();
	material->setShader(Egg::Mesh::ShaderStageFlag::Vertex, vertexShader);
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

	// LABTODO: create perSwapChainConstantBuffer
	Egg::Math::float4 billboardSize(.1, .1, 0, 0);
	D3D11_SUBRESOURCE_DATA initialBbSize;
	initialBbSize.pSysMem = &billboardSize;
	constantBufferDesc.ByteWidth = sizeof(Egg::Math::float4) * 1;
	Egg::ThrowOnFail("Failed to create per frame constant buffer.", __FILE__, __LINE__) ^
		device->CreateBuffer(&constantBufferDesc, &initialBbSize, perSwapChainConstantBuffer.GetAddressOf());


	// bind the constant buffer resources to the cbuffer declarations in the HLSL shader code
	material->setConstantBuffer("perObject", perObjectConstantBuffer);
	material->setConstantBuffer("perFrame", perFrameConstantBuffer);

	// load texture from file
	std::wstring file = Egg::UtfConverter::utf8to16( App::getSystemEnvironment()
		.resolveMediaPath("giraffe.jpg") );

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
    // for tone mapping: backgroundMaterial->setConstantBuffer("perFrame", perFrameConstantBuffer);

	// create mesh
	com_ptr<ID3D11InputLayout> backgroundInputLayout = inputBinder->getCompatibleInputLayout(backgroundVertexShaderByteCode, backgroundGeometry);
	backgroundMesh = Egg::Mesh::Shaded::create(backgroundGeometry, backgroundMaterial, backgroundInputLayout);

	//LABTODO: particle input layout
	D3D11_INPUT_ELEMENT_DESC particleElements[3];
	particleElements[0].AlignedByteOffset = offsetof(Particle, position);
	particleElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	particleElements[0].InputSlot = 0;
	particleElements[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	particleElements[0].InstanceDataStepRate = 0;
	particleElements[0].SemanticIndex = 0;
	particleElements[0].SemanticName = "POSITION";
	particleElements[1].AlignedByteOffset = offsetof(Particle, lifespan);
	particleElements[1].Format = DXGI_FORMAT_R32_FLOAT;
	particleElements[1].InputSlot = 0;
	particleElements[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	particleElements[1].InstanceDataStepRate = 0;
	particleElements[1].SemanticIndex = 0;
	particleElements[1].SemanticName = "LIFESPAN";
	particleElements[2].AlignedByteOffset = offsetof(Particle, age);
	particleElements[2].Format = DXGI_FORMAT_R32_FLOAT;
	particleElements[2].InputSlot = 0;
	particleElements[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	particleElements[2].InstanceDataStepRate = 0;
	particleElements[2].SemanticIndex = 0;
	particleElements[2].SemanticName = "AGE";

	//LABTOoD: particle buffer()
	Egg::Mesh::VertexStreamDesc particleBufferDesc;

	particleBufferDesc.elements = particleElements;
	particleBufferDesc.nElements = 3;
	particleBufferDesc.nVertices = particles.size();
	particleBufferDesc.vertexStride = sizeof(Particle);
	particleBufferDesc.vertexData = &particles.at(0);
	particleBufferDesc.cpuAccessFlags = D3D11_CPU_ACCESS_WRITE;
	particleBufferDesc.usage = D3D11_USAGE_DYNAMIC;

	Egg::Mesh::VertexStreamP particleVertexStream = Egg::Mesh::VertexStream::create(device, particleBufferDesc);


	//LABTODO: billboard texture and SRV
	com_ptr<ID3D11Resource> bbTexture;
	com_ptr<ID3D11ShaderResourceView> bbSrv;

	std::wstring bbfile = Egg::UtfConverter::utf8to16(App::getSystemEnvironment().resolveMediaPath("particle.dds"));
	Egg::ThrowOnFail("Could not create particle texture and/or SRV.", __FILE__, __LINE__) ^
		DirectX::CreateDDSTextureFromFile(device.Get(), bbfile.c_str(), bbTexture.GetAddressOf(), bbSrv.GetAddressOf());

	//LABTODO:  billboard and fire shaders
	com_ptr<ID3DBlob> billboardVertexShaderByteCode = loadShaderCode("vsBillboard.cso");
	Egg::Mesh::ShaderP billboardVertexShader = Egg::Mesh::Shader::create(device, billboardVertexShaderByteCode);
	com_ptr<ID3DBlob> billboardGeometryShaderByteCode = loadShaderCode("gsBillboard.cso");
	Egg::Mesh::ShaderP billboardGeometryShader = Egg::Mesh::Shader::create(device, billboardGeometryShaderByteCode);
	com_ptr<ID3DBlob> firePixelShaderByteCode = loadShaderCode("psFire.cso");
	Egg::Mesh::ShaderP firePixelShader = Egg::Mesh::Shader::create(device, firePixelShaderByteCode);


	//LABTODO: blend state
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	device->CreateBlendState(&blendDesc, blendState.GetAddressOf());


	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

	device->CreateDepthStencilState(&depthStencilDesc, depthStencilState.GetAddressOf());



	//LABTODO: fire material
	Egg::Mesh::Material::P fireMaterial = Egg::Mesh::Material::create();
	/* feladat: shaderek, constant bufferek, textúra bekötése */
	fireMaterial->setShader(Egg::Mesh::ShaderStageFlag::Vertex, billboardVertexShader);
	fireMaterial->setShader(Egg::Mesh::ShaderStageFlag::Geometry, billboardGeometryShader);
	fireMaterial->setShader(Egg::Mesh::ShaderStageFlag::Pixel, firePixelShader);

	fireMaterial->setConstantBuffer("perSwapChain", perSwapChainConstantBuffer);
	fireMaterial->setConstantBuffer("perObject", perObjectConstantBuffer);
	fireMaterial->setSamplerState("ss", ss);

	fireMaterial->setShaderResource("billboardTexture", bbSrv);
	fireMaterial->setBlendState(blendState);
	fireMaterial->setDepthStencilState(depthStencilState);
	

	//LABTODO: input layout (billboard VS + fire material)
	ComPtr<ID3D11InputLayout> billboardInputLayout = inputBinder->getCompatibleInputLayout(billboardVertexShaderByteCode, particleVertexStream);

	//LABTODO: shaded mesh (particle stream geomery + fire material)
	fireBillboardSet = Egg::Mesh::Shaded::create(particleVertexStream, fireMaterial, billboardInputLayout);

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
	fireBillboardSet.reset();
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

	//LABTODO: upload swap chain buffer data
	float4 worldBillboardSize(50.0, 50.0, 0, 0);
	float4 screenBillboardSize = worldBillboardSize * firstPersonCam->getProjMatrix();
	context->UpdateSubresource(perSwapChainConstantBuffer.Get(), 0, nullptr, &screenBillboardSize, 0, 0);


	// draw scene
	backgroundMesh->draw(context);
	shadedMesh->draw(context);

	// LABTODO: draw fire
	fireBillboardSet->draw(context);
}

void Game::animate(float dt, float t)
{
	if (!firstPersonCam) return;
	firstPersonCam->animate(dt);

	using namespace Egg::Math;
	struct CameraDepthComparator {
		float3 ahead;
		bool operator()(const Particle& a,
			const Particle& b) {
			return
				a.position.dot(ahead) >
				b.position.dot(ahead) + 0.01;
		}
	} comp = { firstPersonCam->getAhead() };
	std::sort(particles.begin(), particles.end(), comp);


	for (int i = 0; i < particles.size(); i++) particles.at(i).move(dt);
	//LABTODO: copy particle data to billboards buffer
	auto vertexBuffer = fireBillboardSet->getGeometry()->getPrimaryBuffer();

	ID3D11DeviceContext* context;
	device->GetImmediateContext(&context);
	D3D11_MAPPED_SUBRESOURCE mappedVertexData;
	context->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVertexData);
	memcpy(mappedVertexData.pData, &particles.at(0), sizeof(Particle) * particles.size());
	context->Unmap(vertexBuffer.Get(), 0);
	context->Release();

}

bool Game::processMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!firstPersonCam) return false;
	firstPersonCam->processMessage(hWnd, uMsg, wParam, lParam);
	return false;
}
