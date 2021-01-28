#include "FinalWavesPass.h"
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
class FFinalWaveComputeShader : public FGlobalShader
{
	// defining uniform variables that's going to be updated.
	/*RWTexture2D<float4> Displacement;
	RWTexture2D<float4> Derivatives;
	RWTexture2D<float4> Turbulence;

	Texture2D<float2> Dx_Dz;
	Texture2D<float2> Dy_Dxz;
	Texture2D<float2> Dyx_Dyz;
	Texture2D<float2> Dxx_Dzz;

	float Lambda;
	float DeltaTime;*/
public:
	DECLARE_GLOBAL_SHADER(FFinalWaveComputeShader);
	SHADER_USE_PARAMETER_STRUCT(FFinalWaveComputeShader, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_UAV(RWTexture2D<float4>, Displacement)
		SHADER_PARAMETER_UAV(RWTexture2D<float4>, Derivatives)
		SHADER_PARAMETER_UAV(RWTexture2D<float4>, Turbulence)
		SHADER_PARAMETER_SRV(Texture2D, TurbulenceInput)
		SHADER_PARAMETER_SRV(Texture2D, Dx_Dz)
		SHADER_PARAMETER_SRV(Texture2D, Dy_Dxz)
		SHADER_PARAMETER_SRV(Texture2D, Dyx_Dyz)
		SHADER_PARAMETER_SRV(Texture2D, Dxx_Dzz)
		SHADER_PARAMETER(float, Lambda)
		SHADER_PARAMETER(float, DeltaTime)
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
//                            ShaderType                            ShaderPath                     Shader function name Type
IMPLEMENT_GLOBAL_SHADER(FFinalWaveComputeShader, "/OceanShaders/Private/TS_FinalWavesCompute.usf", "FillResultTextures", SF_Compute);

void FFinalWaveComputePass::RunComputeShader_RenderThread(FRHICommandListImmediate& RHICmdList, FShaderFinalWavesParams& FinalWaveParams, FShaderResourceViewRHIRef DxDz, FShaderResourceViewRHIRef DyDxz, FShaderResourceViewRHIRef DyxDyz, FShaderResourceViewRHIRef DxxDzz)
{

	QUICK_SCOPE_CYCLE_COUNTER(STAT_ShaderPlugin_ComputeShader); // Used to gather CPU profiling data for the UE4 session frontend
	SCOPED_DRAW_EVENT(RHICmdList, ShaderPlugin_Compute); // Used to profile GPU activity and add metadata to be consumed by for example RenderDoc

	FFinalWaveComputeShader::FParameters PassParameters;

	PassParameters.DeltaTime = FinalWaveParams.deltaTime;
	PassParameters.Lambda = FinalWaveParams.lambda;
	PassParameters.Dx_Dz = DxDz;
	PassParameters.Dy_Dxz = DyDxz;
	PassParameters.Dyx_Dyz = DyxDyz;
	PassParameters.Dxx_Dzz = DxxDzz;
	PassParameters.TurbulenceInput = FinalWaveParams.OutputTurbulenceSRV;
	PassParameters.Displacement = FinalWaveParams.OutputDisplacementUAV;
	PassParameters.Derivatives = FinalWaveParams.OutputDerivativesUAV;
	PassParameters.Turbulence = FinalWaveParams.OutputTurbulenceUAV;

	TShaderMapRef<FFinalWaveComputeShader> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	// 也可以用以下命令一样的， 我只是把底层写出来， 方便大家理解。
	FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, PassParameters,
		FIntVector(FMath::DivideAndRoundUp(FinalWaveParams.Size, NUM_THREADS_PER_GROUP_DIMENSION),
			FMath::DivideAndRoundUp(FinalWaveParams.Size, NUM_THREADS_PER_GROUP_DIMENSION), 1));
}