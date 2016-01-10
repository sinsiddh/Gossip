/***************************************************
 * * Group Info
 * * ssingh24 Siddharth Singh
 * * skumar23 Sudhendu Kumar
 * * mghegde Mahendra Hegde
 * ***************************************************/

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include "neigh.h"

int *dead_nodes;
int dead_count = 0;
char TYPE_IP[3] = "F0";
char TYPE_HB[3] = "F1";
char TYPE_LTIME[3] = "F2";
char TYPE_COUNT[3] = "F3";

extern Neigh_List *head;

pthread_t server;
pthread_mutex_t lock;

int sendok=0;
int recieveok=0;
char myaddress[100];
int b = 0;
int N = 0;
int my_pos;
int isBadNode = 0;
int seeds = 0;
int MAX_FAIL_TIMER = 0;

int countlines(FILE *fp) {
	char ch;
	int number=0;

	while ((ch = fgetc(fp) )!= EOF ) {
		if (ch == '\n') {
			number++;	
		}
	} 	
	return number;
} 

int find_position (FILE *fp) {

  char str[100];
  int pos=0;

  while (fgets(str,100,fp)) {
	pos++;
	if (strcmp(str,myaddress) == 0) {
		return pos;
	} 	
  } 

  return -1;
}

//void send_ok(char *neighbor, int isOK) {
void send_ok(char *neighbor) {

	int sockfd,n;
        struct sockaddr_in servaddr,cliaddr;

	char *token_array;

	token_array = strtok(neighbor, " ");

	char *ip = token_array;
	
	token_array = strtok(NULL, " ");
	char port[7];
	strcpy(port, token_array);

  	sockfd=socket(AF_INET,SOCK_DGRAM,0);

   	bzero(&servaddr,sizeof(servaddr));
   	servaddr.sin_family = AF_INET;
   	servaddr.sin_addr.s_addr=inet_addr(ip);
   	servaddr.sin_port=htons(atoi(port));

	if (sendto(sockfd,"OK\n",3,0,(struct sockaddr *)&servaddr,sizeof(servaddr)) == -1) {
		printf("FAiled to send OK to neighbor\n");
	}
	close(sockfd);
}

void send_line(char * neighbor, char *sendline, int len)
{
   int sockfd,n;
   char neighbors[100];
   strcpy(neighbors, neighbor);
   char *token_array;
   struct sockaddr_in servaddr,cliaddr;
   token_array = strtok(neighbors, " ");
   char *ip = token_array;
   token_array = strtok(NULL, " ");
   char port[7];
   strcpy(port, token_array);

   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr(ip);
   servaddr.sin_port=htons(atoi(port));

   if (sendto(sockfd,sendline,len,0,(struct sockaddr *)&servaddr,sizeof(servaddr)) == -1) {
   }
   close(sockfd);
}

void update_neighbor_list(char *addr,int hbeat,int l_time,int cnt) {
	
   pthread_mutex_lock(&lock);
   Neigh_List* ptr=head;
   
   while(ptr != NULL) {
	if (strlen(ptr->address) != strlen(addr)) {
	}

	int sub=0;
	if (ptr->address[strlen(ptr->address)-1] == '\n') {
		sub =1;
	} else {
		sub=0;

	}

	if (strncmp(ptr->address,addr,strlen(ptr->address)-sub) == 0) {
  
		if (ptr->heartbeat < hbeat) {
			ptr->heartbeat =hbeat;
			ptr->counter = 0;
			ptr->failbit = 0;				
		}

		ptr = ptr->next;

  		pthread_mutex_unlock(&lock);
		return;
	}
 	
	ptr=ptr->next;		
   } 

   /*If controle comes here it indicates this IP is not present in myList. So I'll add it */

  addr[strlen(addr)]= '\n';
  addr[strlen(addr)+1]= '\0';
  insert_node(addr, hbeat, 0, 0, 0, my_pos, 0);

  pthread_mutex_unlock(&lock);
}

