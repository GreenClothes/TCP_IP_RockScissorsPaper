#include "Common.h"

char *SERVERIP = (char *)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    50

enum { NO_WIN = 0, FIRST_WIN = 1, SECOND_WIN = 2 };
enum { GAWI = 0, BAWI = 1, BO = 2 };

// 게임 시작 단계 확인 변수
int error = 0;

// 패킷 구조
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
// 사용자 정의 데이터 수신 함수
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

	// 명령행 인수가 있으면 IP 주소로 사용
	if (argc > 1) SERVERIP = argv[1];

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
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

	// 게임 시작 요청
	send_packet.option = 1;

	retval = send(sock, (char*)&send_packet, sizeof(send_packet), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		error = 1;
	}

	// 게임 시작 요청 승인 수신
	retval = recv(sock, (char*)&rs_PACKET1, sizeof(PACKET), 0);
	if (retval == SOCKET_ERROR) {
		err_display("recv()");
		error = 1;
	}

	rs_PACKET2 = (PACKET*)&rs_PACKET1;

	// 승인되지 않으면 연결 X
	if (rs_PACKET2->option != 1) {
		error = 1;
	}

	// 서버와 데이터 통신
	while (error == 0) {
		int gbb;
		printf("가위 바위 보, 묵찌빠 게임!!\n");
		printf("보기에서 선택하세요!(가위=0 바위=1 보=2 승률확인=3 게임 종료=4)\n");
		scanf_s("%d", &gbb);
		while (gbb < 0 || gbb>4)
		{
			printf("잘못입력하셨습니다.\n");
			printf("보기에서 선택하세요!(가위=0 바위=1 보=2 승률확인=3 게임 종료=4)\n");
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

		// 데이터 보내기
		retval = send(sock, (char*)&send_packet, sizeof(send_packet), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}

		// 데이터 받기
		retval = recv(sock, (char*)&rs_PACKET1, sizeof(PACKET), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

		rs_PACKET2 = (PACKET*)&rs_PACKET1;

		// 결과 출력
		if (rs_PACKET2->option == 0 || rs_PACKET2->option == 4) {
			printf("%s\n", rs_PACKET2->data);
			option_cache = rs_PACKET2->option;
		}
		else if (rs_PACKET2->option == 2) {
			printf("승률 : %s %%\n", rs_PACKET2->data);
		}
		else if (rs_PACKET2->option == 3) {
			printf("승률 : %s %%\n", rs_PACKET2->data);
			break;
		}

	}

	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
