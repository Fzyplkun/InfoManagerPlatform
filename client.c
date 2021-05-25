#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#define N 128
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

}info;
typedef struct
{
	int row;
	int col;
}list;
char name[N] = {0};
//增加信息
void addmsg(int socked)
{
//	printf("添加用户信息\n");
	int res;
	info msg;
	msg.type = 2;
	printf("请输入姓名:");
	scanf("%s",msg.username);

	printf("请输id:");
	scanf("%d",&msg.id);

	printf("请输名密码:");
	scanf("%s",msg.password);

	printf("请输入地址:");
	scanf("%s",msg.address);
	
	printf("请输入年龄:");
	scanf("%d",&msg.age);

	printf("请输入等级:");
	scanf("%s",msg.grade);

	printf("请输入手机号:");
	scanf("%s",msg.telphone);

	printf("请输入薪水:");
	scanf("%d",&msg.salary);

	printf("请输入职位:");
	scanf("%s",msg.position);

	if(send(socked,&msg,sizeof(msg),0)<0) err("send_addmsg");
	if(recv(socked,&res,sizeof(res),0)<0) err("recv_addmsg");
	if(1==res)
		printf("添加成功\n");
	else
		printf("添加失败\n");
}
//删除信息
void delmsg(int socked)
{
	info msg;
	int res;
	msg.type = 3;
	printf("请输入你要删除id:");
	scanf("%d",&msg.id);
	if(send(socked,&msg,sizeof(msg),0)<0) err("send_delmsg");
	if(recv(socked,&res,sizeof(res),0)<0) err("recv_delmsg");
	if(res==-1)
		printf("删除失败\n");
	else
		printf("删除成功\n");
}
void chmsg(int socked)
{
	info msg;
	int res,req;
	msg.type = 4;
	printf("请输入你要查询id:");
	scanf("%d",&msg.id);
	//发送查询请求
	if(send(socked,&msg,sizeof(msg),0)<0) err("send_delmsg");
	if(recv(socked,&res,sizeof(res),0)<0) err("recv_delmsg");
	if(res==-1)
		printf("信息不存在\n");
	else
	{
		printf("请输名密码:");
		scanf("%s",msg.password);

		printf("请输入地址:");
		scanf("%s",msg.address);
/*	
		printf("请输入手机号:");
		scanf("%s",msg.telphone);
*/
		//发送修改请求
		if(send(socked,&msg,sizeof(msg),0)<0) err("send_chmsg");
		//接受返回状态码
		if(recv(socked,&req,sizeof(req),0)<0) err("recv_chmsg");
		if(req==-1)
			printf("修改失败\n");
		else
			printf("修改成功\n");
	}

}
void findmsg(int socked)
{
	info msg;
	list excel;
	msg.type = 5;
	int num,res;
	char arr[N];
	int i,j,n;
	printf("----------------------------------------\n");	
	printf("--1.查看单个信息 2.查看全部信息 3.返回--\n");
	printf("----------------------------------------\n");
	printf("请输入您的选择>>:");
	scanf("%d",&num);

	if(send(socked,&msg,sizeof(msg),0)<0) err("send_findmsg");
	if(send(socked,&num,sizeof(num),0)<0) err("send_choice");
	//查看单个信息
	if(1==num)
	{
		
		printf("请输入你要查询的id:");
		scanf("%d",&msg.id);
		if(send(socked,&msg,sizeof(msg),0)<0) err("send_findmsg");
		if(recv(socked,&res,sizeof(res),0)<0) err("recv_findmsg");
		if(res==-1)
			printf("没有该信息,查找失败\n");
		else
		{
			if(recv(socked,arr,N,0)<0) err("recv_data");
			printf("姓名  | 密码 | id | 地址 | 年龄 | 等级 | 电话 | 薪水 | 职位\n");
			printf("%-11s  \n",arr);
		}
		
	}
	//查看全部信息
	else if(2==num)
	{
		if(recv(socked,&excel,sizeof(excel),0)<0);
		for(i=0;i<(excel.row+1)*(excel.col);i+=9)
		{	
			if(recv(socked,arr,N,0)<0) err("recv_record");
			printf("%-11s\n",arr);
		}
	}
	else return;
}
void record(int socked)
{
	list excel;
	info msg;
	msg.type = 6;
	char arr[N] = {0};
	int i;
	if(send(socked,&msg,sizeof(msg),0)<0) err("send_record");
	if(recv(socked,&excel,sizeof(excel),0)<0);
	for(i=0;i<(excel.row+1)*(excel.col);i+=9)
	{	
		if(recv(socked,arr,N,0)<0) err("recv_record");
		printf("%-11s \n",arr);
	}

}

