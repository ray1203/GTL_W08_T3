
struct VS_OUTPUT
{
    float4 PositionCS : SV_POSITION; // Clip space position
};

struct PS_OUTPUT
{
    float2 Moments : SV_Target0; // Output: float2(depth, depth^2)
};

PS_OUTPUT mainPS(VS_OUTPUT input)
{
    PS_OUTPUT output;

    // Calculate depth in the range [0, 1]
    float depth = input.PositionCS.z;
    
    // Output depth and depth squared
    output.Moments = float2(depth, depth * depth);

    return output;
}
