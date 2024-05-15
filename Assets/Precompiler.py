import os
import subprocess

compiler_path = "C:\\VulkanSDK\\1.3.280.0\\Bin\\glslc.exe"
compiled_shader_postfix = ".spv"

# Function to compile shader files
def compile_shader(shader_file, compiled_shader_output):
    # Replace this command with your actual shell command for compiling shaders
    compile_command = f"\"{compiler_path}\" \"{shader_file}\" -o \"{compiled_shader_output}\""
    print("Start compiling shader: " + shader_file)
    subprocess.run(compile_command, shell=True)

# Path to the Shaders folder relative to this script
shaders_folder = os.path.join(os.path.dirname(__file__), "Shaders")

# Path to the PrecompiledShaders folder
precompiled_folder = os.path.join(os.path.dirname(__file__), "PrecompiledShaders")

# Check each shader file in the Shaders folder
for shader_file in os.listdir(shaders_folder):
    shader_path = os.path.join(shaders_folder, shader_file)
    precompiled_shader_path = os.path.join(precompiled_folder, shader_file + compiled_shader_postfix)

    # Check if the shader file exists in PrecompiledShaders folder
    if os.path.exists(precompiled_shader_path):
        # Compare modification times
        shader_mtime = os.path.getmtime(shader_path)
        precompiled_mtime = os.path.getmtime(precompiled_shader_path)

        # If shader file is newer than precompiled shader, compile it
        if shader_mtime > precompiled_mtime:
            compile_shader(shader_path, precompiled_shader_path)
            print(f"Compiled {shader_file}")
        else:
            print(f"Up-to-date {shader_file}")
    else:
        # If precompiled shader doesn't exist, compile shader
        compile_shader(shader_path, precompiled_shader_path)
        print(f"Compiled {shader_file}")

print("Compilation complete")