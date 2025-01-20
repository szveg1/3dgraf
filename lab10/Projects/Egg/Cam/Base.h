#pragma once
#include "Math/math.h"

namespace Egg { namespace Cam {

	/// Basic camera interface, to be implemented by camera type classes.
	GG_CLASS(Base)
	public:

		/// Returns eye position.
		virtual const Egg::Math::float3& getEyePosition()=0;
		/// Returns the ahead vector.
		virtual const Egg::Math::float3& getAhead()=0;
		/// Returns the ndc-to-world-view-direction matrix to be used in shaders.
		virtual const Egg::Math::float4x4& getRayDirMatrix()=0;
		/// Returns view matrix to be used in shaders.
		virtual const Egg::Math::float4x4& getViewMatrix()=0;
		/// Returns projection matrix to be used in shaders.
		virtual const Egg::Math::float4x4& getProjMatrix()=0;

		/// Moves camera. To be implemented if the camera has its own animation mechanism.
		virtual void animate(float dt){}

		virtual void processMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){}

		virtual void setAspect(float aspect)=0;
	GG_ENDCLASS

}}