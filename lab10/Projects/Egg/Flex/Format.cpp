#include "stdafx.h"
#include "Flex/Vertex.h"
#include "app/stdConversions.h"
#include "ThrowOnFail.h"

using namespace Egg::Flex;

Format::Format():
	pos(0, DXGI_FORMAT_UNKNOWN),
	normal(12, DXGI_FORMAT_UNKNOWN),
	tangent(12, DXGI_FORMAT_UNKNOWN),
	binormal(12, DXGI_FORMAT_UNKNOWN),
	diffuse(24, DXGI_FORMAT_UNKNOWN),
	specular(28, DXGI_FORMAT_UNKNOWN),
	nTex(0),
	nElements(0),
	vertexStride(0)
{
	unsigned int ii[16] = {24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, 136, 144};
	for(int iu=0; iu<16; iu++)
	{
		tex[iu].byteOffset = ii[iu];
		tex[iu].format = DXGI_FORMAT_UNKNOWN;
	}
}

void Format::fromD3DInputElements(const D3D11_INPUT_ELEMENT_DESC* elements, unsigned int nElements, unsigned int vertexStride)
{
	this->vertexStride = vertexStride;
	unsigned int measuredVertexStride = 0;
	this->nTex = 0;
	this->nElements = nElements;
	for(unsigned int i=0; i< nElements; i++)
	{
		switch(elements[i].Format)
		{
			case DXGI_FORMAT_R32_SINT :
			case DXGI_FORMAT_R32_UINT :
			case DXGI_FORMAT_R32_FLOAT : measuredVertexStride += sizeof(float); break;
			case DXGI_FORMAT_R32G32_SINT:
			case DXGI_FORMAT_R32G32_UINT:
			case DXGI_FORMAT_R32G32_FLOAT : measuredVertexStride += 2 * sizeof(float); break;
			case DXGI_FORMAT_R32G32B32_SINT:
			case DXGI_FORMAT_R32G32B32_UINT:
			case DXGI_FORMAT_R32G32B32_FLOAT : measuredVertexStride += 3 * sizeof(float); break;
			case DXGI_FORMAT_R32G32B32A32_SINT:
			case DXGI_FORMAT_R32G32B32A32_UINT:
			case DXGI_FORMAT_R32G32B32A32_FLOAT : measuredVertexStride += 4 * sizeof(float); break;
		};

		this->elementMap[elements[i].SemanticName].byteOffset = elements[i].AlignedByteOffset;
		if(strcmp(elements[i].SemanticName, "POSITION") == 0)
		{
			this->pos.byteOffset = elements[i].AlignedByteOffset;
			this->pos.format = elements[i].Format;
		}
		if(strcmp(elements[i].SemanticName, "NORMAL") == 0)
		{
			this->normal.byteOffset = elements[i].AlignedByteOffset;
			this->normal.format = elements[i].Format;
		}
		if(strcmp(elements[i].SemanticName, "TANGENT") == 0)
		{
			this->tangent.byteOffset = elements[i].AlignedByteOffset;
			this->tangent.format = elements[i].Format;
		}
		if(strcmp(elements[i].SemanticName, "BINORMAL") == 0)
		{
			this->binormal.byteOffset = elements[i].AlignedByteOffset;
			this->binormal.format = elements[i].Format;
		}
		if(strcmp(elements[i].SemanticName, "COLOR") == 0 && elements[i].SemanticIndex == 0)
		{
			this->diffuse.byteOffset = elements[i].AlignedByteOffset;
			this->diffuse.format = elements[i].Format;
		}
		if(strcmp(elements[i].SemanticName, "COLOR") == 0 && elements[i].SemanticIndex == 1)
		{
			this->specular.byteOffset = elements[i].AlignedByteOffset;
			this->specular.format = elements[i].Format;
		}
		if(strcmp(elements[i].SemanticName, "TEXCOORD") == 0)
		{
			this->tex[elements[i].SemanticIndex].byteOffset = elements[i].AlignedByteOffset;
			if(elements[i].SemanticIndex >= this->nTex)
				this->nTex = elements[i].SemanticIndex+1;
		}
	}
	if(this->vertexStride == 0)
		this->vertexStride = measuredVertexStride;
}