//Sid: Function to fail myself if timers are a factor of P
int try_fail(int N, int my_pos, int B, int P, int random_no) {
   //Sid: Get my pointer in the list
   Neigh_List* ptr=head;
   while (ptr != NULL) {
       if (strcmp(ptr->address, myaddress) == 0) {
          //Sid: Check that time is a factor of P
          if ((ptr->localtime%P) == 0) {
             //Sid: Try to fail.Check if random no % N = my_number
             if (((random_no%N)+1) == my_pos) {
                //Sid: Set Bad Node
                isBadNode = 1;
             }
          }
          ptr=ptr->next;
          break;
       }
   }
}

void update_timer_vals( ) {
   Neigh_List* ptr=head;
	pthread_mutex_lock(&lock);

   while(ptr != NULL) {

	if (strcmp(ptr->address, myaddress) == 0) {
		ptr->heartbeat++;
		ptr->localtime++;	
	        ptr=ptr->next;
		continue;
	} else {
		
	}

	if ( (ptr->counter >=MAX_FAIL_TIMER) && (ptr->failbit ==0)) {
		/*Node failed. Not recieving any message */
		ptr->failbit=1;
		int ret = sel_new_neigh(1,N,seeds,myaddress,ptr->address);
	}
	ptr->counter++;
	ptr=ptr->next;
   }

   pthread_mutex_unlock(&lock);
}

void send_neighbor_entries(int I)
{
   Neigh_List* ptr=head;

   char line[8000];
   int index=0;
   char str[100];
   int sockfd,n;
   struct sockaddr_in servaddr,cliaddr;
  
   bzero(line,sizeof(char)*8000);
 
   int count =0;
   char actr[20];
   char lctr[20];
   char ahbeat[20];

   while (ptr != NULL) {

      line[index++] = TYPE_IP[0];
      line[index++] = TYPE_IP[1];
      line[index++] = TYPE_IP[2];
      strcpy(line+index, ptr->address);
      index += strlen(ptr->address);

      line[index++] = TYPE_HB[0];
      line[index++] = TYPE_HB[1];
      line[index++] = TYPE_HB[2];
      sprintf(ahbeat, "%d", ptr->heartbeat);
      memcpy(&line[index], ahbeat, strlen(ahbeat)+1);
      index += strlen(ahbeat)+1;

      sprintf(lctr, "%d", ptr->localtime);
      line[index++] = TYPE_LTIME[0];
      line[index++] = TYPE_LTIME[1];
      line[index++] = TYPE_LTIME[2];
      memcpy(&line[index], lctr, strlen(lctr)+1);
      index += strlen(lctr)+1;

      line[index++] = TYPE_COUNT[0];
      line[index++] = TYPE_COUNT[1];
      line[index++] = TYPE_COUNT[2];
      sprintf(actr, "%d", ptr->counter);
      memcpy(&line[index], actr, strlen(actr)+1);
      index += strlen(actr)+1;
      
      ptr = ptr->next;
      count ++;
   }

  char neighbor[100];
  int pos=0;

  ptr =head;

	int j;
  while(ptr != NULL) {
       if ((strcmp(myaddress, ptr->address )==0) || (ptr->is_neigh ==0) ) {
             /*Skip me */
	     ptr=ptr->next;
       	     continue;
       }
       if (ptr->failbit == 1 ) {
	        ptr=ptr->next;
		continue; 		
       }
 	if (ptr == NULL ) {
		printf("Pointer is NULL\n");
		exit(0);
	}
	
       send_line(ptr->address,line, index);
       ptr=ptr->next;
  }
}


