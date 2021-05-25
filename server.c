#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sqlite3.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#define DBNAME "stu.db"
#define N 128
char *errmsg; //错误信息地址
int flags = 1;
sqlite3 *db;

char timer_out[N];
char timer_in[N];
void err(const char *str)
{
	perror(str);
	exit(1);
}
typedef struct
{                                                                     
	int type;
	char username[N];
	char password[N];
	int id;
	char address[N];
	int age;
	char grade[5];
	char telphone[N];
	int salary;
	char position[20];
}userdata;
typedef struct
{
	int row;
	int col;
}list;
//查看数据库全部内容
void do_get_table(sqlite3 *db, char const *tab)
{

	char cmd[128];
	sprintf(cmd, "select *from '%s'", tab);
	char **ret;
	int nrow, ncol;
	if (sqlite3_get_table(db, cmd, &ret, &nrow, &ncol, &errmsg) != SQLITE_OK)
	{
		printf("error:%s\n", errmsg);
	}
	int i, j, n = 0;
	for (i = 0; i < nrow; i++)
	{
		for (j = 0; j < ncol; j++)
		{
			printf("%-11s ", ret[n++]);
		}
		putchar(10);
	}
	printf("-------------------------------------\n");
}
void getTime(char *str)
{
	time_t t1;
	time(&t1);
	struct tm *info = localtime(&t1);
	if (NULL == info)
		err("localtime");
	sprintf(str, "%d-%02d-%02d %02d:%02d:%02d", info->tm_year + 1900,
					info->tm_mon + 1, info->tm_mday+1, info->tm_hour-12,
					info->tm_min, info->tm_sec);
	fflush(stdout);
//	printf("%s\n", str);
}

