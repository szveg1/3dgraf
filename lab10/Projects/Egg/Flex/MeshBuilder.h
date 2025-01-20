#pragma once

#include "Flex/VertexArray.h"
#include "Mesh/Indexed.h"

struct aiMesh;

namespace Egg { namespace Flex {

	GG_CLASS(MeshBuilder)
		void* vertexData;
		VertexArray vertices;
		unsigned int nVertices;

		DXGI_FORMAT indexFormat;
		unsigned int nPrimitives;
		D3D11_PRIMITIVE_TOPOLOGY topology;
		unsigned int nIndices;
		void* indexData;

		// adjacency computation
		struct PointRepSorter
		{
			PointRepSorter(VertexArray& vertices, float epsilon = 0.00001f):vertices(vertices), epsilon(epsilon), axis(0){}
			VertexArray& vertices;
			float epsilon;
			unsigned char axis;
			void build(unsigned int *lo0, unsigned int* hi0, unsigned char axis);
			void sort(unsigned int *lo0, unsigned int* hi0);
			bool compare(unsigned int aIndex, unsigned int bIndex);
		};
		struct PointRepFinder
		{
			PointRepFinder(VertexArray& vertices, unsigned int* kd, unsigned int* lookup, float epsilon = 0.00001f):vertices(vertices), epsilon(epsilon), kd(kd), lookup(lookup){}
			unsigned int* kd;
			unsigned int* lookup;
			VertexArray& vertices;
			float epsilon;
			unsigned int q;
			Math::float3 qvec;
			void kdTrace(unsigned int q, unsigned int nVertices);
			void trace( int t0,  int t1, unsigned char axis);
		};

		//TODO internal format loading removed. could be useful for expensive preprocessing?		void fromBinary(BYTE* binaryData, unsigned int nBytes);

		MeshBuilder(Format& format, unsigned int nVertices, void* vertexData, unsigned int nPrimitives, unsigned int nIndices, void* indexData, D3D11_PRIMITIVE_TOPOLOGY topology);
		MeshBuilder(aiMesh* assMesh);
		//TODO internal format loading removed. could be useful for expensive preprocessing?		MeshBuilder(std::wstring filename);

	public:
		using P = std::shared_ptr<MeshBuilder>;
		static MeshBuilder::P create(Format& format, unsigned int nVertices, void* vertexData, unsigned int nPrimitives, unsigned int nIndices, void* indexData, D3D11_PRIMITIVE_TOPOLOGY topology) { return MeshBuilder::P(new MeshBuilder(format, nVertices, vertexData, nPrimitives, nIndices, indexData, topology));}
		//TODO internal format loading removed. could be useful for expensive preprocessing?		static MeshBuilder::P create(std::wstring filename) { return MeshBuilder::P(new MeshBuilder(filename));}
		static MeshBuilder::P create(aiMesh* assMesh) { return MeshBuilder::P(new MeshBuilder(assMesh));}

		~MeshBuilder();

		unsigned int getVertexCount(){return nVertices;}
		unsigned int getPrimitiveCount(){return nPrimitives;}

		void reformat(Format& format);

		struct AdjacencyDesc
		{
		public:
			float epsilon;
			bool transcendCreases;
			unsigned int* triangleAdjacencies;
			AdjacencyDesc() { epsilon = 0.00001f; transcendCreases = true; triangleAdjacencies = nullptr;}
		};

		class AdjacencyAlgorithm
		{
		public:
			virtual void onStart(MeshBuilder& meshBuilder){}
			virtual void onCreaseFound(MeshBuilder& meshBuilder, unsigned int a, unsigned int b, unsigned int na, unsigned int nb, unsigned int tri, unsigned int ntri){}
			virtual void onEnd(MeshBuilder& meshBuilder){}
		};
		using AdjacencyAlgorithmVector = std::vector<AdjacencyAlgorithm*>;

