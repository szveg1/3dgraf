#pragma once
#include "Mesh/Shaded.h"
#include "Mesh/Mien.h"

namespace Egg { namespace Mesh
{
	GG_CLASS(Flip)
		typedef std::map<Mien, ShadedP> MienShadedMap;
		MienShadedMap mienShadedMap;

	protected:
		Flip();
	public:

		~Flip(void);

		void add(Mien mien, ShadedP shaded);

		ShadedP& getShaded(Mien mien)
		{
			return mienShadedMap[mien];
		}

		void draw(com_ptr<ID3D11DeviceContext> context, Mien mien);
	GG_ENDCLASS

}} // namespace Egg::Mesh
