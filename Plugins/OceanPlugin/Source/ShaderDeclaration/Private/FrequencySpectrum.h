#pragma once
#include "RHIResources.h"
#include "RHICommandList.h"
#include "CoreMinimal.h"

// This struct contains all the data we need to pass from the game thread to draw our effect.
struct FFrequencySpectrumParameters
{
public:
    float WorldTimeSeconds;

    FTextureRHIRef OutputDxDz;
    FUnorderedAccessViewRHIRef OutputDxDz_UAV;
    FShaderResourceViewRHIRef OutputDxDz_SRV;

    FTextureRHIRef OutputDyDxz;
    FUnorderedAccessViewRHIRef OutputDyDxz_UAV;
    FShaderResourceViewRHIRef OutputDyDxz_SRV;

    FTextureRHIRef OutputDyxDyz;
    FUnorderedAccessViewRHIRef OutputDyxDyz_UAV;
    FShaderResourceViewRHIRef OutputDyxDyz_SRV;

    FTextureRHIRef OutputDxxDzz;
    FUnorderedAccessViewRHIRef OutputDxxDzz_UAV;
    FShaderResourceViewRHIRef OutputDxxDzz_SRV;

    int Size;

    FFrequencySpectrumParameters() { }

    FFrequencySpectrumParameters(float Time, int TextureSize)
    {
        Size = TextureSize; 

        FRHIResourceCreateInfo CreateInfo;

        OutputDxDz = RHICreateTexture2D(TextureSize, TextureSize, PF_A32B32G32R32F, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
        OutputDxDz_UAV = RHICreateUnorderedAccessView(OutputDxDz);
        OutputDxDz_SRV = RHICreateShaderResourceView(OutputDxDz, 0);

        OutputDyDxz = RHICreateTexture2D(TextureSize, TextureSize, PF_A32B32G32R32F, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
        OutputDyDxz_UAV = RHICreateUnorderedAccessView(OutputDyDxz);
        OutputDyDxz_SRV = RHICreateShaderResourceView(OutputDyDxz, 0);

        OutputDyxDyz = RHICreateTexture2D(TextureSize, TextureSize, PF_A32B32G32R32F, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
        OutputDyxDyz_UAV = RHICreateUnorderedAccessView(OutputDyxDyz);
        OutputDyxDyz_SRV = RHICreateShaderResourceView(OutputDyxDyz, 0);

        OutputDxxDzz = RHICreateTexture2D(TextureSize, TextureSize, PF_A32B32G32R32F, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
        OutputDxxDzz_UAV = RHICreateUnorderedAccessView(OutputDxxDzz);
        OutputDxxDzz_SRV = RHICreateShaderResourceView(OutputDxxDzz, 0);

        WorldTimeSeconds = Time;
    }

private:

    FIntPoint CachedRenderTargetSize;
};


/**************************************************************************************/
/* This is just an interface we use to keep all the compute shading code in one file. */
/**************************************************************************************/
class FFreqSpectrumPass
{
public:
    static void RunComputeShader_RenderThread(FRHICommandListImmediate& RHICmdList, const FFrequencySpectrumParameters& DrawParameters, FShaderResourceViewRHIRef InputH0SRV, FShaderResourceViewRHIRef InputWaveDataSRV);
};
