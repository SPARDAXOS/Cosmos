#include "Network.h"




int main([[maybe_unused]] int* argc, [[maybe_unused]] char** argv) {


	Network NetworkObject;
	UDP_Socket Socket;
	IP_Address MyAddress;
	if (!Socket.Open(MyAddress)) {
		printf("Failed to open udp socket!");
		return 1;
	}
	Socket.GetAddress(MyAddress);
	printf("Successfully opened a udp socket!");

	return 0;
}