// Fill out your copyright notice in the Description page of Project Settings.

#include "HistogramComponent.h"

const int32 HISTOGRAM_BUFFER_SIZE = 256 * 128;


// Sets default values for this component's properties
UHistogramComponent::UHistogramComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHistogramComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UHistogramComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHistogramComponent::InitHistogram(UTextureRenderTarget2D* InputTexture)
{
	TArray<uint32> HistogramBufferCPU;
	HistogramBufferCPU.Init(0, HISTOGRAM_BUFFER_SIZE);
	HistogramBufferRA.SetNum(HISTOGRAM_BUFFER_SIZE);
	FMemory::Memcpy(HistogramBufferRA.GetData(), HistogramBufferCPU.GetData(), sizeof(uint32) * HISTOGRAM_BUFFER_SIZE);

	HistogramBufferResource.ResourceArray = &HistogramBufferRA;
	HistogramBufferRef = RHICreateStructuredBuffer(sizeof(uint32), sizeof(float) * HISTOGRAM_BUFFER_SIZE, BUF_ShaderResource, HistogramBufferResource);
	HistogramBufferSRV = RHICreateShaderResourceView(HistogramBufferRef);

	const FTexture2DRHIRef &TextureRef = InputTexture->GetRenderTargetResource()->GetRenderTargetTexture();
	InputTextureRef = TextureRef;



	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(InitHistogramBufferCommand,
		UHistogramComponent*, ShaderComponent, this, {
		  ShaderComponent->InitHistogram_RenderThread();
		});

	RenderCommandFence.BeginFence();
	RenderCommandFence.Wait();
}

void UHistogramComponent::InitHistogram_RenderThread()
{
	check(IsInRenderingThread());

	// Get global RHI command list
	FRHICommandListImmediate& rhi_command_list = GRHICommandList.GetImmediateCommandList();

	// Get the actual shader instance off the ShaderMap
	TShaderMapRef<FHistogramShader> shader(ShaderMap);

	rhi_command_list.SetComputeShader(shader->GetComputeShader());
	shader->SetHistogramBuffer(rhi_command_list, HistogramBufferSRV);
	shader->SetInputTexture(rhi_command_list, InputTextureRef);
}

