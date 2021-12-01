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

// 내 위치 좌표 (2)
int my_locX = 0;
int my_locY = 1;

// 상대방의 위치 좌표 (3)
int co_locX = 0;
int co_locY = 2;

// 전송 시 사용하게 될 버퍼
char buf[BUFSIZ];

// 전송할때 쓸 버퍼입니다.
int my_buf[2] = {0, 1};		// 내 위치(발신용)
int co_buf[2] = {0, 2};		// 상대방 위치(수신 및 업데이트용)

// maze
char maze[MAX_SIZE][MAX_SIZE] = 
{		{ '1','1','1','1','1','1','1','1','1','1','1','1' },  
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

int getKey(int is_echo);
void print_maze(char a[][MAX_SIZE]);
int is_end();
int moving();
void co_loc();
void my_loc(int buffer[]);

int main(int argc, char * argv[])
{		
		if(argc != 2)
		{
				printf("Usage : ./server <PORT>\n");
				exit(1);
		}

		// we need port
		int port;
		port = atoi(argv[1]);

		// sockets...
		int serv_sock;
		int clnt_sock;

		// length
		int serv_len;
		int clnt_len;

		// select
		fd_set readfds;
		int ret;

		// using af_inet socket
		struct sockaddr_in serv_addr;
		struct sockaddr_in clnt_addr;

		// socket()
		serv_sock = socket(AF_INET, SOCK_STREAM, 0);
		
		// inet address
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr.sin_port = htons(port);

		// inet bind
		serv_len = sizeof(serv_addr);
		bind(serv_sock, (struct sockaddr *)&serv_addr, serv_len);

		// inet listen and accept
		listen(serv_sock, 5);
		clnt_len = sizeof(clnt_addr);
		clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_len);
		
		// non-blocking
		int flag = fcntl(clnt_sock, F_GETFL, 0);
		fcntl(clnt_sock, F_SETFL, flag | O_NONBLOCK);

		/* function */
		while(1)
		{
			// select 구문 사용해서 변경이 확인되면 미로의 변경사항 버퍼에 저장 후 미로 버퍼 송신
			FD_ZERO(&readfds);
			FD_SET(clnt_sock, &readfds);
			ret = select(clnt_sock + 1, &readfds, NULL, NULL, NULL);

			switch(ret) {
				case -1:
					perror("select");
					break;
				case 0:
					printf("select return 0\n");
					break;
				default:
					while(ret > 0) {
						if(FD_ISSET(clnt_sock, &readfds)) {
							memset(buf, '\0', BUFSIZ);
							if(recv(clnt_sock, buf, BUFSIZ, 0) == -1) {
								perror("recv");
								break;
							}
							printf("recv\n");
							moving(atoi(buf));
							if(send(clnt_sock, maze, sizeof(maze), 0) == -1) {
								perror("send");
								break;
							}
							printf("send\n");
							ret--;
						}
					}
					break;
			}

		}
		return 0;
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
								printf(" A ");		// 2는 여기서 본인

						else if(maze[i][j] == '3')
								printf(" B ");		// 3은 여기서 상대방

						else if(maze[i][j] == '4')
								printf(" & ");		// 4는 여기서 밀수있는 장애물
						
						else
								printf(" X ");		// 포탈입니다.
				}
		printf("\n");
		}
}

// 내 좌표를 상대방에게 전송할 변수에 넣음
void my_loc(int buffer[])
{
	// buffer[0]은 내 X좌표, buffer[1]은 내 Y좌표.

	buffer[0] = my_locX;
	buffer[1] = my_locY;
}

// 상대방의 좌표 동기화
void co_loc()
{
		maze[co_locY][co_locX] = '0';
		co_locX = co_buf[0];
		co_locY = co_buf[1];
		maze[co_locY][co_locX] = '3';
}

int is_end()
{
		if((my_locY == 5 && my_locX == 10) && (co_locY == 5 && co_locX == 11) )
		{
				system("clear");
				printf("CLEAR!!\n");
				return 0;
		}

		return - 1;
}

