// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShaderDirectory.h"

#define LOCTEXT_NAMESPACE "FShaderDirectoryModule"

void FShaderDirectoryModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FString ShaderDir = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("ShaderDirectory/Shaders"));
	AddShaderSourceDirectoryMapping("/Plugin/ShaderDirectory", ShaderDir);
}

void FShaderDirectoryModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FShaderDirectoryModule, ShaderDirectory)