#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"
/*
Programmers: Steeve Soni & Shangda Wu
Date: October 20th, 2019
Purpose: This is the second assignment called ++malloc where we have to design our own
malloc that also includes errors for when the user cannot malloc something or when the user
is trying to free something that cannot be freed.

*/
static char myBlock[totalMemory];
int keepTrackOfMemory = totalMemory;
Metadata * begOfArray = (Metadata*) &myBlock[0];


void * mymalloc(size_t currentSize , int currentLine , char * currentFile){

	//If they request 0 bytes, that is not possible so we will return error
	if(currentSize == 0){
		printf("Error in line %d and file %s: You cannot request 0 bytes.\n", currentLine, currentFile);
		return  NULL;	
	}
  	Metadata * currentMeta = begOfArray;
  	int metaSize = sizeof(Metadata);
  	int returnPointer = 0;
  	int currentBlockOfArray = 0;
  	//This is only for the first case. If all these are true, that means we are at the beginning of the sequence and we have to set everything 
  	if(currentMeta->isInUse == false && currentMeta->memorySize == false && currentMeta == (Metadata*) &myBlock[0] && currentMeta->hasMetadata == false){
  		 keepTrackOfMemory = keepTrackOfMemory - metaSize;

  		
  		 // Will check if what we are trying to put in will fit in the first block. If it does not that means it is too big and we should print error and return
  		if(currentSize > keepTrackOfMemory){
  			currentMeta->isInUse = false;
  			currentMeta->hasMetadata = true;
  			currentMeta->memorySize = keepTrackOfMemory;
  			currentMeta->previousSize = startOfData;  		 	 
			printf("Error in line %d and file %s: Not enough space to create what was requested.\n", currentLine, currentFile);
  		 	return  NULL;	
   	}
   	// Otherwise, if it is big enough to fit in the first block, we will set the metadata accordingly
  		currentMeta->isInUse = true;
		currentMeta->hasMetadata = true;
  		currentMeta->memorySize = currentSize;
  		keepTrackOfMemory = keepTrackOfMemory - currentSize;

  		currentMeta->previousSize = startOfData;
  		returnPointer = metaSize;
  		currentBlockOfArray = currentSize + metaSize;
  		
  
  		
  		//This is making second metadata for the second block. If there is not enough space, it will not create another block at all.
  		if(keepTrackOfMemory > metaSize){
  			currentMeta = (Metadata*) &myBlock[currentBlockOfArray];
  			currentMeta->isInUse = false;
  			currentMeta->memorySize = keepTrackOfMemory - metaSize;
  			currentMeta->previousSize = currentSize;
  			currentMeta->hasMetadata = true;
  			keepTrackOfMemory = keepTrackOfMemory - metaSize;
  		}
  		
  		// There is only room for one block and not enough room for the metadata and at least 1 byte of information for the second block so it will not create.
  		else{
  			currentMeta->memorySize = totalMemory - metaSize;  			
  		}
  		//If it gets to here, it should return the correct pointer that needs to be in place.

  		return &myBlock[returnPointer];
  	}
  	//This is for every other case besides the first one where the metadata has not even been set yet.
  	else{
  		//If the metadata for the next one has not already been set, this will set it.
  		if(currentMeta->hasMetadata == false){
  			keepTrackOfMemory = keepTrackOfMemory - metaSize;
  		}
  		//If what the user is inputting is bigger than all the memory we have there is no point of checking individual blocks, just return NULL.
  		if(currentSize  > totalMemory){
  			printf("Error in line %d and file %s: Not enough space to create what was requested.\n", currentLine, currentFile);
  			return NULL;
   	
   	}  		
 
		//To attempt to look for an open spot, we will search another method called findSpace.
  		returnPointer = findSpace(currentSize, metaSize, currentMeta);
		//If we returned -1, that means it could not find space to store it and this is an error.
  		if(returnPointer == -1){	
  			printf("Error in line %d and file %s: Not enough space to create what was requested.\n", currentLine, currentFile);
  			return NULL;
  		}
		//Otherwise if we found space for it, it will return the location for their memory.
  		else{

  			return &myBlock[returnPointer];
  		} 
  		
  	}
  	
  	
  	
  	
  	
  	
  	
  	
  	printf("Error in line %d and file %s: Not enough space to create what was requested.\n", currentLine, currentFile);
	return NULL;

}

