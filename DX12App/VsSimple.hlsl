struct VsOut
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

VsOut main(float3 inPos : POSITION, float4 inColor : COLOR)
{
    VsOut output;
    output.pos = float4(inPos.x, inPos.y, inPos.z, 1.0f);
    output.color = inColor;
	return output;
}