/* 
This program is written by: Ricardo Carvalheira */ 

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "leak_detector_c.h"

#define MAXLSTYLISTS 10
#define MAXCUSTOMERS 100000
#define STYLISTSNAMESIZE 21
#define CUSTNAMESIZE 21

typedef struct customers
{
    char customer[CUSTNAMESIZE];
    char preferredStylist[STYLISTSNAMESIZE];
    int stylistIndex;
    int loyaltyPoints;
    int timeOfArrival;
    int timeOfHairCut;
    int leaving_time;
  
}customers;

typedef struct stylists
{
  char stylistsName[STYLISTSNAMESIZE];
  int waitingListSize;
  customers** customerArr;

}stylists;

void initStylistsHeap(stylists* stylistArr);
customers* initCustomer(int numCustomers, stylists *stylistList, int numStylists );
int findMyStylist(stylists* stylistList,int numStylists,char preferredStylist[]);
void isAnyoneLeaving(stylists * stylistList, int time, int *numCustomers);
void insertCustomer(stylists *stylist, customers *customer);
customers* nextCustomer(stylists* stylistList);
void percolateDown(stylists *s, int index);
void swap(customers** stylist, int index1, int index2);
int compareTo(customers* custA, customers* custB);
int minimum(customers* a, int indexa, customers* b, int indexb);

FILE *input,*out;

int main()
{   
   	atexit(report_mem_leak);
    input = fopen("in.txt", "r");
    out = fopen("out.txt", "w");
    if (input==NULL)
    {
        printf("Missing input file!");
        fprintf(out,"Missing input file!");
        exit(-1);
    }

    int numCustomers, numStylists;

    fscanf(input, "%d %d", &numCustomers, &numStylists);
    stylists stylistsList[MAXLSTYLISTS];
    initStylistsHeap(stylistsList);
    int i;
    
    for(i=0;i<numStylists;i++)
    {
        fscanf(input,"%s", stylistsList[i].stylistsName);
    }
    
    customers* custArr = initCustomer(numCustomers,stylistsList,numStylists);

  int timeNow=custArr[numCustomers-1].timeOfArrival;
  
	while(numCustomers != 0)
  {
		timeNow ++;
		for (int i = 0; i < numStylists; i++)
    	isAnyoneLeaving(&stylistsList[i], timeNow, &numCustomers);
	}
  free(custArr);
  return 0;
}

//Initializes the customer's information in their array
customers* initCustomer(int numCustomers, stylists* stylistList, int numStylists )
{
    customers* tempCustomer=(customers*) malloc(numCustomers*sizeof(customers));
    int i;
    int currTime=0;
    for(i=0;i<numCustomers;i++)
    {     
          fscanf(input,"%d %s %s %d %d", &tempCustomer[i].timeOfArrival, tempCustomer[i].customer, tempCustomer[i].preferredStylist, &tempCustomer[i].loyaltyPoints, &tempCustomer[i].timeOfHairCut);

          tempCustomer[i].leaving_time=tempCustomer[i].timeOfHairCut+tempCustomer[i].timeOfArrival;//minimum time waiting
          int stylIndex=findMyStylist(stylistList,numStylists, tempCustomer[i].preferredStylist);
        
          tempCustomer[i].stylistIndex=stylIndex;
        isAnyoneLeaving(&stylistList[stylIndex], tempCustomer[i].timeOfArrival, &numCustomers);
       insertCustomer(&stylistList[stylIndex], &tempCustomer[i]);

  }
  return tempCustomer;
}

void isAnyoneLeaving(stylists * stylistList, int time, int* numCustomers)
{
  if(stylistList->customerArr!=NULL && stylistList->customerArr[0]!=NULL)
  {
    if(stylistList->customerArr[0]->leaving_time<=time)
    {
      strcpy(stylistList->customerArr[0]->preferredStylist, stylistList->stylistsName);

      stylistList->customerArr[0]->loyaltyPoints=((stylistList->customerArr[0]->timeOfHairCut)/10)+stylistList->customerArr[0]->loyaltyPoints;

      printf("%s %d %d %s\n",stylistList->customerArr[0]->customer, stylistList->customerArr[0]->leaving_time,stylistList->customerArr[0]->loyaltyPoints,stylistList->customerArr[0]->preferredStylist);

      fprintf(out,"%s %d %d %s\n",stylistList->customerArr[0]->customer, stylistList->customerArr[0]->leaving_time,stylistList->customerArr[0]->loyaltyPoints,stylistList->customerArr[0]->preferredStylist);

      stylistList->customerArr[0] =  nextCustomer(&stylistList[0]);
      if(stylistList->customerArr[0]!=NULL)
      {
        stylistList->customerArr[0]->leaving_time=stylistList->customerArr[0]->timeOfHairCut+time;
      }     
    *numCustomers-=1;
    }
  }
}

