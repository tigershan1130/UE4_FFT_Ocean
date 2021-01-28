#include "Shader_Module.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "RHI.h"
#include "GlobalShader.h"
#include "RHICommandList.h"
#include "RenderGraphBuilder.h"
#include "RenderTargetPool.h"
#include "Interfaces/IPluginManager.h"

IMPLEMENT_MODULE(FShaderDeclarationModule, ShaderDeclaration)

// Declare some GPU stats so we can track them later
DECLARE_GPU_STAT_NAMED(ShaderPlugin_Render, TEXT("ShaderPlugin: Root Render"));
DECLARE_GPU_STAT_NAMED(ShaderPlugin_Compute, TEXT("ShaderPlugin: Render Compute Shader"));
DECLARE_GPU_STAT_NAMED(ShaderPlugin_Pixel, TEXT("ShaderPlugin: Render Pixel Shader"));


// this will point our shader folder to the correct path, we need to manually change the folder path.
void FShaderDeclarationModule::StartupModule()
{
	OnPostResolvedSceneColorHandle.Reset();
	bCachedParametersValid = false;


	// Maps virtual shader source directory to the plugin's actual shaders directory.
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("OceanPlugin"))->GetBaseDir(), TEXT("Shaders"));

	AddShaderSourceDirectoryMapping(TEXT("/OceanShaders"), PluginShaderDir);
}

void FShaderDeclarationModule::ShutdownModule()
{
	EndRendering();
}

void FShaderDeclarationModule::BeginRendering()
{
	if (OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}

	bCachedParametersValid = false;

	const FName RendererModuleName("Renderer");

	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);

	if (RendererModule)
	{
		OnPostResolvedSceneColorHandle = RendererModule->GetResolvedSceneColorCallbacks().AddRaw(this, &FShaderDeclarationModule::PostResolveSceneColor_RenderThread);
	}
}

void FShaderDeclarationModule::EndRendering()
{
	if (!OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}

	const FName RendererModuleName("Renderer");

	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);

	if (RendererModule)
	{
		RendererModule->GetResolvedSceneColorCallbacks().Remove(OnPostResolvedSceneColorHandle);
	}

	OnPostResolvedSceneColorHandle.Reset();
}


void FShaderDeclarationModule::UpdateParameters(const FString& Key, FRenderFFTPassParams& FFTParams)
{

	RenderEveryFrameLock.Lock();

	if (FFTCachedDatas.Contains(Key))
	{
		// THis kind of copy is needed, otherwise, We could be getting blank textures.
		if (!FFTCachedDatas[Key].InitialDataGenerated)
		{
			FFTCachedDatas[Key].TwiddleFactorParams = FFTParams.TwiddleFactorParams;
		}

		FFTCachedDatas[Key].H0Params = FFTParams.H0Params;
		FFTCachedDatas[Key].FrequencySpecParams = FFTParams.FrequencySpecParams;
		FFTCachedDatas[Key].ConjSpectrumParams = FFTParams.ConjSpectrumParams;
		FFTCachedDatas[Key].ButterFlyDxDzParams = FFTParams.ButterFlyDxDzParams;
		FFTCachedDatas[Key] .ButterFlyDyDxzParams = FFTParams.ButterFlyDyDxzParams;
		FFTCachedDatas[Key].ButterFlyDyxDyzParams = FFTParams.ButterFlyDyxDyzParams;
		FFTCachedDatas[Key].ButterFlyDxxDzzParams = FFTParams.ButterFlyDxxDzzParams;

		FFTCachedDatas[Key].FinalwavesParams = FFTParams.FinalwavesParams;
		FFTCachedDatas[Key].DisplacementRT = FFTParams.DisplacementRT;
		FFTCachedDatas[Key].DerivativesRT = FFTParams.DerivativesRT;
		FFTCachedDatas[Key].FoldRT = FFTParams.FoldRT;
	}
	else
	{
		FFTCachedDatas.Add(Key, FFTParams);
	}


	bCachedParametersValid = true;
	RenderEveryFrameLock.Unlock();
}



