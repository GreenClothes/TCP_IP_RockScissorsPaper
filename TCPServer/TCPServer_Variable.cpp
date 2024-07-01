#include "Common.h"

#define SERVERPORT 9000
#define BUFSIZE    512

enum { NO_WIN = 0, FIRST_WIN = 1, SECOND_WIN = 2 };
enum { GAWI = 0, BAWI = 1, BO = 2 };

const char* Result_Msg[] = { "�����ϴ�!", "����!", "����!" };
const char* Result_Msg1[] = { "���", "�̰���ϴ�!", "�����ϴ�!" };
const char* Result_Msg2[] = { "���", "�����ϴ�!", "�̰���ϴ�!" };
const char* END_Msg[] = { "�����߽��ϴ�!" };
const char* winner[] = { " ", "client1", "client0" };
const char* Res_Msg[] = { "���� ��ȿ! "};

float score_0 = 0;
float score_1 = 0;
int who;

int gbb_cnt = 0;
int MJB_cnt = 0;

// ���� ���� �ܰ� Ȯ�� ����
int error = 0;

// ��Ŷ ����
#pragma pack(1)
typedef struct _PACKET
{
	int size;
	int totalsize;
	// option
	// 1 : ���� ����, 2 : �·�, 3 : ���� ����
	int option;
	int gbb;
	char data[15];
}PACKET;
#pragma pack()

int who_win(int part1, int part2)
{
	if (part1 == part2) {
		who = 0;
		return NO_WIN;
	}
	else if (part1 % 3 == (part2 + 1) % 3) {
		who = 1;
		return FIRST_WIN;
	}
	else {
		who = 2;
		return SECOND_WIN;
	}
}

int MJB(int part1, int part2, int who_atk)
{

	if ((part1 == part2) && (who_atk == 1)) {
		return FIRST_WIN;
	}
	else if ((part1 == part2) && (who_atk == 2)) {
		return SECOND_WIN;
	}
	else {
		return NO_WIN;
	}
}

