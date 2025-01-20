#pragma once
#include <map>
#include <string>

namespace Egg { namespace Flex {

	struct Element
	{
		DXGI_FORMAT format;
		unsigned int byteOffset;
		Element(unsigned int byteOffset, DXGI_FORMAT format):byteOffset(byteOffset), format(format){}
		Element():byteOffset(0), format(DXGI_FORMAT_UNKNOWN){}

		unsigned int getByteWidth() const;
	};

}} // namespace Egg::Flex
