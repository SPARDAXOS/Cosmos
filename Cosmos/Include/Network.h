#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>

#include <stdio.h>
#include "Utility.h"
#include <vector>

#ifndef NETWORK_HPP
#define NETWORK_HPP

static uint16 SERVER_PORT = 21435;

struct ByteStream {


	int32 m_Size = 0;
	uint8 m_Data[1024] = {}; //1024 x 8 = 8192 bits - Must be divisible by 4.
};
struct ByteStreamWriter : public ByteStream {

};
struct ByteStreamReader : public ByteStream {

};



struct IP_Address {
	static bool GetLocalAddresses(std::vector<IP_Address>& addresses);

	static constexpr uint32 K_Any_Host = 0;
	static constexpr uint32 K_Any_Port = 0;
	static uint32 DNS_Query(const char* name);

	IP_Address() = default;
	IP_Address(const IP_Address& rhs);
	IP_Address(const char* name, uint16 port = K_Any_Port);
	IP_Address(const uint32 host, uint16 port = K_Any_Port);
	IP_Address(uint8 a, uint8 b, uint8 c, uint8 d, uint16 port = K_Any_Port);

	bool operator==(const IP_Address& rhs) const;
	bool operator!=(const IP_Address & rhs) const;

	uint8 A() const;
	uint8 B() const;
	uint8 C() const;
	uint8 D() const;

	uint32 m_Host{ K_Any_Host };
	uint16 m_Port{ K_Any_Port };
};

struct Socket {


	uint64 m_Handle;
};
struct UDP_Socket {

	UDP_Socket();

	bool Valid();
	bool Open(const IP_Address& address);
	void Close();

	bool Send(const IP_Address& address, const ByteStream& stream);
	bool Receive(IP_Address& address, ByteStream& stream);

	bool GetAddress(IP_Address& address);

	uint64 m_Handle;
};
struct TCP_Socket : Socket {



};





struct Network {


	Network();
	~Network();


	WSADATA m_Data{};
};



#endif // !NETWORK_HPP