void myfree(void * freePtr, int currentLine, char * currentFile){

	// Searching for the free pointer will start at the beginning
   Metadata * freeingPtr = begOfArray;
   int metaSize = sizeof(Metadata);
   int currentBlockOfArray = 0;
   currentBlockOfArray += metaSize;
   freeingPtr = (Metadata*) &myBlock[currentBlockOfArray];
   // This will keep searching for the pointer until the character we are at reaches the end.
   while(currentBlockOfArray < totalMemory){

		// If the searching pointer equals the pointer they are trying to free it will enter this and now check if that pointer can be freed
   	if(freeingPtr == freePtr){


			//This is the current block metadata so we can actually check the information about this pointer
   		Metadata * currentBlockMeta = (Metadata*) &myBlock[currentBlockOfArray-metaSize];
   		//If this is not in use that means it has already been freed or it has never been accessed, therefore it should be an error.
   		if(currentBlockMeta->isInUse == false){
	  			printf("Error in line %d and file %s: You are trying to free something that cannot be freed.\n", currentLine, currentFile);

   			return;
   			
   		}
   		//This is if the block is in use so they can free this and it will be.
   		else if(currentBlockMeta->isInUse == true){

   			currentBlockMeta->isInUse = false;
   			if(currentBlockOfArray + currentBlockMeta->memorySize == totalMemory){
   				keepTrackOfMemory = currentBlockMeta->memorySize;
   			}

				//This will be the if statement to make sure this is not the last block
   			if(currentBlockOfArray + currentBlockMeta->memorySize != totalMemory){
					int nextMetadata = currentBlockOfArray + currentBlockMeta->memorySize;
					if(!(nextMetadata > totalMemory - 1)){
						Metadata * nextBlockMeta = (Metadata*) &myBlock[nextMetadata];
						// If it is not the last block and it is not in use, we will change the size
						if(nextBlockMeta->isInUse == false){

							if(keepTrackOfMemory + currentBlockMeta->memorySize + metaSize + currentBlockOfArray == totalMemory){
								keepTrackOfMemory = currentBlockMeta->memorySize + metaSize + nextBlockMeta->memorySize;

							}
							currentBlockMeta->memorySize += nextBlockMeta->memorySize + metaSize;
							nextBlockMeta->memorySize = false;
							nextBlockMeta->hasMetadata = false;
							nextBlockMeta->previousSize = 0;
							Metadata * changeNextBlock = (Metadata*) &myBlock[currentBlockOfArray+currentBlockMeta->memorySize];
							changeNextBlock->previousSize = currentBlockMeta->memorySize;
							
						}
								
					}
   				//Metadata * nextBlockMeta = (Metadata*) &myBlock[nextMetadata];
   				//printf("current block of Array: %d, current metadata: %d\n", currentBlockOfArray, currentBlockMeta->memorySize);
   				// Now we have to check the right side. If the right side is empty than both of these will be attached together
   			}
   			//Now we have to check the left side. If the left is not in use we will combine the two together and form one block that they can use.
   			if(currentBlockMeta->previousSize != -1){
   				Metadata * prevBlockMeta = (Metadata*) &myBlock[currentBlockOfArray -(currentBlockMeta->previousSize + metaSize + metaSize)];


   				if(prevBlockMeta->isInUse == false){
   					Metadata * changeNextBlock = (Metadata*) &myBlock[currentBlockOfArray + currentBlockMeta->memorySize];

   					if(currentBlockMeta->memorySize + currentBlockOfArray == totalMemory){

   						keepTrackOfMemory = prevBlockMeta->memorySize + metaSize + currentBlockMeta->memorySize;



   					}


   					prevBlockMeta->memorySize += currentBlockMeta->memorySize + metaSize;
   					changeNextBlock->previousSize = prevBlockMeta->memorySize;

   				}
   			}

   				return;

   			
   		}
   		
   		
   	}
   	//If the current ptr we are checking is not equal to what they are trying to free we will move on to the next block.
   	else if(freeingPtr != freePtr){
   		Metadata* currentBlockMeta = (Metadata*) &myBlock[currentBlockOfArray-metaSize];
   		currentBlockOfArray += currentBlockMeta->memorySize + metaSize;
   		freeingPtr = (Metadata*) &myBlock[currentBlockOfArray];
   		
   	}
   	
   	
   }
   //If it gets all the way down here that means it searched through the entire list and did not find the pointer. Therefore, this is an error because what they are trying to free
   //is not located in the memory blocks we have given to them.
  	printf("Error in line %d and file %s: You are trying to free something that cannot be freed.\n", currentLine, currentFile);

	return;


}