// ���� ������ �Լ�
int _recv_ahead(SOCKET s, char *p)
{
	__declspec(thread) static int nbytes = 0;
	__declspec(thread) static char buf[1024];
	__declspec(thread) static char *ptr;

	if (nbytes == 0 || nbytes == SOCKET_ERROR) {
		nbytes = recv(s, buf, sizeof(buf), 0);
		if (nbytes == SOCKET_ERROR) {
			return SOCKET_ERROR;
		}
		else if (nbytes == 0)
			return 0;
		ptr = buf;
	}

	--nbytes;
	*p = *ptr++;
	return 1;
}

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

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// ������ ��ſ� ����� ����
	SOCKET client_sock[2];
	SOCKADDR_IN clientaddr[2];
	int addrlen;

	PACKET send_packet;
	PACKET cli0_packet_cache;
	PACKET cli1_packet_cache;
	PACKET* cli0_packet;
	PACKET* cli1_packet;

	while (1) {
		// client 0
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock[0] = accept(listen_sock, (struct sockaddr *)&clientaddr, &addrlen);
		if (client_sock[0] == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr[0].sin_addr, addr, sizeof(addr));
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr[0].sin_port));

		// client 1
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock[1] = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock[1] == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		inet_ntop(AF_INET, &clientaddr[1].sin_addr, addr, sizeof(addr));
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr[1].sin_port));

		// ���� ���� ��û Ȯ��
		retval = recv(client_sock[0], (char*)&cli0_packet_cache, sizeof(PACKET), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			error = 1;
		}
		cli0_packet = (PACKET*)&cli0_packet_cache;

		retval = recv(client_sock[1], (char*)&cli1_packet_cache, sizeof(PACKET), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			error = 1;
		}
		cli1_packet = (PACKET*)&cli1_packet_cache;

		// �ɼ��� 1�̸� ���� ���� ��û ����
		if((cli0_packet->option == 1) && (cli1_packet->option == 1)){
			send_packet.size = 0;
			send_packet.size = sizeof(int) * 3 + send_packet.size;
			send_packet.option = 1;

			retval = send(client_sock[0], (char*)&send_packet, sizeof(send_packet), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				error = 1;
			}

			// ���εǾ����� �˸�
			send_packet.size = 0;
			send_packet.size = sizeof(int) * 3 + send_packet.size;
			send_packet.option = 1;

			retval = send(client_sock[1], (char*)&send_packet, sizeof(send_packet), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				error = 1;
			}
		}
		// �ϳ��� �ɼ��� 1�� �ƴϸ� ��� ����
		else {
			error = 1;
		}

		// Ŭ���̾�Ʈ�� ������ ���
		while (error == 0) {
			// Ŭ���̾�Ʈ 0 ������ ����
			retval = recv(client_sock[0], (char*)&cli0_packet_cache, sizeof(PACKET), 0);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			cli0_packet = (PACKET*)&cli0_packet_cache;

			// Ŭ���̾�Ʈ 1 ������ ����
			retval = recv(client_sock[1], (char*)&cli1_packet_cache, sizeof(PACKET), 0);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			cli1_packet = (PACKET*)&cli1_packet_cache;

			//////////////////
			// �·� ���
			float a;
			float b;
			char win_rate[10];

			if ((score_0 == 0) && (score_1 == 0)) {
				a = 0;
				b = 0;
			}
			else if (score_0 == 0) {
				a = 0;
				b = 100;
			}
			else if (score_1 == 0) {
				a = 100;
				b = 0;
			}
			else {
				a = 100 * (score_0 / (score_0 + score_1));
				b = 100 * (score_1 / (score_0 + score_1));
			}

			/////////////////////////
			// �·� ���� option == 2
			while(cli0_packet->option == 2) {
				sprintf(win_rate, "%.2f", a);
				int size = strlen((char*)&win_rate);
				int totalsize = sizeof(int) * 3 + size;
				
				send_packet.option = 2;
				send_packet.size = size;
				send_packet.totalsize = totalsize;
				strcpy_s(send_packet.data, send_packet.size + 1, win_rate);

				//retval = send(client_sock[0], buf, totalsize, 0);
				retval = send(client_sock[0], (char*)&send_packet, sizeof(send_packet), 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}

				// �����
				retval = recv(client_sock[0], (char*)&cli0_packet_cache, sizeof(PACKET), 0);
				if (retval == SOCKET_ERROR) {
					err_display("recv()");
					break;
				}

				cli0_packet = (PACKET*)&cli0_packet_cache;
			}
			while(cli1_packet->option == 2) {
				sprintf(win_rate, "%.2f", b);
				int size = strlen((char*)&win_rate);
				int totalsize = sizeof(int) * 3 + size;
				
				send_packet.option = 2;
				send_packet.size = size;
				send_packet.totalsize = totalsize;
				strcpy_s(send_packet.data, send_packet.size + 1, win_rate);

				retval = send(client_sock[1], (char*)&send_packet, sizeof(send_packet), 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}

				//�����
				retval = recv(client_sock[1], (char*)&cli1_packet_cache, sizeof(PACKET), 0);
				if (retval == SOCKET_ERROR) {
					err_display("recv()");
					break;
				}
				cli1_packet = (PACKET*)&cli1_packet_cache;
			}

			//////////////////////////////////////
			//cli0 or cli1 ���� ���� option == 3
			if (cli0_packet->option == 3 || cli1_packet->option == 3) {
				sprintf(win_rate, "%.2f", a);
				int size = strlen((char*)&win_rate);
				int totalsize = sizeof(int) * 3 + size;

				send_packet.totalsize = totalsize;
				send_packet.size = size;
				send_packet.option = 3;
				strcpy_s(send_packet.data, send_packet.size + 1, win_rate);

				retval = send(client_sock[0], (char*)&send_packet, sizeof(send_packet), 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}

				sprintf(win_rate, "%.2f", b);
				size = strlen((char*)&win_rate);
				totalsize = sizeof(int) * 3 + size;

				send_packet.totalsize = totalsize;
				send_packet.size = size;
				send_packet.option = 3;
				strcpy_s(send_packet.data, send_packet.size + 1, win_rate);

				retval = send(client_sock[1], (char*)&send_packet, sizeof(send_packet), 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				break;
			}

			/////////////////////////////
			// ���������� ���� option == 0
			if(cli0_packet->option == 0 && cli1_packet->option == 0){
				int size = strlen(Result_Msg[who_win(cli0_packet->gbb, cli1_packet->gbb)]);
				int totalsize = sizeof(int) * 3 + cli0_packet->size;
				
				// ���������� ����� ���� option ����
				if (who_win(cli0_packet->gbb, cli1_packet->gbb) == 0) {
					send_packet.option = 0;
					gbb_cnt++;
				}
				else {
					send_packet.option = 4;
					gbb_cnt = 0;
				}

				if (gbb_cnt <= 4) {
					totalsize = sizeof(int) * 3 + size;
					send_packet.totalsize = totalsize;
					send_packet.size = size;
					strcpy_s(send_packet.data, send_packet.size + 1, Result_Msg[who_win(cli0_packet->gbb, cli1_packet->gbb)]);

					retval = send(client_sock[0], (char*)&send_packet, sizeof(send_packet), 0);
					if (retval == SOCKET_ERROR) {
						err_display("send()");
						break;
					}

					size = strlen(Result_Msg[who_win(cli1_packet->gbb, cli0_packet->gbb)]);
					totalsize = sizeof(int) * 3 + size;
					send_packet.totalsize = totalsize;
					send_packet.size = size;
					strcpy_s(send_packet.data, send_packet.size + 1, Result_Msg[who_win(cli1_packet->gbb, cli0_packet->gbb)]);

					retval = send(client_sock[1], (char*)&send_packet, sizeof(send_packet), 0);
					if (retval == SOCKET_ERROR) {
						err_display("send()");
						break;
					}
				}
				// ���������� 5ȸ ����
				else {
					int size1 = strlen(Res_Msg[0]);
					int totalsize1 = sizeof(int) * 3 + size1;

					// ���� ó���� ���� client 0 option 0 ����
					send_packet.option = 0;
					send_packet.totalsize = totalsize1;
					send_packet.size = size1;
					strcpy_s(send_packet.data, send_packet.size + 1, Res_Msg[0]);

					retval = send(client_sock[0], (char*)&send_packet, sizeof(send_packet), 0);
					if (retval == SOCKET_ERROR) {
						err_display("send()");
						break;
					}

					size1 = strlen(Res_Msg[0]);
					totalsize1 = sizeof(int) * 3 + size1;

					// ���� ó���� ���� client1 option 0 ����
					send_packet.option = 0;
					send_packet.totalsize = totalsize1;
					send_packet.size = size1;
					strcpy_s(send_packet.data, send_packet.size + 1, Res_Msg[0]);

					retval = send(client_sock[1], (char*)&send_packet, sizeof(send_packet), 0);
					if (retval == SOCKET_ERROR) {
						err_display("send()");
						break;
					}
				}
			}

			/////////////////////////////
			// ����� ���� option == 4
			//if (who != 0 && cli0_packet->option == 4 && cli0_packet->option == 4) {
			if (cli0_packet->option == 4 && cli1_packet->option == 4) {
				if (cli0_packet->gbb != cli1_packet->gbb) {
					if (MJB_cnt <= 4) {
						int size = strlen(Result_Msg[who_win(cli0_packet->gbb, cli1_packet->gbb)]);
						int totalsize = sizeof(int) * 3 + size;

						send_packet.option = 4;
						send_packet.totalsize = totalsize;
						send_packet.size = size;
						strcpy_s(send_packet.data, send_packet.size + 1, Result_Msg[who_win(cli0_packet->gbb, cli1_packet->gbb)]);

						retval = send(client_sock[0], (char*)&send_packet, sizeof(send_packet), 0);
						if (retval == SOCKET_ERROR) {
							err_display("send()");
							break;
						}

						size = strlen(Result_Msg[who_win(cli1_packet->gbb, cli0_packet->gbb)]);
						totalsize = sizeof(int) * 3 + size;

						send_packet.option = 4;
						send_packet.totalsize = totalsize;
						send_packet.size = size;
						strcpy_s(send_packet.data, send_packet.size + 1, Result_Msg[who_win(cli1_packet->gbb, cli0_packet->gbb)]);

						retval = send(client_sock[1], (char*)&send_packet, sizeof(send_packet), 0);
						if (retval == SOCKET_ERROR) {
							err_display("send()");
							break;
						}
						MJB_cnt++;
					}
					//����� 5ȸ ����
					else {
						MJB_cnt = 0;
						int size1 = strlen(Res_Msg[0]);
						int totalsize1 = sizeof(int) * 3 + size1;

						// ���� ó���� ���� client 0 option 0 ����
						send_packet.option = 0;
						send_packet.totalsize = totalsize1;
						send_packet.size = size1;
						strcpy_s(send_packet.data, send_packet.size + 1, Res_Msg[0]);

						retval = send(client_sock[0], (char*)&send_packet, sizeof(send_packet), 0);
						if (retval == SOCKET_ERROR) {
							err_display("send()");
							break;
						}

						size1 = strlen(Res_Msg[0]);
						totalsize1 = sizeof(int) * 3 + size1;

						// ���� ó���� ���� client1 option 0 ����
						send_packet.option = 0;
						send_packet.totalsize = totalsize1;
						send_packet.size = size1;
						strcpy_s(send_packet.data, send_packet.size + 1, Res_Msg[0]);

						retval = send(client_sock[1], (char*)&send_packet, sizeof(send_packet), 0);
						if (retval == SOCKET_ERROR) {
							err_display("send()");
							break;
						}
					}
				}
				else if (cli0_packet->gbb == cli1_packet->gbb) {
					MJB_cnt = 0;
					//if (who == 1) {
					if (who == 1) {
						score_1++;
					}
					else {
						score_0++;
					}
					printf("winner : %s\n", winner[who]);

					int size1 = strlen(Result_Msg2[MJB(cli0_packet->gbb, cli1_packet->gbb, who)]);
					int totalsize1 = sizeof(int) * 3 + size1;
					
					// ����� ���� ���ῡ ���� client 0 option 0 ����
					send_packet.option = 0;
					send_packet.totalsize = totalsize1;
					send_packet.size = size1;
					strcpy_s(send_packet.data, send_packet.size + 1, Result_Msg2[MJB(cli0_packet->gbb, cli1_packet->gbb, who)]);

					retval = send(client_sock[0], (char*)&send_packet, sizeof(send_packet), 0);
					if (retval == SOCKET_ERROR) {
						err_display("send()");
						break;
					}

					size1 = strlen(Result_Msg1[MJB(cli0_packet->gbb, cli1_packet->gbb, who)]);
					totalsize1 = sizeof(int) * 3 + size1;
					
					// ����� ���� ���ῡ ���� client1 option 0 ����
					send_packet.option = 0;
					send_packet.totalsize = totalsize1;
					send_packet.size = size1;
					strcpy_s(send_packet.data, send_packet.size + 1, Result_Msg1[MJB(cli0_packet->gbb, cli1_packet->gbb, who)]);

					retval = send(client_sock[1], (char*)&send_packet, sizeof(send_packet), 0);
					if (retval == SOCKET_ERROR) {
						err_display("send()");
						break;
					}

					printf("client0 : %0.f�� , client1 : %0.f��\n", score_0, score_1);
				}
			}
		}

		// ���� �ݱ� (client0, client1)
		closesocket(client_sock[0]);
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr[0].sin_port));

		closesocket(client_sock[1]);
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr[1].sin_port));

		error = 0;
	}

	// ���� �ݱ�
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}
