#pragma once
#include <map>
#include <string>
#include "Flex/Element.h"

namespace Egg { namespace Flex {

	class Format
	{
		friend class Vertex;
		friend class VertexAccessor;
		friend class VertexArray;

		Element pos;
		Element normal;
		Element tangent;
		Element binormal;
		Element diffuse;
		Element specular;
		Element tex[16];
		unsigned int nTex;
		using ElementMap = std::map<std::string, Element>;
		ElementMap elementMap;

		unsigned int nElements;
		unsigned int vertexStride;

	public:
		Format();

		bool hasElement(std::string s) const
		{
			ElementMap::const_iterator i = elementMap.find(s);
			return i != elementMap.end();
		}

		Element operator[](std::string s)
		{
			return elementMap[s];
		}

		Element operator[](std::string s) const
		{
			ElementMap::const_iterator i = elementMap.find(s);
			return i->second;
		}


		void fromD3DInputElements(const D3D11_INPUT_ELEMENT_DESC* elements, unsigned int nElements, unsigned int vertexStride=0);
		void toD3DInputElements(D3D11_INPUT_ELEMENT_DESC* elements, unsigned int& nElements);

		void appendElement(std::string name, DXGI_FORMAT format);

		unsigned int getVertexStride() {return vertexStride;}

		void setVertexStride(unsigned int vertexStride) { this->vertexStride = vertexStride;}
	};

}} // namespace Egg::Flex
