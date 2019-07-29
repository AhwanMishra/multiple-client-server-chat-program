/* Assignment: 4
Name: client.c
Creator: Ahwan Mishra (ahwan100@gmail.com)
Date: 31st Jan, 2019


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
	FILE* fp;
	char buf[10000],bufx[10000],temp1[10000],temp2[10000],temp3[10000];
	char *buffer;
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
	recv(soc,bufx,10000,0);
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
        		char file_name[50],file_content[10000];
        		int i,count;
        		memset(bufx,0,10000);
        		memset(file_name,0,50);
        		memset(file_content,0,10000);
        		recv(soc,bufx,10000,MSG_DONTWAIT);
        		
        		if(strlen(bufx)>0){
        		strncpy(temp1,bufx,5);
        		if(strcmp(temp1,"$file")==0)
        		{
        			i=5;
        			count=0;
        			while(bufx[i]!='$')
        			{
        				i++;
        				count++;
        			}
        			strncpy(file_name,bufx+5,count-1);
        			strncpy(file_content,bufx+5+count+1,strlen(bufx));
        			printf("File (%s) is downloaded. \n",file_name);
        			
        			//Writing to the file
        			fp = fopen(file_name, "a+");
				fprintf(fp, "%s",file_content); 
				fclose(fp);
        		}
        		else
        		{
        			printf("\n %s  \n",bufx);			
			}
		}
        	}
	}
	else	//Parent process sends messages
	{
		while(1)
        	{
        		char c1;
        		int i;
        		memset(buf,0,10000);
			fgets (buf, 10000, stdin);
			unsigned long fileLen;
			if(connection_flag==1) 
			{
			
			
				strncpy(temp1,buf,7);
				
        			if(strcmp (temp1, "sendfil" ) == 0)
				{
					strncpy(temp2,buf+15,strlen(buf)-16);
					//printf("File Name: %s", temp2);
					
					
					fp = fopen(temp2, "r");
					if(!fp)
					{
						printf("Wrong file name! \n");
						continue;
					}
					
					
					//Putting the file content in to file buffer
					i=0;
					temp3[i]='$';
					i++;
					while( (c1=fgetc(fp))!=EOF) 
					{
    						temp3[i]=c1;
    						i++;
    						if(i>9500)
    						{
    							printf("File length is huge ! Not supported. ! \n");
    							continue;
    						}
					}
					
					fclose(fp);
					strcat(buf,temp3);			
					
				}
			
				if(strlen(buf)>0) send(soc,buf,sizeof(buf),0);
			}
			else 
			{
				send(soc,"Ctrl",sizeof("Ctrl"),0);
				close(soc);exit(0); 
			}
        		if(strcmp("quit\n",buf)==0) {close(soc);exit(0);}
        	}
	}
	close(soc);
}
