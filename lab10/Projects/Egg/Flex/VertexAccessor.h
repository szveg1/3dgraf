#pragma once
#include <map>
#include <string>

#include "Flex/Vertex.h"

namespace Egg { namespace Flex {

	class ConstVertexAccessor
	{
		friend class VertexAccessor;

		const Format& format;
		const Vertex& vertex;

	public:
		ConstVertexAccessor(const Format& format, const Vertex& vertex):format(format), vertex(vertex){}

		float float1(std::string elementName) const
		{
			return vertex.float1(elementName, format);
		}

		Math::float2 float2(std::string elementName) const
		{
			return vertex.float2(elementName, format);	
		}

		Math::float3 float3(std::string elementName) const
		{
			return vertex.float3(elementName, format);	
		}

		Math::float4 float4(std::string elementName) const
		{
			return vertex.float4(elementName, format);	
		}

		Math::float3 pos() const
		{
			return vertex.pos(format);	
		}

		Math::float3 normal() const
		{
			return vertex.normal(format);	
		}

		Math::float3 tangent() const
		{
			return vertex.tangent(format);	
		}

		Math::float3 binormal() const
		{
			return vertex.binormal(format);	
		}

		Math::float4 tangent4() const
		{
			return vertex.tangent4(format);	
		}

		Math::float4 binormal4() const
		{
			return vertex.binormal4(format);	
		}

		Math::float2 tex0() const
		{
			return vertex.tex0(format);
		}

		Math::float2 tex1() const
		{
			return vertex.tex1(format);
		}


		Math::float2 tex(unsigned int i) const
		{
			return vertex.tex(i, format);
		}

//		D3DCOLOR diffuse() const
//		{
//			return vertex.diffuse(format);
//		}
//
//		D3DCOLOR specular() const
//		{
//			return vertex.specular(format);
//		}

	};

	class VertexAccessor
	{
		Format& format;
		Vertex& vertex;

	public:
		VertexAccessor(Format& format, Vertex& vertex):format(format), vertex(vertex){}

		float& float1(std::string elementName)
		{
			return vertex.float1(elementName, format);
		}

		float float1(std::string elementName) const
		{
			return vertex.float1(elementName, format);
		}

		Math::float2& float2(std::string elementName)
		{
			return vertex.float2(elementName, format);	
		}

		Math::float2 float2(std::string elementName) const
		{
			return vertex.float2(elementName, format);	
		}

		Math::float3& float3(std::string elementName)
		{
			return vertex.float3(elementName, format);	
		}

		Math::float3 float3(std::string elementName) const
		{
			return vertex.float3(elementName, format);	
		}

		Math::float4& float4(std::string elementName)
		{
			return vertex.float4(elementName, format);	
		}

		Math::float4 float4(std::string elementName) const
		{
			return vertex.float4(elementName, format);	
		}

		Math::float3& pos()
		{
			return vertex.pos(format);	
		}

		Math::float3 pos() const
		{
			return vertex.pos(format);	
		}

		Math::float3& normal()
		{
			return vertex.normal(format);	
		}

		Math::float3 normal() const
		{
			return vertex.normal(format);	
		}

		Math::float3& tangent()
		{
			return vertex.tangent(format);	
		}

		Math::float3 tangent() const
		{
			return vertex.tangent(format);	
		}

		Math::float3& binormal()
		{
			return vertex.binormal(format);	
		}

		Math::float3 binormal() const
		{
			return vertex.binormal(format);	
		}

		Math::float4& tangent4()
		{
			return vertex.tangent4(format);	
		}

		Math::float4 tangent4() const
		{
			return vertex.tangent4(format);	
		}

		Math::float4& binormal4()
		{
			return vertex.binormal4(format);	
		}

		Math::float4 binormal4() const
		{
			return vertex.binormal4(format);	
		}

		Math::float2& tex0()
		{
			return vertex.tex0(format);
		}

		Math::float2 tex0() const
		{
			return vertex.tex0(format);
		}

		Math::float2& tex1()
		{
			return vertex.tex1(format);
		}

		Math::float2 tex1() const
		{
			return vertex.tex1(format);
		}


		Math::float2& tex(unsigned int i)
		{
			return vertex.tex(i, format);
		}

		Math::float2 tex(unsigned int i) const
		{
			return vertex.tex(i, format);
		}

//		D3DCOLOR& diffuse()
//		{
//			return vertex.diffuse(format);
//		}
//
//		D3DCOLOR diffuse() const
//		{
//			return vertex.diffuse(format);
//		}
//
//		D3DCOLOR& specular()
//		{
//			return vertex.specular(format);
//		}
//
//		D3DCOLOR specular() const
//		{
//			return vertex.specular(format);
//		}

		void operator=(const VertexAccessor& co)
		{
			memcpy(
				this,
				&co.vertex,
				format.vertexStride);
		}

		void operator=(const ConstVertexAccessor& co)
		{
			memcpy(
				this,
				&co.vertex,
				format.vertexStride);
		}
	};

}} // namespace Egg::Flex
