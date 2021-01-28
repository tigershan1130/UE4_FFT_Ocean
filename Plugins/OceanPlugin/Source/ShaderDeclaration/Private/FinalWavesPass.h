#pragma once
#include "RHIResources.h"
#include "RHICommandList.h"
#include "CoreMinimal.h"


// This struct contains all the data we need to pass from the game thread to draw our effect.
struct FShaderFinalWavesParams
{
public:
    FTextureRHIRef OutputDisplacement;
    FUnorderedAccessViewRHIRef OutputDisplacementUAV;
    FShaderResourceViewRHIRef OutputDisplacementSRV;

    FTextureRHIRef OutputDerivatives;
    FUnorderedAccessViewRHIRef OutputDerivativesUAV;
    FShaderResourceViewRHIRef OutputDerivativesSRV;

    FTextureRHIRef OutputTurbulence;
    FUnorderedAccessViewRHIRef OutputTurbulenceUAV;
    FShaderResourceViewRHIRef OutputTurbulenceSRV;
 
    int Size;

    float deltaTime;
    float lambda;

    FShaderFinalWavesParams() { }

    FShaderFinalWavesParams(int TextureSize, float DeltaTime, float Lambda)
    {

        Size = TextureSize;

        deltaTime = DeltaTime;
        lambda = Lambda;

        FRHIResourceCreateInfo CreateInfo;
        OutputDisplacement = RHICreateTexture2D(TextureSize, TextureSize, PF_A32B32G32R32F, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
        OutputDisplacementUAV = RHICreateUnorderedAccessView(OutputDisplacement);
        OutputDisplacementSRV = RHICreateShaderResourceView(OutputDisplacement, 0);

        OutputDerivatives = RHICreateTexture2D(TextureSize, TextureSize, PF_A32B32G32R32F, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
        OutputDerivativesUAV = RHICreateUnorderedAccessView(OutputDerivatives);
        OutputDerivativesSRV = RHICreateShaderResourceView(OutputDerivatives, 0);

        OutputTurbulence = RHICreateTexture2D(TextureSize, TextureSize, PF_A32B32G32R32F, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
        OutputTurbulenceUAV = RHICreateUnorderedAccessView(OutputTurbulence);
        OutputTurbulenceSRV = RHICreateShaderResourceView(OutputTurbulence, 0);
    }
};


/**************************************************************************************/
/* This is just an interface we use to keep all the compute shading code in one file. */
/**************************************************************************************/
class FFinalWaveComputePass
{
public:
    static void RunComputeShader_RenderThread(FRHICommandListImmediate& RHICmdList, FShaderFinalWavesParams& FinalWaveParams, FShaderResourceViewRHIRef DxDz, FShaderResourceViewRHIRef DyDxz, FShaderResourceViewRHIRef DyxDyz, FShaderResourceViewRHIRef DxxDzz);
};
