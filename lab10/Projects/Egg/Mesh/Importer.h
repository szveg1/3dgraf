#pragma once

#include "Mesh/Indexed.h"
#include "Mesh/Multi.h"
#include "Mesh/InputBinder.h"
#include "Mesh/Mien.h"

struct aiMesh;

namespace Egg { namespace Mesh
{
	class Geometry;

	class Importer
	{
	public:
		static Mesh::IndexedP fromAiMesh(com_ptr<ID3D11Device> device, aiMesh* assMesh);
	};

}} // namespace Egg::Mesh