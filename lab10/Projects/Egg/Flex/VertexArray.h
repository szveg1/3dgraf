#pragma once
#include <map>
#include <string>

#include "Flex/VertexAccessor.h"

namespace Egg { namespace Flex {

	//! an array of vertices with runtime layout
	class VertexArray
	{
		Format format;

		Vertex* arrayStart;

		void** getDataPointerReference()
		{
			return (void**)&arrayStart;
		}
	public:

		VertexAccessor operator[](unsigned int index)
		{
			return VertexAccessor(format,
				*((Vertex*)((char*)arrayStart + format.vertexStride * index)) );
		}

		ConstVertexAccessor operator[](unsigned int index) const
		{
			return ConstVertexAccessor(format,
				*((Vertex*)((char*)arrayStart + format.vertexStride * index)) );
		}


		VertexArray():arrayStart(NULL){}

		VertexArray(void* vertexData, Format& format)
		{
			*getDataPointerReference() = vertexData;
			this->format = format;
		}

		unsigned int getVertexStride()
		{
			return format.vertexStride;
		}

		Format& getFormat()
		{
			return format;
		}

		void fillFrom(const VertexArray& o, unsigned int nVertices);
	};

}} // namespace Egg::Flex
