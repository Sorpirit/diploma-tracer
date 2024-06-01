import os
import subprocess

compiler_path = "C:\\VulkanSDK\\1.3.280.0\\Bin\\glslc.exe"
compiled_shader_postfix = ".spv"

# Function to compile shader files
def compile_shader(shader_file, compiled_shader_output):
    # Replace this command with your actual shell command for compiling shaders
    compile_command = f"\"{compiler_path}\" \"{shader_file}\" -o \"{compiled_shader_output}\""
    print("Start compiling shader: " + shader_file)
    result = subprocess.Popen(compile_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    std_out, std_err = result.communicate()
    print(std_out.decode("utf-8"))
    if result.returncode != 0:
        print(std_err.decode("utf-8"))
        print(f"Failed to compile {shader_file}! Exit core: {result.returncode}")
        exit(result.returncode)

# Path to the Shaders folder relative to this script
shaders_folder = os.path.join(os.path.dirname(__file__), "Shaders")

# Path to the PrecompiledShaders folder
precompiled_folder = os.path.join(os.path.dirname(__file__), "PrecompiledShaders")

def compile_shaders_in_folder(folder_path):
    for shader_file in os.listdir(folder_path):
        shader_path = os.path.join(folder_path, shader_file)
        if os.path.isdir(shader_path):
            compile_shaders_in_folder(shader_path)
            continue
        
        shader_ext = os.path.splitext(shader_file)[1]
        precompiled_shader_path = os.path.join(precompiled_folder, shader_file + compiled_shader_postfix)

        #skip include shaders
        if shader_ext == '.glsl':
            continue

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


compile_shaders_in_folder(shaders_folder)
print("Compilation complete")