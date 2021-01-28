#include "FrequencySpectrum.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"
#include "Shader.h"
#include "GlobalShader.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"

#define NUM_THREADS_PER_GROUP_DIMENSION 8

/**********************************************************************************************/
/* This class carries our parameter declarations and acts as the bridge between cpp and HLSL. */
/**********************************************************************************************/
class FFrequencySpectrumComputeShader : public FGlobalShader
{
	// defining uniform variables that's going to be updated.
public:
	DECLARE_GLOBAL_SHADER(FFrequencySpectrumComputeShader);
	SHADER_USE_PARAMETER_STRUCT(FFrequencySpectrumComputeShader, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_SRV(Texture2D, H0)
		SHADER_PARAMETER_SRV(Texture2D, WavesData)
		SHADER_PARAMETER_UAV(RWTexture2D<float4>, Dx_Dz)
		SHADER_PARAMETER_UAV(RWTexture2D<float4>, Dy_Dxz)
		SHADER_PARAMETER_UAV(RWTexture2D<float4>, Dyx_Dyz)
		SHADER_PARAMETER_UAV(RWTexture2D<float4>, Dxx_Dzz)
		SHADER_PARAMETER(float, Time)
		END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}
};


// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FFrequencySpectrumComputeShader, "/OceanShaders/Private/TS_FFTFrequencySpectrumCompute.usf", "CalculateAmplitudes", SF_Compute);

void FFreqSpectrumPass::RunComputeShader_RenderThread(FRHICommandListImmediate& RHICmdList, const FFrequencySpectrumParameters& DrawParameters, FShaderResourceViewRHIRef InputH0SRV, FShaderResourceViewRHIRef InputWaveDataSRV)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_ShaderPlugin_ComputeShader); // Used to gather CPU profiling data for the UE4 session frontend
	SCOPED_DRAW_EVENT(RHICmdList, ShaderPlugin_Compute); // Used to profile GPU activity and add metadata to be consumed by for example RenderDoc

	FFrequencySpectrumComputeShader::FParameters PassParameters;
	PassParameters.H0  = InputH0SRV;
	PassParameters.WavesData= InputWaveDataSRV;
	PassParameters.Dx_Dz = DrawParameters.OutputDxDz_UAV;
	PassParameters.Dy_Dxz = DrawParameters.OutputDyDxz_UAV;
	PassParameters.Dyx_Dyz = DrawParameters.OutputDyxDyz_UAV;
	PassParameters.Dxx_Dzz = DrawParameters.OutputDxxDzz_UAV;

	PassParameters.Time = DrawParameters.WorldTimeSeconds;

	TShaderMapRef<FFrequencySpectrumComputeShader> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	// 也可以用以下命令一样的， 我只是把底层写出来， 方便大家理解
	FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, PassParameters, 
								FIntVector(FMath::DivideAndRoundUp(DrawParameters.Size, NUM_THREADS_PER_GROUP_DIMENSION),
										   FMath::DivideAndRoundUp(DrawParameters.Size, NUM_THREADS_PER_GROUP_DIMENSION), 1));
}