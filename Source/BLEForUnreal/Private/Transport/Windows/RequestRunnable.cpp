#include "RequestRunnable.h"

FWindowsRequestRunnable::FWindowsRequestRunnable()
{
	WakeEvent = FPlatformProcess::GetSynchEventFromPool(false);
	Thread.Reset(FRunnableThread::Create(this, TEXT("BLETransportWindows Request Thread")));
}

FWindowsRequestRunnable::~FWindowsRequestRunnable()
{
	Stop();
	Thread->WaitForCompletion();
	FPlatformProcess::ReturnSynchEventToPool(WakeEvent);
}

uint32 FWindowsRequestRunnable::Run()
{
	while (!bStop)
	{
		WakeEvent->Wait();

		// Handle next request
		while (!bStop && 0/* Still requests left*/)
		{
			// Process request
		}
	}

	return 0;
}

void FWindowsRequestRunnable::Stop()
{
	bStop = true;
	WakeEvent->Trigger();
}