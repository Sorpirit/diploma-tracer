ECHO OFF
set COMPILER_PATH="C:\VulkanSDK\1.3.280.0\Bin\glslc.exe"
%COMPILER_PATH% "%~dp0Shaders\Flat.vert" -o "%~dp0PrecompiledShaders\Flat.vert.spv"
%COMPILER_PATH% "%~dp0Shaders\Flat.frag" -o "%~dp0PrecompiledShaders\Flat.frag.spv"