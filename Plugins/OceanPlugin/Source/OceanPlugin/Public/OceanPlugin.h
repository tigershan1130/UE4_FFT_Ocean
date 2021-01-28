// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FOceanPluginModule : public IModuleInterface
{
public:

	///** IModuleInterface implementation */
	//virtual void StartupModule() override;
	//virtual void ShutdownModule() override;

public:
	static inline FOceanPluginModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FOceanPluginModule>("OceanPlugin");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("OceanPlugin");
	}
};
