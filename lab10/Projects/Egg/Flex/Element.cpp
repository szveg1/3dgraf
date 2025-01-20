#include "stdafx.h"
#include "Element.h"
#include "Mesh/D3DEnumOps.h"

using namespace Egg::Flex;

unsigned int Element::getByteWidth() const
{
	return Egg::Mesh::D3DEnumOps::getFormatByteWidth(format);
}
