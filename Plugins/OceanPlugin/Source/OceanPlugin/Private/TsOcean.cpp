// Fill out your copyright notice in the Description page of Project Settings.

#include "TsOcean.h"

// Sets default values
ATsOcean::ATsOcean()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATsOcean::BeginPlay()
{
	Super::BeginPlay();

	ConstructGaussianNoise();

	// setting up initial parameters
	SwellSpectrumParams = FSpectrumParameters();
	
	SwellSpectrumParams.scale = SwellScale;
	SwellSpectrumParams.angle = SwellWindDir / 180 * PI;
	SwellSpectrumParams.spreadBlend = SwellSpread;
	SwellSpectrumParams.swell = SwellSwell;
	SwellSpectrumParams.alpha = JonswapAlpha(Gravity, SwellFetch, SwellWindSpeed);
	SwellSpectrumParams.peakOmega = JonswapPeakFrequency(Gravity, SwellFetch, SwellWindSpeed);
	SwellSpectrumParams.gamma = SwellPeak;
	SwellSpectrumParams.shortWavesFade = SwellShortWaveFade;

	LocalSpectrumParams = FSpectrumParameters();
	LocalSpectrumParams.scale = LocalScale;
	LocalSpectrumParams.angle = LocalWindDir / 180 * PI;
	LocalSpectrumParams.spreadBlend = LocalSpread;
	LocalSpectrumParams.swell = LocalSwell;
	LocalSpectrumParams.alpha = JonswapAlpha(Gravity, LocalFetch, LocalWindSpeed);
	LocalSpectrumParams.peakOmega = JonswapPeakFrequency(Gravity, LocalFetch, LocalWindSpeed);
	LocalSpectrumParams.gamma = LocalPeak;
	LocalSpectrumParams.shortWavesFade = LocalShortWaveFade;

	FShaderDeclarationModule::Get("ShaderDeclaration").BeginRendering();

}

void ATsOcean::BeginDestroy()
{
	Super::BeginDestroy();

	FShaderDeclarationModule::Get("ShaderDeclaration").EndRendering();

}

