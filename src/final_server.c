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

#define MAX_SIZE 20
#define MAX_USER 2

// 다중 클라이언트 접속을 위한 소켓 배열화
int clnt_sock[MAX_USER];
int usr1, usr2;

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
char maze[MAX_SIZE][MAX_SIZE] = // 20 * 20 맵
{		{ '1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1'},  
		{ '2','0','0','0','0','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1'},
		{ '3','0','0','0','0','1','0','0','1','1','1','1','1','1','1','1','1','1','1','1'},
		{ '1','1','1','1','0','1','0','1','1','1','1','1','1','1','1','1','1','1','1','1'},
		{ '1','1','1','1','0','1','0','1','0','1','1','1','1','1','1','1','1','1','1','1'},
		{ '1','0','4','0','0','1','0','0','4','0','0','1','1','1','1','1','1','1','1','1'},
		{ '1','0','4','0','0','1','1','1','0','1','0','1','1','1','1','1','1','1','1','1'},
		{ '1','1','0','1','1','1','0','0','0','1','0','1','1','1','1','1','1','1','1','1'},
		{ '1','1','0','1','1','1','0','1','1','1','0','1','1','1','1','1','1','1','1','1'},
		{ '1','1','0','0','0','0','0','1','1','1','0','1','1','1','1','1','1','1','1','1'},
		{ '1','1','1','0','1','1','1','1','0','4','0','1','1','1','1','1','1','1','1','1'},
		{ '1','0','0','0','1','1','1','1','0','4','0','1','1','1','1','1','1','1','1','1'},
		{ '1','0','1','1','1','0','0','0','1','0','1','1','1','1','1','1','1','1','1','1'},
		{ '1','1','1','1','0','0','1','0','1','0','1','1','1','1','1','1','1','1','1','1'},
		{ '1','1','1','1','1','1','1','0','0','0','1','1','1','1','1','1','1','1','1','1'},
		{ '1','1','1','1','1','1','1','1','1','0','1','1','1','1','1','1','5','5','1','1'},  // (16,15), (17,15) (goal)
		{ '1','1','1','1','1','1','1','1','1','0','0','0','0','0','0','0','0','0','1','1'},
		{ '1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1'},
		{ '1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1'},
		{ '1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1'}
};

int getKey(int is_echo);
void print_maze(char a[][MAX_SIZE]);
int is_end();
int moving();
void co_loc();
void my_loc(int buffer[]);
int maxArr(const int array[], int count);

int main(int argc, char * argv[])
{		
		if(argc != 2)
		{
				printf("Usage : ./server <PORT>\n");
				exit(1);
		}

		// Variable
		int i,j;
		char user_name1[12];
		char user_name2[12];

		// We need port
		int port;
		port = atoi(argv[1]);

		// socket
		int serv_sock;

		// socket length
		int serv_len;
		int clnt_len;

		// using for select()
		fd_set readfds;
		int ret;

		// socket struct
		struct sockaddr_in serv_addr;
		struct sockaddr_in clnt_addr;

		// socket()
		serv_sock = socket(AF_INET, SOCK_STREAM, 0);
		
		// inet address
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr.sin_port = htons(port);

		// bind()
		serv_len = sizeof(serv_addr);
		bind(serv_sock, (struct sockaddr *)&serv_addr, serv_len);

		// listen()
		listen(serv_sock, 5);
		clnt_len = sizeof(clnt_addr);

		// accept() : 다중 클라이언트 받기
		for(i = 0; i < MAX_USER; i++)
		{
			// 다중 클라이언트 accept 함수로 받기
			memset(buf,'\0',BUFSIZ);
			clnt_sock[i] = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_len);
            printf("accept!\n");

			// 클라이언트로부터 사용자 구분 태그(?)를 받을 것임 (user1 인지? 혹은 user2인지?)
			recv(clnt_sock[i], buf, BUFSIZ, 0);
		     
			int flag = fcntl(clnt_sock[i], F_GETFL, 0);			// NON_BLOCKING 처리
			fcntl(clnt_sock[i], F_SETFL, flag | O_NONBLOCK);	// NON_BLOCKING 처리

			// 사용자로 부터 받은 태그를 사용하기 위해선 어딘가에 저장을 해야함...
			if(strcmp(buf,"user1") == 0)
			{
				usr1 = clnt_sock[i];
			}
			else
			{
				usr2 = clnt_sock[i];
			}
		}
			
		/* function */
		while(1)
		{
			// select() 구문 사용해서 변경이 확인되면 미로의 변경사항 버퍼에 저장 후 미로 버퍼 송신
			FD_ZERO(&readfds);
			for(i = 0; i<MAX_USER;i++){
				FD_SET(clnt_sock[i], &readfds);
			}
			ret = select(maxArr(clnt_sock,MAX_USER) +1, &readfds, NULL, NULL, NULL);
			// ret > 0 일 때 값의 변경이 일어난 것이기 때문에 send recv 동작
			switch(ret) {
				case -1:
					perror("select");
					break;
				case 0:
					printf("select return 0\n");
					break;
				default:
					j = 0;
					while(ret > 0) {
						if(FD_ISSET(clnt_sock[j], &readfds)) {
							memset(buf, '\0', BUFSIZ);
							
							// 키보드 입력 받기
							if(recv(clnt_sock[j], buf, BUFSIZ, 0) == -1) {
								perror("recv");
								break;
							}
							printf("recv\n");

							moving(atoi(buf), clnt_sock[j]);
							
							// send 는 입력한대로 변경된 지도를 보내주는데 for문을 통해서 모든 사용자에게 전송한다.
							for(i = 0 ; i < MAX_USER; i++)
							{
								if(send(clnt_sock[i], maze, sizeof(maze), 0) == -1) {
									perror("send");
									break;
								}
							}							
							printf("send\n");

							// 목적지에 도착
                            if(is_end() == 0) {
                                strcpy(buf, "CLEAR");
                                for(i = 0; i < MAX_USER; i++)
                                {
									break;
                                }
                                break;
                            }
							ret--;
						}
						j++;
					}
					break;
			}
            if(strcmp(buf, "CLEAR") == 0)
                break;
		}
        close(serv_sock);
        for(i = 0; i < MAX_USER, i++;)
        {
            close(clnt_sock[i]);
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
								printf(" ▷ ");		// 2는 여기서 본인

						else if(maze[i][j] == '3')
								printf(" ▶ ");		// 3은 여기서 상대방

						else if(maze[i][j] == '4')
								printf(" & ");		// 4는 여기서 밀수있는 장애물
						
						else
								printf(" X ");		// 포탈입니다.
				}
		printf("\n");
		}
}