void *serverthread(void * arg) {
   FILE *fp;
   socklen_t len;
   char mesg[3001];
   int sockfd,n;

   time_t t = time(0); 
   
   int myport;
   int N = *(int *)arg;
   srand(t);

   struct sockaddr_in servaddr,cliaddr;
   sockfd=socket(AF_INET,SOCK_DGRAM,0);
   
   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
   myport =(rand()%64000)+1025;
   servaddr.sin_port=htons(myport);
   
   while (bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) == EADDRINUSE) {
	myport = (rand()%64000)+1025;
   	servaddr.sin_port=htons(myport);
   }

   fp =fopen("endpoints","a"); 

   if (fp == NULL ) {
		printf("File open failed\n");
		exit(0);
   }
   
   char address[100];
   sprintf(address,"%s %d\n","127.0.0.1",myport);
  
   fputs(address,fp);
   fclose(fp);

   strcpy(myaddress,address);

   fp =fopen("endpoints","r"); 

   if (fp == NULL ) {
                exit(0);
   }

   int count = countlines(fp);
   fclose(fp);

   if (count == N ) {
	sendok =1;
   }
 
   char type_addr[3],addr[100];
   char type_hb[3],hb[50] ;
   char type_ltime[3],ltime[50] ;
   char type_count[3],kount[50];

   int hbeat,l_time,cnt;

   int in;
   
   for (;;)
   {
      len = sizeof(cliaddr);
      n = recvfrom(sockfd,mesg,3000,0,(struct sockaddr *)&cliaddr,&len);
      in=0;
      if (strncmp(mesg,"OK\n",3)== 0) {
         recieveok=1;
      } else {
	 char buf[3000];
	 memcpy(buf, mesg, n);
         int j;
	 int l;

	 while (in < n) {

	 	memcpy(type_addr,buf,3);
	 	if (strcmp(type_addr,TYPE_IP)==0) {
			in+=3;
			l=0;
			for (j=in;j<in+100;j++) {
		   		if (buf[j]=='\n' || buf[j] =='\0') {	
					addr[l++]='\0';
					break;
		   		}		
		
		   		addr[l++]=buf[j];				   	
			}

			in+=l;

	 	} else {
	 	} 

	 	memcpy(type_hb,&buf[in],3);

	 	if (strcmp(type_hb,TYPE_HB)==0) {
              	in+=3;
              	l=0;
              	for (j=in;j<in+100;j++) {
                	 if (buf[j]=='\n'|| buf[j] == '\0') {  
                     	 hb[l++]='\0';
                    	 break;
                 	 }    

                	 hb[l++]=buf[j];
              	}

              	in+=l;
	      	hbeat=atoi(hb);


		} else {
			hbeat=0;
		}

         	memcpy(type_ltime,&buf[in],3);

         	if (strcmp(type_ltime,TYPE_LTIME)==0) {
              		in+=3;
              		l=0;
              		for (j=in;j<in+50;j++) {
                 		if (buf[j]=='\n' || buf[j] == '\0') {
                     			ltime[l++]='\0';
                     			break;
                 		}
                 
                 		ltime[l++]=buf[j];
              		}
              
              		in+=l;
	      		l_time=atoi(ltime);

        	} else {
			l_time=0;
        	}

         	memcpy(type_count,&buf[in],3);

         	if (strcmp(type_count,TYPE_COUNT)==0) {
              		in+=3;
              		l=0;
              		for (j=in;j<in+50;j++) {
                 		if (buf[j]=='\n' || buf[j] == '\0') {
                     			kount[l++]='\0';
                     			break;
                 		}

                 		kount[l++]=buf[j];
              		}

              		in+=l;
	      		cnt = atoi(kount);

        	} else {
			cnt=0;
        	}

       	update_neighbor_list(addr,hbeat,l_time,cnt);
        bzero(&hbeat,sizeof(hbeat));
        bzero(&addr,sizeof(addr));
        bzero(&l_time,sizeof(l_time));
        bzero(&cnt,sizeof(cnt));
      	}
      }
  } 
}

