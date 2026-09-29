#pragma once

#include "CoreMinimal.h"

#include "EdGraph/EdGraph.h"
#include "BlueprintCompilerExtension.h"

#include "IgnoredHandleCheckExtension.generated.h"

UCLASS()
class UIgnoredHandleCheckExtension : public UBlueprintCompilerExtension
{
	GENERATED_BODY()

public:

	virtual void ProcessBlueprintCompiled(
		const FKismetCompilerContext& CompilationContext,
		const FBlueprintCompiledData& Data) override;

	void CheckGraph(UEdGraph* Graph, const FKismetCompilerContext& CompilationContext);
};