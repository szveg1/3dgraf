#pragma once
#include "Mesh/Geometry.h"
#include "Mesh/VertexStream.h"

namespace Egg { namespace Mesh
{
	struct IndexBufferDesc
	{
		static const unsigned short defaultData[6];

		DXGI_FORMAT indexFormat;
		unsigned int nPrimitives;
		D3D11_PRIMITIVE_TOPOLOGY topology;
		unsigned int nIndices;
		void* indexData;
		D3D11_USAGE usage;
		UINT bindFlags;
		UINT cpuAccessFlags;
		UINT miscFlags;

		unsigned int getIndexStride()
		{
			return (indexFormat==DXGI_FORMAT_R16_UINT)?sizeof(unsigned short):sizeof(unsigned int);
		}

		IndexBufferDesc():indexFormat(DXGI_FORMAT_R16_UINT), nPrimitives(2), topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST), nIndices(0), indexData((void*)defaultData), usage(D3D11_USAGE_IMMUTABLE), bindFlags(D3D11_BIND_INDEX_BUFFER), cpuAccessFlags(0), miscFlags(0) {}
	};

	class VertexStream;

	GG_SUBCLASS( Indexed, Mesh::Geometry)
		com_ptr<ID3D11Buffer>* vertexBuffers;
		unsigned int* vertexStrides;
		unsigned int nVertexBuffers;

		const D3D11_INPUT_ELEMENT_DESC* elements;
		unsigned int nElements;

		com_ptr<ID3D11Buffer> indexBuffer;
	
		DXGI_FORMAT indexFormat;
		unsigned int nPrimitives;
		unsigned int nIndices;

		D3D11_PRIMITIVE_TOPOLOGY topology;

		Indexed(com_ptr<ID3D11Device> device, IndexBufferDesc& desc, VertexStream::A& vertexStreams);
	public:
		static std::shared_ptr<Indexed> createFromSingleStream(com_ptr<ID3D11Device> device, IndexBufferDesc& desc, VertexStreamP vertexStream) { VertexStream::A arr(1); arr.at(0)=vertexStream; return IndexedP(new Indexed(device, desc, arr));}
		static std::shared_ptr<Indexed> createQuad(com_ptr<ID3D11Device> device) { return Indexed::createFromSingleStream(device, IndexBufferDesc(), VertexStream::create(device, VertexStreamDesc()) );}

		~Indexed(void);

		void getElements(const D3D11_INPUT_ELEMENT_DESC*& elements, unsigned int& nElements);

		void draw(com_ptr<ID3D11DeviceContext> context);

		unsigned int getVertexBufferCount() {return nVertexBuffers;}
		com_ptr<ID3D11Buffer>* getVertexBuffers() {return vertexBuffers;}
		com_ptr<ID3D11Buffer> getPrimaryBuffer(){return vertexBuffers[0];}
		unsigned int* getVertexStrides() {return vertexStrides;}

		unsigned int getPrimitiveCount() {return nPrimitives;}
		DXGI_FORMAT getIndexFormat() {return indexFormat;}
		D3D11_PRIMITIVE_TOPOLOGY getTopology() {return topology;}
		com_ptr<ID3D11Buffer> getIndexBuffer() {return indexBuffer;}

		unsigned int getIndexCount() { return nIndices; }

	GG_ENDCLASS

}} // namespace Egg::Mesh