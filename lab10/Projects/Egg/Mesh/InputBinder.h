#pragma once
#include <vector>
#include "Geometry.h"

namespace Egg { namespace Mesh {

	GG_CLASS(InputBinder)
		class InputConfiguration
		{
			friend class Egg::Mesh::InputBinder;

			const D3D11_INPUT_ELEMENT_DESC* elements;
			unsigned int nElements;
			com_ptr<ID3DBlob> byteCode;

			com_ptr<ID3D11InputLayout> inputLayout;

			InputConfiguration(com_ptr<ID3DBlob> byteCode, GeometryP geometry);

			/// Returns true if input signatures are identical and elements with shared semantics are also identical.
			bool isCompatible(const InputConfiguration& other) const;

			HRESULT createInputLayout(com_ptr<ID3D11Device> device);
		public:
			~InputConfiguration();
		};

		using InputConfigurationList = std::vector<InputConfiguration*>;
		InputConfigurationList inputConfigurationList;

		com_ptr<ID3D11Device> device;
	protected:
		InputBinder(com_ptr<ID3D11Device> device);
	public:

		~InputBinder();

		com_ptr<ID3D11InputLayout> getCompatibleInputLayout(com_ptr<ID3DBlob> byteCode, GeometryP geometry);

	GG_ENDCLASS

}} // namespace Egg::Mesh