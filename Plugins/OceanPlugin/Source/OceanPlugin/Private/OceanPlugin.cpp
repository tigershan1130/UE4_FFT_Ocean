// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "OceanPlugin.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FOceanPluginModule"

//void FOceanPluginModule::StartupModule()
//{
//	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
//	FString PluginBaseDirectory = IPluginManager::Get().FindPlugin(FString(TEXT("OceanPlugin")))->GetBaseDir();
//	FString ShaderDiretory = FPaths::Combine(PluginBaseDirectory, TEXT("Shaders"));
//
//	FString VirtualShaderDirectory = FString::Printf(TEXT("/Plugin/%s"), *FString(TEXT("OceanPlugin")));
//	AddShaderSourceDirectoryMapping(VirtualShaderDirectory, ShaderDiretory);
//}
//
//void FOceanPluginModule::ShutdownModule()
//{
//	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
//	// we call this function before unloading the module.
//	ResetAllShaderSourceDirectoryMappings();
//}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOceanPluginModule, OceanPlugin)