void out(userdata recvmsg, int accepted)
{
	//记录当前时间
	char note[N];
	getTime(timer_out);
	//把要查询的单词和时间存放到记录表中
	sprintf(note, "insert into list values('%s','%s','%s')",recvmsg.username,timer_in,timer_out);
	if (sqlite3_exec(db, note, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		printf("error:%s\n", errmsg);
		printf("%d\n", __LINE__);
	}
	do_get_table(db,"list");
}
void record(userdata recvmsg,int accepted)
{
	char cmd[N];
	char **ret;
	list excel;
	int nrow, ncol;
	int status = 0,res;
	char arr[N] = {0};
	int i, j, n = 0;
	sprintf(cmd, "select *from list");
	if (sqlite3_get_table(db, cmd, &ret, &nrow, &ncol, &errmsg) != SQLITE_OK)
	{
		printf("error:%s\n", errmsg);
	}
	excel.row = nrow;
	excel.col = ncol;

	if(send(accepted, &excel,sizeof(excel), 0) < 0)  err("send");
	for (i = 0; i < (nrow + 1) * ncol;i = i + 9)
	{
		bzero(arr, sizeof(arr));
		sprintf(arr, "%s %s %s ", ret[i], ret[i + 1],ret[i+2]);

		if(send(accepted, &arr,sizeof(arr), 0) < 0)  err("send");
		printf("%s\n",arr);
	}

	if(send(accepted,arr,N,0)<0) err("send_record"); 

}
//id条件查找
int search(userdata recvmsg)
{
	char cmd[128];
	sprintf(cmd, "select * from usr where id=%d",recvmsg.id);
	char **ret;
	int nrow, ncol;
	if (sqlite3_get_table(db, cmd, &ret, &nrow, &ncol, &errmsg) != SQLITE_OK)
	{
		printf("error:%s\n", errmsg);
	}
	//不存在
	if (nrow == 0) 
		return -1;
	//存在
	else  
		return 1;

}

//查看信息
void find(userdata recvmsg,int accepted)
{
	char cmd[N];
	list excel;
	char **ret;
	int nrow, ncol;
	int status = 0,res;
	char arr[N] = {0};
	int i, j, n = 0;
	userdata temp;
	//接收选择条件信息
	if(recv(accepted,&res,sizeof(res),0)<0) err("recv_choice");
	//单条信息查询
	if(1==res)
	{
		//接受传过来的查询用户id信息
		if(recv(accepted,&temp,sizeof(temp),0)<0) err("recv_choice2");
		//判读id是否存在
		sprintf(cmd, "select * from usr where id = %d", temp.id);
		printf("id=%d\n",temp.id);
		if (sqlite3_get_table(db, cmd, &ret, &nrow, &ncol, &errmsg) != SQLITE_OK)
		{
			printf("error:%s\n", errmsg);

		}
		//不存在
		if (nrow == 0) 
		{
			status = -1;
			if(send(accepted,&status,sizeof(status),0)<0) err("send_find_server");
		}
		//存在
		else  
		{
			status = 1;
			//发送确认消息
			if(send(accepted,&status,sizeof(status),0)<0) err("send_find_server");
			//发送数据
			for (i = 0; i < (nrow + 1) * ncol;i = i + 9)
			{
				bzero(arr, sizeof(arr));
				sprintf(arr, "%s %s %s %s %s %s %s %s %s", ret[i], ret[i + 1], ret[i + 2],ret[i+3],\
						ret[i+4],ret[i+5],ret[i+6],ret[i+7],ret[i+8]);
 			}

			if (send(accepted, arr, N, 0) < 0)  err("send");
			printf("%s\n",arr);
		}
		
	}
	//查看全部信息
	else if(2==res)
	{
		sprintf(cmd, "select *from usr");
		if (sqlite3_get_table(db, cmd, &ret, &nrow, &ncol, &errmsg) != SQLITE_OK)
		{
			printf("error:%s\n", errmsg);
		}
		excel.row = nrow;
		excel.col = ncol;

		if(send(accepted, &excel,sizeof(excel), 0) < 0)  err("send");
		for (i = 0; i < (nrow + 1) * ncol;i = i + 9)
		{
			bzero(arr, sizeof(arr));
			sprintf(arr, "%s %s %s %s %s %s %s %s %s", ret[i], ret[i + 1], ret[i + 2],ret[i+3],\
					ret[i+4],ret[i+5],ret[i+6],ret[i+7],ret[i+8]);
			
			if(send(accepted, &arr,sizeof(arr), 0) < 0)  err("send");
			printf("%s\n",arr);
		}
	}
	else return;
	
		
}
//修改信息
void chmsg(userdata recvmsg,int accepted)
{
	printf("---------1------\n");
	userdata temp;
	int status = 0,mode = 0;
	char cmd[128];
	char **ret;
	int res = search(recvmsg);
	if(1==res)
	{
		//id存在,发送确认信息	
		status = 1;
		if(send(accepted,&status,sizeof(status),0)<0) err("send_chmsg_server");
		if(recv(accepted,&temp,sizeof(temp),0)<0) err("recv_change");	
		//执行修改操作
		sprintf(cmd,"update usr set password='%s',address='%s' where id=%d",temp.password,temp.address\
				 ,temp.id);
		printf("------2----\n");
		if(sqlite3_exec(db, cmd, NULL,NULL, &errmsg) != SQLITE_OK)
		{
			printf("error:%s\n", errmsg);
			mode = -1;
		}
		mode = 1;
		
		if(send(accepted,&mode,sizeof(mode),0)<0) err("send_delmsg_server");
	}
	else 
	{
		status = -1;
	}

	if(send(accepted,&status,sizeof(status),0)<0) err("send_delmsg_server");
}
//删除信息
void delmsg(userdata recvmsg,int accepted)
{
	int status = 0;
	char cmd[128];
	char **ret;
	int res = search(recvmsg);
	if(1==res)
	{
		sprintf(cmd, "delete from usr where id=%d",recvmsg.id);
		int nrow, ncol;
		if (sqlite3_get_table(db, cmd, &ret, &nrow, &ncol, &errmsg) != SQLITE_OK)
		{
			printf("error:%s\n", errmsg);
			status = -1;
		}
		status = 1;
	}
	else 
	{
		status = -1;
	}
	
	do_get_table(db,"usr");
	if(send(accepted,&status,sizeof(status),0)<0) err("send_delmsg_server");
}
//登录查询
int find_login(userdata recvmsg)
{
	char cmd[128];
	sprintf(cmd, "select * from usr where username='%s' and password='%s'",
					recvmsg.username, recvmsg.password);
	char **ret;
	int nrow, ncol;
	if (sqlite3_get_table(db, cmd, &ret, &nrow, &ncol, &errmsg) != SQLITE_OK)
	{
		printf("error:%s\n", errmsg);
	}
	if (nrow == 0) 
		return 0;
	else  
		return 1;
}
//登录
int login(userdata recvmsg)
{
	//先查找信息是否存在
	int res = find_login(recvmsg);
	printf("name:%s,password=%s\n",recvmsg.username,recvmsg.password);
	if(1==res)
	{
		getTime(timer_in);
		//存在,判断是谁
		if(0==strcmp(recvmsg.username,"admin"))
		{
			//管理员模式
			return 1;
		}
		else
		{
			//用户模式
			return 2;
		}
	}
	else
	{
		//不存在
		return -1;
	}
}
void root()
{
	char cmd[128];
	sprintf(cmd, "select * from usr where id=%d",0);
	char **ret;
	int nrow, ncol;
	if (sqlite3_get_table(db, cmd, &ret, &nrow, &ncol, &errmsg) != SQLITE_OK)
	{
		printf("error:%s\n", errmsg);
	}
	//不存在,则创建管理员
	if (nrow == 0) 
	{

		sprintf(cmd, "insert into usr values('%s','%s',%d,'%s',%d,'%s','%s',%d,'%s')",\
				"admin","123",0,"-",1,"top","-",1,"-");

		if(sqlite3_exec(db, cmd, NULL,NULL, &errmsg) != SQLITE_OK)
		{
			printf("error:%s\n", errmsg);
		}
	}
	//存在
	else  
		return;

}
//添加信息
void addmsg(userdata recvmsg,int accepted)
{
	int status = 0;
	
	//将信息添加到数据库当中去
	char cmd[128];
	sprintf(cmd, "insert into usr values('%s','%s',%d,'%s',%d,'%s','%s',%d,'%s')",\
			recvmsg.username,recvmsg.password,recvmsg.id,recvmsg.address,recvmsg.age,\
			recvmsg.grade,recvmsg.telphone,recvmsg.salary,recvmsg.position);

	if(sqlite3_exec(db, cmd, NULL,NULL, &errmsg) != SQLITE_OK)
	{
		printf("error:%s\n", errmsg);
		status = -1;
	}
	else
		status = 1;
	do_get_table(db,"usr");
	if(send(accepted,&status,sizeof(status),0)<0) err("send_add_server");
}
void user_find(userdata recvmsg,int accepted)
{
	char cmd[128];
	char **ret;
	int nrow, ncol;
	int i;
	char arr[N] = {0};
	//判读id是否存在
	sprintf(cmd, "select * from usr where username = '%s'", recvmsg.username);
	if (sqlite3_get_table(db, cmd, &ret, &nrow, &ncol, &errmsg) != SQLITE_OK)
	{
		printf("error:%s\n", errmsg);
	}
	printf("%d\n",nrow);
	//发送数据
	for (i = 0; i < (nrow + 1) * ncol;i = i + 9)
	{
		bzero(arr, sizeof(arr));
		sprintf(arr, "%s %s %s %s %s %s %s %s %s", ret[i], ret[i + 1], ret[i + 2],ret[i+3],\
				ret[i+4],ret[i+5],ret[i+6],ret[i+7],ret[i+8]);
	}
	printf("%s\n",arr);
	if(send(accepted,arr,N,0)<0) err("server_send_user_find");

}
void user_change(userdata recvmsg,int accepted)
{
	int status = 0,mode = 0;
	char cmd[128];
	char **ret;
	printf("------232---\n");
	printf("%s\n",recvmsg.username);
	printf("%s\n",recvmsg.password);
	//执行修改操作
	sprintf(cmd,"update usr set password='%s',address='%s',\
			telphone='%s' where username='%s'",recvmsg.password,recvmsg.address\
			,recvmsg.telphone ,recvmsg.username);
	int nrow, ncol;
	if (sqlite3_get_table(db, cmd, &ret, &nrow, &ncol, &errmsg) != SQLITE_OK)
	{
		printf("error:%s\n", errmsg);
		mode = -1;
	}
	mode = 1;

	if(send(accepted,&mode,sizeof(mode),0)<0) err("send_delmsg_server");

}
void user_time(userdata recvmsg,int accepted)
{
	char cmd[128];
	char **ret;
	int nrow, ncol;
	int i;
	char arr[N] = {0};
	//判读id是否存在
	sprintf(cmd, "select * from list where user = '%s'", recvmsg.username);
	if (sqlite3_get_table(db, cmd, &ret, &nrow, &ncol, &errmsg) != SQLITE_OK)
	{
		printf("error:%s\n", errmsg);
	}
	printf("%d\n",nrow);
	//发送数据
	for (i = 0; i < (nrow + 1) * ncol;i = i + 3)
	{
		bzero(arr, sizeof(arr));
		sprintf(arr, "%s %s %s", ret[i], ret[i + 1], ret[i + 2]);
	}
	printf("%s\n",arr);
	if(send(accepted,arr,N,0)<0) err("server_send_user_find");
	
}
void user_out(userdata recvmsg,int accepted)
{
	//记录当前时间
	char note[N];
	getTime(timer_out);
	//把要查询的单词和时间存放到记录表中
	sprintf(note, "insert into list values('%s','%s','%s')",recvmsg.username,timer_in,timer_out);
	if (sqlite3_exec(db, note, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		printf("error:%s\n", errmsg);
		printf("%d\n", __LINE__);
	}

}
//子线程负责通信
void *hadler(void*arg)
{

	char buf[N] = {0};
	ssize_t res;
	userdata recvmsg;
	int accepted = *(int*)arg;
	int dataType = 0;
	int ret;
	while(1)
	{
		//接收
		res = recv(accepted,&recvmsg,sizeof(recvmsg),0);
		if(res<0) err("recv");
		else if(0==res)
		{
			printf("client was quit\n");
			pthread_exit(NULL);
		}
		dataType = recvmsg.type;
		printf("type=%d\n",dataType);
	//	do_get_table(db,"usr");
		switch(dataType)
		{
			//登录
		case 1:
			ret = login(recvmsg);
			if(send(accepted,&ret,sizeof(ret),0)<0) err("send");
			break;
			//增加
		case 2:
			addmsg(recvmsg,accepted);
			break;
			//删
		case 3:
			delmsg(recvmsg,accepted);
			break;
			//改
		case 4:
			chmsg(recvmsg,accepted);
			break;
			//查
		case 5:
			find(recvmsg,accepted);
			break;
			//记录
		case 6:
			record(recvmsg,accepted);
			break;
			//退出
		case 7:
			out(recvmsg,accepted);
			break;
		case 8:
			user_find(recvmsg,accepted);
			break;
		case 9:
			user_change(recvmsg,accepted);
			break;
		case 10:
			user_time(recvmsg,accepted);
		case 11:
			user_out(recvmsg,accepted);
			break;
		default:
			printf("未知命令码\n");

		}

	}
	close(accepted);
}
int main()
{
	int on = 1;
	pthread_t tid;
	//套接字	
	int socked = socket(AF_INET,SOCK_STREAM,0);
	if(socked<0) err("socket");
	//设置端口复用
	if (setsockopt(socked, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
	err("setsockopt");

	//填充网络信息结构体
	struct sockaddr_in serveraddr,clientaddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(8888);
	serveraddr.sin_addr.s_addr = inet_addr("192.168.1.186");;
	socklen_t addrlen = sizeof(serveraddr);

	//绑定
	int bd = bind(socked,(struct sockaddr*)&serveraddr,addrlen);
	if(bd<0) err("bind");
	//监听
	if(listen(socked,5)<0) err("listen");
		//创建数据库
	if (sqlite3_open(DBNAME, &db) != SQLITE_OK)
	{
		printf("error:%s", sqlite3_errmsg(db));
		exit(1);
	}
	printf("The database has created\n");

	//创建用户信息表
	char cmd[] = "create table usr(username char,password char,id int,address char\
					,age int,grade char,telphone char,salary int,position char)";

	if (sqlite3_exec(db, cmd, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		printf("error:%s\n", errmsg);
	}
	root();
	//创建查询历史记录表
	char mysql[] = "create table list(user char,timer_in char,timer_out char)";
	if (sqlite3_exec(db, mysql, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		printf("error:%s\n", errmsg);
	}
	printf("The table has created\n");
	//父线程负责连接
	while(1)
	{
		//连接
		int accepted = accept(socked,(struct sockaddr*)&clientaddr,&addrlen);
		if(accepted<0) err("accept");
		//创建线程
		pthread_create(&tid, NULL, hadler, (int *)&accepted);

	}
	pthread_join(tid,NULL);
	return 0;
}
