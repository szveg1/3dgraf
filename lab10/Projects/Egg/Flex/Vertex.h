#pragma once
#include <map>
#include <string>
#include <cassert>

#include "Flex/Format.h"
#include "Math/math.h"

namespace Egg { namespace Flex {

	//! a class that represents a vertex with runtime layout
	class Vertex
	{
	public:
		float& float1(std::string elementName, const Format& format)
		{
			assert(
				format[elementName].format == DXGI_FORMAT_R32_FLOAT
				|| format[elementName].format == DXGI_FORMAT_R32G32_FLOAT
				|| format[elementName].format == DXGI_FORMAT_R32G32B32_FLOAT
				|| format[elementName].format == DXGI_FORMAT_R32G32B32A32_FLOAT);
			return *(float*)(((char*)this) + format[elementName].byteOffset);	
		}

		float float1(std::string elementName, const Format& format) const
		{
			return *(float*)(((char*)this) + format[elementName].byteOffset);	
		}

		Egg::Math::float2& float2(std::string elementName, const Format& format)
		{
			assert(format[elementName].format == DXGI_FORMAT_R32G32_FLOAT
				|| format[elementName].format == DXGI_FORMAT_R32G32B32_FLOAT
				|| format[elementName].format == DXGI_FORMAT_R32G32B32A32_FLOAT);
			return *(Egg::Math::float2*)(((char*)this) + format[elementName].byteOffset);	
		}

		Egg::Math::float2 float2(std::string elementName, const Format& format) const
		{
			const Element& element = format[elementName];
			Egg::Math::float2 ret(0, 0);
			for(unsigned int i=0; i<element.getByteWidth() / sizeof(float) && i<2; i++)
				ret[i] = *(float*)(((char*)this) + element.byteOffset + i * sizeof(float));
			return ret;
		}

		Egg::Math::float3& float3(std::string elementName, const Format& format)
		{
			assert(format[elementName].format == DXGI_FORMAT_R32G32B32_FLOAT 
				|| format[elementName].format == DXGI_FORMAT_R32G32B32A32_FLOAT );
			return *(Egg::Math::float3*)(((char*)this) + format[elementName].byteOffset);	
		}

		Egg::Math::float3 float3(std::string elementName, const Format& format) const
		{
			const Element& element = format[elementName];
			Egg::Math::float3 ret(0, 0, 0);
			for(unsigned int i=0; i<element.getByteWidth() / sizeof(float) && i<3; i++)
				ret[i] = *(float*)(((char*)this) + element.byteOffset + i * sizeof(float));
			return ret;
		}

		Egg::Math::float4& float4(std::string elementName, const Format& format)
		{
			assert(format[elementName].format == DXGI_FORMAT_R32G32B32A32_FLOAT);
			return *(Egg::Math::float4*)(((char*)this) + format[elementName].byteOffset);
		}
		Egg::Math::float4 float4(std::string elementName, const Format& format) const
		{
			const Element& element = format[elementName];
			Egg::Math::float4 ret(0, 0, 0, 1);
			for(unsigned int i=0; i<element.getByteWidth() / sizeof(float) && i < 4; i++)
				ret[i] = *(float*)(((char*)this) + element.byteOffset + i * sizeof(float));
			return ret;
		}
		Egg::Math::float3& pos(const Format& format)
		{
			assert(format.pos.format == DXGI_FORMAT_R32G32B32_FLOAT );
			return *(Egg::Math::float3*)(((char*)this) + format.pos.byteOffset);
		}
		Egg::Math::float3 pos(const Format& format) const
		{
			assert(format.pos.format == DXGI_FORMAT_R32G32B32_FLOAT );
			return *(Egg::Math::float3*)(((char*)this) + format.pos.byteOffset);
		}
		Egg::Math::float3& normal(const Format& format)
		{
			assert(format.normal.format == DXGI_FORMAT_R32G32B32_FLOAT );
			return *(Egg::Math::float3*)(((char*)this) + format.normal.byteOffset);
		}
		Egg::Math::float3 normal(const Format& format) const
		{
			assert(format.normal.format == DXGI_FORMAT_R32G32B32_FLOAT );
			return *(Egg::Math::float3*)(((char*)this) + format.normal.byteOffset);
		}

		Egg::Math::float3& tangent(const Format& format)
		{
			assert(format.tangent.format == DXGI_FORMAT_R32G32B32_FLOAT || format.tangent.format == DXGI_FORMAT_R32G32B32A32_FLOAT);
			return *(Egg::Math::float3*)(((char*)this) + format.tangent.byteOffset);
		}
		Egg::Math::float3 tangent(const Format& format) const
		{
			assert(format.tangent.format == DXGI_FORMAT_R32G32B32_FLOAT || format.tangent.format == DXGI_FORMAT_R32G32B32A32_FLOAT);
			return *(Egg::Math::float3*)(((char*)this) + format.tangent.byteOffset);
		}

