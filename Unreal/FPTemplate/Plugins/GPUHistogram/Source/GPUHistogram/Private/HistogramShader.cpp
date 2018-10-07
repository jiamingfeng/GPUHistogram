// Fill out your copyright notice in the Description page of Project Settings.

#include "HistogramShader.h"
#include "ShaderParameterUtils.h" // Necessary for SetShaderValue, SetUniformBufferParameter.

FHistogramShader::FHistogramShader(const ShaderMetaType::CompiledShaderInitializerType& initializer)
	: FGlobalShader(initializer)
{
	Level.Bind(initializer.ParameterMap, TEXT("level"), SPF_Mandatory);
	HistogramBuffer.Bind(initializer.ParameterMap, TEXT("HistogramBuffer"), SPF_Mandatory);
	InputTexture.Bind(initializer.ParameterMap, TEXT("InputTexture"), SPF_Mandatory);
	HistogramTexture.Bind(initializer.ParameterMap, TEXT("HistogramTexture"), SPF_Mandatory);
}

//FHistogramShader::~FHistogramShader()
//{
//}

bool FHistogramShader::Serialize(FArchive& Ar) {
	bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
	Ar << HistogramBuffer << InputTexture << HistogramTexture;
	return bShaderHasOutdatedParameters;
}

void FHistogramShader::SetLevel(FRHICommandList& rhi_command_list, const float level) {
	SetShaderValue(rhi_command_list, GetComputeShader(), Level, level);
}

void FHistogramShader::SetInputTexture(FRHICommandList& rhi_command_list, const FTexture* Texture)
{
	checkSlow(InputTexture.IsInitialized());
	if (InputTexture.IsBound())
	{
		Texture->LastRenderTime = FApp::GetCurrentTime();
		rhi_command_list.SetShaderTexture(GetComputeShader(), InputTexture.GetBaseIndex(), Texture->TextureRHI);
	}
}

void FHistogramShader::SetHistogramBuffer(FRHICommandList& rhi_command_list, FShaderResourceViewRHIRef bufferRef)
{
	if (HistogramBuffer.IsBound()) {
		rhi_command_list.SetShaderResourceViewParameter(GetComputeShader(), HistogramBuffer.GetBaseIndex(), bufferRef);
	}
}

void FHistogramShader::SetHistogramTexture(FRHICommandList& rhi_command_list,
	FUnorderedAccessViewRHIParamRef histogramTextureRef)
{
	if (HistogramTexture.IsBound()) {
		rhi_command_list.SetUAVParameter(GetComputeShader(), HistogramTexture.GetBaseIndex(), histogramTextureRef);
	}
}

void FHistogramShader::ClearParameters(FRHICommandList& rhi_command_list)
{
	if (HistogramBuffer.IsBound()) {
		rhi_command_list.SetShaderResourceViewParameter(GetComputeShader(), HistogramBuffer.GetBaseIndex(), FShaderResourceViewRHIParamRef());
	}
	if (InputTexture.IsBound()) {
		rhi_command_list.SetShaderResourceViewParameter(GetComputeShader(), InputTexture.GetBaseIndex(), FShaderResourceViewRHIParamRef());
	}
}

void FHistogramShader::ClearOutput(FRHICommandList& rhi_command_list)
{
	if (HistogramTexture.IsBound())
	{
		rhi_command_list.SetUAVParameter(GetComputeShader(), HistogramTexture.GetBaseIndex(), FUnorderedAccessViewRHIParamRef());
	}
}

IMPLEMENT_SHADER_TYPE(, FHistogramShaderMain, TEXT("/Plugin/GPUHistogram/Private/HistogramCompute.usf"), TEXT("HistogramMain"), SF_Compute);
IMPLEMENT_SHADER_TYPE(, FHistogramShaderTextureCompute, TEXT("/Plugin/GPUHistogram/Private/HistogramCompute.usf"), TEXT("HistogramTextureCompute"), SF_Compute);