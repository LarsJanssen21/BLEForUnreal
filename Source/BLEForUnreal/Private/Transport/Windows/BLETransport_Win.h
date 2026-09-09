#pragma once

#include "IBLETransport.h"

class BLETransportWindows : public IBLETransport
{
public:
	BLETransportWindows();
	virtual ~BLETransportWindows() override;

	virtual void StartScan() override;
	virtual void StopScan() override;
};