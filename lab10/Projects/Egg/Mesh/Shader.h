#pragma once

#include <string>
#include <D3D11Shader.h>

namespace Egg { namespace Mesh {

	GG_CLASS(Shader)
		com_ptr<ID3D11ShaderReflection> reflection;
		com_ptr<ID3D11DeviceChild> shader;
		com_ptr<ID3DBlob> byteCode;

	protected:
		Shader(
			com_ptr<ID3D11Device> device,
			com_ptr<ID3DBlob> byteCode);
	public:

		com_ptr<ID3DBlob> getByteCode()
		{
			return byteCode;
		}

		template<class ID3D11ShaderType>
		com_ptr<ID3D11ShaderType> as()
		{
			com_ptr<ID3D11ShaderType> vs;
			if (shader.As(&vs) == S_OK)
				return vs;
			else
				return nullptr;
		}

		com_ptr<ID3D11ShaderReflection> getReflection()
		{
			return reflection;
		}

		uint getResourceIndex(std::string name);
	GG_ENDCLASS

}} // namespace Egg::Mesh