void Format::toD3DInputElements(D3D11_INPUT_ELEMENT_DESC* elements, unsigned int& nElements)
{
	if (nElements < this->nElements)
		throw new Egg::HrException(E_FAIL, "Not enough elements.", __FILE__, __LINE__);
	nElements = this->nElements;
//	elements = boost::shared_array<D3D11_INPUT_ELEMENT_DESC>( new D3D11_INPUT_ELEMENT_DESC[nElements] );

	unsigned int iElement = 0;
	if(pos.format != DXGI_FORMAT_UNKNOWN)
	{
		elements[iElement].Format = pos.format;
		elements[iElement].InputSlot = 0;
		elements[iElement].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		elements[iElement].InstanceDataStepRate = 0;
		elements[iElement].SemanticIndex = 0;
		elements[iElement].AlignedByteOffset = pos.byteOffset;
		elements[iElement].SemanticName = "POSITION";
		iElement++;
	}
	if(normal.format != DXGI_FORMAT_UNKNOWN)
	{
		elements[iElement].Format = normal.format;
		elements[iElement].InputSlot = 0;
		elements[iElement].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		elements[iElement].InstanceDataStepRate = 0;
		elements[iElement].SemanticIndex = 0;
		elements[iElement].AlignedByteOffset = normal.byteOffset;
		elements[iElement].SemanticName = "NORMAL";
		iElement++;
	}
	if(tangent.format != DXGI_FORMAT_UNKNOWN)
	{
		elements[iElement].Format = tangent.format;
		elements[iElement].InputSlot = 0;
		elements[iElement].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		elements[iElement].InstanceDataStepRate = 0;
		elements[iElement].SemanticIndex = 0;
		elements[iElement].AlignedByteOffset = tangent.byteOffset;
		elements[iElement].SemanticName = "TANGENT";
		iElement++;
	}
	if(binormal.format != DXGI_FORMAT_UNKNOWN)
	{
		elements[iElement].Format = binormal.format;
		elements[iElement].InputSlot = 0;
		elements[iElement].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		elements[iElement].InstanceDataStepRate = 0;
		elements[iElement].SemanticIndex = 0;
		elements[iElement].AlignedByteOffset = binormal.byteOffset;
		elements[iElement].SemanticName = "BINORMAL";
		iElement++;
	}
	if(diffuse.format != DXGI_FORMAT_UNKNOWN)
	{
		elements[iElement].Format = diffuse.format;
		elements[iElement].InputSlot = 0;
		elements[iElement].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		elements[iElement].InstanceDataStepRate = 0;
		elements[iElement].SemanticIndex = 0;
		elements[iElement].AlignedByteOffset = diffuse.byteOffset;
		elements[iElement].SemanticName = "COLOR";
		iElement++;
	}
	if(specular.format != DXGI_FORMAT_UNKNOWN)
	{
		elements[iElement].Format = specular.format;
		elements[iElement].InputSlot = 0;
		elements[iElement].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		elements[iElement].InstanceDataStepRate = 0;
		elements[iElement].SemanticIndex = 1;
		elements[iElement].AlignedByteOffset = specular.byteOffset;
		elements[iElement].SemanticName = "COLOR";
		iElement++;
	}
	for(unsigned int iTex=0; iTex<nTex; iTex++)
		if(tex[iTex].format != DXGI_FORMAT_UNKNOWN)
		{
			elements[iElement].Format = tex[iTex].format;
			elements[iElement].InputSlot = 0;
			elements[iElement].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
			elements[iElement].InstanceDataStepRate = 0;
			elements[iElement].SemanticIndex = iTex;
			elements[iElement].AlignedByteOffset = tex[iTex].byteOffset;
			elements[iElement].SemanticName = "TEXCOORD";
			iElement++;
		}


	ElementMap::iterator imElement = elementMap.begin();
	ElementMap::iterator emElement = elementMap.end();
	while(imElement != emElement)
	{
		elements[iElement].Format = imElement->second.format;
		elements[iElement].InputSlot = 0;
		elements[iElement].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		elements[iElement].InstanceDataStepRate = 0;
		elements[iElement].SemanticIndex = 0;
		elements[iElement].AlignedByteOffset = imElement->second.byteOffset;
		elements[iElement].SemanticName = imElement->first.c_str();
		iElement++;

		imElement++;
	}

}

void Format::appendElement(std::string name, DXGI_FORMAT format)
{
	nElements++;

	if(name.compare("POSITION") == 0 || name.compare("POSITION0") == 0)
	{
		pos.byteOffset = vertexStride;
		pos.format = format;
	}
	else if(name.compare("NORMAL") == 0 || name.compare("NORMAL0") == 0)
	{
		normal.byteOffset = vertexStride;
		normal.format = format;
	}
	else if(name.compare("TANGENT") == 0 || name.compare("TANGENT0") == 0)
	{
		tangent.byteOffset = vertexStride;
		tangent.format = format;
	}
	else if(name.compare("BINORMAL") == 0 || name.compare("BINORMAL0") == 0)
	{
		binormal.byteOffset = vertexStride;
		binormal.format = format;
	}
	else if(name.compare("COLOR") == 0 || name.compare("COLOR0") == 0)
	{
		diffuse.byteOffset = vertexStride;
		diffuse.format = format;
	}
	else if(name.compare("COLOR1") == 0)
	{
		specular.byteOffset = vertexStride;
		specular.format = format;
	}
	else if(name.find("TEXCOORD") == 0)
	{
		unsigned int i = Egg::fromString<unsigned int>( name.substr(8, 12) );
		tex[i].byteOffset = vertexStride;
		tex[i].format = format;
		if(i+1>nTex)
			nTex=i+1;
	}
	else
		elementMap[name] = Element(vertexStride, format);

	switch(format)
	{
		case DXGI_FORMAT_R32_SINT :
		case DXGI_FORMAT_R32_UINT :
		case DXGI_FORMAT_R32_FLOAT : vertexStride += sizeof(float); break;
		case DXGI_FORMAT_R32G32_SINT:
		case DXGI_FORMAT_R32G32_UINT:
		case DXGI_FORMAT_R32G32_FLOAT : vertexStride += 2 * sizeof(float); break;
		case DXGI_FORMAT_R32G32B32_SINT:
		case DXGI_FORMAT_R32G32B32_UINT:
		case DXGI_FORMAT_R32G32B32_FLOAT : vertexStride += 3 * sizeof(float); break;
		case DXGI_FORMAT_R32G32B32A32_SINT:
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_FLOAT : vertexStride += 4 * sizeof(float); break;
	};

}
