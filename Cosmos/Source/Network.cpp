#include "Network.h"


bool IP_Address::GetLocalAddresses(std::vector<IP_Address>& addresses) {

}

IP_Address::IP_Address(const IP_Address& rhs) 
	:	m_Host(rhs.m_Host), m_Port(rhs.m_Port)
{
}
IP_Address::IP_Address(const char* name, uint16 port) 
	:   m_Host(DNS_Query(name)), m_Port(port)
{
}
IP_Address::IP_Address(const uint32 host, uint16 port) 
	: m_Host(host), m_Port(port)
{
}
IP_Address::IP_Address(uint8 a, uint8 b, uint8 c, uint8 d, uint16 port) 
	:	m_Host(K_Any_Host), m_Port(port)
{
	m_Host = uint32(a) << 24;
	m_Host |= uint32(b) << 16;
	m_Host |= uint32(c) << 8;
	m_Host |= uint32(d);
}

bool IP_Address::operator==(const IP_Address& rhs) const {
	return m_Host == rhs.m_Host && m_Port == rhs.m_Port;
}
bool IP_Address::operator!=(const IP_Address& rhs) const {
	return !((*this) == rhs);
}

uint8 IP_Address::A() const{
	return uint8(m_Host >> 24);
}
uint8 IP_Address::B() const{
	return uint8(m_Host >> 16);
}
uint8 IP_Address::C() const{
	return uint8(m_Host >> 8);
}
uint8 IP_Address::D() const{
	return uint8(m_Host & 0xff);
}






uint32 IP_Address::DNS_Query(const char* name) {

	addrinfo Hints{};
	Hints.ai_family = AF_UNSPEC;
	Hints.ai_socktype = SOCK_STREAM;
	Hints.ai_protocol = IPPROTO_TCP; //TCP

	addrinfo* Results = nullptr;
	int Resolve = getaddrinfo(name, nullptr, &Hints, &Results);
	if (Resolve == WSAHOST_NOT_FOUND)
		return 0;

	uint32 Host = 0;
	for (addrinfo* it = Results; it != nullptr; it = it->ai_next) {
		if (it->ai_family == AF_INET) {
			sockaddr_in& Addr = *(sockaddr_in*)it->ai_addr;
			Host = ntohl(Addr.sin_addr.s_addr);
			break;
		}
	}

	freeaddrinfo(Results);
	return Host;
}





Socket::Socket()
	: m_Handle(INVALID_SOCKET)
{
}

bool Socket::Valid() {

	return m_Handle != INVALID_SOCKET;
}

bool Socket::Open(const IP_Address& address) {




	return true;
}













Network::Network() {
	auto Results = WSAStartup(MAKEWORD(2, 2), &m_Data);
	if (Results != 0) {
		printf("Failed to initialize winsock! Error: %d", Results);
		return;
	}
}
Network::~Network() {
	WSACleanup();
}


