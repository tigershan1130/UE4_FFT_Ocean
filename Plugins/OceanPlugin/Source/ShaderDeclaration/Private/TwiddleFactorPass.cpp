#include "TwiddleFactorPass.h"
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
class FTwiddleFactorComputeShader : public FGlobalShader
{
	// defining uniform variables that's going to be updated.
public:
	DECLARE_GLOBAL_SHADER(FTwiddleFactorComputeShader);
	SHADER_USE_PARAMETER_STRUCT(FTwiddleFactorComputeShader, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_UAV(RWTexture2D<uint>, OutputSurface0)
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
IMPLEMENT_GLOBAL_SHADER(FTwiddleFactorComputeShader, "/OceanShaders/Private/TS_TwiddleFactorsCompute.usf", "TwiddleFactorsCS", SF_Compute);

void FTwiddleFactorPass::RunComputeShader_RenderThread(FRHICommandListImmediate& RHICmdList, const FShaderTwiddleFactorParameters& DrawParameters)
{
	FTwiddleFactorComputeShader::FParameters PassParameters;
	PassParameters.OutputSurface0 = DrawParameters.OutputSurfaceTextureUAV;
	PassParameters.Size = DrawParameters.mSize;

	int logSize = (int)FMath::Log2(DrawParameters.mSize);

	TShaderMapRef<FTwiddleFactorComputeShader> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	// 也可以用以下命令一样的， 我只是把底层写出来， 方便大家理解
	FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, PassParameters,
		FIntVector(logSize,
			FMath::DivideAndRoundUp(int(DrawParameters.mSize/2), NUM_THREADS_PER_GROUP_DIMENSION), 1));
}