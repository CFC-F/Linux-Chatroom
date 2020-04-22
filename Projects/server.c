#include "FIFO.h"

int main()
{
	
    //创建信号，用于退出
    if(signal(SIGINT, &signal_quit) == -1)
	{
        printf("Couldn't register signal\n");
        exit(1);
    }
		
    //创建共有管道，用于通信
    Create_FIFO(Public_FIFO);
	//记录客户端数量
    Client_Number = 0;  
    while(1)
	{

        if((PublicFd = open(Public_FIFO, O_RDONLY)) < 0)
		{
            printf("fail open Public_FIFO\n");
            exit(1);
        }

        //通过公共管道进行读入客户端数据
        if(read(PublicFd, &Client_Send_Msg, sizeof(struct FIFO_Message)) > 0)
		{
            printf("客户端的进程号是 : %d\n", Client_Send_Msg.client_pid);
			//printf("%c\n",Client_Send_Msg.type);	
            printf("客户端的信息 : %s", Client_Send_Msg.message);
			//通过pid号，创建私有管道名
            Client_FIFO_Name = Creat_Client_Name(Client_Send_Msg.client_pid);

			//使用switch，通过标志位来区分登录与聊天
			//通过识别Signal_QUIT，判断某客户端退出，关闭私有管道
	
			switch(Client_Send_Msg.type)
			{
				case 'L':
					printf("这是新建的客户端!\n");
					Create_FIFO(Client_FIFO_Name);
					Store_Client_Message();
					Server_Sendto_New();
					close(PublicFd);
					break;
				case 'C':
					//用来记录是否为私聊信息
					Client_Private_Flag = 1;
					Server_Sendto_Private();
					Server_Sendto();
					
					if(strcmp(Client_Send_Msg.message, Signal_QUIT) == 0)
					{
						unlink(Client_FIFO_Name);
						Delete_Client_Message();
						Client_Number --;
						printf("关闭 Client_%d 私有管道\n", Client_Send_Msg.client_pid);
						printf("客户端的数量 : %d\n", Client_Number);
					}
					close(PublicFd);
					break;
				default:
					printf("Error Message\n");
			}
				
        }
        else
		{
            printf("Read Publicfd error!\n");
            exit(1);
        }
		
    }
    
    return 0;

}

//创建私有管道名
char* Creat_Client_Name(int Client_pid)
{

    char buf[6];
    strcpy(Client_Name, Private_FIFO);
    sprintf(buf, "%d", Client_pid);
    strcat(Client_Name, buf);
    return Client_Name;

}


//创建管道
void Create_FIFO(char *FIFO_Name)
{
    int Fd;
    if((Fd = open(FIFO_Name, O_RDONLY)) == -1)
	{
        umask(0);
        mknod(FIFO_Name, S_IFIFO|0666, 0);
        printf("%s-管道被创建\n", FIFO_Name);
    }
    else
	{
        close(Fd);
    }

}

//信号处理函数
void signal_quit(int signum)
{
	printf("\n服务端已退出\n");
    unlink(Public_FIFO);
    exit(0);

}

//存储客户端登录信息
void Store_Client_Message(void)
{

    Client_Spaces_Pid[Client_Number] = Client_Send_Msg.client_pid;
	strcpy(Client_Spaces_Name[Client_Number],Client_Send_Msg.client_name);
    Client_Number ++;
    printf("客户端数量为 : %d\n", Client_Number);

}

//服务器转发聊天信息
void Server_Sendto(void)
{
	if(Client_Private_Flag == 1)       ////用来判断是否为私聊信息
	{
		int count;
		sprintf(Client_Get_Msg.message, "[%s] :  ", Client_Send_Msg.client_name);
		strcat(Client_Get_Msg.message, Client_Send_Msg.message);

		//服务端通过循环打开私有管道发送给每一个客户端
		for(count = 0; count < Client_Number; count ++)
		{
			if((PrivateFd = open(Creat_Client_Name(Client_Spaces_Pid[count]), O_WRONLY)) > 0)
			{
				Client_Get_Msg.client_pid = Client_Send_Msg.client_pid;
				if(write(PrivateFd, &Client_Get_Msg, sizeof(struct FIFO_Message)) > 0)
				{
					printf("Client_%d 信息写入成功!\n", Client_Spaces_Pid[count]);
					close(PrivateFd);
				}
			}
			usleep(100000);
		}
		printf("\n");
	}

}


