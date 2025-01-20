#include "trafo.hlsli"

[maxvertexcount(3)]
void gsAdjacencyTest(
	triangleadj VsosTrafo input[6] : SV_POSITION,
	inout TriangleStream< VsosTrafo > output
)
{
	for (uint i = 0; i < 3; i++)
	{
		VsosTrafo  element;
//		element = input[i*2];
		element = input[i*2+1];
		output.Append(element);
	}
}