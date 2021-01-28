// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "GameFramework/Actor.h"
#include "ShaderDeclaration/Private/Shader_Module.h"
#include "Engine/TextureRenderTarget2D.h"
#include "TsOcean.generated.h"


UCLASS()
class OCEANPLUGIN_API ATsOcean : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATsOcean();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// called when begin to destory
	virtual void BeginDestroy() override;

	// generate gaussian noise 
	float GetGaussianRandomFloat()
	{
		float u1 = FMath::Rand() / ((float)RAND_MAX);
		float u2 = FMath::Rand() / ((float)RAND_MAX);
		if (u1 < 1e-6f)
		{
			u1 = 1e-6f;
		}
		return sqrtf(-2.0f * logf(u1)) * cosf(2.0f * PI * u2);
	}

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean")
		UTextureRenderTarget2D* FFT_Displacement_Low = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean")
		UTextureRenderTarget2D* FFT_Displacement_Mid = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean")
		UTextureRenderTarget2D* FFT_Displacement_High = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean")
		UTextureRenderTarget2D* FFT_Derivatives_Low = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean")
		UTextureRenderTarget2D* FFT_Derivatives_Mid = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean")
		UTextureRenderTarget2D* FFT_Derivatives_High = nullptr;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean")
		UTextureRenderTarget2D* FFT_Fold_Low = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean")
		UTextureRenderTarget2D* FFT_Fold_Mid = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean")
		UTextureRenderTarget2D* FFT_Fold_High = nullptr;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean")
		UTextureRenderTarget2D* Debug_RT = nullptr;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ocean")
		UTexture2D* GaussianNoise;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ocean")
		float Size = 512;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ocean")
		float Depth = 500;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ocean")
		float LengthScale0 = 250;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ocean")
		float LengthScale1 = 17;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ocean")
		float LengthScale2 = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ocean")
		float Gravity = 9.81;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ocean")
		float TimeFactor = 1;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean Spectrum Swell")
		float SwellScale = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean Spectrum Swell")
		float SwellWindSpeed = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean Spectrum Swell")
		float SwellWindDir = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ocean Spectrum Swell")
		float SwellFetch = 300000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ocean Spectrum Swell")
		float SwellSpread = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean Spectrum Swell")
		float SwellSwell = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean Spectrum Swell")
		float SwellPeak = 3.3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean Spectrum Swell")
		float SwellShortWaveFade  = 0.01;

	// ===============================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean Spectrum Local")
		float LocalScale = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean Spectrum Local")
		float LocalWindSpeed = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean Spectrum Local")
		float LocalWindDir = -29.81f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ocean Spectrum Local")
		float LocalFetch = 100000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ocean Spectrum Local")
		float LocalSpread = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean Spectrum Local")
		float LocalSwell = 0.198;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean Spectrum Local")
		float LocalPeak = 3.3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean Spectrum Local")
		float LocalShortWaveFade = 0.01;

	// ==================================================== FFT Stuff ==================================

	// 构造高斯分布图， 这个只需要叫一次. 因为只需要叫一次我们可以用GPU pixel shader 生成， 也可以用 
	// CPU 生成。
	void ConstructGaussianNoise()
	{
		//GaussianNoise 随机生成 512x512 个像素点 对应顶点信息
		uint32 resolution = Size * Size;
		TArray<FVector4> rawData;
		rawData.SetNumZeroed(resolution);
		for (uint32 i = 0; i < resolution; i++)
		{
			rawData[i].X = GetGaussianRandomFloat();
			rawData[i].Y = GetGaussianRandomFloat();
			rawData[i].Z = GetGaussianRandomFloat();
			rawData[i].W = GetGaussianRandomFloat();
		}

		GaussianNoise = UTexture2D::CreateTransient(Size, Size, PF_A32B32G32R32F);
		GaussianNoise->UpdateResource();
		FTexture2DMipMap& mip = GaussianNoise->PlatformData->Mips[0];
		void* data = mip.BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memcpy(data, rawData.GetData(), resolution * 4 * sizeof(float));
		mip.BulkData.Unlock();
		GaussianNoise->UpdateResource();
	}


	float JonswapAlpha(float g, float fetch, float windSpeed)
	{
		return 0.076f * FMath::Pow(g * fetch / windSpeed / windSpeed, -0.22f);
	}

	float JonswapPeakFrequency(float g, float fetch, float windSpeed)
	{
		return 22 * FMath::Pow(windSpeed * fetch / g / g, -0.33f);
	}

	FVector2D ComputeWindDirection(float WindAngleVal)
	{
		WindAngleVal = FMath::Fmod(WindAngleVal, 360.0f);
		float WindAngleDirectionRad = FMath::DegreesToRadians(WindAngleVal);
		return FVector2D(FMath::Cos(WindAngleDirectionRad), FMath::Sin(WindAngleDirectionRad));
	}



	// 把 Texture2D gaussian 信息放入到 resource view 里
	void CopyGaussianNoiseTextureToResourceViewOnce()
	{
		//Let's enqueue a render command to create our gaussian noise distribution and store it into a shader resource view that later we'll pass to the ocean simulator
		//This one will be a one shot operation. A better place for this operation will be in the BeginPlay method of this actor.
		if (GaussianNoise != nullptr && GaussianNoiseSRV == nullptr)
		{
			ENQUEUE_RENDER_COMMAND(GaussianNoiseSRVCreationCommand)(
				[this](FRHICommandListImmediate& RHICmdList)
				{
					FTexture2DRHIRef Tex2DRHIRef = ((GaussianNoise->Resource))->GetTexture2DRHI();
					GaussianNoiseSRV = RHICreateShaderResourceView(Tex2DRHIRef, 0);
				});
			FlushRenderingCommands();
		}
	}



protected:

	float TotalElaspedTime = 0;

	//Gaussian noise shader resource view
	FShaderResourceViewRHIRef GaussianNoiseSRV;

	FSpectrumParameters SwellSpectrumParams;
	FSpectrumParameters LocalSpectrumParams;




	FRHITexture* GetRHITextureFromRT(UTextureRenderTarget2D* RenderTarget)
	{
		FTextureReferenceRHIRef OutputRenderTargetTextureRHI = RenderTarget->TextureReference.TextureReferenceRHI;
		checkf(OutputRenderTargetTextureRHI != nullptr, TEXT("Can't get render target %d texture"));
		FRHITexture* RenderTargetTextureRef = OutputRenderTargetTextureRHI->GetTextureReference()->GetReferencedTexture();

		return RenderTargetTextureRef;
	}

private:



};
