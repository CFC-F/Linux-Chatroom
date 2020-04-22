#ifndef __MYFIFO__
#define __MYFIFO__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define Public_FIFO  "Server_FIFO"
#define Private_FIFO "Client_FIFO_"
#define Signal_QUIT  "quit\n"
#define Client_Number_LEN 10
#define Client_Name_LEN 30

enum
{
	L,  //登录
	C, //聊天
};

struct FIFO_Message
{
	char type;
    int client_pid;
	char client_name[Client_Name_LEN];
    char message[100];
};



void signal_quit(int signum);
void Create_FIFO(char *FIFO_Name);
char* Creat_Client_Name(int Client_pid);
void Store_Client_Message(void);
void Client_Write(int flag, int Child_pid);
void Client_Read(void);
void Server_Sendto(void);
void Server_Sendto_New(void);
void Server_Sendto_Private(void);
void Show_Time(void);
void Delete_Client_Message(void);

int Client_Private_Flag;                                        //群聊，私聊标志位
int Client_Number;
int Client_Spaces_Pid[Client_Number_LEN];                        //保存客户端的pid
char Client_Spaces_Name[Client_Number_LEN][Client_Name_LEN];     //保存客户端的用户名
int PublicFd, PrivateFd;
char Client_Name[Client_Name_LEN];
char* Client_FIFO_Name;
struct FIFO_Message Client_Send_Msg, Client_Get_Msg;             //结构体变量定义，Client_Get_Msg即server的发送

#endif 
