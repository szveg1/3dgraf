#include "stdafx.h"
#include "MeshBuilder.h"
#include "Vertex.h"
#include <map>
#include "ThrowOnFail.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postProcess.h> // Post processing flags
#include "App/stdConversions.h"
#include "Mesh/D3DEnumOps.h"

#define GBS_FORMAT_VERSION 0x1;
#define GBS_BLOCK_D3D11_VERTEX_BUFFER 0x004e77e3;
#define GBS_BLOCK_D3D11_INDEX_BUFFER 0x00010de3;

using namespace Egg::Flex;
using namespace Egg::Mesh;

void MeshBuilder::CreaseDirsAlgorithm::onStart(MeshBuilder& meshBuilder)
{
	using namespace Egg::Math;

	Format format = meshBuilder.vertices.getFormat();
	format.appendElement("CREASEIN", DXGI_FORMAT_R32G32B32A32_FLOAT);
	format.appendElement("CREASEOUT", DXGI_FORMAT_R32G32B32A32_FLOAT);
	meshBuilder.reformat(format);

	creaseInDirs = new float3[meshBuilder.nVertices];
	creaseOutDirs = new float3[meshBuilder.nVertices];
	memset(creaseInDirs, 0, sizeof(float3) * meshBuilder.nVertices);
	memset(creaseOutDirs, 0, sizeof(float3) * meshBuilder.nVertices);
}

void MeshBuilder::CreaseDirsAlgorithm::onCreaseFound(MeshBuilder& meshBuilder, unsigned int a, unsigned int b, unsigned int na, unsigned int nb, unsigned int tri, unsigned int ntri)
{
	using namespace Egg::Math;

	float3 edgeDir = meshBuilder.vertices[b].pos() - meshBuilder.vertices[a].pos();
	edgeDir = edgeDir.normalize();
	creaseOutDirs[a] = edgeDir;
	creaseInDirs[b] = edgeDir;
}

void MeshBuilder::CreaseDirsAlgorithm::onEnd(MeshBuilder& meshBuilder)
{
	using namespace Egg::Math;

	for(unsigned int iVertex=0; iVertex<meshBuilder.nVertices; iVertex++)
	{
		bool c0in = creaseInDirs[iVertex].lengthSquared() < 0.1;
		bool c0out = creaseOutDirs[iVertex].lengthSquared() < 0.1;
		if( c0in || c0out )
		{
			meshBuilder.vertices[iVertex].float4("CREASEIN") = float4(0, 0, 1, 10000.1);
			meshBuilder.vertices[iVertex].float4("CREASEOUT") = float4(0, 0, 1, 10000.1);
		}
		else
		{
			float3 crd = creaseOutDirs[iVertex] - creaseInDirs[iVertex];
			crd = crd.normalize();
			meshBuilder.vertices[iVertex].float3("CREASEIN") = -creaseInDirs[iVertex];
			meshBuilder.vertices[iVertex].float3("CREASEOUT") = creaseOutDirs[iVertex];
			meshBuilder.vertices[iVertex].float4("CREASEIN").w = 0.0f;
			meshBuilder.vertices[iVertex].float4("CREASEOUT").w = 0.0f;

			float3 vericross;
		}
	}
	delete creaseInDirs;
	delete creaseOutDirs;
}


void MeshBuilder::CreaseLoopsAlgorithm::onCreaseFound(MeshBuilder& meshBuilder, unsigned int a, unsigned int b, unsigned int na, unsigned int nb, unsigned int tri, unsigned int ntri)
{
	halfedgeStartToEnd[a].end = b;
	halfedgeStartToEnd[a].tri = tri;
	halfedgeStartToEnd[na].end = nb;
	halfedgeStartToEnd[na].tri = ntri;
}

