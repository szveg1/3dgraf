#include "stdafx.h"
#include "Mesh/Flip.h"
#include "ThrowOnFail.h"

using namespace Egg;

Mesh::Flip::Flip()
{
}

Mesh::Flip::~Flip()
{
}

void Mesh::Flip::add(Mien mien, ShadedP shaded)
{
	MienShadedMap::iterator i = mienShadedMap.find(mien);
	if(i != mienShadedMap.end())
		mienShadedMap.erase(i);
	mienShadedMap[mien] = shaded;
}

void Mesh::Flip::draw(com_ptr<ID3D11DeviceContext> context, Mien mien)
{
	MienShadedMap::iterator i = mienShadedMap.find(mien);
	if(i != mienShadedMap.end())
	{
		i->second->draw(context);
	}
	else
	{
		// warning. could be intentional
	}
}