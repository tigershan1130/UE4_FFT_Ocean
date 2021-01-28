#pragma once
#include "RHIResources.h"
#include "RHICommandList.h"
#include "CoreMinimal.h"


struct FSpectrumParameters
{
    float scale;
    float angle;
    float spreadBlend;
    float swell;
    float alpha;
    float peakOmega;
    float gamma;
    float shortWavesFade;
};


// This struct contains all the data we need to pass from the game thread to draw our effect.
struct FShaderH0Parameters
{
public:

    FShaderResourceViewRHIRef GaussianNoiseInputResource;

    FTextureRHIRef OutputH0K;
    FUnorderedAccessViewRHIRef OutputH0KUAV;
    FShaderResourceViewRHIRef OutputH0KSRV;


    FTextureRHIRef OutputPrecomputedWave;
    FUnorderedAccessViewRHIRef OutputPrecomputedWaveUAV;
    FShaderResourceViewRHIRef OutputPrecomputedWaveSRV;

    float mCutOffHigh;
    float mCutOffLow;
    float mGravity;
    float mLengthScale;
    float mDepth;

    TResourceArray<FSpectrumParameters> mSpectrumParameters;
    FStructuredBufferRHIRef SpectrumBuffer;
    FShaderResourceViewRHIRef SpectrumInputSRV;

    FIntPoint GetRenderTargetSize() const
    {
        return CachedRenderTargetSize;
    }

    FShaderH0Parameters() { }

    FShaderH0Parameters(FShaderResourceViewRHIRef InGaussianNoiseInput, FSpectrumParameters SwellSpectrumParameters, FSpectrumParameters LocalSpectrumParameters, float CutOffHigh, float CutOffLow, float GravityAcceleration, float Depth, float LengthScale, uint32 TextureSize)
    {
        CachedRenderTargetSize = FIntPoint(TextureSize, TextureSize);
        
        mCutOffHigh = CutOffHigh;
        mCutOffLow = CutOffLow;
        mGravity = GravityAcceleration;
        mLengthScale = LengthScale;
        mDepth = Depth;  

        GaussianNoiseInputResource = InGaussianNoiseInput;


        FRHIResourceCreateInfo CreateInfo;
        OutputH0K = RHICreateTexture2D(TextureSize, TextureSize, PF_A32B32G32R32F, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
        OutputH0KUAV = RHICreateUnorderedAccessView(OutputH0K);
        OutputH0KSRV = RHICreateShaderResourceView(OutputH0K, 0);


        OutputPrecomputedWave = RHICreateTexture2D(TextureSize, TextureSize, PF_A32B32G32R32F, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
        OutputPrecomputedWaveUAV = RHICreateUnorderedAccessView(OutputPrecomputedWave);
        OutputPrecomputedWaveSRV = RHICreateShaderResourceView(OutputPrecomputedWave, 0);

        // Structured Buffer Conversion Not RW, just read only.
        mSpectrumParameters.Reset();

        if (LocalSpectrumParameters.scale <= 0) // local scale can't be 0 other wise there will be no fft calculation... 
            LocalSpectrumParameters.scale = 0.1f;

        mSpectrumParameters.Add(LocalSpectrumParameters);
        mSpectrumParameters.Add(SwellSpectrumParameters);
        mSpectrumParameters.SetAllowCPUAccess(true);

        FRHIResourceCreateInfo BufferCreateInfo(&mSpectrumParameters);
        SpectrumBuffer = RHICreateStructuredBuffer(sizeof(FSpectrumParameters), mSpectrumParameters.GetResourceDataSize(), BUF_Static | BUF_ShaderResource, BufferCreateInfo);
        SpectrumInputSRV = RHICreateShaderResourceView(SpectrumBuffer);
    }

private:

    FIntPoint CachedRenderTargetSize;
};


/**************************************************************************************/
/* This is just an interface we use to keep all the compute shading code in one file. */
/**************************************************************************************/
class FHZeroPass
{
public:
	static void RunComputeShader_RenderThread(FRHICommandListImmediate& RHICmdList, const FShaderH0Parameters& DrawParameters);
};
