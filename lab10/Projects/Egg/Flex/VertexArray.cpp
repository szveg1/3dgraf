#include "stdafx.h"
#include "VertexArray.h"

using namespace Egg::Flex;

void VertexArray::fillFrom(const VertexArray& o, unsigned int nVertices)
{
	if(format.pos.format != DXGI_FORMAT_UNKNOWN && o.format.pos.format != DXGI_FORMAT_UNKNOWN)
	{
		for(unsigned int i=0; i<nVertices; i++)
		{
			(*this)[i].pos() = o[i].pos();
		}
	}
	if(format.normal.format != DXGI_FORMAT_UNKNOWN && o.format.normal.format != DXGI_FORMAT_UNKNOWN)
	{
		for(unsigned int i=0; i<nVertices; i++)
		{
			(*this)[i].normal() = o[i].normal();
		}
	}
	if(format.tangent.format != DXGI_FORMAT_UNKNOWN && o.format.tangent.format != DXGI_FORMAT_UNKNOWN)
	{
		for(unsigned int i=0; i<nVertices; i++)
		{
			(*this)[i].tangent() = o[i].tangent();
		}
	}
	if(format.binormal.format != DXGI_FORMAT_UNKNOWN && o.format.binormal.format != DXGI_FORMAT_UNKNOWN)
	{
		for(unsigned int i=0; i<nVertices; i++)
		{
			(*this)[i].binormal() = o[i].binormal();
		}
	}
//	if(format.diffuse.format != DXGI_FORMAT_UNKNOWN && o.format.diffuse.format != DXGI_FORMAT_UNKNOWN)
//	{
//		for(unsigned int i=0; i<nVertices; i++)
//		{
//			(*this)[i].diffuse() = o[i].diffuse();
//		}
//	}
//	if(format.specular.format != DXGI_FORMAT_UNKNOWN && o.format.specular.format != DXGI_FORMAT_UNKNOWN)
//	{
//		for(unsigned int i=0; i<nVertices; i++)
//		{
//			(*this)[i].specular() = o[i].specular();
//		}
//	}
	for(unsigned int iTex=0; iTex<16; iTex++)
	{
		if(format.tex[iTex].format != DXGI_FORMAT_UNKNOWN && o.format.tex[iTex].format != DXGI_FORMAT_UNKNOWN)
		{
			for(unsigned int i=0; i<nVertices; i++)
			{
				(*this)[i].tex(iTex) = o[i].tex(iTex);
			}
		}
	}

	Format::ElementMap::iterator iElement = format.elementMap.begin();
	Format::ElementMap::iterator eElement = format.elementMap.end();
	while(iElement != eElement)
	{
		if(o.format.hasElement(iElement->first))
		{
			if(iElement->second.getByteWidth() == sizeof(float) * 4)
				for(unsigned int i=0; i<nVertices; i++)
					(*this)[i].float4(iElement->first) = o[i].float4(iElement->first);
			if(iElement->second.getByteWidth() == sizeof(float) * 3)
				for(unsigned int i=0; i<nVertices; i++)
					(*this)[i].float3(iElement->first) = o[i].float3(iElement->first);
			if(iElement->second.getByteWidth() == sizeof(float) * 2)
				for(unsigned int i=0; i<nVertices; i++)
					(*this)[i].float2(iElement->first) = o[i].float2(iElement->first);
			if(iElement->second.getByteWidth() == sizeof(float))
				for(unsigned int i=0; i<nVertices; i++)
					(*this)[i].float1(iElement->first) = o[i].float1(iElement->first);
		}
		iElement++;
	}
}
