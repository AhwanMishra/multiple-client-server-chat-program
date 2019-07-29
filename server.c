/* Assignment: 3
Name: server.c
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


#define P(s) semop(s, &pop, 1) 
#define V(s) semop(s, &vop, 1)

#define CTRL(x) (#x[0]-'a'+1)
int main()
{
	int i=0;
	int max_user=5;
	int shmid1 = shmget(IPC_PRIVATE, max_user*sizeof(int), 0777|IPC_CREAT); //shared memory1 for Client id
	int shmid2 = shmget(IPC_PRIVATE, max_user*sizeof(int), 0777|IPC_CREAT); //shared memory2 for corresponding connfd
	int num_userx= shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT); //shared memory for number of users currently
	
	
	int shmid10 = shmget(IPC_PRIVATE, sizeof(int[10][10]), 0777|IPC_CREAT); //shared memory10 for group client IDs
	int shmid11 = shmget(IPC_PRIVATE, sizeof(int[10][10]), 0777|IPC_CREAT); //shared memory10 for group connfds
	int num_groupx= shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT); //shared memory for number of groups currently
	int num_groupx_mem= shmget(IPC_PRIVATE, 10*sizeof(int), 0777|IPC_CREAT); //shared memory for number of clients in each groups currently
	int random_num_groupx=shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT); //shared memory for random number for group creation
	int group_idx= shmget(IPC_PRIVATE, 10*sizeof(int), 0777|IPC_CREAT); //shared memory for group ids
	
	
	int *a1,*a2,*a3,*a4,*num_user,*num_group,*num_group_mem,*random_num_group,*group_id;
	int (*b1)[10],(*b2)[10],(*b3)[10],(*b4)[10];
	a1 = (int *) shmat(shmid1, 0, 0); //Parent Process attaches itself to shared memory1
	a3 = (int *) shmat(shmid2, 0, 0); //Parent Process attaches itself to shared memory1
	
	num_user=(int *) shmat(num_userx, 0, 0); //Parent Process attaches itself to number of user
	num_group=(int *) shmat(num_groupx, 0, 0); //Parent Process attaches itself to number of group
	num_group_mem=(int *) shmat(num_groupx_mem, 0, 0); //Parent Process attaches itself to number of group user count
	random_num_group=(int *) shmat(random_num_groupx, 0, 0); //Parent Process attaches itself to the random number for group
	group_id=(int *) shmat(group_idx, 0, 0); //Parent Process attaches itself  for group ids
	
	b1 = (int *) shmat(shmid10, 0, 0); //Parent Process attaches itself to shared memory10
	b3 = (int *) shmat(shmid11, 0, 0); //Parent Process attaches itself to shared memory11
	

	*num_user=0;	//Currently no user is there
	*num_group=0;	//Currently no group is there
	*random_num_group=20000;
	for(i=0;i<10;i++)	num_group_mem[i]=0;	//Currently no member is there in any group
	//shmdt(a1);//At the end the current process detach the shared memory from itself
	//shmctl(shmid1, IPC_RMID, 0); //Parent process should delete the shared memory in the end


	
	int semid1; //Mutex For client table
	struct sembuf pop, vop ;
	
	semid1 = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
	semctl(semid1, 0, SETVAL, 1);	//Mutex for access in to critical section

	pop.sem_num = vop.sem_num = 0;
	pop.sem_flg = vop.sem_flg = 0;
	pop.sem_op = -1 ; vop.sem_op = 1 ;
	
	

	int childpid,client_id=10000;
	printf("\n:::::: SERVER ::::::\n");
	int flag1=0;

	int sockfd,connfd;
	struct sockaddr_in server;
	struct sockaddr_in client;
	int len,addrlen=sizeof(client);
	char buf[1024],buf2[1024],bufx[1024];
	memset(buf,0,1024);
	memset(bufx,0,1024);
	memset(buf2,0,1024);

	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd<0)
	{
		perror("\n Error in opening socket");
		exit(1);
	}

	server.sin_family=AF_INET;
	server.sin_addr.s_addr=htons(INADDR_ANY);
	server.sin_port=0;

	if(bind(sockfd,(struct sockaddr*)&server,sizeof(server))){
		perror("\n Error in bind");
		exit(2);
	}

	len=sizeof(server);
	if(getsockname(sockfd,(struct sockaddr*)&server,&len)){
		perror("\n error in getting port");
		exit(3);
	}

	printf("\n socket port # %d\n",ntohs(server.sin_port));
	listen(sockfd,5);



	clock_t start,end;
	double time_in_sec;
	FILE* fp;
	while(1)
	{
		
		//Put connfd in critical section
		connfd=accept(sockfd,(struct sockaddr *) &client,&addrlen);
		if(connfd==-1)
	        {
			exit(4);
		}
		
		
		
		if(*num_user==max_user)	//Check if maximum user limit exceeded
		{
			//Send disconnect message.
			send(connfd,"exit",sizeof("exit"),0);
			continue;
		}
		else
		{
			client_id++;
			(*num_user)++;
			sprintf(buf, "%d", client_id);
			send(connfd,buf,sizeof(buf),0);
		}
		//printf("\nCurrently number of users is %d\n",*num_user);
		
		
		//Add user to Client table (use synchronization too)
		//Later take care of removing;
	
		P(semid1);
		a1[*num_user-1]=client_id;
		a3[*num_user-1]=connfd;
		V(semid1);
		
		printf("Connection accepted from %s:%d Client id: %d connfd %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port),client_id,connfd);
		
		
		if((childpid = fork()) == 0)
		{
			a2 = (int *) shmat(shmid1, 0, 0); //Child Process (Client) attaches itself to shared memory1 (Client Table)
			a4 = (int *) shmat(shmid2, 0, 0); //Child Process (Client) attaches itself to shared memory2 (connfd Table)
			num_user=(int *) shmat(num_userx, 0, 0); //Child Process attaches itself to number of user
			num_group=(int *) shmat(num_groupx, 0, 0); //Child Process attaches itself to number of group
			num_group_mem=(int *) shmat(num_groupx_mem, 0, 0); //Child Process attaches itself to number of group user count
			random_num_group=(int *) shmat(random_num_groupx, 0, 0); //Child Process attaches itself to the random number for group
			group_id=(int *) shmat(group_idx, 0, 0); //Child Process attaches itself  for group ids
			
			b2 = (int *) shmat(shmid10, 0, 0); //Child Process attaches itself to shared memory10
			b4 = (int *) shmat(shmid11, 0, 0); //Child Process attaches itself to shared memory11
			
			
			//close(sockfd);
			//start=clock();
	        	while(1)
	        	{
	        		char temp1[100],temp2[100],substr1[50],substr2[50],substr3[50];
	        		int i,temp3,temp4,temp5;
				memset(buf,0,1024);
				memset(bufx,0,1024);
				memset(temp1,0,1024);
				memset(temp2,0,1024);
				recv(connfd,buf,1024,0);
				printf("\n :COMMAND from Client %d :    %s  \n",client_id,buf);
				
				strncpy(substr1,buf,7);
				strncpy(substr2,buf,5);
				strncpy(substr3,buf,12);
				
				
				if(strcmp (substr1, "active\n" ) == 0 )
				{
					printf("Hello.");
					sprintf(bufx,"Currently %d users are active.",*num_user);
					send(connfd,bufx,sizeof(bufx),0);
					for(i=0;i<*num_user;i++) 
					{
						sprintf(bufx,"%d",a2[i]);
						send(connfd,bufx,sizeof(buf),0);
					}
				}
				
				
				
				// 2) send <dest client id> <Message>
				else if(strcmp (substr2, "send " ) == 0)
				{
					strncpy(temp1,buf+5,5); //client_id
					strncpy(temp2,buf+11,strlen(buf)); //message is in temp2
					sscanf(temp1,"%d",&temp3); //client_id is in temp3
				

					
					// find the position of client in client table
					temp4=-1;
					for(i=0;i<(*num_user);i++)
					{
						if(a2[i]==temp3) temp4=i;
					}
					//printf("connfd for client id %d is %d \n",temp3,a4[temp4]);
					if(temp4==-1)
					{
						//send wrong error message
						sprintf(bufx,"Wrong Client ID.");
						send(connfd,bufx,sizeof(bufx),0);
					} 
					else // send message to corresponding connfd
					{
						send(a4[temp4],temp2,sizeof(temp2),0);  //connfd=a4[temp4]
					}				
				}
				
				//3) /broadcast <Message>
				else if(strcmp (substr1, "broadca" ) == 0 )
				{
					strncpy(temp2,buf+10,strlen(buf)); //message is in temp2
					for(i=0;i<(*num_user);i++)
					{
						send(a4[i],temp2,sizeof(temp2),0);	
					}
				}
	
					
				else if(strcmp (substr1, "makegro" ) == 0 )
				{
					// 4) makegroup <client id1> <client id2> ... <client idn>
				
					i=0;
					int count=0;
				
					P(semid1);
					int flagx=0,j;
					(*random_num_group)++;
					(*num_group)++;
					group_id[(*num_group)-1]=(*random_num_group);
					while(i+6<strlen(buf))
					{
						strncpy(temp1,buf+10+i,5);
						i=i+6;
						sscanf(temp1,"%d",&temp3); //client_id is in temp3
						b2[(*num_group)-1][num_group_mem[(*num_group)-1]]=temp3;
						num_group_mem[(*num_group)-1]=num_group_mem[(*num_group)-1]+1;
						
						
						temp4=-1;
						for(j=0;j<(*num_user);j++)
						{
							if(a2[j]==temp3) temp4=j;
						}
						if(temp4==-1)
						{
							//send wrong error message
							sprintf(bufx,"Wrong Client ID.");
							send(connfd,bufx,sizeof(bufx),0);
							
							
							flagx=1;
							
							(*random_num_group)--;
							(*num_group)--;
							group_id[(*num_group)-1]=(*random_num_group);
							break;
							
							/*
							group_id[(*num_group)-1]=-1;
							(*random_num_group)--;
							(*num_group)--;
							break;*/
						}	 
						else 
						{
							b4[(*num_group)-1][num_group_mem[(*num_group)-1]]=a4[temp4];
							//b2[(*num_group)-1][num_group_mem[(*num_group)-1]]=a2[temp4];
						}					
						
					}
					if(flagx==0)
					{
						sprintf(bufx,"Group is made succefully. Group ID : %d",*random_num_group);
						send(connfd,bufx,sizeof(bufx),0);
					}
					V(semid1);
				}
				
				
				
				else if(strcmp (substr1, "makegro" ) == 0 && strcmp (substr3, "makegroupreq" ) == 0 )
				{
					// 7) makegroupreq <client id1> <client id2> ... <client idn>
					i=0;
					int count=0;
				
					P(semid1);
					int flagx=0,j;
					(*random_num_group)++;
					(*num_group)++;
					group_id[(*num_group)-1]=(*random_num_group);
					while(i+6<strlen(buf))
					{
						strncpy(temp1,buf+10+i,5);
						i=i+6;
						sscanf(temp1,"%d",&temp3); //client_id is in temp3
						b2[(*num_group)-1][num_group_mem[(*num_group)-1]]=temp3;
						num_group_mem[(*num_group)-1]=num_group_mem[(*num_group)-1]+1;
						
						
						temp4=-1;
						for(j=0;j<(*num_user);j++)
						{
							if(a2[j]==temp3) temp4=j;
						}
						if(temp4==-1)
						{
							//send wrong error message
							sprintf(bufx,"Wrong Client ID.");
							send(connfd,bufx,sizeof(bufx),0);
							flagx=1;
							
							
							group_id[(*num_group)-1]=-1;
							(*random_num_group)--;
							(*num_group)--;
							break;
						}	 
						else 
						{
							sprintf(bufx,"Enter 'Y' if you want to join the group %d, Enter anything else otherwise",*random_num_group);
							send(a4[temp4],bufx,sizeof(bufx),0);
							
							memset(buf2,0,1024);
							recv(connfd,buf2,1024,0);
							if(strcmp(buf2,"Y\n")==0)
							{	
								printf("YOO");
								b4[(*num_group)-1][num_group_mem[(*num_group)-1]]=a4[temp4];
							}
							//b2[(*num_group)-1][num_group_mem[(*num_group)-1]]=a2[temp4];
						}					
						
					}
					if(flagx==0)
					{
						sprintf(bufx,"Group is made succefully. Group ID : %d",*random_num_group);
						send(connfd,bufx,sizeof(bufx),0);
					}
					V(semid1);
				
				}
				
					
				else if(strcmp (substr1, "sendgro" ) == 0 )
				{
					//sendgroup <group id> <Message>​ :
					strncpy(temp1,buf+10,5); //group_id
					strncpy(temp2,buf+16,strlen(buf)); //message is in temp2
					sscanf(temp1,"%d",&temp3); //group_id is in temp3
				
					
					// find the position of group in group table
					temp4=-1;
					for(i=0;i<(*num_group);i++)
					{
						if(group_id[i]==temp3) temp4=i;
					}
					if(temp4==-1)
					{	
						//send error message
						sprintf(bufx,"Wrong Group ID.");
						send(connfd,bufx,sizeof(bufx),0);
					} 
					else
					{
						for(i=0;i<(num_group_mem[temp4]);i++)
						{
							send(b4[temp4][i],temp2,sizeof(temp2),0);	
						}
					}
				}
				
				else if(strcmp (substr1, "activea" ) == 0 ) 
				{
					sprintf(bufx,"Currently %d groups are active.",*num_group);
					send(connfd,bufx,sizeof(bufx),0);
					for(i=0;i<*num_group;i++)
					{
						sprintf(bufx,"%d",group_id[i]);
						send(connfd,bufx,sizeof(buf),0);
					}
				}
				
				else if(strcmp (substr1, "activeg" ) == 0 )
				{
					int flagy=0,j;
					for(i=0;i<*num_group;i++)
					{
						for(j=0;j<num_group_mem[i];j++)
						{
							if(connfd==b4[i][j])
							{
								flagy=1;
								sprintf(bufx,"%d",group_id[i]);
								send(connfd,bufx,sizeof(buf),0);
								break;
							}
						}
					}
					if(flagy==0)
					{
						sprintf(bufx,"You are not in any group.. \n");
						send(connfd,bufx,sizeof(buf),0);
					}
				}
				
				else if(strcmp (substr1, "quit\n" ) == 0 || strcmp (substr1, "Ctrl" ) == 0 )
				{
					int pos=-1,j;

					P(semid1);
					for(i=0;i<(*num_user);i++)
					{
						if(connfd==a4[i])
						{
							pos=i;
							break;
						}
					}
					*num_user=*num_user-1;
					for(i=pos;i<(*num_user);i++)
					{
						a4[i]=a4[i+1];
						a2[i]=a2[i+1];
					}
					
					for(i=0;i<*num_group;i++)
					{
						pos=-1;
						for(j=0;j<num_group_mem[i];j++)
						{
							if(connfd==b4[i][j])
							{
								sprintf(bufx,"%d",group_id[i]);
								pos=j;
								break;
							}
						}
						num_group_mem[i]=num_group_mem[i]-1;
						for(j=pos;j<num_group_mem[i];j++)
						{
							b4[i][j]=b4[i][j+1];
							b2[i][j]=b2[i][j+1];
						}
					}
					V(semid1);
				}
				
				else if(strcmp (substr1, "joingro" ) == 0 )
				{
					strncpy(temp1,buf+10,5);
					sscanf(temp1,"%d",&temp3);
					
					
					//Finding index of the particular group
					temp4=-1;
					for(i=0;i<(*num_group);i++)
					{
						if(group_id[i]==temp3)
						{
							temp4=i;

							break;
						}
					}
					
					//Finding index of the particular client
					temp5=-1;
					for(i=0;i<(*num_user);i++)
					{
						if(a4[i]==connfd)
						{
							temp5=i;
							break;
						}
					}
					
					if(temp4==-1)
					{
						//Send no group exists message
						sprintf(bufx,"No such group exists.");
						send(connfd,bufx,sizeof(bufx),0);
						break;
					}
					else
					{
						P(semid1);
						num_group_mem[temp4]=num_group_mem[temp4]+1;
						b4[temp4][num_group_mem[temp4]-1]=a4[temp5];
						b2[temp4][num_group_mem[temp4]-1]=a2[temp5];
						sprintf(bufx,"You are added to the group %d \n",temp3);
						send(connfd,bufx,sizeof(bufx),0);
						V(semid1);
					}
				}
				else
				{
					sprintf(bufx,"Invalid Command.");
					send(connfd,bufx,sizeof(bufx),0);
				}
			
			}	
		}	
        		
	}	
	close(sockfd);
	close(connfd);
	return 0;
}
