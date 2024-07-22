
struct PS_INPUT
{
    float4 Pos : SV_Position;
    float4 Color : COLOR0;
};

float4 PS(PS_INPUT input) : SV_Target
{
    return input.Color; // Yellow, with Alpha = 1
}

