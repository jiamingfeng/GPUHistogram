using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

[RequireComponent(typeof(Camera))]
public class HistogramScript : MonoBehaviour
{

    public ComputeShader shader;
	//private Camera camera;
    private RenderTexture InputTexture;

	private RenderTexture HistogramTexture;

    private uint[] histogramData;

	//private Camera MainCamera;

	private RawImage HistogramUIImage;

    ComputeBuffer histogramBuffer;
    int handleMain;
    int handleInitialize;

	int handleTextureInit;
	int handleTextureCompute;

    void Start()
    {
        if (null == shader)
        {
            Debug.Log("Shader or input texture missing.");
            return;
        }

        handleInitialize = shader.FindKernel("HistogramInitialize");
        handleMain = shader.FindKernel("HistogramMain");
		handleTextureInit = shader.FindKernel("HistogramTextureInit");
		handleTextureCompute = shader.FindKernel("HistogramTextureCompute");

        histogramBuffer = new ComputeBuffer(256, sizeof(uint));
        histogramData = new uint[256];

        if (handleInitialize < 0 || handleMain < 0 ||
			handleTextureInit < 0 || handleTextureCompute < 0 ||
           null == histogramBuffer || null == histogramData)
        {
            Debug.Log("Initialization failed.");
            return;
        }


		HistogramTexture = new RenderTexture(256, 128, 0);			
		HistogramTexture.enableRandomWrite = true;
		HistogramTexture.Create();

        shader.SetBuffer(handleMain, "HistogramBuffer", histogramBuffer);
        shader.SetBuffer(handleInitialize, "HistogramBuffer", histogramBuffer);


		shader.SetTexture(handleTextureInit, "HistogramTexture", HistogramTexture);
		
		shader.SetBuffer(handleTextureCompute, "HistogramBuffer", histogramBuffer);
		shader.SetTexture(handleTextureCompute, "HistogramTexture", HistogramTexture);

		GameObject HistogramUI = GameObject.Find("HistogramImage");
		if ( HistogramUI == null )
		{
			Debug.LogError("Cant find raw image UI.");
		}
		else
		{
			HistogramUIImage = HistogramUI.GetComponent<RawImage>();
			HistogramUIImage.texture = HistogramTexture;
		}

    }

    void OnDestroy()
    {
        if (null != histogramBuffer)
        {
            histogramBuffer.Release();
            histogramBuffer = null;
        }

		if (null != InputTexture)
		{
			InputTexture.Release();
		}

		if (null != HistogramTexture)
		{
			HistogramTexture.Release();
		}		


    }

    // RenderTexture _rt;
    // void OnPreRender()
    // {
    //     GetComponent<Camera>().targetTexture = _rt;
	// }	

    void OnRenderImage(RenderTexture source, RenderTexture destination)
    {
        if (null == shader ||
           0 > handleInitialize || 0 > handleMain ||
           null == histogramBuffer || null == histogramData)
        {
            Debug.Log("Cannot compute histogram");
        }

        else
        {
            if (null == InputTexture || source.width != InputTexture.width
                || source.height != InputTexture.height)
            {
                if (null != InputTexture)
                {
                    InputTexture.Release();
                }

                InputTexture = new RenderTexture(source.width, source.height, 0);
                InputTexture.enableRandomWrite = true;
                bool result = InputTexture.Create();
                if (result)
                {
                    shader.SetTexture(handleMain, "InputTexture", InputTexture);
                    shader.SetFloat("level", (float)(source.width * source.height) / 16.0f);

                    Debug.LogFormat("Createdn input texture with size: {0}, {1}", InputTexture.width, InputTexture.height);

                    //MainCamera.targetTexture = inputTexture;		
                }
                else
                {
                    Debug.LogError("Failed to create input texture.");
                }
            }

			//Graphics.CopyTexture(source, InputTexture);
            Graphics.Blit(source, InputTexture);

            shader.Dispatch(handleInitialize, 256 / 64, 1, 1);
            shader.Dispatch(handleTextureInit, 256 / 32, 128 / 32, 1);
            // divided by 64 in x because of [numthreads(64,1,1)] in the compute shader code
            shader.Dispatch(handleMain, (InputTexture.width + 7) / 8, (InputTexture.height + 7) / 8, 1);
            // divided by 8 in x and y because of [numthreads(8,8,1)] in the compute shader code

            shader.Dispatch(handleTextureCompute, 256 / 32, 128 / 32, 1);
        }

		//GetComponent<Camera>().targetTexture = null;
		//Graphics.Blit(source, null as RenderTexture);
        Graphics.Blit(source, destination); // just copy		

    }
}