		Egg::Math::float3& binormal(const Format& format)
		{
			assert(format.binormal.format == DXGI_FORMAT_R32G32B32_FLOAT || format.binormal.format == DXGI_FORMAT_R32G32B32A32_FLOAT);
			return *(Egg::Math::float3*)(((char*)this) + format.binormal.byteOffset);
		}
		Egg::Math::float3 binormal(const Format& format) const
		{
			assert(format.binormal.format == DXGI_FORMAT_R32G32B32_FLOAT || format.binormal.format == DXGI_FORMAT_R32G32B32A32_FLOAT);
			return *(Egg::Math::float3*)(((char*)this) + format.binormal.byteOffset);
		}

		Egg::Math::float4& tangent4(const Format& format)
		{
			assert(format.tangent.format == DXGI_FORMAT_R32G32B32A32_FLOAT);
			return *(Egg::Math::float4*)(((char*)this) + format.tangent.byteOffset);
		}
		Egg::Math::float4 tangent4(const Format& format) const
		{
			assert(format.tangent.format == DXGI_FORMAT_R32G32B32A32_FLOAT);
			return *(Egg::Math::float4*)(((char*)this) + format.tangent.byteOffset);
		}

		Egg::Math::float4& binormal4(const Format& format)
		{
			assert(format.binormal.format == DXGI_FORMAT_R32G32B32A32_FLOAT);
			return *(Egg::Math::float4*)(((char*)this) + format.binormal.byteOffset);
		}
		Egg::Math::float4 binormal4(const Format& format) const
		{
			assert(format.binormal.format == DXGI_FORMAT_R32G32B32A32_FLOAT);
			return *(Egg::Math::float4*)(((char*)this) + format.binormal.byteOffset);
		}

		Egg::Math::float2& tex0(const Format& format)
		{
			assert(format.tex[0].format == DXGI_FORMAT_R32G32_FLOAT
				|| format.tex[0].format == DXGI_FORMAT_R32G32B32_FLOAT
				|| format.tex[0].format == DXGI_FORMAT_R32G32B32A32_FLOAT);
			return *(Egg::Math::float2*)(((char*)this) + format.tex[0].byteOffset);
		}
		Egg::Math::float2 tex0(const Format& format) const
		{
			assert(format.tex[0].format == DXGI_FORMAT_R32G32_FLOAT
				|| format.tex[0].format == DXGI_FORMAT_R32G32B32_FLOAT
				|| format.tex[0].format == DXGI_FORMAT_R32G32B32A32_FLOAT);
			return *(Egg::Math::float2*)(((char*)this) + format.tex[0].byteOffset);
		}

		Egg::Math::float2& tex1(const Format& format)
		{
			assert(format.tex[1].format == DXGI_FORMAT_R32G32_FLOAT
				|| format.tex[1].format == DXGI_FORMAT_R32G32B32_FLOAT
				|| format.tex[1].format == DXGI_FORMAT_R32G32B32A32_FLOAT);

			return *(Egg::Math::float2*)(((char*)this) + format.tex[1].byteOffset);
		}
		Egg::Math::float2 tex1(const Format& format) const
		{
			assert(format.tex[1].format == DXGI_FORMAT_R32G32_FLOAT
				|| format.tex[1].format == DXGI_FORMAT_R32G32B32_FLOAT
				|| format.tex[1].format == DXGI_FORMAT_R32G32B32A32_FLOAT);

			return *(Egg::Math::float2*)(((char*)this) + format.tex[1].byteOffset);
		}

		Egg::Math::float2& tex(unsigned int i, const Format& format)
		{
			assert(format.tex[i].format == DXGI_FORMAT_R32G32_FLOAT
				|| format.tex[i].format == DXGI_FORMAT_R32G32B32_FLOAT
				|| format.tex[i].format == DXGI_FORMAT_R32G32B32A32_FLOAT);
			return *(Egg::Math::float2*)(((char*)this) + format.tex[i].byteOffset);
		}
		Egg::Math::float2 tex(unsigned int i, const Format& format) const
		{
			assert(format.tex[i].format == DXGI_FORMAT_R32G32_FLOAT
				|| format.tex[i].format == DXGI_FORMAT_R32G32B32_FLOAT
				|| format.tex[i].format == DXGI_FORMAT_R32G32B32A32_FLOAT);
			return *(Egg::Math::float2*)(((char*)this) + format.tex[i].byteOffset);
		}

//		D3DCOLOR& diffuse(const Format& format)
//		{
//			assert(format.diffuse.format == DXGI_FORMAT_R8G8B8A8_UNORM);
//			return *(D3DCOLOR*)(((char*)this) + format.diffuse.byteOffset);
//		}
//		D3DCOLOR diffuse(const Format& format) const
//		{
//			assert(format.diffuse.format == DXGI_FORMAT_R8G8B8A8_UNORM);
//			return *(D3DCOLOR*)(((char*)this) + format.diffuse.byteOffset);
//		}
//
//		D3DCOLOR& specular(const Format& format)
//		{
//			assert(format.specular.format == DXGI_FORMAT_R8G8B8A8_UNORM);
//			return *(D3DCOLOR*)(((char*)this) + format.specular.byteOffset);
//		}
//
//		D3DCOLOR specular(const Format& format) const
//		{
//			assert(format.specular.format == DXGI_FORMAT_R8G8B8A8_UNORM);
//			return *(D3DCOLOR*)(((char*)this) + format.specular.byteOffset);
//		}

	};

}} // namespace Egg::Flex