//This is the function to search and store the data the user is asking for. If we cannot find it, we will return -1.
int findSpace(size_t input, int metaSize, Metadata * checkArray){
	int thePreviousSize = 0; // Keeps track of the previous Size so we can change that value
	int currentChar = 0; // Keeps track of the current character we are on in the static array. This will only increase by metaSize and size of the current space
	//This is a while loop that will continue until it reaches the end of the static array or if it finds open space it is looking for.
	while(currentChar < totalMemory-1){
		// If the first space we found is in use, we will increase currentChar to get us to the next one
		if(checkArray->isInUse == true){
			thePreviousSize = checkArray->memorySize;
			currentChar += (checkArray->memorySize + metaSize);
			checkArray = (Metadata*) &myBlock[currentChar];
			
		}
		//Other wise, if it is not in use, that means there is free space. Now we will check if the input is greater than the memory size allowed at this space.
		//If it is, then it will continue to the next one unless this is the last space.
		else if(input > checkArray->memorySize){
			if(checkArray->memorySize + currentChar + metaSize == (totalMemory - 1)){

				return -1;

			}

			currentChar += (checkArray->memorySize + metaSize);
			checkArray = (Metadata*) &myBlock[currentChar];
			
		}
		//Now if the input is less than or equal to the memory size that means we have found enough space. We will return this location to them and if we have enough space
		//to store another metadata as well as space for at least 1 byte of information we will split the blocks. If we don't have enough space for that, the blocks will not split
		else if(input <= checkArray->memorySize){


			checkArray->isInUse = true;
			// The next data will be set
			int newData = checkArray->memorySize - input; // This is whats leftover after splitting blocks Ex: if you have 20 bytes and putting in 1, 20 - 1 = 19 bytes left

			if(currentChar + checkArray->memorySize + metaSize == totalMemory){
				keepTrackOfMemory = keepTrackOfMemory - input;
			}
			checkArray->memorySize = input;
			if(checkArray->previousSize != -1){ 
				checkArray->previousSize = thePreviousSize;
			}

			

			
			int nextChar = currentChar + input + metaSize;
			//If this is the last element in the array and it the array isn't full, we will come into here to try to make more metadata

			if(nextChar == (totalMemory) - keepTrackOfMemory && keepTrackOfMemory != 0){
				//As long as the amount left is bigger than the metaSize we will create more metadata since it will be able to store at least 1 byte
				if(keepTrackOfMemory > metaSize){

					Metadata * nextCheckArray = (Metadata*) &myBlock[nextChar];

					nextCheckArray->isInUse = false;
					nextCheckArray->hasMetadata = true;

					nextCheckArray->memorySize = newData - metaSize;
					keepTrackOfMemory = keepTrackOfMemory - metaSize;
					nextCheckArray->previousSize = input;
					Metadata * changeNextBlock =  (Metadata*) &myBlock[nextChar + nextCheckArray->memorySize + metaSize];
					changeNextBlock->previousSize = nextCheckArray->memorySize;
					
					
				}
				else{
					//otherwise if it is not bigger than the metadata, the current block will stay its size
					checkArray->memorySize = newData + input; //puts it back to 20 19 + 1
					//keepTrackOfMemory = 0;
				}
				
				
			}
			//Otherwise if you are in the middle, it will just create a new block and keep track of memory wont change.
			else if(newData > metaSize){
				//
				Metadata* nextCheckArray = (Metadata*) &myBlock[nextChar];
				nextCheckArray->isInUse = false;
				nextCheckArray->memorySize = newData - metaSize;
				nextCheckArray->previousSize = input;
				Metadata * changeNextBlock =  (Metadata*) &myBlock[nextChar + nextCheckArray->memorySize + metaSize];
				changeNextBlock->previousSize = nextCheckArray->memorySize;
			}
			else{

				checkArray->memorySize = newData + input;
			}
			
			
			
			
			
			
			


			return (currentChar + metaSize);
			
			
		}


	}

	return -1;
	
}

int printOut(int metaSize){
	Metadata * check = begOfArray;
	int currentChar = 0;
	currentChar += check->memorySize + metaSize;
	check = (Metadata*) &myBlock[currentChar];
	return 1;
}
