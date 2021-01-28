#pragma once
#include "RHIResources.h"
#include "RHICommandList.h"
#include "CoreMinimal.h"

// This struct contains all the data we need to pass from the game thread to draw our effect.
struct FButterFlyParameters
{
public:
    int32 mSize;

    float WorldTimeSeconds;

    FTextureRHIRef SurfaceTexture;
    FUnorderedAccessViewRHIRef SurfaceTexture_UAV;
    FShaderResourceViewRHIRef SurfaceTexture_SRV;

    //FShaderResourceViewRHIRef FinalTextureSRV;
    //FTextureRHIRef FinalTexture;

    float WaveAmplitude;
    FVector2D WindDir;
    float WindDependency;


    FButterFlyParameters() { }

    FButterFlyParameters(int32 Size)
    {
        mSize = Size;

        FRHIResourceCreateInfo CreateInfo;

        SurfaceTexture = RHICreateTexture2D(Size, Size, PF_A32B32G32R32F, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
        SurfaceTexture_UAV = RHICreateUnorderedAccessView(SurfaceTexture);
        SurfaceTexture_SRV = RHICreateShaderResourceView(SurfaceTexture, 0);

        //FinalTexture = RHICreateTexture2D(Size, Size, PF_A32B32G32R32F, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
        //FinalTextureSRV = RHICreateShaderResourceView(FinalTexture, 0);
    }

private:

    FIntPoint CachedRenderTargetSize;
};


/**************************************************************************************/
/* This is just an interface we use to keep all the compute shading code in one file. */
/**************************************************************************************/
class FIFFTButterFlyPass
{
public:
    static void RunComputeShader_RenderThread(FRHICommandListImmediate& RHICmdList, FButterFlyParameters DrawParameters, FShaderResourceViewRHIRef BufferSRV, FUnorderedAccessViewRHIRef BufferUAV, FShaderResourceViewRHIRef TwiddleTexture, FShaderResourceViewRHIRef FrequencySpectrumSRV, FUnorderedAccessViewRHIRef FrequencySpectrumUAV);

};
