/****************************************************
* (c) 2017, Zuzeng Lin
*           zuzeng@kth.se
****************************************************
*/

#include "stdafx.h"
#include "mirror.h"


WSADATA wsaData;
WORD sockVersion;
SOCKET serSocket;
sockaddr_in serAddr;
sockaddr_in remoteAddr;
int port;
bool terminating = false;
std::thread udploop;

int call_from_thread() {
	sockVersion = MAKEWORD(2, 2);
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return 0;
	}

	serSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serSocket == INVALID_SOCKET)
	{
		return 0;
	}

	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(port);
	serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(serSocket, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		closesocket(serSocket);
		return 0;
	}
	int nAddrLen = sizeof(remoteAddr);
	char recvData[512];
	while (true) {
		int nAddrLen = sizeof(remoteAddr);
		char recvData[512];
		int ret = recvfrom(serSocket, recvData, 512, 0, (sockaddr *)&remoteAddr, &nAddrLen);
		if (ret > 0)
		{
			recvData[ret] = 0x00;
			//printf("%s \r\n", inet_ntoa(remoteAddr.sin_addr));	
			bool set = true;
			int offset;
			char *data = recvData;
			for (int i = 0; i < TOTAL_NR_OF_CHANNELS; i++) {
				unsigned int recitmp;
				if (sscanf(data, "%d%n", &recitmp, &offset) == 1) {
					data += offset;
					if (recitmp <= MAX_AMPLITUDE) {
						voltage[i] = recitmp;
					}
					else {
						if (recitmp == 9999) {
							PINFO("TERMINATING MAGIC FOUND.");
							terminating = true;
						}
						else {
							voltage[i] = MAX_AMPLITUDE;
							PINFO("[ERROR] MAX_AMPLITUDE exceeded at ch %d, setting to max.", i);
							PINFO("%s", recvData);
						}
						break;
					}
				}
				else {
					PINFO("[ERROR] partial data in a packet.");
					set = false;
					break;
				}
			
			} 
			if (set) {
				set_mirror();
				Sleep(10);
			}

			char sendData[512];
			sendData[0] = 0x00;
			for (int i = 0; i < TOTAL_NR_OF_CHANNELS; i++)
				sprintf(sendData + strlen(sendData), "%d ", voltage[i]);
			sprintf(sendData + strlen(sendData), "\n");
			sendto(serSocket, sendData, strlen(sendData), 0, (sockaddr *)&remoteAddr, nAddrLen);
		}
		if (terminating) break;
	}
	closesocket(serSocket);
	WSACleanup();
}

int main(int argc, char *argv[])
{
	int devindex;
	char mess[255];
	PINFO("OKO Mirror Controller")
	if (argc <= 1)
		port = 8888;
	else
		sscanf(argv[1], "%d", &port);
	if (argc <= 2) {
		printf("Input device id for Mirror DAC: ");
		scanf("%d", &devindex);	
	}
	else
		sscanf(argv[2], "%d", &devindex);

	if (!init_dac(devindex)) {
		PERROR("Mirror DAC not found.")
	}
	PINFO("receiving command from UDP port: %d", port)
	PINFO("max voltage: %d", MAX_AMPLITUDE);
	PINFO("total chn: %d", TOTAL_NR_OF_CHANNELS);
	udploop = std::thread(call_from_thread);
	udploop.join();
	PERROR("de-init mirror.")
	close_dac();
	
	return 0;
}