//Initializes the stylists

void initStylistsHeap(stylists* stylistArr)
{
  int i;
  for(i=0;i<MAXLSTYLISTS;i++)
  {
    strcpy(stylistArr[i].stylistsName, "");
    stylistArr[i].customerArr=NULL;
    stylistArr[i].waitingListSize=0;
  }
}

int findMyStylist(stylists* stylistList, int numStylists, char preferredStylist[])
{
  int i, indexStylist=-1;
  int min=MAXCUSTOMERS;

  for(i=0;i<numStylists;i++)
  {
    if(!strcmp(stylistList[i].stylistsName, preferredStylist))
    {
      return i;
    }
    if(min>stylistList[i].waitingListSize)
    {
      min=stylistList[i].waitingListSize;
      indexStylist=i;
    }
  }

  return indexStylist;
}

customers*  nextCustomer(stylists* stylistList)
{
  customers* tempNext;
  if(stylistList->waitingListSize>1)
  {
    tempNext=stylistList->customerArr[1];
    stylistList->customerArr[1]=stylistList->customerArr[stylistList->waitingListSize-1];
    stylistList->waitingListSize--;
    percolateDown(stylistList, 1);
    return tempNext;
  }
  else 
  {
    stylistList->waitingListSize--;
    return NULL;

  }
}

void percolateDown(stylists *stylist, int index){

  int min;
  
  // Only try to percolate down internal nodes.
  if ((2*index+1) <= stylist->waitingListSize) {

    
      // Find the minimum value of the two children of this node.
      min = minimum(stylist->customerArr[2*index], 2*index, stylist->customerArr[2*index+ 1], 2*index+1);

    // If this value is less than the current value, then we need to move
    // our current value down the heap.
      
      if (compareTo(stylist->customerArr[index], stylist->customerArr[min]) < 0) {
          swap(stylist->customerArr, index, min);

          // This part is recursive and allows us to continue percolating
          // down the element in question.
          percolateDown(stylist, min);
      }
  }

  // Case where our current element has exactly one child, a left child.
  else if (stylist->waitingListSize == 2*index) {

      // Here we only compare the current item to its only child.
      // Clearly, no recursive call is needed since the child of this node
      // is a leaf.
      if (stylist->customerArr[index] > stylist->customerArr[2*index])
          swap(stylist->customerArr, index, 2*index);
  }
}

// Swaps the values stored in the heap pointed to by h in index1 and index2.
void swap(customers** stylist, int index1, int index2) 
{
    customers* temp = stylist[index1];
    stylist[index1] = stylist[index2];
    stylist[index2] = temp;
}

int minimum(customers* a, int indexa, customers* b, int indexb) 
{

	if (compareTo(a, b) > 0){
		return indexa;
	}
	else{
		return indexb;
	}

}

int compareTo(customers* custA, customers* custB) 
{
	int result = custA->loyaltyPoints - custB->loyaltyPoints;
  //if A has more loyalty than B, this result is a positive number
  //else it will return a negative number
  //if they are the same size, we check which one of them comes first alphabetically
	if(result == 0)
  {
	  return strcmp(custB->customer, custA->customer);
	}
  /*
  If two customers have the same number of loyalty points, and only one of those customers has 
  labeled this stylist (the one whose line she is in) as their preferred stylist, then that customer gets 
priority. 
  */
  if(custA->stylistIndex!=1&&custB->stylistIndex==1)
    return 1;
  if(custA->stylistIndex==1&&custB->stylistIndex!=1)
    return -1;
	return result; 
}

void percolateUp(customers **customer, int index){

	//only check anything other than "root"
	if(index > 1){
			
		 if (compareTo(customer[index], customer[(index+1)/2]) > 0){

			// Move our node up one level.
			swap(customer, index, (index+1)/2);

			// See if it needs to be done again.
			percolateUp(customer, (index+1)/2);

		 }
		
	}

}


void insertCustomer(stylists *stylist, customers *customer)
{

	stylist->waitingListSize++;

	stylist->customerArr = realloc(stylist->customerArr, stylist->waitingListSize * sizeof(struct Customer*));


	if(stylist->customerArr[0] == NULL)
  {
		stylist->customerArr[0] = customer;
	}

	else
  {

		stylist->customerArr[stylist->waitingListSize - 1] = customer;
	}


	percolateUp(stylist->customerArr, stylist->waitingListSize - 1);	//placing the customers in correct priority

}
