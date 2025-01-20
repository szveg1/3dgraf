#pragma once
#include "Mesh/Geometry.h"
#include "Mesh/Material.h"

namespace Egg { namespace Mesh
{
	GG_CLASS(Shaded)
		GeometryP geometry;
		MaterialP material;
		com_ptr<ID3D11InputLayout> inputLayout;

	protected:
		Shaded(GeometryP geometry, MaterialP material, com_ptr<ID3D11InputLayout> inputLayout):geometry(geometry), material(material), inputLayout(inputLayout){}
	public:

		MaterialP getMaterial(){return material;}
		GeometryP getGeometry(){return geometry;}
		com_ptr<ID3D11InputLayout> getInputLayout(){return inputLayout;}

		void draw(com_ptr<ID3D11DeviceContext> context);
	GG_ENDCLASS

}} // namespace Egg::Mesh