//服务器转发客户端登录信息
void Server_Sendto_New(void)
{
	int count;
	
	sprintf(Client_Get_Msg.message, "新用户[%s] 登录 成功\n", Client_Send_Msg.client_name);
	for(count = 0; count < Client_Number; count ++)
	{
		if((PrivateFd = open(Creat_Client_Name(Client_Spaces_Pid[count]), O_WRONLY)) > 0)
		{
			Client_Get_Msg.client_pid = Client_Send_Msg.client_pid;
			if(write(PrivateFd, &Client_Get_Msg, sizeof(struct FIFO_Message)) > 0)
			{
				printf("Client_%d 信息写入成功!\n", Client_Spaces_Pid[count]);
				close(PrivateFd);
			}
		}
		usleep(100000);
	}
    
    printf("\n");
}

//服务器转发客户端私聊信息
void Server_Sendto_Private(void)
{
	char* temp;
	char* ret;
	char tempbuf[20]={0};
	int Temp_pid=0,len;
	//printf("success\n");
	if(strlen(Client_Send_Msg.message) != 0)
	{
		if((ret=strstr(Client_Send_Msg.message,"@")) != NULL)
		{
			//printf("successs1!!!\n");
			//temp存储客户端发送信息，@之前的信息
			//temp = strstr(Client_Send_Msg.message,"@",true);    PHP语言用法
			memset(tempbuf, '\0', sizeof(tempbuf));
			len = strlen(Client_Send_Msg.message)-strlen(ret);
			temp = strncpy(tempbuf,Client_Send_Msg.message,len);
			//printf("%s\n",tempbuf);
			//printf("%s\n",ret);
			//printf("%d\n",len);
			if(strncmp(tempbuf,Client_Send_Msg.message,len) == 0)
			{
				for(int count = 0;count < Client_Number;count ++)
				{
					if(strcmp(tempbuf,Client_Spaces_Name[count]) == 0)
					{
						Temp_pid = count;
						break;
					}
					else
					{
						continue;	
					}
									
				}
				//printf("%d\n",Temp_pid);
				//printf("Private FIFO Name : %s\n", Creat_Client_Name(Client_Spaces_Pid[Temp_pid]));
				if((PrivateFd = open(Creat_Client_Name(Client_Spaces_Pid[Temp_pid]) , O_WRONLY)) > 0)
				{
					Client_Get_Msg.client_pid = Client_Send_Msg.client_pid;
					sprintf(Client_Get_Msg.message, "[%s] 与你私聊 : ", Client_Spaces_Name[Temp_pid]);
					//strcat(Client_Get_Msg.message, Client_Send_Msg.message);
					strcat(Client_Get_Msg.message, ret);
					
					
					if(write(PrivateFd, &Client_Get_Msg, sizeof(struct FIFO_Message)) > 0)
					{
						//用于屏蔽转发信息函数
						Client_Private_Flag = 0;
						close(PrivateFd);
					}
				}
				else
					printf("open failed!\n");
				
			}
		}
	}
	//}
}

//删除客户端退出的信息
void Delete_Client_Message(void)
{
    int count;
    int tempcount;

    for(tempcount = 0; tempcount < Client_Number; tempcount++)
	{
        if(Client_Send_Msg.client_pid == Client_Spaces_Pid[tempcount])
		{
            break;
        }
    }
    for(count = tempcount; count < Client_Number; count ++)
	{
        Client_Spaces_Pid[count] = Client_Spaces_Pid[count + 1];
        strcpy(Client_Spaces_Name[count], Client_Spaces_Name[count + 1]);
    }

}