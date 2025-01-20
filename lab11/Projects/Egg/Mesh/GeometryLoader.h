#pragma once

#include "Mesh/Geometry.h"

namespace Egg { namespace Mesh
{
	class Geometry;

	class GeometryLoader
	{
		static Mesh::GeometryP createGeometryFromMemory(ID3D11Device* device, BYTE* data, unsigned int nBytes);

	public:
		static Mesh::GeometryP createGeometryFromFile(ID3D11Device* device, const char* filename);
	};

}} // namespace Egg::Mesh