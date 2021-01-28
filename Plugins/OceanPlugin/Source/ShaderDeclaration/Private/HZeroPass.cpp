#include "HZeroPass.h"
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
class FHZeroPassComputeShader : public FGlobalShader
{
	// defining uniform variables that's going to be updated.
public:
	DECLARE_GLOBAL_SHADER(FHZeroPassComputeShader);
	SHADER_USE_PARAMETER_STRUCT(FHZeroPassComputeShader, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_UAV(RWTexture2D<float4>, H0K)
		SHADER_PARAMETER_UAV(RWTexture2D<float4>, WavesData)
		//SHADER_PARAMETER_UAV(RWTexture2D<float4>, Dx_Dz)
		//SHADER_PARAMETER_UAV(RWTexture2D<float4>, Dy_Dxz)
		//SHADER_PARAMETER_UAV(RWTexture2D<float4>, Dyx_Dyz)
		//SHADER_PARAMETER_UAV(RWTexture2D<float4>, Dxx_Dzz)
		SHADER_PARAMETER_SRV(StructuredBuffer<FSpectrumParameters>, Spectrums)
		SHADER_PARAMETER_SRV(Texture2D, InputTexture)
		SHADER_PARAMETER(UINT, Size) // Metal doesn't support GetDimensions(), so we send in this data via our parameters.
		SHADER_PARAMETER(float, LengthScale)
		SHADER_PARAMETER(float, CutoffHigh)
		SHADER_PARAMETER(float, CutoffLow)
		SHADER_PARAMETER(float, GravityAcceleration)
		SHADER_PARAMETER(float, Depth)
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
IMPLEMENT_GLOBAL_SHADER(FHZeroPassComputeShader, "/OceanShaders/Private/TS_H0GenerationCompute.usf", "CalculateInitialSpectrum", SF_Compute);

void FHZeroPass::RunComputeShader_RenderThread(FRHICommandListImmediate& RHICmdList, const FShaderH0Parameters& DrawParameters)
{

	QUICK_SCOPE_CYCLE_COUNTER(STAT_ShaderPlugin_ComputeShader); // Used to gather CPU profiling data for the UE4 session frontend
	SCOPED_DRAW_EVENT(RHICmdList, ShaderPlugin_Compute); // Used to profile GPU activity and add metadata to be consumed by for example RenderDoc
	
	FHZeroPassComputeShader::FParameters PassParameters;

	//PassParameters.Dx_Dz = DrawParameters.BufferDxDzUAV;
	//PassParameters.Dy_Dxz = DrawParameters.BufferDyDxzUAV;
	//PassParameters.Dyx_Dyz = DrawParameters.BufferDyxDyzUAV;
	//PassParameters.Dxx_Dzz = DrawParameters.BufferDxxDzzUAV;
	PassParameters.H0K = DrawParameters.OutputH0KUAV;
	PassParameters.WavesData = DrawParameters.OutputPrecomputedWaveUAV;
	PassParameters.InputTexture = DrawParameters.GaussianNoiseInputResource;
	PassParameters.CutoffHigh = DrawParameters.mCutOffHigh;
	PassParameters.CutoffLow = DrawParameters.mCutOffLow;
	PassParameters.Depth = DrawParameters.mDepth;
	PassParameters.LengthScale = DrawParameters.mLengthScale;
	PassParameters.Size = DrawParameters.GetRenderTargetSize().X;
	PassParameters.GravityAcceleration = DrawParameters.mGravity;
	PassParameters.Spectrums = DrawParameters.SpectrumInputSRV;

	TShaderMapRef<FHZeroPassComputeShader> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));	

	// 也可以用以下命令一样的， 我只是把底层写出来， 方便大家理解。
	FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, PassParameters,
		FIntVector(FMath::DivideAndRoundUp(DrawParameters.GetRenderTargetSize().X, NUM_THREADS_PER_GROUP_DIMENSION),
			FMath::DivideAndRoundUp(DrawParameters.GetRenderTargetSize().Y, NUM_THREADS_PER_GROUP_DIMENSION), 1));

	// basically above does things below:

	//FRHIComputeShader* ShaderRHI = ComputeShader.GetComputeShader();
	

	//RHICmdList.SetComputeShader(ShaderRHI);

	//SetShaderParameters(RHICmdList, ComputeShader, ShaderRHI, PassParameters);

	//RHICmdList.DispatchComputeShader(DrawParameters.GetRenderTargetSize().X / NUM_THREADS_PER_GROUP_DIMENSION, DrawParameters.GetRenderTargetSize().Y / NUM_THREADS_PER_GROUP_DIMENSION, 1);
	//
	//// TODO(RDG): Once all shader sets their parameter through this, can refactor RHI so all UAVs of a shader get unset through a single RHI function call.
	//const FShaderParameterBindings& Bindings = ComputeShader->Bindings;

	//// unbind shaders and uavs
	//checkf(Bindings.RootParameterBufferIndex == FShaderParameterBindings::kInvalidBufferIndex, TEXT("Can't use UnsetShaderUAVs() for root parameter buffer index."));

	//// 4.26 的 更新 -_-.. 可以查FComputeShaderUtils::Dispatch 的底层方法
	//for (const FShaderParameterBindings::FResourceParameter& ParameterBinding : Bindings.ResourceParameters)
	//{
	//	if (ParameterBinding.BaseType == UBMT_UAV ||
	//		ParameterBinding.BaseType == UBMT_RDG_TEXTURE_UAV ||
	//		ParameterBinding.BaseType == UBMT_RDG_BUFFER_UAV)
	//	{
	//		RHICmdList.SetUAVParameter(ShaderRHI, ParameterBinding.BaseIndex, nullptr);
	//	}
	//}

}