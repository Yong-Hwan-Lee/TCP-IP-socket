#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
void error_handling(char *webpage);
#define BUFFER_SIZE 1024
#define RESPONSE webpage


int main(int argc, char * argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	char request[] = "GET / HTTP/1.1 200 OK\r\nServer:Linux Web Server\r\n";
	char webpage[BUFFER_SIZE];
	int str_len;

	if(argc!=3)
	{
		printf("Usage : %s <IP> <port> \n", argv[0]);
		exit(1);
	}

	sock=socket(PF_INET, SOCK_STREAM, 0);
	if(sock ==-1)
		error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));


	if(connect(sock,(struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error!");

	connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	send(sock, request, strlen(request), 0);

	memset(webpage, 0, sizeof(webpage));
	while (recv(sock, webpage, BUFFER_SIZE -1,0) >0){
		printf("%s", webpage);
		memset(webpage,0, sizeof(webpage));
	}

	
  str_len=read(sock,  webpage, sizeof(webpage)-1);
	if(str_len==-1)
		error_handling("read() error!");


	
	close(sock);
	return 0;
}

void error_handling(char *webpage)
{
	fputs(webpage, stderr);
	fputc('\n', stderr);
	exit(1);
}
