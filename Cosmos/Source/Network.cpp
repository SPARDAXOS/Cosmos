#include "Network.h"


bool IP_Address::GetLocalAddresses(std::vector<IP_Address>& addresses) {

	DWORD Size = 0;
	GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &Size);

	IP_ADAPTER_ADDRESSES* AdapterAddresses = (IP_ADAPTER_ADDRESSES*)calloc(1, Size);
	GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, AdapterAddresses, &Size);

	for (IP_ADAPTER_ADDRESSES* Iter = AdapterAddresses; Iter != NULL; Iter = Iter->Next) {
		if (Iter->OperStatus == IfOperStatusUp && (Iter->IfType == IF_TYPE_ETHERNET_CSMACD || Iter->IfType == IF_TYPE_IEEE80211)) {
			for (IP_ADAPTER_UNICAST_ADDRESS* UA = Iter->FirstUnicastAddress; UA != NULL; UA = UA->Next) {
				char Addrstr[1024] = {};
				getnameinfo(UA->Address.lpSockaddr, UA->Address.iSockaddrLength, Addrstr, sizeof(Addrstr), NULL, 0, AI_NUMERICHOST);

				if (UA->Address.lpSockaddr->sa_family == AF_INET) {
					sockaddr_in AI = *(sockaddr_in*)UA->Address.lpSockaddr;
					IP_Address Address;
					Address.m_Host = ntohl(AI.sin_addr.s_addr);
					Address.m_Port = ntohs(AI.sin_port);
					addresses.push_back(Address);
				}
			}
		}
	}


	free(AdapterAddresses);

	return !addresses.empty();
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





UDP_Socket::UDP_Socket()
	: m_Handle(INVALID_SOCKET)
{
}

bool UDP_Socket::Valid() {

	return m_Handle != INVALID_SOCKET;
}

bool UDP_Socket::Open(const IP_Address& address) {

	SOCKET fd = ::socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == INVALID_SOCKET)
		return false;

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(address.m_Port);
	addr.sin_addr.s_addr = htonl(address.m_Host);
	if (::bind(fd, (const sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		closesocket(fd);
		return false;
	}

	u_long non_blocking = 1;
	if (ioctlsocket(fd, FIONBIO, &non_blocking) == SOCKET_ERROR) {
		closesocket(fd);
		return false;
	}

	m_Handle = fd;

	return Valid();
}

void UDP_Socket::Close() {
	if (Valid())
		closesocket(m_Handle);

	m_Handle = INVALID_SOCKET;
}



bool UDP_Socket::Send(const IP_Address& address, const ByteStream& stream) {

	if (!Valid())
		return false;

	const int32 Length = stream.m_Size;
	const char* Data = (const char*)stream.m_Data;

	sockaddr_in Addr{};
	Addr.sin_family = AF_INET;
	Addr.sin_port = htons(address.m_Port);
	Addr.sin_addr.s_addr = htonl(address.m_Host);
	if (::sendto(m_Handle, Data, Length, 0, (const sockaddr*)&Addr, sizeof(Addr)) == SOCKET_ERROR)
		return false;

	return true;
}

bool UDP_Socket::Receive(IP_Address& address, ByteStream& stream) {

	if (!Valid())
		return false;

	sockaddr_in Addr{};
	int32 Addrlen = sizeof(Addr);
	int32 Receive = ::recvfrom(m_Handle, (char*)stream.m_Data, sizeof(stream.m_Data), 0, (sockaddr*)&Addr, &Addrlen);
	if (Receive == SOCKET_ERROR)
		return false;

	address.m_Host = htonl(Addr.sin_addr.s_addr);
	address.m_Port = htons(Addr.sin_port);
	stream.m_Size = Receive;

	return true;
}
bool UDP_Socket::GetAddress(IP_Address& address) {

	if (!Valid())
		return false;

	sockaddr_in Addr = {};
	int Addrlen = sizeof(Addr);
	if (getsockname(m_Handle, (sockaddr*)&Addr, &Addrlen) == SOCKET_ERROR)
		return false;

	address.m_Host = htonl(Addr.sin_addr.s_addr);
	address.m_Port = htons(Addr.sin_port);

	return true;
}






Network::Network() {
	auto Results = WSAStartup(MAKEWORD(2, 2), &m_Data);
	if (Results != 0) {
		printf("Failed to initialize winsock! Error: %d", Results);
		return;
	}
	printf("Winsock initialized successfully!\n");
}
Network::~Network() {
	WSACleanup();
}


