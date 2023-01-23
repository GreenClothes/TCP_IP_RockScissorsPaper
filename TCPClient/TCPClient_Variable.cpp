#include "Common.h"

char *SERVERIP = (char *)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    50

enum { NO_WIN = 0, FIRST_WIN = 1, SECOND_WIN = 2 };
enum { GAWI = 0, BAWI = 1, BO = 2 };

// ���� ���� �ܰ� Ȯ�� ����
int error = 0;

// ��Ŷ ����
#pragma pack(1)
typedef struct _PACKET
{
	int size;
	int totalsize;
	int option;
	int gbb;
	char data[15];
}PACKET;
#pragma pack()

// Internet Inference
// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char* buf, int len, int flags)
{
	int received;
	char* ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

int main(int argc, char *argv[])
{
	int retval;

	// ����� �μ��� ������ IP �ּҷ� ���
	if (argc > 1) SERVERIP = argv[1];

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// Inference code
	int totalsize;
	int size;
	int result;
	char msg[BUFSIZE];
	int option_cache = 0;

	PACKET send_packet;
	PACKET rs_PACKET1;
	PACKET* rs_PACKET2;

	// ���� ���� ��û
	send_packet.option = 1;

	retval = send(sock, (char*)&send_packet, sizeof(send_packet), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		error = 1;
	}

	// ���� ���� ��û ���� ����
	retval = recv(sock, (char*)&rs_PACKET1, sizeof(PACKET), 0);
	if (retval == SOCKET_ERROR) {
		err_display("recv()");
		error = 1;
	}

	rs_PACKET2 = (PACKET*)&rs_PACKET1;

	// ���ε��� ������ ���� X
	if (rs_PACKET2->option != 1) {
		error = 1;
	}

	// ������ ������ ���
	while (error == 0) {
		int gbb;
		printf("���� ���� ��, ����� ����!!\n");
		printf("���⿡�� �����ϼ���!(����=0 ����=1 ��=2 �·�Ȯ��=3 ���� ����=4)\n");
		scanf_s("%d", &gbb);
		while (gbb < 0 || gbb>4)
		{
			printf("�߸��Է��ϼ̽��ϴ�.\n");
			printf("���⿡�� �����ϼ���!(����=0 ����=1 ��=2 �·�Ȯ��=3 ���� ����=4)\n");
			scanf_s("%d", &gbb);
		}

		if (gbb == 3) {
			send_packet.option = 2;
			send_packet.gbb = 3;
		}
		else if (gbb == 4) {
			send_packet.option = 3;
			send_packet.gbb = 4;
		}
		else {
			send_packet.option = option_cache;
			send_packet.gbb = gbb;
		}
		send_packet.size = sizeof(int);
		send_packet.totalsize = sizeof(int) * 3;

		// ������ ������
		retval = send(sock, (char*)&send_packet, sizeof(send_packet), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}

		// ������ �ޱ�
		retval = recv(sock, (char*)&rs_PACKET1, sizeof(PACKET), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

		rs_PACKET2 = (PACKET*)&rs_PACKET1;

		// ��� ���
		if (rs_PACKET2->option == 0 || rs_PACKET2->option == 4) {
			printf("%s\n", rs_PACKET2->data);
			option_cache = rs_PACKET2->option;
		}
		else if (rs_PACKET2->option == 2) {
			printf("�·� : %s %%\n", rs_PACKET2->data);
		}
		else if (rs_PACKET2->option == 3) {
			printf("�·� : %s %%\n", rs_PACKET2->data);
			break;
		}

	}

	// ���� �ݱ�
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}
