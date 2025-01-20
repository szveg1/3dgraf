#include "stdafx.h"
#include "Mesh/Shader.h"

using namespace Egg;

Mesh::Shader::Shader(
	com_ptr<ID3D11Device> device,
	com_ptr<ID3DBlob> byteCode)
	: byteCode(byteCode)
{
	D3DReflect(byteCode->GetBufferPointer(), byteCode->GetBufferSize(),
		IID_ID3D11ShaderReflection, (void**)reflection.GetAddressOf());
	
	D3D11_SHADER_DESC shaderDesc;
	reflection->GetDesc(&shaderDesc);

	if (D3D11_SHVER_GET_TYPE(shaderDesc.Version) == D3D11_SHVER_VERTEX_SHADER) {
		com_ptr<ID3D11VertexShader> vs;
		device->CreateVertexShader(byteCode->GetBufferPointer(), byteCode->GetBufferSize(), nullptr, vs.GetAddressOf());
		shader = vs;
	} else if (D3D11_SHVER_GET_TYPE(shaderDesc.Version) == D3D11_SHVER_PIXEL_SHADER) {
		com_ptr<ID3D11PixelShader> ps;
		device->CreatePixelShader(byteCode->GetBufferPointer(), byteCode->GetBufferSize(), nullptr, ps.GetAddressOf());
		shader = ps;
	}
	else if (D3D11_SHVER_GET_TYPE(shaderDesc.Version) == D3D11_SHVER_GEOMETRY_SHADER) {
		com_ptr<ID3D11GeometryShader> gs;
		device->CreateGeometryShader(byteCode->GetBufferPointer(), byteCode->GetBufferSize(), nullptr, gs.GetAddressOf());
		shader = gs;
	}
}

uint Mesh::Shader::getResourceIndex(std::string name)
{
	D3D11_SHADER_INPUT_BIND_DESC bindDesc;
	if (S_OK == reflection->GetResourceBindingDescByName(name.c_str(), &bindDesc))
		return bindDesc.BindPoint;
	else
		return -1;
}