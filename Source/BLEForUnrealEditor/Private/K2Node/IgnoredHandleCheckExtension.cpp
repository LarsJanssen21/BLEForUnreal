#include "IgnoredHandleCheckExtension.h"

#include "K2Node_CallFunction.h"

void UIgnoredHandleCheckExtension::ProcessBlueprintCompiled(
	const FKismetCompilerContext& CompilationContext,
	const FBlueprintCompiledData& Data)
{
	for (UEdGraph* Graph : CompilationContext.Blueprint->FunctionGraphs)
	{
		CheckGraph(Graph, CompilationContext);
	}
	for (UEdGraph* Graph : CompilationContext.Blueprint->UbergraphPages)
	{
		CheckGraph(Graph, CompilationContext);
	}
}

void UIgnoredHandleCheckExtension::CheckGraph(UEdGraph* Graph, const FKismetCompilerContext& CompilationContext)
{
	for (UEdGraphNode* Node : Graph->Nodes)
	{
		UK2Node_CallFunction* CallNode = Cast<UK2Node_CallFunction>(Node);
		if (!CallNode)
		{
			continue;
		}

		UFunction* Func = CallNode->GetTargetFunction();
		if (Func && Func->HasMetaData(TEXT("ReturnValueShouldBeUsed")))
		{
			UEdGraphPin* ReturnPin = CallNode->GetReturnValuePin();
			if (ReturnPin->LinkedTo.Num() == 0)
			{
				CompilationContext.MessageLog.Error(
					*FString::Printf(TEXT("@@: Return value of %s is being discarded"), *Func->GetName()),
					CallNode
				);
			}
		}
	}
}