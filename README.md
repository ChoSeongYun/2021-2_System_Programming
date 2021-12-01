># 2021-02 System Programming Term Project
+ 소켓프로그래밍을 이용한 멀티 미로탈출 게임
<br/>

>## Member
+ 빅데이터 전공 20175164 조성윤(C)
  + 기본 소켓 통신 구현, 미로 알고리즘 구현(이동, 이동불가, 장애물처리), ncurses를 이용한 인터페이스 구현
+ 컴퓨터공학과 20175140 신재성
  + 기본 소켓 통신 구현, 다중 클라이언트 접속 구현, NON_BLOCKING 입출력 구현
<br/>

>## Discription

**1. What is it?**
+ 소켓프로그래밍을 이용한 멀티플레이 게임으로<br/>최대 2명의 사용자가 한 미로에서 장애물을 헤쳐 나가며 서로 목적지에 도달하는 게임.
<br/>

**2. How to run?**
+ Linux 환경에서 사용 가능하다.
+ 방향키(W,A,S,D)를 이용해 조작하며 특정 목적지(도착지)에 도달할 경우 클리어 된다.
<br/>

**3. File**
+ just_maze.c (21/11/11)
  + 솔로플레이 미로 게임(미로에서 이동 및 장애물 처리 알고리즘 구현)

+ maze-server_1.c, maze-client_1.c (21/11/18) **-- 프로젝트 중간 결과물**
  + client의 미로 이동을 서버로 전송하여 즉각적인 동기화 및 터미널로 표기.

+ final_server.c, final_client.c, final_input.c (21/12/01) **-- 프로젝트 최종 결과물**
  + final_server.c : 클라이언트로 부터 받은 입력을 처리하고 클라이언트에게 데이터를 전송해주는 서버 소스코드(INET 통신 사용)
  + final_client.c : 서버와 통신하면서 서버에서 받은 데이터를 시각화하여 사용자에게 보여주는 클라이언트 소스코드(INET & UNIX 통신 사용)
  + final_input.c : 사용자의 입력을 NON_BLOCKING으로 받기 위한 입력 소스코드(UNIX 통신 사용)
<br/>

**4. Diagram**
![최종 다이어그램](https://user-images.githubusercontent.com/83500747/144180959-8d93d7e7-0488-47d0-b2be-53f392170bb0.PNG)
<br/>

>## Screenshot
<br/>
