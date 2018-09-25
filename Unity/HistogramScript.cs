using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

[RequireComponent(typeof(Camera))]
public class HistogramScript : MonoBehaviour
{

    public ComputeShader shader;
	//private Camera camera;
    public RenderTexture inputTexture;

	public RenderTexture HistogramTexture;

    public uint[] histogramData;

	private Camera myCam;

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

		myCam = gameObject.GetComponent<Camera>();

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

		if (null != inputTexture)
		{
			inputTexture.Release();
		}

		if (null != HistogramTexture)
		{
			HistogramTexture.Release();
		}		


    }


    void Update()
    {
        if (null == shader ||
           0 > handleInitialize || 0 > handleMain ||
           null == histogramBuffer || null == histogramData)
        {
            Debug.Log("Cannot compute histogram");
            return;
        }

        if (null == inputTexture || myCam.pixelWidth != inputTexture.width
             || myCam.pixelHeight != inputTexture.height)
        {
            if (null != inputTexture)
            {
                inputTexture.Release();
            }

            inputTexture = new RenderTexture(myCam.pixelWidth, myCam.pixelHeight, 0);
			inputTexture.enableRandomWrite = true;
            bool result = inputTexture.Create();
			if ( result )
			{
				shader.SetTexture(handleMain, "InputTexture", inputTexture);
				shader.SetFloat("level", (float)(myCam.pixelWidth * myCam.pixelHeight) / 16.0f);

				Debug.LogFormat("Createdn input texture with size: {0}, {1}",inputTexture.width, inputTexture.height );

				myCam.targetTexture = inputTexture;		
			}
			else{
				Debug.LogError("Failed to create input texture.");				
			}
		}

		myCam.Render();

        // }

		// shader.SetTexture(handleMain, "InputTexture", inputTexture);


        shader.Dispatch(handleInitialize, 256 / 64, 1, 1);
		shader.Dispatch(handleTextureInit, 256 / 32, 128 / 32, 1);
        // divided by 64 in x because of [numthreads(64,1,1)] in the compute shader code
        shader.Dispatch(handleMain, (inputTexture.width + 7) / 8, (inputTexture.height + 7) / 8, 1);
        // divided by 8 in x and y because of [numthreads(8,8,1)] in the compute shader code

		shader.Dispatch(handleTextureCompute,  256 / 32, 128 / 32,  1);

        //histogramBuffer.GetData(histogramData);


        //Graphics.Blit(source, destination);
		HistogramUIImage.texture = HistogramTexture;
    }
}