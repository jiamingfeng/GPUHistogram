// Fill out your copyright notice in the Description page of Project Settings.

#include "HistogramShader.h"
#include "ShaderParameterUtils.h" // Necessary for SetShaderValue, SetUniformBufferParameter.


FHistogramShader::FHistogramShader()
{}

FHistogramShader::FHistogramShader(const ShaderMetaType::CompiledShaderInitializerType& initializer)
	: FGlobalShader(initializer)
{
	Level.Bind(initializer.ParameterMap, TEXT("level"), SPF_Optional);
	HistogramBuffer.Bind(initializer.ParameterMap, TEXT("HistogramBuffer"), SPF_Optional);
	InputTexture.Bind(initializer.ParameterMap, TEXT("InputTexture"), SPF_Optional);
	HistogramTexture.Bind(initializer.ParameterMap, TEXT("HistogramTexture"), SPF_Optional);
}

bool FHistogramShader::Serialize(FArchive& Ar) {
	bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
	Ar << Level << HistogramBuffer << InputTexture << HistogramTexture;
	return bShaderHasOutdatedParameters;
}

void FHistogramShader::SetLevel(FRHICommandList& rhi_command_list, const float level) {
	if (Level.IsBound())
	{
		SetShaderValue(rhi_command_list, GetComputeShader(), Level, level);
	}	
}

void FHistogramShader::SetInputTexture(FRHICommandList& rhi_command_list, FTextureRHIParamRef textureRef)
{
	//checkSlow(InputTexture.IsInitialized());
	if (InputTexture.IsBound())
	{
		SetTextureParameter(rhi_command_list, GetComputeShader(), InputTexture, textureRef);
	}
}

void FHistogramShader::SetHistogramBuffer(FRHICommandList& rhi_command_list, FUnorderedAccessViewRHIParamRef bufferRef)
{
	if (HistogramBuffer.IsBound()) {
		rhi_command_list.SetUAVParameter(GetComputeShader(), HistogramBuffer.GetBaseIndex(), bufferRef);
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
	if (InputTexture.IsBound()) {
		rhi_command_list.SetShaderResourceViewParameter(GetComputeShader(), InputTexture.GetBaseIndex(), FShaderResourceViewRHIParamRef());
	}
}

void FHistogramShader::ClearOutput(FRHICommandList& rhi_command_list)
{
	if (HistogramBuffer.IsBound()) {
		rhi_command_list.SetUAVParameter(GetComputeShader(), HistogramBuffer.GetBaseIndex(), FUnorderedAccessViewRHIParamRef());
	}

	if (HistogramTexture.IsBound())
	{
		rhi_command_list.SetUAVParameter(GetComputeShader(), HistogramTexture.GetBaseIndex(), FUnorderedAccessViewRHIParamRef());
	}
}

FHistogramShaderMain::FHistogramShaderMain()
{}

FHistogramShaderMain::FHistogramShaderMain(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
	: FHistogramShader(Initializer)
{
}

FHistogramShaderTextureCompute::FHistogramShaderTextureCompute()
{}

FHistogramShaderTextureCompute::FHistogramShaderTextureCompute(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
	: FHistogramShader(Initializer)
{
}

IMPLEMENT_SHADER_TYPE(, FHistogramShader, TEXT("/Plugin/GPUHistogram/Private/HistogramCompute.usf"), TEXT("HistogramMain"), SF_Compute);
IMPLEMENT_SHADER_TYPE(, FHistogramShaderMain, TEXT("/Plugin/GPUHistogram/Private/HistogramCompute.usf"), TEXT("HistogramMain"), SF_Compute);
IMPLEMENT_SHADER_TYPE(, FHistogramShaderTextureCompute, TEXT("/Plugin/GPUHistogram/Private/HistogramCompute.usf"), TEXT("HistogramTextureCompute"), SF_Compute);
