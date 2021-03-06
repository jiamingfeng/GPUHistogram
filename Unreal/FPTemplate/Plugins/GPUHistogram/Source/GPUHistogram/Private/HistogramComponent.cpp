// Fill out your copyright notice in the Description page of Project Settings.

#include "HistogramComponent.h"

const int32 HISTOGRAM_BUCKET_SIZE = 128;
const int32 HISTOGRAM_LEVEL = 256;

static void PrintBuffer(const TArray<uint32> &Buffer)
{
	FString content;
	for (auto value : Buffer)
	{
		content.AppendInt(value);
		content.Append(TEXT(", "));
	}
	UE_LOG(LogTemp, Warning, TEXT("Buffer Array: [ %s ]"), *content);
}


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
	HistogramBufferCPU.Init(0, HISTOGRAM_BUCKET_SIZE);
	HistogramBufferRA.SetNum(HISTOGRAM_BUCKET_SIZE);
	FMemory::Memcpy(HistogramBufferRA.GetData(), HistogramBufferCPU.GetData(), sizeof(uint32) * HISTOGRAM_BUCKET_SIZE);

	HistogramBufferResource.ResourceArray = &HistogramBufferRA;
	HistogramBufferRef = RHICreateStructuredBuffer(sizeof(uint32), sizeof(uint32) * HISTOGRAM_BUCKET_SIZE, 
		BUF_ShaderResource | BUF_UnorderedAccess, HistogramBufferResource);
	HistogramBufferUAV = RHICreateUnorderedAccessView(HistogramBufferRef, false, false);

	InputTextureRef = InputTexture->Resource->TextureRHI;

	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(InitHistogramBufferCommand,
		UHistogramComponent*, ShaderComponent, this,
		TArray<uint32>*, Readback, &HistogramBufferCPU, {
		  ShaderComponent->InitHistogram_RenderThread(Readback);
		});

	RenderCommandFence.BeginFence();
	RenderCommandFence.Wait();

	UE_LOG(LogTemp, Warning, TEXT("===== Histogram Buffer output ====="));
	PrintBuffer(HistogramBufferCPU);

}

void UHistogramComponent::InitHistogram_RenderThread(TArray<uint32>* readback)
{
	check(IsInRenderingThread());

	// Get global RHI command list
	FRHICommandListImmediate& rhi_command_list = GRHICommandList.GetImmediateCommandList();

	// Get the actual shader instance off the ShaderMap
	TShaderMapRef<FHistogramShaderMain> shader(ShaderMap);

	rhi_command_list.SetComputeShader(shader->GetComputeShader());
	shader->SetLevel(rhi_command_list, HISTOGRAM_BUCKET_SIZE * HISTOGRAM_LEVEL );
	shader->SetInputTexture(rhi_command_list, InputTextureRef);
	shader->SetHistogramBuffer(rhi_command_list, HistogramBufferUAV);
	
	FIntPoint ThreadGroupCountValue(
		FMath::DivideAndRoundUp(HISTOGRAM_BUCKET_SIZE, 8),
		FMath::DivideAndRoundUp(HISTOGRAM_BUCKET_SIZE, 8));

	// Could I run two shader in one pass?? hmmm
	DispatchComputeShader(rhi_command_list, *shader, ThreadGroupCountValue.X, ThreadGroupCountValue.Y, 1);

	shader->ClearOutput(rhi_command_list);
	shader->ClearParameters(rhi_command_list);

	const auto* shader_data = (const uint32*)rhi_command_list.LockStructuredBuffer(HistogramBufferRef, 0, sizeof(uint32) * HISTOGRAM_BUCKET_SIZE, EResourceLockMode::RLM_ReadOnly);
	FMemory::Memcpy(readback->GetData(), shader_data, sizeof(uint32) * HISTOGRAM_BUCKET_SIZE);

	rhi_command_list.UnlockStructuredBuffer(HistogramBufferRef);

}

