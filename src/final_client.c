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
#include <ncurses.h>

#define MAX_SIZE 20
#define COLOR_BLACK 0
#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_BLUE 4
#define COLOR_BROWN 3

char socket_name[12];   // unix 소켓 이름(사용자 구분)

// 전송 시 사용하게 될 버퍼
char buf[BUFSIZ];

// maze
char maze[MAX_SIZE][MAX_SIZE] = 
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
		{ '1','1','1','1','1','1','1','1','1','0','1','1','1','1','1','1','5','5','1','1'},
		{ '1','1','1','1','1','1','1','1','1','0','0','0','0','0','0','0','0','0','1','1'},
		{ '1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1'},
		{ '1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1'},
		{ '1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1'}
};

void print_maze(char a[][MAX_SIZE]);
int initunixsocket(); // unix 소켓 생성과 서버와 연결하는 함수
int unixcommunicate(int socket); // unix 소켓을 이용한 통신 함수
void ncurses_start (void);
int is_end();

int main(int argc, char * argv[])
{		
		if(argc != 4)
		{
				printf("Usage : ./client <IP> <PORT> <UNIX>\n");
				exit(1);
		}

        int unix_sock;
        strcpy(socket_name, argv[3]);
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
		clnt_addr.sin_addr.s_addr = inet_addr(argv[1]);
		clnt_addr.sin_port = htons(port);

		// inet connect
		clnt_len = sizeof(clnt_addr);
		connect(clnt_sock, (struct sockaddr *)&clnt_addr, clnt_len);

        int flag = fcntl(clnt_sock, F_GETFL, 0);
		fcntl(clnt_sock, F_SETFL, flag | O_NONBLOCK);

        unix_sock = initunixsocket();

        send(clnt_sock, argv[3], strlen(argv[3]), 0);

		ncurses_start();
        curs_set (0);
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        has_colors();
        start_color();

        print_maze(maze);

		while(1)
		{
			// 사용자는 입력을 보내고, 서버에서 전송받은 좌표를 통해 미로를 만듬
			memset(buf, '\0', BUFSIZ); // 버퍼 초기화

            if(unixcommunicate(unix_sock))
            {
                if(send(clnt_sock, buf, strlen(buf), 0) > 0)
                {
                    printw("send\n");
                }
            }
			
            memset(buf, '\0', BUFSIZ);
			if(recv(clnt_sock, maze, sizeof(maze), 0) > 0)
            {
                // 게임을 클리어 할 경우 break;
                if(is_end() == 0)
                {
                    break;
                }
                print_maze(maze);
            }
        }
        endwin();
        system("clear");
        printf("Thank you for playing game!!\n");
        close(unix_sock);
        close(clnt_sock);

        system("rm user*");

		return 0;
}

// 미로의 상태를 출력하는 함수(완료)
void print_maze(char a[][MAX_SIZE])
{
		has_colors();
        start_color();
        
        erase();

        init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_BLUE, COLOR_BLACK);
        init_pair(4, COLOR_BROWN, COLOR_BLACK);
        
        int i, j;
        for(i = 0; i < MAX_SIZE; i++)
        {
                for(j = 0; j < MAX_SIZE; j++)
                {
                        if(maze[i][j] == '1')	
                        {
                            attron(COLOR_PAIR(2));
                            addch(' ' | A_REVERSE);
                            attroff(COLOR_PAIR(2));
                        }
                        else if(maze[i][j] == '0')
                        {
                            addch(' ');		// 0일경우 길출력
                        }

                        else if(maze[i][j] == '2')
                        {
                            attron(COLOR_PAIR(3));
                            addch(' ' | A_REVERSE);		// 플레이어 1
                            attroff(COLOR_PAIR(3));
                        }

                        else if(maze[i][j] == '3')
                        {
                            attron(COLOR_PAIR(1));
                            addch(' ' | A_REVERSE);     // 플레이어 2
                            attroff(COLOR_PAIR(1));
                        }
                                
                        else if(maze[i][j] == '4')
                        {
                            attron(COLOR_PAIR(4));
                            addch(' ' | A_REVERSE);
                            attroff(COLOR_PAIR(4));
                        }
						else
                        {
                            printw("X");		// 포탈입니다.
                        }
                                
                }
            printw("\n");
        }

        // 설명서
        attron(COLOR_PAIR(2));
        mvaddch(0,30,' ' | A_REVERSE);
        attroff(COLOR_PAIR(2));
        mvprintw(0,32,"Wall");
        
        attron(COLOR_PAIR(3));
        mvaddch(1,30,' ' | A_REVERSE);		// 플레이어 1
        attroff(COLOR_PAIR(3));
        mvprintw(1,32,"Player 1");

        attron(COLOR_PAIR(1));
        mvaddch(2,30,' ' | A_REVERSE);		// 플레이어 2
        attroff(COLOR_PAIR(1));
        mvprintw(2,32,"Player 2");

        attron(COLOR_PAIR(4));
        mvaddch(3,30,' ' | A_REVERSE);
        attroff(COLOR_PAIR(4));
        mvprintw(3,32,"Box");

        refresh();
}

int initunixsocket() // unix 소켓 생성과 서버와 연결하는 함수
{
    struct sockaddr_un unix_cli, unix_ser;
    int sd, nsd, len, clen;

    if((sd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    memset((char *)&unix_ser, '\0', sizeof(unix_ser));
    unix_ser.sun_family = AF_UNIX;
    strcpy(unix_ser.sun_path, socket_name);
    len = sizeof(unix_ser.sun_family) + strlen(unix_ser.sun_path);

    if(bind(sd, (struct sockaddr *)&unix_ser, len))
    {
        perror("bind");
        exit(1);
    }

    if(listen(sd, 5) < 0)
    {
        perror("listen");
        exit(1);
    }

    printw("Waiting ...\n");
    if ((nsd = accept(sd, (struct sockaddr *)&unix_cli, &clen)) == -1)
    {
        perror("accept");
        exit(1);
    }

    int flag = fcntl(nsd, F_GETFL, 0);
	fcntl(nsd, F_SETFL, flag | O_NONBLOCK);

    return nsd;
}

int unixcommunicate(int socket) // unix 소켓을 이용한 통신 함수
{
    int unix_sock;

    unix_sock = socket;

    if(unix_sock == -1)
    {
        perror("socket");
        exit(1);
    }

    if(recv(unix_sock, buf, sizeof(buf), 0) > 0)
    {
        printw("recv\n");
        return 1;
    }

    return -1;
}

void ncurses_start (void)
{
  initscr ();
  noecho ();
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
