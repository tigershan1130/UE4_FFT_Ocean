#include "InitialSpectrumCalculation.h"
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
class FInitialSpectrumComputeShader : public FGlobalShader
{
	// defining uniform variables that's going to be updated.
public:
	DECLARE_GLOBAL_SHADER(FInitialSpectrumComputeShader);
	SHADER_USE_PARAMETER_STRUCT(FInitialSpectrumComputeShader, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_SRV(Texture2D, H0K)
		SHADER_PARAMETER_UAV(RWTexture2D<float4>, H0)
		SHADER_PARAMETER(UINT, Size)
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
IMPLEMENT_GLOBAL_SHADER(FInitialSpectrumComputeShader, "/OceanShaders/Private/TS_ConjugatedSpectrum.usf", "CalculateConjugatedSpectrum", SF_Compute);

void FConjSpectrumPass::RunComputeShader_RenderThread(FRHICommandListImmediate& RHICmdList, FShaderResourceViewRHIRef InputH0KSRV, const FShaderConjSpectrumParameters& DrawParameters)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_ShaderPlugin_ComputeShader); // Used to gather CPU profiling data for the UE4 session frontend
	SCOPED_DRAW_EVENT(RHICmdList, ShaderPlugin_Compute); // Used to profile GPU activity and add metadata to be consumed by for example RenderDoc

	FInitialSpectrumComputeShader::FParameters PassParameters;

	PassParameters.H0K = InputH0KSRV;
	PassParameters.H0 = DrawParameters.OutputH0UAV;
	PassParameters.Size = DrawParameters.mSize;

	TShaderMapRef<FInitialSpectrumComputeShader> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));


	FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, PassParameters,
		FIntVector(FMath::DivideAndRoundUp(DrawParameters.mSize, NUM_THREADS_PER_GROUP_DIMENSION),
			FMath::DivideAndRoundUp(DrawParameters.mSize, NUM_THREADS_PER_GROUP_DIMENSION), 1));


}
