// Copyright Epic Games, Inc. All Rights Reserved.

#include "BLEForUnrealEditor.h"

#define LOCTEXT_NAMESPACE "FBLEForUnrealEditorModule"

#include "BlueprintCompilationManager.h"

#include "K2Node/IgnoredHandleCheckExtension.h"

void FBLEForUnrealEditorModule::StartupModule()
{
	UIgnoredHandleCheckExtension* Extension = NewObject<UIgnoredHandleCheckExtension>(
	);
	FBlueprintCompilationManager::RegisterCompilerExtension(
		UBlueprint::StaticClass(),
		Extension
	);

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FBLEForUnrealEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBLEForUnrealEditorModule, BLEForUnrealEditor)