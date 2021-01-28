#pragma once
#include <Runtime\Engine\Classes\Engine\TextureRenderTarget2D.h>

struct FShaderPixelCopyParameter
{
public:
    
    UTextureRenderTarget2D* RenderTarget;

    FShaderPixelCopyParameter() { }

    FShaderPixelCopyParameter(UTextureRenderTarget2D* InRenderTarget)
    {
        RenderTarget = InRenderTarget;
    }


};

/**************************************************************************************/
/* This is just an interface we use to keep all the pixel shading code in one file.   */
/**************************************************************************************/

class FCopyTexturePixelShader
{
public:

    static void DrawToRenderTarget_RenderThread(FRHICommandListImmediate& RHICmdList, const FShaderPixelCopyParameter& DrawParameters, FShaderResourceViewRHIRef InputTextureRT);

};