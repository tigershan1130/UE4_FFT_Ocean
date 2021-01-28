#pragma once
#include "RHIResources.h"
#include "RHICommandList.h"
#include "CoreMinimal.h"

// This struct contains all the data we need to pass from the game thread to draw our effect.
struct FShaderConjSpectrumParameters
{
public:

    FTextureRHIRef OutputH0;
    FUnorderedAccessViewRHIRef OutputH0UAV;
    FShaderResourceViewRHIRef OutputH0SRV;

   int mSize;

    FShaderConjSpectrumParameters() {}

    FShaderConjSpectrumParameters(int Size)
    {
        mSize = Size;

        FRHIResourceCreateInfo CreateInfo;
        OutputH0 = RHICreateTexture2D(Size, Size, PF_A32B32G32R32F, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
        OutputH0UAV = RHICreateUnorderedAccessView(OutputH0);
        OutputH0SRV = RHICreateShaderResourceView(OutputH0, 0);

    }

};


/**************************************************************************************/
/* This is just an interface we use to keep all the compute shading code in one file. */
/**************************************************************************************/
class FConjSpectrumPass
{
public:
    static void RunComputeShader_RenderThread(FRHICommandListImmediate& RHICmdList, FShaderResourceViewRHIRef InputH0KSRV, const FShaderConjSpectrumParameters& DrawParameters);
};