		class CreaseDirsAlgorithm : public AdjacencyAlgorithm
		{
		public:
			Egg::Math::float3* creaseInDirs;
			Egg::Math::float3* creaseOutDirs;
			void onStart(MeshBuilder& meshBuilder);
			void onCreaseFound(MeshBuilder& meshBuilder, unsigned int a, unsigned int b, unsigned int na, unsigned int nb, unsigned int tri, unsigned int ntri);
			void onEnd(MeshBuilder& meshBuilder);
		};

		class CreaseLoopsAlgorithm : public AdjacencyAlgorithm
		{
			struct HalfEdgeTriEnd{
				unsigned int tri;
				unsigned int end;
			};
			using HalfedgeStartToEnd = std::map<unsigned int, HalfEdgeTriEnd>;
			HalfedgeStartToEnd halfedgeStartToEnd;
		public:
			void onCreaseFound(MeshBuilder& meshBuilder, unsigned int a, unsigned int b, unsigned int na, unsigned int nb, unsigned int tri, unsigned int ntri);
			void onEnd(MeshBuilder& meshBuilder);

			class HalfedgeEncoder {
			public:
				virtual void onHalfedgeTraced(MeshBuilder& meshBuilder, unsigned int lTri, unsigned int p3, unsigned int p2, unsigned int p1, unsigned int lead)=0;
				virtual void onEnd(MeshBuilder& meshBuilder)=0;
				virtual void onCloseLoop(MeshBuilder& meshBuilder){}
			};
			using HalfedgeEncoderVector = std::vector<HalfedgeEncoder*>;
			HalfedgeEncoderVector encoders;
			void addEncoder(HalfedgeEncoder& encoder){ encoders.push_back(&encoder);}

		};


		class CreaseIndicesEncoder : public CreaseLoopsAlgorithm::HalfedgeEncoder
		{
			std::vector<unsigned int> creaseStrips;
		public:
			Egg::Mesh::IndexBufferDesc creaseIndices;
			void onHalfedgeTraced(MeshBuilder& meshBuilder, unsigned int lTri, unsigned int p3, unsigned int p2, unsigned int p1, unsigned int lead);
			void onEnd(MeshBuilder& meshBuilder);
		};

		class CreaseTrianglesEncoder : public CreaseLoopsAlgorithm::HalfedgeEncoder
		{
			std::vector<unsigned int> creaseTriangleStrips;
		public:
			unsigned int* creaseTriangles;
			CreaseTrianglesEncoder(){ creaseTriangles = nullptr; }

			void onHalfedgeTraced(MeshBuilder& meshBuilder, unsigned int lTri, unsigned int p3, unsigned int p2, unsigned int p1, unsigned int lead);
			void onEnd(MeshBuilder& meshBuilder);
		};

	private:
		template<typename IndexType>
		void generateAdjacencyWithIndexFormat(AdjacencyDesc& desc, AdjacencyAlgorithmVector algos);
	public:
		void generateAdjacency(AdjacencyDesc& desc, AdjacencyAlgorithmVector algos);

		void forceStandardElementLayout();

		//TODO internal format saving removed. could be useful for expensive preprocessing?		void save(std::wstring filename);

		Egg::Mesh::Indexed::P commitToDevice(ID3D11Device* device, 
			D3D11_USAGE vertexBufferUsage=D3D11_USAGE_IMMUTABLE, UINT vertexBufferBindFlags = D3D11_BIND_VERTEX_BUFFER, UINT vertexBufferMiscFlags = 0,
			D3D11_USAGE indexBufferUsage=D3D11_USAGE_IMMUTABLE, UINT indexBufferBindFlags = D3D11_BIND_INDEX_BUFFER, UINT indexBufferMiscFlags = 0);

//TODO internal format		static Mesh::Indexed::P createIndexedMeshFromFile(ID3D11Device* device, std::wstring filename);

		const VertexArray& getVertices() const {return vertices;}
		unsigned int getPerPrimitiveIndex(unsigned int lTri, unsigned int p2);
	GG_ENDCLASS

}} // namespace Egg::Flex
