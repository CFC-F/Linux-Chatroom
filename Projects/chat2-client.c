#include "FIFO.h"


int main()
{
	
    //设置登录信息
    printf("请输入你的用户名: ");
    scanf("%s", Client_Send_Msg.client_name);
    getchar();
    Client_Send_Msg.client_pid = getpid();

    //通过pid,创建客户端的管道名
    Client_FIFO_Name = Creat_Client_Name(getpid());
    printf("客户端进程号为: %d\n客户端发送信息为: %s\n", Client_Send_Msg.client_pid, Client_Send_Msg.message);

    //客户端第一次写入，将登陆信息写入到共有管道中
    Client_Write(!1, -1);   

    //创建进程，用于进行通信
	pid_t pid;
    if((pid = fork()) < 0)
	{
        printf("fail create process\n");
        exit(1);
    }

    //父进程写入信息
    else if(pid > 0)
	{
        while(1)
		{
            // 通过标准输入流，得到客户端写入的信息
            fgets(Client_Send_Msg.message, 60, stdin);
            Client_Send_Msg.client_pid = getpid();
            Client_Write(1, pid);
        }
    }

    //子进程读取信息
    else
	{
        while(1)
		{
            Client_Read();
        }
    }
}

//创建客户端的私有管道名
char* Creat_Client_Name(int Client_pid)
{

    char buf[6];

    strcpy(Client_Name, Private_FIFO);
    sprintf(buf, "%d", Client_pid);
    strcat(Client_Name, buf);
    return Client_Name;

}

//客户端写入函数
void Client_Write(int Client_Write_flag, int Child_pid)
{
	
	//判断写入信息为登陆还是聊天
	if(Client_Write_flag)
		Client_Send_Msg.type = 'C';
	else
		Client_Send_Msg.type = 'L';

    
	//客户端打开共有管道进行写入
    if((PublicFd = open(Public_FIFO, O_WRONLY)) > 0)
	{
        //客户端通过结构体将信息传入到共有管道
        if(write(PublicFd, &Client_Send_Msg, sizeof(struct FIFO_Message)) > 0)
		{
            close(PublicFd);
        }
        else
		{
            printf("fail write client message\n");
        }
        usleep(200000);

        //客户端本身判断信息中是否存在特殊信号，有，则输出退出信息
        if(strcmp(Client_Send_Msg.message, Signal_QUIT) == 0)
		{
            printf("Client_%d 退出\n", Client_Send_Msg.client_pid);
            printf("移除客户端管道 %s\n", Client_FIFO_Name);
            //由于客户端退出，主动发送SIGSTOP信号，用于结束子进程
            if(Client_Write_flag)
			{
                kill(Child_pid, SIGSTOP);
                usleep(1000);
                printf("子进程被停止\n");
            }
            exit(0);
        }
    }
    else
	{
        printf("fail to open Public_FIFO\n");
        exit(1);
    }

}

//客户端读入信息
void Client_Read(void)
{

    //通过私有管道读入信息
    if((PrivateFd = open(Client_FIFO_Name, O_RDONLY)) > 0)
	{
        if(read(PrivateFd, &Client_Get_Msg, sizeof(struct FIFO_Message)) > 0)
		{
            Show_Time();
            printf("%s\n", Client_Get_Msg.message);
            close(PrivateFd);
        }
    }
    else
	{
        printf("Fail to open %s\n", Client_FIFO_Name);
        exit(1);
    }

}

//时间显示函数，使用方式在学习笔记中
void Show_Time(void)
{

    time_t tmpcal_ptr;
	struct tm *tmp_ptr = NULL;
    time(&tmpcal_ptr);
    tmp_ptr = localtime(&tmpcal_ptr);
	printf ("%d-%02d-%02d ", (1900+tmp_ptr->tm_year), (1+tmp_ptr->tm_mon), tmp_ptr->tm_mday);
	printf("%02d:%02d:%02d \n", tmp_ptr->tm_hour, tmp_ptr->tm_min, tmp_ptr->tm_sec);

}

