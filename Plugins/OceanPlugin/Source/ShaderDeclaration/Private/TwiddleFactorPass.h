#pragma once
#include "RHIResources.h"
#include "RHICommandList.h"
#include "CoreMinimal.h"

// This struct contains all the data we need to pass from the game thread to draw our effect.
struct FShaderTwiddleFactorParameters
{
public:
    
    FTextureRHIRef OutputSurfaceTexture;
    FUnorderedAccessViewRHIRef OutputSurfaceTextureUAV;
    FShaderResourceViewRHIRef OutputSurfaceTextureSRV;

    uint32 mSize;


    FShaderTwiddleFactorParameters() { }

    FShaderTwiddleFactorParameters(uint32 Size)
    {
        FRHIResourceCreateInfo CreateInfo;

        int logSize = (int)FMath::Log2(Size);

        OutputSurfaceTexture = RHICreateTexture2D(logSize, Size, PF_A32B32G32R32F, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
        OutputSurfaceTextureUAV = RHICreateUnorderedAccessView(OutputSurfaceTexture);
        OutputSurfaceTextureSRV = RHICreateShaderResourceView(OutputSurfaceTexture, 0);

        mSize = Size;
    }
};


/**************************************************************************************/
/* This is just an interface we use to keep all the compute shading code in one file. */
/**************************************************************************************/
class FTwiddleFactorPass
{
public:
    static void RunComputeShader_RenderThread(FRHICommandListImmediate& RHICmdList, const FShaderTwiddleFactorParameters& DrawParameters);
};