void MeshBuilder::CreaseLoopsAlgorithm::onEnd(MeshBuilder& meshBuilder)
{
	unsigned int nStrips = 0;
	while( !halfedgeStartToEnd.empty() )
	{
		nStrips++;
		HalfedgeStartToEnd::iterator i;
		unsigned int loopStart = halfedgeStartToEnd.begin()->first;
		unsigned int lead = loopStart;
		unsigned int p3 = lead;
		i = halfedgeStartToEnd.find(lead);
		if(i == halfedgeStartToEnd.end() )
		{
			halfedgeStartToEnd.erase(halfedgeStartToEnd.begin());
			continue;
		}
		lead = i->second.end;
		unsigned int p2 = lead;
		i = halfedgeStartToEnd.find(lead);
		if(i == halfedgeStartToEnd.end() )
		{
			halfedgeStartToEnd.erase(halfedgeStartToEnd.begin());
			continue;
		}
		lead = i->second.end;
		unsigned int lTri = i->second.tri;
		unsigned int p1 = lead;
		i = halfedgeStartToEnd.find(lead);
		if(i == halfedgeStartToEnd.end() )
		{
			halfedgeStartToEnd.erase(halfedgeStartToEnd.begin());
			continue;
		}
		unsigned int lTriNext = i->second.tri;
		lead = i->second.end;
		do
		{
			for(auto encoder : encoders)
				encoder->onHalfedgeTraced(meshBuilder, lTri, p3, p2, p1, lead);

			p3 = p2; p2 = p1; p1 = lead;
			lTri = lTriNext;
			i = halfedgeStartToEnd.find(lead);
			if(i == halfedgeStartToEnd.end() )
			{
				halfedgeStartToEnd.erase(halfedgeStartToEnd.begin());
				break;
			}
			lTriNext = i->second.tri;
			lead = i->second.end;
			halfedgeStartToEnd.erase(i);
		} while(p3 != loopStart);

		for(auto encoder : encoders)
			encoder->onCloseLoop(meshBuilder);

	}

	for(auto encoder : encoders)
		encoder->onEnd(meshBuilder);
}

void MeshBuilder::CreaseIndicesEncoder::onEnd(MeshBuilder& meshBuilder)
{
	creaseIndices.indexFormat = meshBuilder.indexFormat;
	creaseIndices.nPrimitives = creaseStrips.size()/4;
	creaseIndices.topology = D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;

	creaseIndices.nIndices = creaseStrips.size();

	if(meshBuilder.indexFormat == DXGI_FORMAT_R32_UINT)
	{
		unsigned int* creaseIndexData = new unsigned int[creaseStrips.size()];
		memcpy(creaseIndexData, &creaseStrips.at(0),  creaseStrips.size() * sizeof(unsigned int));
		creaseIndices.indexData = creaseIndexData;
	}
	else
	{
		unsigned short* creaseIndexData = new unsigned short[creaseStrips.size()];
		for(unsigned int u=0; u<creaseStrips.size(); u++)
		{
			if(creaseStrips.at(u) == (unsigned int)-1)
				creaseIndexData[u] = (unsigned short)-1;
			else
				creaseIndexData[u] = creaseStrips.at(u);
		}
		creaseIndices.indexData = creaseIndexData;
	}
}

void MeshBuilder::CreaseTrianglesEncoder::onEnd(MeshBuilder& meshBuilder)
{
	creaseTriangles = new unsigned int[creaseTriangleStrips.size()];
	memcpy(creaseTriangles, &creaseTriangleStrips[0], sizeof(unsigned int) * creaseTriangleStrips.size());
}
//		for(int iter=0; iter<0; iter++)
//		{
//			for(int fi=0; fi<nPrimitives; fi++)
//			{
//				unsigned short* indices = (unsigned short*)indexData;
//				for(int par1=0; par1<3; par1++)
//				{
//					using namespace Egg::Math;
//
//					float4 rCreaseData = vertices[ indices[fi*6 + par1 * 2] ].float4("CREASEDIFFERENCE");
//					if(rCreaseData.x < 10000)
//					{
//						float3 rPosition = vertices[ indices[fi*6 + par1 * 2] ].pos();
//						for(int par2=0; par2<3; par2++)
//						{
//							if(par2 == par1) continue;
//							float3 dPosition = vertices[ indices[fi*6 + par2 * 2] ].pos();
//							float dist = (rPosition - dPosition).length() * 1.0;
//							float4& dCreaseData = vertices[ indices[fi*6 + par2 * 2] ].float4("CREASEDIFFERENCE");
//							if(rCreaseData.x < dCreaseData.x)
//								dCreaseData.x = rCreaseData.x;
//	//						dCreaseData.y = std::min<float>(dCreaseData.y, rCreaseData.y + dist);
//						}
//					}
//				}	
//			}
//		}

void MeshBuilder::generateAdjacency(AdjacencyDesc& desc, AdjacencyAlgorithmVector algos)
{
	if(indexFormat == DXGI_FORMAT_R32_UINT)
		generateAdjacencyWithIndexFormat<unsigned int>(desc, algos);
	else
		generateAdjacencyWithIndexFormat<unsigned short>(desc, algos);

}

