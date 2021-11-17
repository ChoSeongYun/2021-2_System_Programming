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

#define MAX_SIZE 12
char maze[MAX_SIZE][MAX_SIZE] = 
{	{ '1','1','1','1','1','1','1','1','1','1','1','1' },  
		{ '2','0','1','1','1','1','1','1','1','1','1','1' },
		{ '1','0','1','1','0','0','0','1','1','1','1','1' },
		{ '1','4','0','0','0','1','0','1','1','0','0','0' },
		{ '1','0','1','0','1','1','0','1','1','0','1','0' },
		{ '1','1','1','1','1','1','0','0','0','0','1','y' },
		{ '1','1','1','1','1','1','1','1','0','1','1','1' },
		{ '1','1','1','1','1','1','1','1','0','0','0','1' },
		{ '1','1','1','1','1','1','1','1','0','0','0','1' },
		{ '1','1','1','1','1','1','1','1','1','1','0','0' },
		{ '1','1','1','1','1','1','1','1','1','1','1','1' },
		{ '1','1','1','1','1','1','1','1','1','1','1','1' },
};

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

void print_maze(char a[][MAX_SIZE])
{
		int i, j;
		for(i = 0; i < MAX_SIZE; i++)
		{
				for(j = 0; j < MAX_SIZE; j++)
				{
						if(maze[i][j] == '1')
								printf("@");
						else if(maze[i][j] == '0')
								printf("#");
						else if(maze[i][j] == '2')
								printf("O");
						else if(maze[i][j] == '4')
								printf("&");
						else
								printf("X");
				}
				printf("\n");
		}

}

int main(void)
{

		print_maze(maze);
		int ch;
		int mylocX = 0;
		int mylocY = 1;
		int is_echo = 0;

		while(1)
		{
				if(mylocY == 5 && mylocX == 11)
				{
						system("clear");
						printf("CLEAR!!\n");
						break;
				}

				ch = getkey(is_echo);
				//printf("key : %d",ch);

				switch(ch)
				{
						case 119:
								if(maze[mylocY - 1][mylocX] == '1')
								{
										printf("\ncant move\n");
								}
								else
								{
										system("clear");
										maze[mylocY-1][mylocX] = '2';
										maze[mylocY][mylocX] = '0';
										print_maze(maze);

										mylocY--;
										printf("\ncan move!! row(Y) : %d col(X) : %d\n",mylocY, mylocX);
								}
								break;
						case 115:
								if(maze[mylocY + 1][mylocX] == '1')
								{
										printf("\ncant move\n");
								}

								else if(maze[mylocY + 1][mylocX] == '4')
								{
										if(maze[mylocY + 2][mylocX] == '1')
										{
												system("clear");
												print_maze(maze);

												printf("can't move\n");
										}

										else {
												system("clear");
												maze[mylocY + 2][mylocX] = '4';
												maze[mylocY+1][mylocX] = '2';
												maze[mylocY+0][mylocX] = '0';
												print_maze(maze);

												mylocY++;
												printf("can move box!!\n");
										}
								}
								else
								{
										system("clear");
										maze[mylocY+1][mylocX] = '2';
										maze[mylocY][mylocX] = '0';
										print_maze(maze);

										mylocY++;
										printf("\ncan move!! row(Y) : %d col(X) : %d\n",mylocY,mylocX);
								}
								break;
						case 97:
								if(maze[mylocY][mylocX - 1] == '1')
								{
										printf("\ncant move\n");
								}
								else
								{
										system("clear");
										maze[mylocY][mylocX-1] = '2';
										maze[mylocY][mylocX] = '0';
										print_maze(maze);

										mylocX--;
										printf("\ncan move!! row(Y) : %d col(X) : %d\n",mylocY,mylocX);
								}
								break;
						case 100:
								if(maze[mylocY][mylocX + 1] == '1')
								{
										printf("\ncant move\n");
								}
								else
								{
										system("clear");
										maze[mylocY][mylocX+1] = '2';
										maze[mylocY][mylocX] = '0';
										print_maze(maze);
										mylocX++;
										printf("\ncan move!! row(Y) : %d col(X) : %d\n",mylocY,mylocX);
								}
								break;
				}
		}
		return 0;
}

