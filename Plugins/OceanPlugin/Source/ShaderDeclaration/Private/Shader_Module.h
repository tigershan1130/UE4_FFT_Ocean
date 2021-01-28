#pragma once

#include "CoreMinimal.h"
#include "HZeroPass.h"
#include "FinalWavesPass.h"
#include "TwiddleFactorPass.h"
#include "IFFTPass.h"
#include "InitialSpectrumCalculation.h"
#include "FrequencySpectrum.h"
#include "CopyRTPixelShader.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "RenderGraphResources.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"
#include "Shader_Module.generated.h"
/*
 * Since we already have a module interface due to us being in a plugin, it's pretty handy to just use it
 * to interact with the renderer. It gives us the added advantage of being able to decouple any render
 * hooks from game modules as well, allowing for safer and easier cleanup.
 *
 * With the advent of UE4.22 the renderer got an overhaul to more modern usage patterns present in API's
 * like DX12. The biggest differences for us are that we must now wrap all rendering code either in either
 * a "Render Graph" or a "Render Pass". They're good for different things of course.
 *
 * Render graphs:
 * In UE4, you generally want to work with graphs if you are working with larger rendering jobs and working with engine
 * pooled render targets. As the engine almost exclusively uses task graphs now for rendering tasks as well, learning
 * by example is also easier right now if you elect to use them. They are quite hard (if not impossible?) to use when
 * interfacing with UObject render resources like UTextures however, so if you want to do that, you need to use passes instead.
 * Graphs generally perform better if you use them for larger jobs. For smaller work, you yet again want to look at passes instead.
 *
 * Render passes: 
 * Passes are very similar to the previous graphics API and are now used when using the rasterizer.
 * Instead of setting the render target for a rasterization operation, you now set that up when beginning a render pass instead.
 * Operations that don't use the rasterizer (like compute, copy and other operations) simply use the RHICommandList directly like before.
 * There are of course many resources you can leverage to get a better understanding of the new API. A good initial overview
 * can be found in this presentation:
 * See: https://developer.nvidia.com/sites/default/files/akamai/gameworks/blog/GDC16/GDC16_gthomas_adunn_Practical_DX12.pdf
 */


USTRUCT()
struct SHADERDECLARATION_API FRenderFFTPassParams
{
    GENERATED_BODY();

    // cached values
    // H0  Compute Shader 传进去或者更新的数据
    FShaderH0Parameters H0Params;

    // FFT 吐出的数据 渲染到 Debug RT 上
    FShaderPixelCopyParameter FFTRTParams;

    // Frequency Specturm Parameters 
    FFrequencySpectrumParameters FrequencySpecParams;

    // Twiddle Factor Parameters. 
    FShaderTwiddleFactorParameters TwiddleFactorParams;

    // Shader conjecture spectrum parameters
    FShaderConjSpectrumParameters ConjSpectrumParams;

    // Butterfly Parameters 
    FButterFlyParameters ButterFlyDxDzParams;
    FButterFlyParameters ButterFlyDyDxzParams;
    FButterFlyParameters ButterFlyDyxDyzParams;
    FButterFlyParameters ButterFlyDxxDzzParams;

    FShaderFinalWavesParams FinalwavesParams;

    UPROPERTY()
    UTextureRenderTarget2D* DisplacementRT;

    UPROPERTY()
    UTextureRenderTarget2D* DerivativesRT;

    UPROPERTY()
    UTextureRenderTarget2D* FoldRT;

    bool InitialDataGenerated = false;
};


class SHADERDECLARATION_API FShaderDeclarationModule : public IModuleInterface
{
public:

    static inline FShaderDeclarationModule& Get(FName moduleName)
    {
        return FModuleManager::LoadModuleChecked<FShaderDeclarationModule>(moduleName);
    }

    static inline bool IsAvailable(FName moduleName)
    {
        return FModuleManager::Get().IsModuleLoaded(moduleName);
    }

public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

public:

    void BeginRendering();

    void EndRendering();

    // Update Parameters from TsOcean.Cpp Tick() to pass in datas from CPU
    void UpdateParameters(const FString& Key, FRenderFFTPassParams& FFTParams);

    FDelegateHandle OnPostResolvedSceneColorHandle;
    FCriticalSection RenderEveryFrameLock;

    volatile bool bCachedParametersValid;

    // auto called by render thread every tick()
    void PostResolveSceneColor_RenderThread(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext);

private:
    // This holds all datas that would need to be updated into FFT render thread.
    UPROPERTY()
    TMap<FString, FRenderFFTPassParams> FFTCachedDatas;

    // Draw Render thread
    void Draw_RenderThread(FRenderFFTPassParams& FFTParams, const FString& Key);
};