// 배열에서 큰 값
int maxArr(const int array[], int count) {
	int h;
	int max = array[0];
	for(h = 0; h < count; h++) {
		if(max < array[h]) max = array[h];
	}
	return max;
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

// 게임 클리어
int is_end()
{
		if((maze[15][16] == '2') && (maze[15][17] == '3'))
		{
				system("clear");
				printf("CLEAR!!\n");
				return 0;
		}else if(((maze[15][16] == '3') && (maze[15][17] == '2')))
		{
				system("clear");
				printf("CLEAR!!\n");
				return 0;
		}else
		{
			return - 1;
		}
}

int moving(int ch, int clnt_sock)
{
		int is_echo = 0;
		char my_ch;
		char co_ch;
		
				// 내 이동만 하는건가?
				switch(ch)
				{
						case 119:	// UP(W)
								if(clnt_sock == usr1)	// 사용자 1이 눌렀을 경우?	
								{
									// 움직일 곳에 혹은 상대방이 있을 경우
									if((maze[my_locY - 1][my_locX] == '1') || (maze[my_locY - 1][my_locX] == '3'))
									{
										printf("\ncant move\n");
									}
									else if(maze[my_locY - 1][my_locX] == '4')	// 움직일 곳에 장애물이 있을 경우?
									{
										// 장애물 앞에 상대방 혹은 벽이 있음
										if((maze[my_locY - 2][my_locX] == '1') || (maze[my_locY - 2][my_locX] == '3') || (maze[my_locY - 2][my_locX] == '4'))
										{
												print_maze(maze);

												printf("can't move\n");
										}
										else // 장애물 앞에 길있음(장애물 이동 가능)
										{
												maze[my_locY - 2][my_locX] = '4';
												maze[my_locY - 1][my_locX] = '2';
												maze[my_locY][my_locX] = '0';
												print_maze(maze);
												my_locY--;
												printf("can move box!!\n");
												return 0;
										}
									}
									// 그냥 길일 경우?
									else
									{
										maze[my_locY-1][my_locX] = '2';
										maze[my_locY][my_locX] = '0';
										print_maze(maze);
										my_locY--;
										printf("\ncan move!! row(Y) : %d col(X) : %d\n",my_locY, my_locX);
										return 0;
									}
								}
								else	// 사용자 2의 처리		
								{
									// 움직일 곳에 벽 또는 상대방이 있을 경우?
									if((maze[co_locY - 1][co_locX] == '1') || (maze[co_locY - 1][co_locX] == '2'))
									{
										printf("\ncant move\n");
									}
									else if(maze[co_locY - 1][co_locX] == '4')	// 움직일 곳에 장애물이 있을 경우?
									{
										// 장애물 앞에 벽 혹은 사람이 있다?
										if((maze[co_locY - 2][co_locX] == '1') || (maze[co_locY - 2][co_locX] == '2') || (maze[co_locY - 2][co_locX] == '4'))
										{
												print_maze(maze);
												printf("can't move\n");
										}

										else {
												maze[co_locY - 2][co_locX] = '4';	// 장애물은 앞앞에
												maze[co_locY - 1][co_locX] = '3';	// 원래 장애물 위치에는 내가
												maze[co_locY][co_locX] = '0';		// 처음 내가 있던곳은 길이 될 것
												print_maze(maze);

												co_locY--;
												printf("can move box!!\n");
												return 0;
										}
									}
									// 움직일 곳이 그냥 길일 경우?
									else
									{
										maze[co_locY-1][co_locX] = '3';
										maze[co_locY][co_locX] = '0';
										print_maze(maze);
										co_locY--;
										printf("\ncan move!! row(Y) : %d col(X) : %d\n",co_locY, co_locX);
										return 0;
									}
								}
								
								break;
						case 115:		// DOWN
								if(clnt_sock == usr1)	// 사용자 1에 대한 처리
								{
									// 움직일 곳에 벽 또는 상대방이 있을 경우?
									if((maze[my_locY + 1][my_locX] == '1') || (maze[my_locY + 1][my_locX] == '3'))
									{
										printf("\ncant move\n");
									}

									// 움직일 곳에 장애물이 있을 경우?
									else if(maze[my_locY + 1][my_locX] == '4')
									{
										// 장애물 앞에 벽 혹은 사람이 있을 경우?
										if((maze[my_locY + 2][my_locX] == '1') || (maze[my_locY + 2][my_locX] == '3') || (maze[my_locY + 2][my_locX] == '4'))
										{
												print_maze(maze);

												printf("can't move\n");
										}
										
										// 그게 아니면?
										else {
												maze[my_locY + 2][my_locX] = '4';	// 앞앞엔 장애물
												maze[my_locY+1][my_locX] = '2';		// 장애물 위치엔 내가
												maze[my_locY+0][my_locX] = '0';		// 원래 있던 곳은 길
												print_maze(maze);
												my_locY++;
												printf("can move box!!\n");
												return 0;
										}
									}
									// 그냥 길일 경우?
									else
									{
										maze[my_locY+1][my_locX] = '2';
										maze[my_locY][my_locX] = '0';
										print_maze(maze);

										my_locY++;
										printf("\ncan move!! row(Y) : %d col(X) : %d\n",my_locY,my_locX);
										return 0;
									}
								}
								else	// 사용자 2의 처리
								{
									// 움직일 곳에 벽 또는 상대방이 있을경우?
									if((maze[co_locY + 1][co_locX] == '1') || (maze[co_locY + 1][co_locX] == '2'))
									{
										printf("\ncant move\n");
									}
									// 움직일 곳에 장애물이 있다면?
									else if(maze[co_locY + 1][co_locX] == '4')
									{
										// 장애물 앞에 벽 또는 사람이 있을 경우?
										if((maze[co_locY + 2][co_locX] == '1') || (maze[co_locY + 2][co_locX] == '2') || (maze[co_locY + 2][co_locX] == '4'))
										{
												print_maze(maze);
												printf("can't move\n");
										}

										// 장애물 이동 가능
										else {
												maze[co_locY + 2][co_locX] = '4';
												maze[co_locY+1][co_locX] = '3';
												maze[co_locY+0][co_locX] = '0';
												print_maze(maze);

												co_locY++;
												printf("can move box!!\n");
												return 0;
										}
									}
									// 그냥 길일 경우?
									else
									{
										maze[co_locY+1][co_locX] = '3';
										maze[co_locY][co_locX] = '0';
										print_maze(maze);

										co_locY++;
										printf("\ncan move!! row(Y) : %d col(X) : %d\n",co_locY,co_locX);
										return 0;
									}
								}
								break;
						case 97:	// left
								if(clnt_sock == usr1)		// 사용자 1에 대한 처리
								{
									// 움직일 곳에 장애물 또는 상대방이 있을 경우?
									if((maze[my_locY][my_locX - 1] == '1') || (maze[my_locY][my_locX - 1] == '3'))
									{
										printf("\ncant move\n");
									}
									// 움직일 곳에 장애물이 있을 경우?
									else if(maze[my_locY][my_locX - 1] == '4')
									{
										// 장애물 앞에 벽 또는 상대방이 있을 경우?
										if((maze[my_locY][my_locX - 2] == '1') || (maze[my_locY][my_locX - 2] == '3') || (maze[my_locY][my_locX - 2] == '4'))
										{
												print_maze(maze);

												printf("can't move\n");
										}
										
										// 장애물 이동 가능
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

									// 그냥 길일 경우?
									else
									{
										maze[my_locY][my_locX-1] = '2';
										maze[my_locY][my_locX] = '0';
										print_maze(maze);

										my_locX--;
										printf("\ncan move!! row(Y) : %d col(X) : %d\n",my_locY,my_locX);
										return 0;
									}
								}

								else		// 사용자 2 처리
								{
									// 움직일 곳 앞에 벽 혹은 상대방이 있다?
									if((maze[co_locY][co_locX - 1] == '1') || (maze[co_locY][co_locX - 1] == '2'))
									{
										printf("\ncant move\n");
									}

									// 움직일 곳 앞에 장애물이 있다?
									else if(maze[co_locY][co_locX - 1] == '4')
									{
										// 장애물 앞에 사람 또는 벽 있을 경우?
										if((maze[co_locY][co_locX - 2] == '1') || (maze[co_locY][co_locX - 2] == '2') || (maze[co_locY][co_locX - 2] == '4'))
										{
												print_maze(maze);

												printf("can't move\n");
										}

										// 장애물 이동 가능
										else {
												maze[co_locY][co_locX - 2] = '4';
												maze[co_locY][co_locX - 1] = '3';
												maze[co_locY][co_locX] = '0';
												print_maze(maze);

												co_locX--;
												printf("can move box!!\n");
												return 0;
										}
									}
									// 그냥 길일경우?
									else
									{
										maze[co_locY][co_locX-1] = '3';
										maze[co_locY][co_locX] = '0';
										print_maze(maze);

										co_locX--;
										printf("\ncan move!! row(Y) : %d col(X) : %d\n",co_locY,co_locX);
										return 0;
									}
								}		
								break;
						case 100:	// right
								if(clnt_sock == usr1)		// 사용자 1 처리
								{
									// 움직일 곳에 벽 또는 상대방이 있다?
									if((maze[my_locY][my_locX + 1] == '1') || (maze[my_locY][my_locX + 1] == '3'))
									{
										printf("\ncant move\n");
									}
									// 움직일 곳에 장애물이 있다?
									else if(maze[my_locY][my_locX + 1] == '4')
									{
										// 장애물 앞에 벽 또는 사람이 있다?
										if((maze[my_locY][my_locX + 2] == '1') || (maze[my_locY][my_locX + 2] == '3') || (maze[my_locY][my_locX + 2] == '4'))
										{
												print_maze(maze);

												printf("can't move\n");
										}
										// 장애물 이동 가능
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
									// 그냥 길일 경우?
									else
									{
										maze[my_locY][my_locX+1] = '2';
										maze[my_locY][my_locX] = '0';
										print_maze(maze);
										my_locX++;
										printf("\ncan move!! row(Y) : %d col(X) : %d\n",my_locY,my_locX);
										return 0;
									}
								}
								// user 2 일 경우
								else
								{
									// 움직일 곳 앞에 벽 또는 상대방이 있다?
									if((maze[co_locY][co_locX + 1] == '1') || (maze[co_locY][co_locX + 1] == '2'))
									{
										printf("\ncant move\n");
									}
									
									// 움직일 곳 앞에 장애물이 있다?
									else if(maze[co_locY][co_locX + 1] == '4')
									{
										// 장애물 앞에 벽 또는 상대방이 있음
										if((maze[co_locY][co_locX + 2] == '1') || (maze[co_locY][co_locX + 2] == '2') || (maze[co_locY][co_locX + 2] == '4'))
										{
												print_maze(maze);

												printf("can't move\n");
										}
										// 장애물 이동 가능
										else {
												maze[co_locY][co_locX + 2] = '4';
												maze[co_locY][co_locX + 1] = '3';
												maze[co_locY][co_locX] = '0';
												print_maze(maze);

												co_locX++;
												printf("can move box!!\n");
												return 0;
										}
									}
									// 그냥 길일 경우?
									else
									{
										maze[co_locY][co_locX+1] = '3';
										maze[co_locY][co_locX] = '0';
										print_maze(maze);
										co_locX++;
										printf("\ncan move!! row(Y) : %d col(X) : %d\n",co_locY,co_locX);
										return 0;
									}
								}
								break;
				}

				return -1;

}