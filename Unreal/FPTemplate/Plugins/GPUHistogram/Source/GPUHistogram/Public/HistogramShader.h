// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h" //ShaderCore module
#include "UniformBuffer.h" // RenderCore module
#include "RHICommandList.h" // RHI module
#include "Engine/TextureRenderTarget2D.h"
#include "Public/GlobalShader.h"



class FHistogramShader : public FGlobalShader
{
	 
	DECLARE_SHADER_TYPE(FHistogramShader, Global);
public:
	/* Essential functions */

	FHistogramShader();
	FHistogramShader(const ShaderMetaType::CompiledShaderInitializerType& initializer);

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return true;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& PermutationParams, FShaderCompilerEnvironment& OutEnvironment) {
		FGlobalShader::ModifyCompilationEnvironment(PermutationParams, OutEnvironment);
		OutEnvironment.CompilerFlags.Add(CFLAG_StandardOptimization);
	}
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& PermutationParams) {
		// Useful when adding a permutation of a particular shader
		return true;
	}

	virtual bool Serialize(FArchive& Ar) override;

	void SetLevel(FRHICommandList& rhi_command_list, const float level);
	void SetInputTexture(FRHICommandList& rhi_command_list, FTextureRHIParamRef textureRef);

	void SetHistogramBuffer(FRHICommandList& rhi_command_list, FUnorderedAccessViewRHIParamRef bufferRef);
	void SetHistogramTexture(FRHICommandList& rhi_command_list, FUnorderedAccessViewRHIParamRef histogramTextureRef);

	void ClearParameters(FRHICommandList& rhi_command_list); // for RWTexture2D.
	void ClearOutput(FRHICommandList& rhi_command_list); // for RWTexture2D.

protected:

	FShaderParameter         Level;
	FShaderResourceParameter HistogramBuffer; //RWBuffer<uint> HistogramBuffer;
	FShaderResourceParameter InputTexture; //RWTexture2D<float4> InputTexture;

	FShaderResourceParameter HistogramTexture; //RWTexture2D<float4> HistogramTexture;
};

class FHistogramShaderMain : public FHistogramShader
{
	DECLARE_SHADER_TYPE(FHistogramShaderMain, Global);

public:

	/** Default constructor. */
	FHistogramShaderMain();

	/** Initialization constructor. */
	FHistogramShaderMain(const ShaderMetaType::CompiledShaderInitializerType& Initializer);
};

class FHistogramShaderTextureCompute : public FHistogramShader
{
	DECLARE_SHADER_TYPE(FHistogramShaderTextureCompute, Global);

public:

	/** Default constructor. */
	FHistogramShaderTextureCompute();

	/** Initialization constructor. */
	FHistogramShaderTextureCompute(const ShaderMetaType::CompiledShaderInitializerType& Initializer);
};
