#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

char socket_name[12];
char buf[BUFSIZ];
int getkey(); // key값 받아오는 함수
int initunixsocket(); // unix 소켓 생성과 서버와 연결하는 함수
void unixcommunicate(int socket); // unix 소켓을 이용한 통신 함수

int main(int argc, char *argv[])
{
    int unix_sock;
    strcpy(socket_name, argv[1]);
    unix_sock = initunixsocket();
    while(1)
        unixcommunicate(unix_sock);
    return 0;
}

int initunixsocket()
{
    struct sockaddr_un unix_cli;
    int sd, len;

    if((sd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    memset((char *)&unix_cli, '\0', sizeof(unix_cli));
    unix_cli.sun_family = AF_UNIX;
    strcpy(unix_cli.sun_path, socket_name);
    len = sizeof(unix_cli.sun_family) + strlen(unix_cli.sun_path);

    if(connect(sd, (struct sockaddr *)&unix_cli, len) < 0)
    {
        return -1;
        perror("connect");
        exit(1);
    }
    return sd;
}

void unixcommunicate(int socket) // unix 소켓을 이용한 통신 함수
{
    int ch;
	int is_echo = 0;
    int unix_sock;

    unix_sock = socket;

    if(unix_sock == -1)
    {
        perror("socket");
        exit(1);
    }

    printf("키를 입력하세요(WASD로 이동)\n");

	ch = getkey(is_echo);
    
    sprintf(buf, "%d", ch); // 버퍼에 키보드로 입력받은 값 저장

	printf("buf = %s, key = %d\n", buf, ch);

	if(send(unix_sock, buf, strlen(buf), 0) == -1) {
		perror("send");
		exit(1);
	}
	printf("send\n");

}

// 키보드 입력 받는 부분(가져온 것.)
int getkey(int is_echo) { 
		int ch;
		struct termios old;
		struct termios current; /* 현재 설정된 terminal i/o 값을 backup함 */
		tcgetattr(0, &old); /* 현재의 설정된 terminal i/o에 일부 속성만 변경하기 위해 복사함 */
		current = old; /* buffer i/o를 중단함 */
		current.c_lflag &= ~ICANON;

		if (is_echo) { // 입력값을 화면에 표시할 경
				current.c_lflag |= ECHO; }
		else { // 입력값을 화면에 표시하지 않을 경우
				current.c_lflag &= ~ECHO; } /* 변경된 설정값으로 설정합니다. */


		tcsetattr(0, TCSANOW, &current);
		ch = getchar();
		tcsetattr(0, TCSANOW, &old);
		return ch;
}