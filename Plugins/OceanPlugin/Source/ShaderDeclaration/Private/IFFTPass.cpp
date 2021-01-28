#include "IFFTPass.h"
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
class FButterflyComputeShader : public FGlobalShader
{
	// defining uniform variables that's going to be updated.
public:
	DECLARE_GLOBAL_SHADER(FButterflyComputeShader);
	SHADER_USE_PARAMETER_STRUCT(FButterflyComputeShader, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_SRV(Texture2D, PrecomputedData)
		SHADER_PARAMETER_SRV(Texture2D, InputBuffer0)
		SHADER_PARAMETER_UAV(RWTexture2D<float4>, Buffer0)
		SHADER_PARAMETER(UINT, Dir)
		SHADER_PARAMETER(UINT, Step)
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
IMPLEMENT_GLOBAL_SHADER(FButterflyComputeShader, "/OceanShaders/Private/TS_ButterFlyCompute.usf", "ComputeButterflyCS", SF_Compute);

void FIFFTButterFlyPass::RunComputeShader_RenderThread(FRHICommandListImmediate& RHICmdList, FButterFlyParameters DrawParameters, FShaderResourceViewRHIRef BufferSRV, FUnorderedAccessViewRHIRef BufferUAV, FShaderResourceViewRHIRef TwiddleTexture, FShaderResourceViewRHIRef FrequencySpectrumSRV, FUnorderedAccessViewRHIRef FrequencySpectrumUAV)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_ShaderPlugin_ComputeShader); // Used to gather CPU profiling data for the UE4 session frontend 
	SCOPED_DRAW_EVENT(RHICmdList, ShaderPlugin_Compute); // Used to profile GPU activity and add metadata to be consumed by for example RenderDoc

	TShaderMapRef<FButterflyComputeShader> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	int logSize = (int)FMath::Log2(DrawParameters.mSize);
	int pingPong = 1;

	FButterflyComputeShader::FParameters PassParameters;
	PassParameters.PrecomputedData = TwiddleTexture;


	for (int Dir = 0; Dir < 2; ++Dir)
	{
		PassParameters.Dir = Dir;

		for (int i = 0; i < logSize; i++)
		{
			pingPong = (pingPong == 0) ? 1 : 0;

			PassParameters.Step = i;

			if (pingPong == 0)
			{
				PassParameters.Buffer0 = BufferUAV;
				PassParameters.InputBuffer0 = FrequencySpectrumSRV;
			}
			else
			{
				PassParameters.Buffer0 = FrequencySpectrumUAV;
				PassParameters.InputBuffer0 = BufferSRV;
			}

			FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, PassParameters,
					FIntVector(FMath::DivideAndRoundUp(DrawParameters.mSize, NUM_THREADS_PER_GROUP_DIMENSION),
						FMath::DivideAndRoundUp(DrawParameters.mSize, NUM_THREADS_PER_GROUP_DIMENSION), 1));
		}
	}
}