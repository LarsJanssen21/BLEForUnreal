#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"

class FWindowsRequestRunnable : public FRunnable
{
public:
	FWindowsRequestRunnable();
	virtual ~FWindowsRequestRunnable() override;

	virtual uint32 Run() override;
	virtual void Stop() override;

private:
	FEvent* WakeEvent = nullptr;
	TUniquePtr<FRunnableThread> Thread = nullptr;
	FThreadSafeBool bStop = false;

	TQueue<int> RequestQueue;
	TArray<int> Results;
};
