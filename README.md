# UE4-FFT-Ocean
An Compute Shader 4.26 updated FFT Ocean Simulation In UE4

Video:
https://www.youtube.com/watch?v=UjRsnwchF0E&lc=UgyNedXensVkL1YMsJ94AaABAg.9O2Py8LA5yG9iSQ_TLQqQE

The plugin includes:
- Set of shaders that perform frequency Specturm, IFFT and Normal in Compute Shader and Pixel Shaders.
- Tessellation Enabled

Todos:
- maybe combine Quadtree with FFT for infinite Render
- normal needs to be in tangent space.
- add bouyancy physics
- add interactive height field wave simulation with render texture and compute shader

![alt text](https://github.com/tigershan1130/UE4_FFT_Ocean/blob/main/HighresScreenshot00000.png)

Part of this project code is also been used alongside with newest most popular ocean plugin in UE marketplace
Oceanology 6 update. If you want something that's more commerical: 
![image](https://user-images.githubusercontent.com/39791762/202949884-3b0d3246-2f3f-4cca-8eb2-95e80f1002b0.png)


https://www.unrealengine.com/marketplace/en-US/product/oceanology?sessionInvalidated=true

References:
1. Tessendorf, Jerry. 2001 Simulating Ocean Water. In SIGGRAPH 2002 Course Notes #9 (Simulating Nature: Realistic and Interactive Techniques), ACM.
2. Christopher J. Horvath. 2015. Empirical directional wave spectra for computer graphics. In Proceedings of the 2015 Symposium on Digital Production (DigiPro '15). ACM.
