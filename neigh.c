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
#include "./neigh.h"


Neigh_List *head = NULL;
extern char myaddress[100];

int insert_node(char *address,int heartbeat, int failbit, int localtime, int counter, int my_pos, int is_neigh) {
   Neigh_List *node = (Neigh_List *)malloc(sizeof(Neigh_List));
   Neigh_List * ptr = NULL;
   strncpy(node->address, address, 100);
   node->heartbeat = heartbeat;
   node->failbit = failbit;
   node->localtime = localtime;
   node->counter = counter;
   node->my_pos = my_pos;
   node->is_neigh = is_neigh;
   //Sid: Check if this is the first neighbour being inserted into the list
   if (head == NULL)
   {
      head = node;
      head->next = head->prev = NULL;
      return SUCCESS;
   }

   //Sid: If not first get to the last node in the list and insert the new Node there
   ptr = head;
   while (ptr->next != NULL) {
      ptr = ptr->next ;
   }

   ptr->next = node;
   node->next = NULL;
   node->prev = ptr;
   return SUCCESS;
}

int delete_dead_node(char *address) {
   //Sid: If list is empty
   if (head == NULL) {
      return FAILURE;
   }
   Neigh_List *ptr = NULL;
   ptr = head;
   //Sid: If Head is the only node
   if (ptr->next == NULL && ptr->prev == NULL) {
      if (strncmp(ptr->address, address, 100) == 0) {
         //Sid: Head node is reserved for myself so i wont delete this entry
         //free(head);
         //head = NULL;
         return SUCCESS;
      }
   }
   
   //Sid: If the first node has to be deleted
   if (strncmp(ptr->address, address, 100) == 0) {
      //Sid: Head node is reserved for myself so i wont delete this entry
      //head = head->next;
      //free(ptr);
      //head->prev = NULL;
     // head = ptr;
      return SUCCESS;
   }
   
   //Sid: Skip the first node
   ptr = ptr->next;
   //Sid: Loop through the list. Search for the dead node
   while (ptr->next != NULL) {
      if (strncmp(ptr->address, address, 100) == 0) {
         ptr->prev->next = ptr->next;
         ptr->next->prev = ptr->prev;
        // free(ptr);
         return SUCCESS;
      }
      ptr = ptr->next;
   }

   //Sid: Deleting the last node
   if (strncmp(ptr->address, address, 100) == 0) {
      ptr->prev->next = NULL;
      //free(ptr);
   }
   return FAILURE;
}

void display_List(Neigh_List* ptr) {
   while (ptr != NULL) {
      if (ptr->next != NULL) {
         ptr = ptr->next;
      } else {
         return;
      }
   }
}

int reset_neigh()
{
	Neigh_List *ptr;
	int size = 0;
	ptr = head;
	while(ptr != NULL)
	{
		ptr->is_neigh = 0;
		ptr = ptr->next;
		size++;
	}
	return size;
}

int is_unique(int n, int *sel_random, int b){
   int i =0;
   for (i = 0; i <b; i++) {
      if (n == sel_random[i]) {
         return 0;
      }
   }
   return 1;
}

int sel_neigh(FILE *fp, int my_pos, int b, int N, int seeds) {
  Neigh_List * ptr = NULL;
  int neigh_no;
  int count = 0;
  char str[100];
  int pos=0;
  int i = 0;
  int *sel_random = (int*) malloc(b * sizeof(int));
  //Sid: Initialized sel_random with -1
  for (i=0; i<b; i++) {
     sel_random[i] = -1;
  }
  sel_random[0] = my_pos;

  /*Mahendra: Add yourself to list.But dont set neighbor bit */
  insert_node(myaddress, 0, 0, 0, 0, my_pos, 0);

  while (fgets(str,100,fp)) {
     //Sid: Insert all nodes from endpoints
     insert_node(str, 0, 0, 0, 0, my_pos, 0);
  }
  
  ptr = head;
  //Sid: Pick b neighbours from file endpoints
  while (count != b) {
     //Sid: Keep selecting a random number between 1 and N till it is not equal to my_pos
     while (1) {
     //Sid: Adjust the picked neighbour number to start from 1
        neigh_no = (rand_r(&seeds)%N) +1;
        if ((is_unique(neigh_no, sel_random, b))== 1) {
           count++;
           sel_random[count] = neigh_no;
           break;
        }
     }
     
     //Sid: Read the neigh_no line from endpoints
     while(ptr != NULL) { 
        pos++;
        if (pos == neigh_no){ 
           ptr->is_neigh = 1;
           pos = 0;
           ptr = head;
           break;
        }
        ptr = ptr->next;
     }
  }
}

int checkIfNeighborsAvail(int numberOfNodes)
{
	Neigh_List *ptr;
	ptr = head;
	int count = 0;
	while(ptr!=NULL)
	{
		ptr=ptr->next;
		if(ptr->is_neigh == 1 || ptr->failbit ==1)	
			count++;
		if(count == numberOfNodes - 2)
			return 0;
		else
			return 1;
	}
}

int sel_new_neigh(int numberOfNeighbors, int numberOfNodes, int seeds, char* myAddress, char* deletedNodeAddress)
{
   	Neigh_List * ptr = NULL;
	int size;
	ptr = head;
	int neighborsSelected = 0;
	int i;
	int flag = 0;
	int ret = checkIfNeighborsAvail(numberOfNodes);
	if(ret == 0)
		return FAILURE;

	if (deletedNodeAddress == NULL) {
	    size = reset_neigh();
        }
	else
	{
		size = numberOfNodes;
		while(ptr!=NULL)
		{
			if((strcmp(ptr->address,deletedNodeAddress) == 0))
			{
				if(ptr->is_neigh == 1)
					flag = 1;
			}
			ptr = ptr-> next;
		}
	}
	if(flag == 1 || deletedNodeAddress == NULL)
	{
		while(1)
		{
			ptr = head;
			int a = rand_r(&seeds) % size + 1;
			int i = 0;
			for(i = 0; i<a; i++)
				if(ptr->next!=NULL)
					ptr=ptr->next;

			if((ptr->is_neigh == 0) && (strcmp(ptr->address,myAddress) != 0) && (ptr->failbit != 1))
			{
				ptr->is_neigh = 1;
				neighborsSelected++;
			}
			if(numberOfNeighbors == neighborsSelected)
				return SUCCESS;
		}
	} else {
		return SUCCESS;
        }
}
