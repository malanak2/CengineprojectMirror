mkdir resources/generated/shaders/spirv
slangc resources/shader/basic.slang `
-no-mangle `
-fvk-use-gl-layout `
-o resources/generated/shaders/spirv/basic.spv