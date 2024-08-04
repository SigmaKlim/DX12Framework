cbuffer transform : register(b0)
{
    float4x4 wvp;
}

struct VsOut
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

VsOut main(float3 inPos : POSITION, float4 inColor : COLOR)
{
    VsOut output;
    
    output.pos = mul(float4(inPos.x, inPos.y, inPos.z, 1.0f), wvp);
    output.color = inColor;
	return output;
}