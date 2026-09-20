#include "PreRTS.h"
#include "GameNetwork/NetworkInterface.h"

NetworkInterface *TheNetwork = NULL;

NetworkInterface *NetworkInterface::createNetwork()
{
	// Network transport remains unsupported in the explicit M20 offline profile.
	// Do not construct the Win32 provider or open any socket at this boundary.
	return NULL;
}