void FShaderDeclarationModule::PostResolveSceneColor_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneRenderTargets& SceneContext)
{
	if (!bCachedParametersValid)
	{
		return;
	}

	// Depending on your data, you might not have to lock here, just added this code to show how you can do it if you have to.
	RenderEveryFrameLock.Lock();

	TMap<FString, FRenderFFTPassParams> FFTPass_Copy;
	FFTPass_Copy.Append(FFTCachedDatas);
	
	RenderEveryFrameLock.Unlock();

	for (TMap<FString, FRenderFFTPassParams>::TIterator It = FFTPass_Copy.CreateIterator(); It; ++It)
	{
		Draw_RenderThread(It.Value(), It.Key());
	}
}

// TODO:这个以后可以全部用 AddPass 方法， 加入到 Render Graph 里
// 现在暂时为了实现效果暂时一个个渲染
void FShaderDeclarationModule::Draw_RenderThread(FRenderFFTPassParams& FFTParams, const FString& Key)
{
	check(IsInRenderingThread());

	if (!FFTCachedDatas.Contains(Key))
		return;

	FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();


	QUICK_SCOPE_CYCLE_COUNTER(STAT_ShaderPlugin_Render); // Used to gather CPU profiling data for the UE4 session frontend
	SCOPED_DRAW_EVENT(RHICmdList, ShaderPlugin_Render); // Used to profile GPU activity and add metadata to be consumed by for example RenderDoc

	// 0. compute twiddle indicies for divide and conquer for IFFT
	if (!FFTCachedDatas[Key].InitialDataGenerated)
	{
		FTwiddleFactorPass::RunComputeShader_RenderThread(RHICmdList, FFTParams.TwiddleFactorParams);
		FFTCachedDatas[Key].InitialDataGenerated = true;
	}

	//1. Compute HZero Pass
	FHZeroPass::RunComputeShader_RenderThread(RHICmdList, FFTParams.H0Params);

	//2. Compute H0 Initial Spectrumpass
	if (FFTParams.H0Params.OutputH0KSRV != nullptr)
		FConjSpectrumPass::RunComputeShader_RenderThread(RHICmdList, FFTParams.H0Params.OutputH0KSRV, FFTParams.ConjSpectrumParams);

	//3. Frequency Spectrum Update Pass
	if (FFTParams.H0Params.OutputH0KSRV != nullptr && FFTParams.H0Params.OutputPrecomputedWaveSRV)
	{
		FFreqSpectrumPass::RunComputeShader_RenderThread(RHICmdList, FFTParams.FrequencySpecParams, FFTParams.ConjSpectrumParams.OutputH0SRV, FFTParams.H0Params.OutputPrecomputedWaveSRV);
	}

	//4. calculate DxDz , DyDxz, DyxDyz, DxxDzz of IFFT complex amplitudes
	// Compute DxDZ
	if (FFTParams.TwiddleFactorParams.OutputSurfaceTextureSRV != nullptr && FFTParams.FrequencySpecParams.OutputDxDz_SRV != nullptr )
	{
		FIFFTButterFlyPass::RunComputeShader_RenderThread(RHICmdList, FFTParams.ButterFlyDxDzParams, FFTParams.H0Params.OutputH0KSRV, FFTParams.H0Params.OutputH0KUAV, 
			FFTParams.TwiddleFactorParams.OutputSurfaceTextureSRV, FFTParams.FrequencySpecParams.OutputDxDz_SRV, FFTParams.FrequencySpecParams.OutputDxDz_UAV);
	}

	// Compute Dy Dxz
	if (FFTParams.TwiddleFactorParams.OutputSurfaceTextureSRV != nullptr && FFTParams.FrequencySpecParams.OutputDyDxz_SRV != nullptr)
	{
		FIFFTButterFlyPass::RunComputeShader_RenderThread(RHICmdList, FFTParams.ButterFlyDyDxzParams, FFTParams.H0Params.OutputH0KSRV, FFTParams.H0Params.OutputH0KUAV,
			FFTParams.TwiddleFactorParams.OutputSurfaceTextureSRV, FFTParams.FrequencySpecParams.OutputDyDxz_SRV, FFTParams.FrequencySpecParams.OutputDyDxz_UAV);
	}

	// Compute DyxDyz
	if (FFTParams.TwiddleFactorParams.OutputSurfaceTextureSRV != nullptr && FFTParams.FrequencySpecParams.OutputDyxDyz_SRV != nullptr)
	{
		FIFFTButterFlyPass::RunComputeShader_RenderThread(RHICmdList, FFTParams.ButterFlyDyxDyzParams, FFTParams.H0Params.OutputH0KSRV, FFTParams.H0Params.OutputH0KUAV,
			FFTParams.TwiddleFactorParams.OutputSurfaceTextureSRV, FFTParams.FrequencySpecParams.OutputDyxDyz_SRV, FFTParams.FrequencySpecParams.OutputDyxDyz_UAV);
	}

	// Compute DxxDzz
	if (FFTParams.TwiddleFactorParams.OutputSurfaceTextureSRV != nullptr && FFTParams.FrequencySpecParams.OutputDxxDzz_SRV != nullptr)
	{
		FIFFTButterFlyPass::RunComputeShader_RenderThread(RHICmdList, FFTParams.ButterFlyDxxDzzParams, FFTParams.H0Params.OutputH0KSRV, FFTParams.H0Params.OutputH0KUAV,
			FFTParams.TwiddleFactorParams.OutputSurfaceTextureSRV, FFTParams.FrequencySpecParams.OutputDxxDzz_SRV, FFTParams.FrequencySpecParams.OutputDxxDzz_UAV);
	}

	if (FFTParams.FrequencySpecParams.OutputDxDz_SRV != nullptr &&
		FFTParams.FrequencySpecParams.OutputDyDxz_SRV != nullptr &&
		FFTParams.FrequencySpecParams.OutputDyxDyz_SRV != nullptr &&
		FFTParams.FrequencySpecParams.OutputDxxDzz_SRV != nullptr)
	{
		FFinalWaveComputePass::RunComputeShader_RenderThread(RHICmdList, FFTParams.FinalwavesParams, FFTParams.FrequencySpecParams.OutputDxDz_SRV, 
			FFTParams.FrequencySpecParams.OutputDyDxz_SRV, FFTParams.FrequencySpecParams.OutputDyxDyz_SRV, FFTParams.FrequencySpecParams.OutputDxxDzz_SRV);
	}
		
	if (FFTParams.DisplacementRT != nullptr, FFTParams.FinalwavesParams.OutputDisplacementSRV != nullptr)
	{
		FCopyTexturePixelShader::DrawToRenderTarget_RenderThread(RHICmdList, FFTParams.DisplacementRT, FFTParams.FinalwavesParams.OutputDisplacementSRV);
	}

	if (FFTParams.DerivativesRT != nullptr, FFTParams.FinalwavesParams.OutputDerivativesSRV != nullptr)
	{
		FCopyTexturePixelShader::DrawToRenderTarget_RenderThread(RHICmdList, FFTParams.DerivativesRT, FFTParams.FinalwavesParams.OutputDerivativesSRV);
	}

	if (FFTParams.FoldRT != nullptr, FFTParams.FinalwavesParams.OutputTurbulenceSRV != nullptr)
	{
		FCopyTexturePixelShader::DrawToRenderTarget_RenderThread(RHICmdList, FFTParams.FoldRT, FFTParams.FinalwavesParams.OutputTurbulenceSRV);
	}
}