int main(int argc, char**argv)
{
   int sockfd,n;
   struct sockaddr_in servaddr,cliaddr;
   socklen_t len;
   char mesg[1001];
   int random_no;
   int i = 0;
   //Sid: Local Counter for time
   int current_time = 0;   

   N = atoi(argv[1]);	//Number of nodes
   b = atoi(argv[2]);   //Number of neighbors
   int c = atoi(argv[3]); //Number of iterations
   int F = atoi(argv[4]); //Number of seconds after which a node is considered to be dead
   MAX_FAIL_TIMER = F;
   int B = atoi(argv[5]); //Number of "BAD" nodes
   int P = atoi(argv[6]); //Number of seconds to wait between failure
   int I;
   int S = atoi(argv[7]); //SEED
   //Sid: Total Time
   int T = atoi(argv[8]);

   dead_nodes = (int*)malloc(B*sizeof(int));
   //Sid: Initialize dead nodes with -1.
   for ( i = 0; i<B; i ++) {
       dead_nodes[i] = -1;
   }
   pthread_mutex_t mut;
   pthread_mutex_init(&mut, NULL);

   pthread_mutex_init(&lock, NULL);

   /*Create Server thread */

   pthread_create(&server, NULL, serverthread, &N);


   while ((sendok ==0) && (recieveok ==0)) ; 

   if (recieveok == 1) {
   }

   FILE *fp =fopen("endpoints","r");
   if (fp == NULL ) {
              printf("File open failed\n");
              exit(0);
   }

   I = find_position(fp);
   /* Read my position */
   if (sendok ==1 )
   {
   	char neighbor[100]; 
   	fseek(fp, 0, SEEK_SET);
   	int pos=0;
   	while(fgets(neighbor,100,fp))
        {
		if (++pos == I )
                {
			/*Skip me */
			continue;
		}
 		send_ok(neighbor);
   	}
   } 

   seeds = S +I;
   rand_r(&seeds);
   my_pos =I;

   fseek(fp, 0, SEEK_SET);
   sel_neigh(fp, I, b, N, S);
   
   sleep(20);

   int s;
   //Sid: Loop till time < T
   while(current_time < T)
   {    
        if (current_time%P == 0 && current_time != 0) {
           while (1) {
               random_no = rand_r(&S);
               if (is_unique(((random_no%N)+1), dead_nodes, B) == 1) {
                  dead_nodes[dead_count] = (random_no%N)+1;
                  dead_count++;
                  break;
               }
           }
        }
	for (s=0;s<c;s++)
	{
		if (isBadNode){
                   sleep(1);
		} else {
   		   send_neighbor_entries(I);
		   sleep(1);
                   //Sid: Check if this node has to fail
                   if (dead_count <= B && current_time%P == 0 && current_time!= 0) {
                       try_fail(N, my_pos, B, P, random_no);
                   }
		   update_timer_vals();
		}
        	current_time++;
        }
	if(isBadNode == 0)
	{
		int ret = sel_new_neigh(b,N,seeds,myaddress,NULL);
	}
   }
        FILE *fp1;
	char filename[6] = "list";
	char list[3];
	sprintf(list,"%d",my_pos);
	strncat(filename,list, 4);
	fp1 = fopen(filename,"w");
	if(isBadNode)
		fprintf(fp1,"FAIL\n");
	else
		fprintf(fp1,"OK\n");
	Neigh_List *ptr;
	ptr = head;
	int lol =0;
	while(ptr!=NULL)
	{
		int temp = ptr->my_pos;
		if(lol == 0)
			fprintf(fp1,"My Pos: %d Neighbour Flag: %d Address: %s Heartbeat: %d Failed Node: %d\n",ptr->my_pos,ptr->is_neigh, ptr->address, ptr->heartbeat, ptr->failbit);
		else if(lol == ptr->my_pos);
		else
			fprintf(fp1,"Other Nodes: %d NeighbourFlag: %d Address: %s Heartbeat: %d Failed Node: %d\n",lol,ptr->is_neigh, ptr->address, ptr->heartbeat, ptr->failbit);
		ptr=ptr->next;
		lol++;
	}
	fclose(fp1);
}

