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

#define MAX_SIZE 12

// 전송 시 사용하게 될 버퍼
char buf[BUFSIZ];

// maze
char maze[MAX_SIZE][MAX_SIZE] =
{       { '1','1','1','1','1','1','1','1','1','1','1','1' },
        { '2','0','0','1','1','1','1','1','1','1','1','1' },
        { '3','0','0','1','1','1','1','1','1','1','1','1' },
        { '1','0','0','4','0','0','0','0','0','0','0','0' },
        { '1','4','4','4','0','0','0','0','0','0','0','0' },
        { '1','0','0','1','1','1','1','1','1','1','y','y' },
        { '1','1','1','1','1','1','1','1','1','1','1','1' },
        { '1','1','1','1','1','1','1','1','1','1','1','1' },
        { '1','1','1','1','1','1','1','1','1','1','1','1' },
        { '1','1','1','1','1','1','1','1','1','1','1','1' },
        { '1','1','1','1','1','1','1','1','1','1','1','1' },
        { '1','1','1','1','1','1','1','1','1','1','1','1' },
};

int getkey(int is_echo);
void print_maze(char a[][MAX_SIZE]);

int main(int argc, char * argv[])
{		
		if(argc != 3)
		{
				printf("Usage : ./client <IP> <PORT>\n");
				exit(1);
		}
		// var
		int is_echo = 0;

		// port
		int port;
		port = atoi(argv[2]);

		// client socket
		int clnt_sock;
		int clnt_len;

		// select
		fd_set readfds;
		int ret;

		// client struct
		struct sockaddr_in clnt_addr;

		// socket()
		clnt_sock = socket(AF_INET, SOCK_STREAM , 0);
		
		// inet address
		clnt_addr.sin_family = AF_INET;
		clnt_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		clnt_addr.sin_port = htons(port);

		// inet connect
		clnt_len = sizeof(clnt_addr);
		connect(clnt_sock, (struct sockaddr *)&clnt_addr, clnt_len);
		
		while(1)
		{
			print_maze(maze);
			// 사용자는 입력을 보내고, 서버에서 전송받은 좌표를 통해 미로를 만듬(select 사용)
			memset(buf, '\0', BUFSIZ); // 버퍼 초기화
			int ch;
			int is_echo = 0;

			ch = getkey(is_echo);

			sprintf(buf, "%d", ch); // 버퍼에 키보드로 입력받은 값 저장

			printf("buf = %s, key = %d\n", buf, ch);

			if(send(clnt_sock, buf, strlen(buf), 0) == -1) {
				perror("send");
				break;
			}
			printf("send\n");

			if(recv(clnt_sock, maze, sizeof(maze), 0) == -1) {
				perror("recv");
				break;
			}
			printf("recv\n");

		}
		return 0;
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

// 미로의 상태를 출력하는 함수(완료)
void print_maze(char a[][MAX_SIZE])
{
        system("clear");	// 화면을 초기화하고 출력 할 것

		int i, j;
        for(i = 0; i < MAX_SIZE; i++)
        {
                for(j = 0; j < MAX_SIZE; j++)
                {
                        if(maze[i][j] == '1')	
                                printf(" @ ");		// 1일경우 벽출력

                        else if(maze[i][j] == '0')
                                printf(" # ");		// 0일경우 길출력

                        else if(maze[i][j] == '2')
                                printf(" A ");		// 2는 여기서 상대방

                        else if(maze[i][j] == '3')
                                printf(" B ");		// 3은 여기서 본인

                        else if(maze[i][j] == '4')
                                printf(" & ");		// 4는 여기서 밀수있는 장애물
                        
						else
                                printf(" X ");		// 포탈입니다.
                }
        printf("\n");
        }
}
