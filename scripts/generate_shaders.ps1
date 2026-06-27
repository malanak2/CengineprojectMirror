$shaderDir = "resources/shader"
$spirvDir  = "resources/generated/shaders/spirv"

if (Test-Path -Path $spirvDir) {
    Remove-Item -Path $spirvDir -Recurse -Force
}
New-Item -Path $spirvDir -ItemType Directory -Force | Out-Null

if (-not (Test-Path -Path $shaderDir)) {
    Write-Error "Source directory '$shaderDir' does not exist!"
    exit
}

Get-ChildItem -Path $shaderDir -File | ForEach-Object {
    $filename = $_.BaseName

    $inputFile  = $_.FullName
    $outputFile = "$spirvDir/$filename.spv"

    Write-Host "Compiling: $_.Name -> $filename.spv"

    & slangc $inputFile `
        -no-mangle `
        -fvk-use-gl-layout `
        -o $outputFile
}

Write-Host "Shader compilation complete!" -ForegroundColor Green