# TCP IP ROCK-SCISSOR-PAPER-GAME

### 묵찌빠 게임 프로젝트
| TCP/IP 소켓 통신을 이용하여 2명의 클라이언트가 묵찌빠 게임을 수행

---

### Server

![server](https://github.com/GreenClothes/TCP_IP_RockScissorsPaper/assets/106455871/5f81e671-971b-401b-b3ff-066bc4d40449)

### Client 0
![client0](https://github.com/GreenClothes/TCP_IP_RockScissorsPaper/assets/106455871/c4f2e8aa-641c-4394-9ca0-bc658d8dadde)

### Client 1
![client1](https://github.com/GreenClothes/TCP_IP_RockScissorsPaper/assets/106455871/d0a733b1-943f-4669-8d2f-846fc2e2ce7c)

---

### 주요 기능

- 서버
  - 클라이언트의 연결 확인<br>(클라이언트 IP 주소, 포트 번호)
  - 게임 후 승리자 노출
  - 클라이언트 종료 확인
- 클라이언트
  - 옵션 선택<br>(가위, 바위, 보, 승률 확인, 게임 종료)
  - 승률 요청
  - 게임 종료(서버와 연결 종료)

--- 

### 데이터 패킷 구조

```C
typedef struct _PACKET
{
	int size;
	int totalsize;
	int option;
	int gbb;
	char data[15];
}PACKET;
```
size : 데이터 길이<br>
total size : 패킷 총 길이<br>
option : 가위바위보 게임/묵찌빠 게임/승률 요청/게임 종료<br>
gbb : 가위/바위/보<br>
data : 승률 데이터/종료 데이터<br>

---