$shaderDir = "../resources/shader"
$spirvDir = "../resources/generated/shaders/spirv"

# Remove existing output directory if it exists
if (Test-Path -Path $spirvDir) {
    Remove-Item -Path $spirvDir -Recurse -Force
}

# Create output directory
New-Item -Path $spirvDir -ItemType Directory -Force | Out-Null

# Verify source directory exists
if (-not (Test-Path -Path $shaderDir -PathType Container)) {
    Write-Error "Error: Source directory '$shaderDir' does not exist!"
    exit 1
}

# Iterate over files in the shader directory
Get-ChildItem -Path $shaderDir -File | ForEach-Object {
    $inputFile = $_.FullName
    $basenameWithExt = $_.Name
    $filename = $_.BaseName

    $outputFile = Join-Path -Path $spirvDir -ChildPath "$filename.spv"

    Write-Host "Compiling: $basenameWithExt -> $filename.spv"

    # Step 1: Compile shader via Slang
    slangc $inputFile `
        -no-mangle `
        -fvk-use-gl-layout `
        -o $outputFile

    # Step 2: Disassemble, patch BuiltIn string, and re-assemble SPIR-V
    $disassembled = spirv-dis $outputFile
    $patched = $disassembled -replace 'BuiltIn InstanceIndex', 'BuiltIn InstanceId'

    $patched | spirv-as --target-env spv1.5 - -o $outputFile
}

Write-Host "Shader compilation complete!" -ForegroundColor Green