// Called every frame
void ATsOcean::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TotalElaspedTime += DeltaTime * TimeFactor;

	// 1. and 2. only need to be executed once.
	// 1. Generate Guassian Noise texture once
	CopyGaussianNoiseTextureToResourceViewOnce();

	// 2. Set wavesSettings. to Shaders
	// intiailize spectrum shader.


	float boundary1 = 2 * PI / LengthScale1 * 6;
	float boundary2 = 2 * PI / LengthScale2 * 6;

	// Low Pass
	FRenderFFTPassParams FFT_LowPass = FRenderFFTPassParams();

	FFT_LowPass.TwiddleFactorParams = FShaderTwiddleFactorParameters(Size);
	FFT_LowPass.H0Params = FShaderH0Parameters(GaussianNoiseSRV, SwellSpectrumParams, LocalSpectrumParams, boundary1, 0.0001f, Gravity, Depth, LengthScale0, Size);
	FFT_LowPass.ConjSpectrumParams = FShaderConjSpectrumParameters(Size);
	FFT_LowPass.FrequencySpecParams = FFrequencySpectrumParameters(TotalElaspedTime, Size);
	FFT_LowPass.ButterFlyDxDzParams = FButterFlyParameters(Size);
	FFT_LowPass.ButterFlyDyDxzParams = FButterFlyParameters(Size);
	FFT_LowPass.ButterFlyDyxDyzParams = FButterFlyParameters(Size);
	FFT_LowPass.ButterFlyDxxDzzParams = FButterFlyParameters(Size);
	FFT_LowPass.FinalwavesParams = FShaderFinalWavesParams(Size, DeltaTime, 1);

	if (FFT_LowPass.DisplacementRT == nullptr)
		FFT_LowPass.DisplacementRT = FFT_Displacement_Low;

	if (FFT_LowPass.DerivativesRT == nullptr)
		FFT_LowPass.DerivativesRT = FFT_Derivatives_Low;

	if (FFT_LowPass.FoldRT == nullptr)
		FFT_LowPass.FoldRT = FFT_Fold_Low;


	// Mid Pass
	FRenderFFTPassParams FFT_MidPass = FRenderFFTPassParams();
	FFT_MidPass.TwiddleFactorParams = FShaderTwiddleFactorParameters(Size);
	FFT_MidPass.H0Params = FShaderH0Parameters(GaussianNoiseSRV, SwellSpectrumParams, LocalSpectrumParams, boundary2, boundary1, Gravity, Depth, LengthScale1, Size);
	FFT_MidPass.ConjSpectrumParams = FShaderConjSpectrumParameters(Size);
	FFT_MidPass.FrequencySpecParams = FFrequencySpectrumParameters(TotalElaspedTime, Size);
	FFT_MidPass.ButterFlyDxDzParams = FButterFlyParameters(Size);
	FFT_MidPass.ButterFlyDyDxzParams = FButterFlyParameters(Size);
	FFT_MidPass.ButterFlyDyxDyzParams = FButterFlyParameters(Size);
	FFT_MidPass.ButterFlyDxxDzzParams = FButterFlyParameters(Size);
	FFT_MidPass.FinalwavesParams = FShaderFinalWavesParams(Size, DeltaTime, 1);

	if (FFT_MidPass.DisplacementRT == nullptr)
		FFT_MidPass.DisplacementRT = FFT_Displacement_Mid;

	if (FFT_MidPass.DerivativesRT == nullptr)
		FFT_MidPass.DerivativesRT = FFT_Derivatives_Mid;

	if (FFT_MidPass.FoldRT == nullptr)
		FFT_MidPass.FoldRT = FFT_Fold_Mid;

	// High Pass
	FRenderFFTPassParams FFT_HighPass = FRenderFFTPassParams();
	FFT_HighPass.TwiddleFactorParams = FShaderTwiddleFactorParameters(Size);
	FFT_HighPass.H0Params = FShaderH0Parameters(GaussianNoiseSRV, SwellSpectrumParams, LocalSpectrumParams, 999999, boundary2, Gravity, Depth, LengthScale2, Size);
	FFT_HighPass.ConjSpectrumParams = FShaderConjSpectrumParameters(Size);
	FFT_HighPass.FrequencySpecParams = FFrequencySpectrumParameters(TotalElaspedTime, Size);
	FFT_HighPass.ButterFlyDxDzParams = FButterFlyParameters(Size);
	FFT_HighPass.ButterFlyDyDxzParams = FButterFlyParameters(Size);
	FFT_HighPass.ButterFlyDyxDyzParams = FButterFlyParameters(Size);
	FFT_HighPass.ButterFlyDxxDzzParams = FButterFlyParameters(Size);
	FFT_HighPass.FinalwavesParams = FShaderFinalWavesParams(Size, DeltaTime, 1);

	if (FFT_HighPass.DisplacementRT == nullptr)
		FFT_HighPass.DisplacementRT = FFT_Displacement_High;

	if (FFT_HighPass.DerivativesRT == nullptr)
		FFT_HighPass.DerivativesRT = FFT_Derivatives_High;

	if (FFT_HighPass.FoldRT == nullptr)
		FFT_HighPass.FoldRT = FFT_Fold_High;


	if (FShaderDeclarationModule::IsAvailable("ShaderDeclaration"))
	{
		FShaderDeclarationModule::Get("ShaderDeclaration").UpdateParameters("Low", FFT_LowPass);
		FShaderDeclarationModule::Get("ShaderDeclaration").UpdateParameters("Mid", FFT_MidPass);
		FShaderDeclarationModule::Get("ShaderDeclaration").UpdateParameters("High", FFT_HighPass);
	}
}