/*
 * EchoUDPProtocol.cpp
 *
 *  Created on: 2014/03/17
 *      Author: Fumiaki
 */

#include "EchoUDPProtocol.h"
#include "OpenECHO.h"

namespace sonycsl_openecho {

const int EchoUDPProtocol::UDP_MAX_PACKET_SIZE = 65507;
const int EchoUDPProtocol::PORT = 3610;

EchoUDPProtocol::EchoUDPProtocol() {
	// TODO Auto-generated constructor stub
	mReceiverSock = 0;

}

EchoUDPProtocol::~EchoUDPProtocol() {
	// TODO Auto-generated destructor stub
}

void EchoUDPProtocol::openUDP() {
	mReceiverSock = socket(AF_INET, SOCK_DGRAM, 0);

	mReceiverSockAddr.sin_family = AF_INET;
	mReceiverSockAddr.sin_port = htons(PORT);
	mReceiverSockAddr.sin_addr.s_addr = INADDR_ANY;

	bind(mReceiverSock, (struct sockaddr *) &mReceiverSockAddr,
			sizeof(mReceiverSockAddr));

	int val = 1;
	ioctl(mReceiverSock, FIONBIO, &val);

	memset(&mReceiverIpMreq, 0, sizeof(mReceiverIpMreq));
	mReceiverIpMreq.imr_interface.s_addr = INADDR_ANY;
	mReceiverIpMreq.imr_multiaddr.s_addr = inet_addr(
			EchoSocket::MULTICAST_ADDRESS.c_str());

	u_char loop = 0; // 0 = invalid, 1 = valid(default);
	if (setsockopt(mReceiverSock, IPPROTO_IP, IP_MULTICAST_LOOP, &loop,
			sizeof(loop)) != 0) {
		perror("EchoUDPProtocol::open()[IP_MULTICAST_LOOP]");
		return;
	}

	in_addr_t ipaddr = inet_addr(EchoSocket::SELF_ADDRESS.c_str());
	if (setsockopt(mReceiverSock, IPPROTO_IP, IP_MULTICAST_IF, (char *) &ipaddr,
			sizeof(ipaddr)) != 0) {
		perror("EchoUDPProtocol::open()[IP_MULTICAST_IF]");
		return;
	}

	if (setsockopt(mReceiverSock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
			(char *) &mReceiverIpMreq, sizeof(mReceiverIpMreq)) != 0) {
		perror("EchoUDPProtocol::open()[IP_ADD_MEMBERSHIP]");
		return;
	}

}

void EchoUDPProtocol::closeUDP() {

	close(mReceiverSock);
}

void EchoUDPProtocol::sendUDP(EchoFrame& frame) {
	if (frame.getDstEchoAddress() == EchoSocket::SELF_ADDRESS) {
		Echo::getEventListenerDelegate().onSendFrame(frame);

		sendToSelf(frame);
		return;
	}
	int sock;
	struct sockaddr_in addr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = inet_addr(frame.getDstEchoAddress().c_str());

	/*
	if(frame.getDstEchoAddress() == EchoSocket::MULTICAST_ADDRESS) {
		in_addr_t ipaddr = inet_addr(EchoSocket::SELF_ADDRESS.c_str());

		if (setsockopt(sock,
				IPPROTO_IP,
				IP_MULTICAST_IF,
				(char *)&ipaddr, sizeof(ipaddr)) != 0) {
			perror("sendUDPFrame");
			return;
		}
	}
	*/


	std::vector<unsigned char> byteArray = frame.getFrameByteArray();
	int size = (int)byteArray.size();
	//unsigned char buffer[size];
	unsigned char* buffer = (unsigned char*)malloc(size);
	for (int i = 0; i < size; i++) {
		buffer[i] = byteArray.at(i);
	}

	sendto(sock, buffer, byteArray.size(), 0, (struct sockaddr *) &addr,
			sizeof(addr));

	//std::cerr << "sendUDPFrame:" << std::hex;
	//for (int i = 0; i < size; i++) {
	//	std::cerr << (int) (buffer[i]) << " ";
	//}
	//std::cerr << ":" << frame.getDstEchoAddress() << std::endl;

	close(sock);
	Echo::getEventListenerDelegate().onSendFrame(frame);

	if (frame.getDstEchoAddress() == EchoSocket::MULTICAST_ADDRESS) {
		EchoFrame f = frame;
		f.setDstEchoAddress(EchoSocket::SELF_ADDRESS);
		sendToSelf(f);
	}
	free(buffer);
}

void EchoUDPProtocol::sendToSelf(EchoFrame& frame) {

	Echo::getEventListenerDelegate().onReceiveFrame(frame);
	std::shared_ptr<EchoTask> task(
			(EchoTask*) (new UDPProtocolTask(frame, *this)));
	EchoSocket::enqueueTask(task);
}

void EchoUDPProtocol::receive() {

	int size;
	unsigned char buffer[UDP_MAX_PACKET_SIZE];

	struct sockaddr_in from;
	unsigned int addrSize = sizeof(struct sockaddr_in);
	memset(buffer, 0, sizeof(buffer));
	size = (int)recvfrom(mReceiverSock, buffer, sizeof(buffer), 0,
			(struct sockaddr *) &from, &addrSize);
	if (size < EchoFrame::MIN_FRAME_SIZE) {
		return;
	}
	std::vector<unsigned char> data;
	for (int i = 0; i < size; i++) {
		data.push_back(buffer[i]);
	}

	const char* caddress = inet_ntoa(from.sin_addr);
	std::string address = caddress;

	//std::cerr << "receiveUDPFrame:" << std::hex;
	//for (int i = 0; i < size; i++) {
	//	std::cerr << (int) (buffer[i]) << " ";
	//}
	//std::cerr << ":" << address << std::endl;

	// check address
	/*
	 int fd;
	 struct ifreq ifr;
	 fd = socket(AF_INET, SOCK_DGRAM, 0);
	 ifr.ifr_addr.sa_family = AF_INET;
	 strncpy(ifr.ifr_name, "en1", IFNAMSIZ-1);
	 ioctl(fd, SIOCGIFADDR, &ifr);
	 close(fd);
	 const char* selfCaddress = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
	 std::string selfAddress = selfCaddress;
	 if(address == selfAddress) {
	 return;
	 }*/
	if (isLoopback(address)) {
		return;
	}

	EchoFrame frame(address, data);
	Echo::getEventListenerDelegate().onReceiveFrame(frame);

	std::shared_ptr<EchoTask> task(
			(EchoTask*) (new UDPProtocolTask(frame, *this)));
	EchoSocket::enqueueTask(task);

}

bool EchoUDPProtocol::isLoopback(std::string address) {
	struct ifaddrs * ifAddrStruct = NULL;
	struct ifaddrs * ifa = NULL;
	void * tmpAddrPtr = NULL;

	getifaddrs(&ifAddrStruct);

	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
			// is a valid IP4 Address
			tmpAddrPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			//printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
			std::string selfAddress = addressBuffer;
			if (address == selfAddress) {

				if (ifAddrStruct != NULL)
					freeifaddrs(ifAddrStruct);
				return true;
			}
		} else if (ifa->ifa_addr->sa_family == AF_INET6) { // check it is IP6
			// is a valid IP6 Address
			tmpAddrPtr = &((struct sockaddr_in6 *) ifa->ifa_addr)->sin6_addr;
			char addressBuffer[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
			//printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
			std::string selfAddress = addressBuffer;

			if (address == selfAddress) {

				if (ifAddrStruct != NULL)
					freeifaddrs(ifAddrStruct);
				return true;
			}
		}
	}
	if (ifAddrStruct != NULL)
		freeifaddrs(ifAddrStruct);
	return false;
}

UDPProtocolTask::UDPProtocolTask(EchoFrame frame, EchoUDPProtocol& protocol) :
		EchoTask(frame), mProtocol(protocol) {
}

UDPProtocolTask::~UDPProtocolTask() {
}

void UDPProtocolTask::respond(EchoFrame& response) {
	mProtocol.sendUDP(response);
}

void UDPProtocolTask::informAll(EchoFrame& response) {
	mProtocol.sendUDP(response);
}

}
;
