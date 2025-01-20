#include "stdafx.h"
#include "Mesh/Shaded.h"

using namespace Egg;

void Mesh::Shaded::draw(com_ptr<ID3D11DeviceContext> context)
{
	material->apply(context);
	context->IASetInputLayout(inputLayout.Get());
	geometry->draw(context);
}