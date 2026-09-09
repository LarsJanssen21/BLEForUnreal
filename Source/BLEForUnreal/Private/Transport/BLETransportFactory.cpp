#pragma once

#include "IBLETransport.h"

#if PLATFORM_WINDOWS
#include "Windows/BLETransport_Win.h"
#endif

TUniquePtr<IBLETransport> IBLETransport::CreatePlatformTransport()
{
#if PLATFORM_WINDOWS
	return MakeUnique<BLETransportWindows>();
#else
	static_assert(false, "no BLE Transport implementation for this platform");
	return nullptr;
#endif
}