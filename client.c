/* Assignment: 3
Name: client.c
Creator: Ahwan Mishra (ahwan100@gmail.com)
Date: 24th Jan, 2019


Instruction for executing:
-> Run the server program.
-> Get the port address.
-> Run the Client Program.
-> In commandline argument put the server ip address (127.0.0.1 for local host) followed by 
   port number which you got from server program
-> In client side, Write the operations to do.
*/

#include<netinet/in.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/shm.h>
#include<signal.h>

/* Signal Handler for CTRL+Z and CTRL+C */

int connection_flag=1;
void sigintHandler(int sig_num) 
{ 
    connection_flag=0;
    fflush(stdout); 
} 


main(int argc,char *argv[]){
	signal(SIGINT, sigintHandler);
	signal(SIGTSTP, sigintHandler);
	
	printf("\n:::::: CLIENT ::::::\n\n");
	int soc,addrlen;
	struct sockaddr_in server;
	char buf[1024],bufx[1024];
	soc=socket(AF_INET,SOCK_STREAM,0);

	if(soc<0){
		perror("\n Error in opening socket");
		exit(1);
	}
	
	
	
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=inet_addr(argv[1]);
	server.sin_port=htons(atoi(argv[2]));

	if(connect(soc,(struct sockaddr *) &server,sizeof(server))<0){
		perror("\n Error in connection");
		exit(2);
	}
	
	//Recieve Connection status
	recv(soc,bufx,1024,0);
	if(strcmp(bufx,"exit")==0)   
        	{
        		printf("Connection limited exceeded. (Max user exceeded)\n");
        		exit(0);
        	}
        	else
        	{
        		printf("Connection accepted from the server. Your Client id: %s\n\n",bufx);
        	}
        	
        int childpid;
	if((childpid = fork()) == 0) //Child Process recieves messages
	{
        	while(1)
        	{
        		memset(bufx,0,1024);
        		recv(soc,bufx,1024,0);
        		//printf("Message recieved! \n");
        		printf("\n %s  \n",bufx);
        	}
	}
	else	//Parent process sends messages
	{
		while(1)
        	{
			fgets (buf, 1024, stdin);
			if(connection_flag==1) 
			{
				send(soc,buf,sizeof(buf),0);
				memset(buf,0,1024);
			}
			else 
			{
				send(soc,"Ctrl",sizeof("Ctrl"),0);
				memset(buf,0,1024);
				close(soc);exit(0); 
			}
        		if(strcmp("quit\n",buf)==0) {close(soc);exit(0);}
        	}
	}
	close(soc);
}




