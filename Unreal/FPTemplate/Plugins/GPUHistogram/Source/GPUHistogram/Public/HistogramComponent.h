// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DynamicRHIResourceArray.h" // Core module
#include "RenderCommandFence.h" // RenderCore module
#include "HistogramShader.h"
#include "Engine/TextureRenderTarget2D.h"
#include "HistogramComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GPUHISTOGRAM_API UHistogramComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHistogramComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Histogram")
	void InitHistogram(UTextureRenderTarget2D* InputTexture);

	void InitHistogram_RenderThread();

	TResourceArray<uint32> HistogramBufferRA;
	FRHIResourceCreateInfo HistogramBufferResource;
	FStructuredBufferRHIRef HistogramBufferRef;
	FShaderResourceViewRHIRef HistogramBufferSRV;

	FTextureRHIParamRef InputTextureRef;

	FRenderCommandFence RenderCommandFence; // Necessary for waiting until a render command function finishes.
	const TShaderMap<FGlobalShaderType>* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	
};