void MeshBuilder::forceStandardElementLayout()
{
	using namespace Egg::Math;
	Format format;
	format.appendElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	format.appendElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	format.appendElement("TEXCOORD0", DXGI_FORMAT_R32G32_FLOAT);

	this->reformat(format);
}

void MeshBuilder::CreaseIndicesEncoder::onHalfedgeTraced(MeshBuilder& meshBuilder, unsigned int lTri, unsigned int p3, unsigned int p2, unsigned int p1, unsigned int lead)
{
	creaseStrips.push_back(p3);
	creaseStrips.push_back(p2);
	creaseStrips.push_back(p1);
	creaseStrips.push_back(lead);
}

void MeshBuilder::CreaseTrianglesEncoder::onHalfedgeTraced(MeshBuilder& meshBuilder, unsigned int lTri, unsigned int p3, unsigned int p2, unsigned int p1, unsigned int lead)
{
	creaseTriangleStrips.push_back(lTri);
	creaseTriangleStrips.push_back(p3);
	creaseTriangleStrips.push_back(p2);
	creaseTriangleStrips.push_back(p1);
	creaseTriangleStrips.push_back(lead);
}

template<typename IndexType>
void MeshBuilder::generateAdjacencyWithIndexFormat(AdjacencyDesc& desc, MeshBuilder::AdjacencyAlgorithmVector algos)
{
	using namespace Egg::Math;

	for(auto algo : algos)
		algo->onStart(*this);

	//check if topology is trilist
	assert(topology == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	unsigned int* kd = new unsigned int[nVertices];
	for(int i=0; i<nVertices; i++)
		kd[i] = i;
	PointRepSorter sorter(vertices, desc.epsilon);
	sorter.build(kd, kd+nVertices-1, 0);
	
	unsigned int* lookup = new unsigned int[nVertices];

	PointRepFinder finder(vertices, kd, lookup, desc.epsilon);
	for(int qui=0; qui<nVertices; qui++)
	{
		finder.kdTrace(qui, nVertices);
	}
	delete kd;

	using Edge = std::pair<unsigned int, unsigned int>;
	struct Neighbor
	{
		unsigned int c;
		unsigned int adjIndexIndex;
		unsigned int a;
		unsigned int b;
		Neighbor(unsigned int c, unsigned int adjIndexIndex,  unsigned int a, unsigned int b)
			:c(c), adjIndexIndex(adjIndexIndex), a(a), b(b)
		{}
		Neighbor():c(0), adjIndexIndex(0), a(0), b(0){}
	};
	using EdgeToNeighbor = std::map<Edge, Neighbor>;
	EdgeToNeighbor edgeToNeighbor;

	desc.triangleAdjacencies = new unsigned int[nPrimitives*3];

	IndexType* rawIndexArray = (IndexType*)indexData;
	IndexType* adjIndexArray = new IndexType[nPrimitives*6];

	for(int fi=0; fi<nPrimitives; fi++)
	{
		IndexType a = adjIndexArray[fi*6 + 0] = rawIndexArray[fi*3 + 0];
		IndexType b = adjIndexArray[fi*6 + 2] = rawIndexArray[fi*3 + 1];
		IndexType c = adjIndexArray[fi*6 + 4] = rawIndexArray[fi*3 + 2];
		IndexType la = lookup[a];
		IndexType lb = lookup[b];
		IndexType lc = lookup[c];
		Edge edge1(la, lb);
		Edge edge1back(lb, la);
		EdgeToNeighbor::iterator ni = edgeToNeighbor.find(edge1);
		if(ni == edgeToNeighbor.end())
		{
			edgeToNeighbor[edge1back] = Neighbor(c, fi*6+1, a, b);
			adjIndexArray[fi*6 + 1] = 0;
			desc.triangleAdjacencies[fi*3] = -1;
		}
		else
		{
			bool isCrease = (a != ni->second.b || b != ni->second.a);
			if(isCrease)
			{
				for(auto algo : algos)
					algo->onCreaseFound(*this, a, b, ni->second.a, ni->second.b, fi, ni->second.adjIndexIndex / 6);
			}
			if(!isCrease || desc.transcendCreases)
			{
				desc.triangleAdjacencies[fi*3] = ni->second.adjIndexIndex / 6;
				desc.triangleAdjacencies[ni->second.adjIndexIndex / 2] = fi;
			}
			else
			{
				desc.triangleAdjacencies[fi*3] = -1;
				desc.triangleAdjacencies[ni->second.adjIndexIndex / 2] = -1;
			}

			adjIndexArray[fi*6 + 1] = ni->second.c;
			adjIndexArray[ni->second.adjIndexIndex] = c;
			edgeToNeighbor.erase(ni);
		}
		Edge edge2(lb, lc);
		Edge edge2back(lc, lb);
		ni = edgeToNeighbor.find(edge2);
		if(ni == edgeToNeighbor.end())
		{
			edgeToNeighbor[edge2back] = Neighbor(a, fi*6+3, b, c);
			adjIndexArray[fi*6 + 3] = 0;
			desc.triangleAdjacencies[fi*3+1] = -1;
		}
		else
		{
			bool isCrease = (b != ni->second.b || c != ni->second.a);
			if(isCrease)
			{
				for(auto algo : algos)
					algo->onCreaseFound(*this, b, c, ni->second.a, ni->second.b, fi, ni->second.adjIndexIndex / 6);
			}
			if(!isCrease || desc.transcendCreases)
			{
				desc.triangleAdjacencies[fi*3+1] = ni->second.adjIndexIndex / 6;
				desc.triangleAdjacencies[ni->second.adjIndexIndex / 2] = fi;
			}
			else
			{
				desc.triangleAdjacencies[fi*3+1] = -1;
				desc.triangleAdjacencies[ni->second.adjIndexIndex / 2] = -1;
			}
			adjIndexArray[fi*6 + 3] = ni->second.c;
			adjIndexArray[ni->second.adjIndexIndex] = a;
			edgeToNeighbor.erase(ni);
		}
		Edge edge3(lc, la);
		Edge edge3back(la, lc);
		ni = edgeToNeighbor.find(edge3);
		/*if(ni == edgeToNeighbor.end())
			ni = edgeToNeighbor.find(edge3back);*/
		if(ni == edgeToNeighbor.end())
		{
			edgeToNeighbor[edge3back] = Neighbor(b, fi*6+5, c, a);
			adjIndexArray[fi*6 + 5] = 0;
			desc.triangleAdjacencies[fi*3+2] = -1;
		}
		else
		{
			bool isCrease = (c != ni->second.b || a != ni->second.a);
			if(isCrease)
			{
				for(auto algo : algos)
					algo->onCreaseFound(*this, c, a, ni->second.a, ni->second.b, fi, ni->second.adjIndexIndex / 6);
			}
			if(!isCrease || desc.transcendCreases)
			{
				desc.triangleAdjacencies[fi*3+2] = ni->second.adjIndexIndex / 6;
				desc.triangleAdjacencies[ni->second.adjIndexIndex / 2] = fi;
			}
			else
			{
				desc.triangleAdjacencies[fi*3+2] = -1;
				desc.triangleAdjacencies[ni->second.adjIndexIndex / 2] = -1;
			}
			adjIndexArray[fi*6 + 5] = ni->second.c;
			adjIndexArray[ni->second.adjIndexIndex] = b;
			edgeToNeighbor.erase(ni);
		}
	}
	unsigned int left = edgeToNeighbor.size();
	indexData = adjIndexArray;
	topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
	nIndices = D3DEnumOps::getIndexCount(topology, nPrimitives);
	delete rawIndexArray;

	for(auto algo : algos)
		algo->onEnd(*this);

}

void MeshBuilder::PointRepSorter::build(unsigned int *lo0, unsigned int* hi0, unsigned char axis)
{
	if(lo0 >= hi0)
		return;
	this->axis = axis;
	sort(lo0, hi0);
	unsigned int* midPos =  lo0 + ( ((unsigned int)hi0 - (unsigned int)lo0) / sizeof(unsigned int) / 2);

	build(lo0, midPos-1, (axis+1)%3);
	build(midPos+1, hi0, (axis+1)%3);
}


void MeshBuilder::PointRepSorter::sort(unsigned int *lo0, unsigned int* hi0)
{
	if ( hi0 > lo0)
	{
		unsigned int* lo = lo0;
		unsigned int* hi = hi0;

         // establish partition element as the midpoint of the array.
		unsigned int* midPos =  
			lo0 + ( ((unsigned int)hi0 - (unsigned int)lo0) / sizeof(unsigned int) / 2);
		unsigned int midIndex = *midPos;

         // loop through the array until indices cross
		while( lo <= hi )
		{
            // find the first element that is greater than or equal to 
            // the partition element starting from the left Index.
			while( ( lo < hi0 ) && ( compare(*lo, midIndex)))
				++lo;

            // find an element that is smaller than or equal to 
            // the partition element starting from the right Index.
            while( ( hi > lo0 ) && ( compare(midIndex, *hi)))
               --hi;            
			// if the indices have not crossed, swap
            if( lo <= hi )
			{
				unsigned int temp = *lo;
				*lo = *hi;
				*hi = temp;
				++lo;
				--hi;
			}         
		}
        // If the right index has not reached the left side of array
        // must now sort the left partition.
        if( lo0 < hi )
            sort( lo0, hi);
		// If the left index has not reached the right side of array
        // must now sort the right partition.
		if( lo < hi0 )
            sort( lo, hi0);
	}
}

bool MeshBuilder::PointRepSorter::compare(unsigned int aIndex, unsigned int bIndex)
{
	Math::float3 a = vertices[aIndex].pos();
	Math::float3 b = vertices[bIndex].pos();

	return vertices[aIndex].pos()[axis] < vertices[bIndex].pos()[axis];
}

void MeshBuilder::PointRepFinder::kdTrace(unsigned int q, unsigned int nVertices)
{
	this->q=q;
	qvec = vertices[kd[q]].pos();
	trace(0, nVertices-1, 0);
}

void MeshBuilder::PointRepFinder::trace( int t0,  int t1, unsigned char axis)
{
	if(t0 > t1)
		return;
	int t = t0 + (t1 - t0) / 2;
	Math::float3 tvec = vertices[kd[t]].pos();
	Math::float3 diff = qvec - tvec;
	if(diff.length() < 0.0001)
	{
		lookup[kd[t]] = kd[q];
	}
	if(t0 == t1)
		return;
	if( diff[axis] < 0.0001)
		trace(t0, t-1, (axis+1)%3);
	if( diff[axis] > -0.0001)
		trace(t+1, t1, (axis+1)%3);
}

void MeshBuilder::reformat(Format& format)
{
	void* newVertexData = new char[format.getVertexStride() * nVertices];
	VertexArray newVertexArray(newVertexData, format);

	newVertexArray.fillFrom(vertices, nVertices);

	vertices = newVertexArray;

	delete vertexData;
	vertexData = newVertexData;
}

Indexed::P MeshBuilder::commitToDevice(ID3D11Device* device,
			D3D11_USAGE vertexBufferUsage, UINT vertexBufferBindFlags, UINT vertexBufferMiscFlags,
			D3D11_USAGE indexBufferUsage, UINT indexBufferBindFlags, UINT indexBufferMiscFlags)
{
	IndexBufferDesc indexBufferDesc;
	indexBufferDesc.indexData = indexData;
	indexBufferDesc.nIndices = nIndices;
	indexBufferDesc.nPrimitives = nPrimitives;
	indexBufferDesc.indexFormat = indexFormat;
	indexBufferDesc.topology = topology;
	indexBufferDesc.usage = indexBufferUsage;
	indexBufferDesc.bindFlags = indexBufferBindFlags;
	indexBufferDesc.miscFlags = indexBufferMiscFlags;

	VertexStreamDesc vertexStreamDesc;
	vertexStreamDesc.nVertices = nVertices;
	vertexStreamDesc.vertexData = vertexData;
	vertexStreamDesc.vertexStride = vertices.getVertexStride();
	vertexStreamDesc.usage = vertexBufferUsage;
	vertexStreamDesc.bindFlags = vertexBufferBindFlags;
	vertexStreamDesc.miscFlags = vertexBufferMiscFlags;
	vertexStreamDesc.nElements = 32;
	D3D11_INPUT_ELEMENT_DESC elementsArray[32];
	vertices.getFormat().toD3DInputElements(elementsArray, vertexStreamDesc.nElements);
	vertexStreamDesc.elements = elementsArray;

	std::vector<VertexStream::P> vertexStreams;
	vertexStreams.push_back( VertexStream::create(device, vertexStreamDesc) );

	return Indexed::create(device, indexBufferDesc, vertexStreams);
}

MeshBuilder::MeshBuilder(aiMesh* assMesh)
{
	Format format;

	if(assMesh->HasPositions())
	{
		format.appendElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	}
	if(assMesh->HasNormals())
	{
		format.appendElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	}
	if(assMesh->HasTangentsAndBitangents())
	{
		format.appendElement("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT);
		format.appendElement("BINORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	}
	if(assMesh->HasTextureCoords(0))
	{
		format.appendElement("TEXCOORD0", DXGI_FORMAT_R32G32_FLOAT);
	}
	if(assMesh->HasTextureCoords(1))
	{
		format.appendElement("TEXCOORD1", DXGI_FORMAT_R32G32B32_FLOAT);
	}

	nVertices = assMesh->mNumVertices;

	vertexData = new char[nVertices * format.getVertexStride()];

	vertices = VertexArray(vertexData, format);

	for(unsigned int iVertex=0; iVertex < assMesh->mNumVertices; iVertex++)
	{
		if(assMesh->HasPositions())
			memcpy( &vertices[iVertex].pos(), &assMesh->mVertices[iVertex], sizeof(float) * 3);
		if(assMesh->HasNormals())
			memcpy( &vertices[iVertex].normal(), &assMesh->mNormals[iVertex], sizeof(float) * 3);
		if(assMesh->HasTangentsAndBitangents())
		{
			memcpy( &vertices[iVertex].tangent(), &assMesh->mTangents[iVertex], sizeof(float) * 3);
			memcpy( &vertices[iVertex].binormal(), &assMesh->mBitangents[iVertex], sizeof(float) * 3);
		}
		if(assMesh->HasTextureCoords(0))
			memcpy( &vertices[iVertex].tex0(), &assMesh->mTextureCoords[0][iVertex], sizeof(float) * 2);
		if(assMesh->HasTextureCoords(1))
			memcpy( &vertices[iVertex].tex1(), &assMesh->mTextureCoords[1][iVertex], sizeof(float) * 3);
	}
	
	nPrimitives = assMesh->mNumFaces;
	indexFormat = (nVertices > USHRT_MAX)?DXGI_FORMAT_R32_UINT:DXGI_FORMAT_R16_UINT;
	topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	nIndices = nPrimitives * 3;

	if(indexFormat == DXGI_FORMAT_R32_UINT)
	{
		unsigned int* sysMemIndices = new unsigned int[nPrimitives*3];
		for(int iFace=0; iFace < nPrimitives; iFace++)
		{
			sysMemIndices[iFace * 3 + 0] = assMesh->mFaces[iFace].mIndices[0];
			sysMemIndices[iFace * 3 + 1] = assMesh->mFaces[iFace].mIndices[1];
			sysMemIndices[iFace * 3 + 2] = assMesh->mFaces[iFace].mIndices[2];
		}
		indexData = sysMemIndices;
	}
	else
	{
		unsigned short* sysMemIndices = new unsigned short[nPrimitives*3];
		for(int iFace=0; iFace < nPrimitives; iFace++)
		{
			sysMemIndices[iFace * 3 + 0] = assMesh->mFaces[iFace].mIndices[0];
			sysMemIndices[iFace * 3 + 1] = assMesh->mFaces[iFace].mIndices[1];
			sysMemIndices[iFace * 3 + 2] = assMesh->mFaces[iFace].mIndices[2];
		}
		indexData = sysMemIndices;
	}
}

MeshBuilder::~MeshBuilder()
{
	delete vertexData;
	delete indexData;
}


//TODO internal format	Indexed::P MeshBuilder::createIndexedMeshFromFile(ID3D11Device* device, std::wstring filename)
//TODO internal format	{
//TODO internal format		MeshBuilder mesh(filename);
//TODO internal format		return mesh.commitToDevice(device);
//TODO internal format	}


unsigned int MeshBuilder::getPerPrimitiveIndex(unsigned int lTri, unsigned int p2)
{
	if(indexFormat == DXGI_FORMAT_R32_UINT)
	{
		unsigned int* indices = (unsigned int*)indexData;
		if(indices[lTri*6] == p2)
			return 0;
		if(indices[lTri*6+2] == p2)
			return 1;
		if(indices[lTri*6+4] == p2)
			return 2;
		return -1;
	}
	else
	{
		unsigned short* indices = (unsigned short*)indexData;
		if(indices[lTri*6] == p2)
			return 0;
		if(indices[lTri*6+2] == p2)
			return 1;
		if(indices[lTri*6+4] == p2)
			return 2;
		return -1;
	}
}