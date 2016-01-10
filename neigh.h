/***************************************************
 * * Group Info
 * * ssingh24 Siddharth Singh
 * * skumar23 Sudhendu Kumar
 * * mghegde Mahendra Hegde
 * ***************************************************/

#define SUCCESS 1
#define FAILURE 0

typedef struct neigh_list {
   char address[100];
   int heartbeat;
   struct neigh_list* next;
   struct neigh_list * prev;
   int failbit;
   int localtime;
   int counter; 
   int my_pos;
   int is_neigh;
}Neigh_List;
int sel_new_neigh(int numberOfNeighbors, int numberOfNodes, int seeds, char* myAddress, char* deletedNodeAddress);
int is_unique(int n, int *sel_random, int b);

int insert_node(char *address, int heartbeat, int failbit, int localtime, int counter, int my_pos,int is_neigh);
int delete_dead_node(char *address);
void display_List(Neigh_List* ptr);

int sel_neigh(FILE *fp, int my_pos, int b, int N, int seeds);

