if (!(Test-Path -PathType Container ../out/bin)) {
    mkdir ../out/bin
}

New-Item -Path ..\out\bin\resources -ItemType SymbolicLink -Value ..\..\resources