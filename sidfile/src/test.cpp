#include <stdlib.h> 
#include <stdio.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <Log.h>

main(int argc, char **argv) { 
	int iPort;
	
	iPort = atoi(argv[1]);
	int sockfd = 0;
	struct sockaddr_in server_addr;
	struct hostent *server_name;
	char buf[1024];
	int n = 0;
	
	memset(buf, 0, sizeof(buf));
	
	// 1.搞定服务器地址
	server_name = gethostbyname("132.122.1.204");
	printf("server_name = %s", server_name);
	//cout << "server_name:" << server_name<<endl;
	cout << "iPort : " <<iPort<<endl;
	
	/* 2.客户程序开始建立 sockfd描述符 */ 
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1) { 
		fprintf(stderr,"create socket Error:%s\a\n",strerror(errno)); 
		exit(1); 
	} 
	/* 3.客户程序填充服务端的资料 */ 
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family=AF_INET; 
	server_addr.sin_port=htons(iPort); 
	server_addr.sin_addr.s_addr=((struct in_addr *)(server_name->h_addr))->s_addr; 
	 /*if( inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0){
        printf("inet_pton error for %s\n",argv[1]);
        exit(0);
    }*/
	/* 4.客户程序发起连接请求 */ 
	if(connect(sockfd,(struct sockaddr *)(&server_addr),sizeof( server_addr) )==-1) { 
		fprintf(stderr,"Connect Error:%s\a\n",strerror(errno)); 
		exit(1); 
	} 

	/* 5.发送消息 */ 
	printf("send message to server :\n");
	char		cMsg[430];
	memset(cMsg, 0, sizeof(cMsg));
	strcpy(cMsg, "000043J00000/home/shcj/bin;/home/shcj/env;1");
	//strcpy(cMsg, "000043K000001");
	
	for (int i=0;i<5;i++){
		printf("i = %d\n", i);
		if (send(sockfd, cMsg, sizeof(cMsg), 0) < 0){
			printf("send message error : %s(errno: %d)\n",strerror(errno),errno);	
		}
	}
	close(sockfd);
	exit(0);

} 