int moving(int ch)
{
		int is_echo = 0;
			
				switch(ch)
				{
						case 119:		// UP
								if((maze[my_locY - 1][my_locX] == '1') || (maze[my_locY - 1][my_locX] == '3'))
								{
										printf("\ncant move\n");
								}
								else if(maze[my_locY - 1][my_locX] == '4')
								{
										if((maze[my_locY - 2][my_locX] == '1') || (maze[my_locY - 2][my_locX] == '3'))
										{
												print_maze(maze);
												printf("can't move\n");
										}

										else {
												maze[my_locY - 2][my_locX] = '4';
												maze[my_locY - 1][my_locX] = '2';
												maze[my_locY][my_locX] = '0';
												print_maze(maze);

												my_locY--;
												printf("can move box!!\n");
												return 0;
										}
								}

								else
								{
										maze[my_locY-1][my_locX] = '2';
										maze[my_locY][my_locX] = '0';
										print_maze(maze);

										my_locY--;
										printf("\ncan move!! row(Y) : %d col(X) : %d\n",my_locY, my_locX);
										return 0;
								}
								break;
						case 115:		// DOWN
								if((maze[my_locY + 1][my_locX] == '1') || (maze[my_locY + 1][my_locX] == '3'))
								{
										printf("\ncant move\n");
								}

								else if(maze[my_locY + 1][my_locX] == '4')
								{
										if((maze[my_locY + 2][my_locX] == '1') || (maze[my_locY + 2][my_locX] == '3'))
										{
												print_maze(maze);

												printf("can't move\n");
										}

										else {
												maze[my_locY + 2][my_locX] = '4';
												maze[my_locY+1][my_locX] = '2';
												maze[my_locY+0][my_locX] = '0';
												print_maze(maze);

												my_locY++;
												printf("can move box!!\n");
												return 0;
										}
								}
								else
								{
										maze[my_locY+1][my_locX] = '2';
										maze[my_locY][my_locX] = '0';
										print_maze(maze);

										my_locY++;
										printf("\ncan move!! row(Y) : %d col(X) : %d\n",my_locY,my_locX);
										return 0;
								}
								break;
						case 97:	// left
								if((maze[my_locY][my_locX - 1] == '1') || (maze[my_locY][my_locX - 1] == '3'))
								{
										printf("\ncant move\n");
								}
								else if(maze[my_locY][my_locX - 1] == '4')
								{
										if((maze[my_locY][my_locX - 2] == '1') || (maze[my_locY][my_locX - 2] == '3'))
										{
												print_maze(maze);

												printf("can't move\n");
										}

										else {
												maze[my_locY][my_locX - 2] = '4';
												maze[my_locY][my_locX - 1] = '2';
												maze[my_locY][my_locX] = '0';
												print_maze(maze);

												my_locX--;
												printf("can move box!!\n");
												return 0;
										}
								}

								else
								{
										maze[my_locY][my_locX-1] = '2';
										maze[my_locY][my_locX] = '0';
										print_maze(maze);

										my_locX--;
										printf("\ncan move!! row(Y) : %d col(X) : %d\n",my_locY,my_locX);
										return 0;
								}
								break;
						case 100:	// right
								if((maze[my_locY][my_locX + 1] == '1') || (maze[my_locY][my_locX + 1] == '3'))
								{
										printf("\ncant move\n");
								}
								else if(maze[my_locY][my_locX + 1] == '4')
								{
										if((maze[my_locY][my_locX + 2] == '1') || (maze[my_locY][my_locX + 2] == '3'))
										{
												print_maze(maze);

												printf("can't move\n");
										}

										else {
												maze[my_locY][my_locX + 2] = '4';
												maze[my_locY][my_locX + 1] = '2';
												maze[my_locY][my_locX] = '0';
												print_maze(maze);

												my_locX++;
												printf("can move box!!\n");
												return 0;
										}
								}

								else
								{
										maze[my_locY][my_locX+1] = '2';
										maze[my_locY][my_locX] = '0';
										print_maze(maze);
										my_locX++;
										printf("\ncan move!! row(Y) : %d col(X) : %d\n",my_locY,my_locX);
										return 0;
								}
								break;
				}

				return -1;

}


