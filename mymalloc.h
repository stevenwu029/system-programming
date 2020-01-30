
#ifndef MYMALLOC_H
#define MYMALLOC_H
#define totalMemory 4096
#define startOfData -1
#define true 1
#define false 0
#define malloc(x) mymalloc(x, __LINE__, __FILE__)
#define free(x) myfree(x, __LINE__, __FILE__)

/*
Programmers: Steeve Soni & Shangda Wu
Date: October 20th, 2019
Purpose: This is the second assignment called ++malloc where we have to design our own
malloc that also includes errors for when the user cannot malloc something or when the user
is trying to free something that cannot be freed.

*/

typedef struct Metadata{
		int isInUse; // This will be either 1 or 0 to see if the current space of memory is being used
		int memorySize; //This will tell us the current memory size
		int previousSize; //This will tell us the memory size of the previous block. No previous block means -1
		int hasMetadata; //This will be either 1 or 0 to see if block has metadata
		
}Metadata;


void * mymalloc(size_t, int,char*);
void myfree(void *,  int ,char*);
int findSpace(size_t, int, Metadata*);
int printOut(int);

#endif