void out(int socked)
{
	info msg;
	msg.type = 7;

	if(send(socked,&msg,sizeof(msg),0)<0) err("out");
	printf("已退出\n");
	exit(1);
}
void admin_menu(int socked)
{
	int num;
	while(1)
	{
		printf("***********************************\n");
		printf("***********************************\n");
		printf("  1.添加用户 2.删除用户 3.修改信息 \n");
		printf("  4.查询信息 5.考勤记录 6.退出系统 \n");
		printf("***********************************\n");
		printf("***********************************\n");
		printf("请输入选择>>:");
		scanf("%d",&num);
		switch(num)
		{
			case 1:
				addmsg(socked);
				break;
			case 2:
				delmsg(socked);
				break;
			case 3:
				chmsg(socked);
				break;
			case 4:
				findmsg(socked);
				break;
			case 5:
				record(socked);
				break;
			case 6:
				out(socked);
			default:
				printf("选择错误,请重试\n");
		}
	}

}
void lookmsg(int socked)
{
	info msg;
	char buf[N] = {0};
	msg.type = 8;
	strcpy(msg.username,name);
	if(send(socked,&msg,sizeof(msg),0)<0) err("send_user");
	if(recv(socked,buf,N,0)<0) err("recv_user");
	printf("%s\n",buf);
}
void modfiy(int socked)
{
	info msg;
	char buf[N] = {0};
	msg.type = 9;
	int req = 0;
	strcpy(msg.username,name);
	printf("请输入修改的密码:");
	scanf("%s",msg.password);
	printf("请输入修改的手机号:");
	scanf("%s",msg.telphone);
	printf("请输入要修改的地址:");
	scanf("%s",msg.address);
	if(send(socked,&msg,sizeof(msg),0)<0) err("send_user");
	//接受返回状态码
	if(recv(socked,&req,sizeof(req),0)<0) err("recv_chmsg");
	if(req==-1)
		printf("修改失败\n");
	else
		printf("修改成功\n");

}
void schdule(int socked)
{
	info msg;
	char buf[N] = {0};
	msg.type = 10;
	strcpy(msg.username,name);
	if(send(socked,&msg,sizeof(msg),0)<0) err("send_user");
	if(recv(socked,buf,N,0)<0) err("recv_user");
	printf("%s\n",buf);

}
void out_user(int socked)
{
	info msg;
	msg.type = 11;

	if(send(socked,&msg,sizeof(msg),0)<0) err("out");
	printf("已退出\n");
	exit(1);

}

void user_menu(int socked)
{
	int num = 0;
	while(1)
	{
     	printf("***********************************\n");
		printf("***********************************\n");
		printf("*  1.查看信息     2.修改信息      *\n");
		printf("*  3.考勤记录     4.退出系统      *\n");
		printf("*  5.返回上一级                   *\n");
		printf("***********************************\n");
		printf("***********************************\n");
		printf("欢迎进入管理界面,请输入功能选择>>:");
		scanf("%d",&num);
		switch(num)
		{
			case 1:
				lookmsg(socked);
				break;
			case 2:
				modfiy(socked);	
				break;
			case 3:
				schdule(socked);
				break;
			case 4:
				out_user(socked);
				break;
			case 5:
				printf("已返回\n");
				return;
			default:
				printf("输入错误,请重试\n");

		}
	}
}
void menu(int socked)
{
	info msg;
	int res,count = 3;
	printf("***************************************\n");
	printf("***************************************\n");
	printf("*********员 工 管 理 系 统 ************\n");
	printf(" 									   \n");
	printf("*********writer:beautiful boy**********\n");
	//登录
	printf("请输入用户名:");
	msg.type = 1;
	scanf("%s",msg.username);
	printf("请输入密码:");
	scanf("%s",msg.password);
	if(send(socked,&msg,sizeof(msg),0)<0) err("send");
	if(recv(socked,&res,sizeof(res),0)<0) err("recv");
	switch(res)
	{
	case -1:
		printf("信息错误,请重试,错误码:%d\n",res);
		count--;
		printf("还有%d次机会\n",count);
		if(0==count)
		{
			printf("次数用完,再见\n");
			exit(1);
		}
		break;
	case 1:
		admin_menu(socked);
		break;
	case 2:
		strcpy(name,msg.username);
		printf("%s\n",name);
		user_menu(socked);
		break;
	default:
		printf("未知错误\n");
	
	}
	
}
int main()
{
	int on = 1;
	//套接字	
	int socked = socket(AF_INET,SOCK_STREAM,0);
	if(socked<0) err("socket");
	//填充网络信息结构体
	struct sockaddr_in serveraddr,clientaddr;
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_addr.s_addr = inet_addr("192.168.1.186");
	clientaddr.sin_port = htons(8888);
	socklen_t addrlen = sizeof(clientaddr);
	//绑定
	int ct = connect(socked,(struct sockaddr*)&clientaddr,addrlen);
	if(ct<0) err("ct");
	//通信
	char buf[N];
	info msg;
	while(1)
	{
		menu(socked);
	}
	return 0;
}
