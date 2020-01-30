#include <stdlib.h>
#include <stdio.h>	
#include "mymalloc.h"
#include <time.h>



/*
Programmers: Steeve Soni & Shangda Wu
Date: October 20th, 2019
Purpose: This is the second assignment called ++malloc where we have to design our own
malloc that also includes errors for when the user cannot malloc something or when the user
is trying to free something that cannot be freed.

*/

int main(int argc, char ** argv){

	int metaSize = sizeof(Metadata);	
	struct timespec tstart = {0,0}, tend = {0,0};
	//Set the running time to zero
	double timeA = 0.0;  
	double timeB = 0.0;
	double timeC = 0.0;
	double timeD = 0.0;
	double timeE = 0.0;
	double timeF = 0.0;
	double totalTime = 0;
	int totalLoop;
	for (totalLoop = 0; totalLoop < 100; totalLoop ++){
		//Test Case A 
		// In this case we malloc() 1 byte and immediately free it for 150 times
		
		int aLoop;
		clock_gettime(CLOCK_MONOTONIC, &tstart);
		for (aLoop = 0; aLoop < 1 ; aLoop ++){
			int i = 0;
			char * test1[150];
			for(i = 0; i < 150; i++){
				test1[i] = malloc(100); //This is mallocing 100 byte to test1[i]
				free(test1[i]);//This is freeing test1[i] immediately
			}
			clock_gettime(CLOCK_MONOTONIC, &tend);
			timeA += (double)tend.tv_sec + 1.0e-9 * tend.tv_nsec-(double)tstart.tv_sec + 1.0e-9 * tstart.tv_nsec;
		}
		
	
	
	
		//Test case B 
		//In this case, we malloc() 1 byte, store the pointer in an array for 150 times,
		//and when 50 byte chunks have been allocated, free them one by one 
		int bLoop;
		
		clock_gettime(CLOCK_MONOTONIC, &tstart);
		for (bLoop = 0; bLoop < 1 ; bLoop ++){
		int l;
		int j;
		char* arr[150];
		int countForMalloc = 0; //countForMalloc is to store how many times we have called malloc
		for (l = 0; l < 150; l++){
			arr[l] = malloc(1); //This is mallocing 1 byte to arr[i]
			countForMalloc ++;	//countForMalloc increases by 1 when a malloc is called
			if (countForMalloc == 50){	//when countForMalloc reaches 50, we free what has been stored in the previous blocks
				for (j = l - countForMalloc + 1; j < l + 1 ; j ++){
					free(arr[j]);
				}
				countForMalloc = 0; //We reset countForMalloc to zero after freeing everything in the previous 50 blocks
			}
		
		} 
		clock_gettime(CLOCK_MONOTONIC, &tend);
		timeB += (double)tend.tv_sec + 1.0e-9 * tend.tv_nsec-(double)tstart.tv_sec + 1.0e-9 * tstart.tv_nsec;
		}
	
	
		//Case C
		//In this case, we randomly choose between a 1 byte malloc() or free()ing a 1 byte pointer,
		//and free everything when we have allocated 50 times.
		int cLoop;
		
		clock_gettime(CLOCK_MONOTONIC, &tstart);
		for (cLoop = 0; cLoop < 1 ; cLoop ++){
		int randomSelection1; //This is unit digit of a random generated number
		char* check1[150]; // This is the char pointer array that stores all the pointers
		int storeCheck1[150];// This is the int array that stores which position in the char pointer array has been stored 
		int o;
		int p;
		int countMalloc1 = 0;// how many malloc we have allocated
		int freeLocation1;// This is the randomly generated position that we free
		//build an array to store 
		for (o = 0; o < 150; o++){
			randomSelection1 =  random() % 10;//This is unit digit of a random generated number
			while(countMalloc1 == 0 && randomSelection1 >= 5){ 
				randomSelection1 =  random() % 10;//the first thing can't be a free, so we regenerate random numbers till it is smaller or equal to 4
			}
			if (randomSelection1 <= 4){ // when the random generated number is smaller or equal to 4, we call the malloc
				check1[countMalloc1] = malloc(1); //we store a 1 byte pointer to check[countMalloc1], which starts from check[0],and goes up everytime when malloc is called
				storeCheck1[countMalloc1] = true;//we set true to check[countMalloc1], when malloc is called
				countMalloc1 ++; //countMalloc1 only goes up when malloc is called
			} else if(randomSelection1 >= 5){
				freeLocation1 = rand() % ((countMalloc1 - 1) + 1 - 0) + 0; // set freeLocation1 to be a random number from 0 to the total call of current malloc
					if (freeLocation1 == p && storeCheck1[p] == false){
						//Stop user from freeing something that has already been freed.
					}
					else if(freeLocation1 == p && storeCheck1[p] == true){
						free(check1[p]); //free the spot that has been randomly chosen
						storeCheck1[p] = false; //set this spot to false after freeing.
					}
			}
			if (countMalloc1 == 50){//when countMalloc1 reaches 50, we free all the blocks
				for (p = 0; p < 50; p ++){
					if (storeCheck1[p] == false){
						//This is when user are trying to free something that has already been freed.
					}
					else if(storeCheck1[p] == true){
						free(check1[p]); //free the spot that has been randomly chosen
						storeCheck1[p] = false; //set this spot to false after freeing.
					}
				}
				break;
			}
		}
		clock_gettime(CLOCK_MONOTONIC, &tend);
		timeC += (double)tend.tv_sec + 1.0e-9 * tend.tv_nsec-(double)tstart.tv_sec + 1.0e-9 * tstart.tv_nsec;
		}
	
	

		//Case D
		//In this case, we randomly choose between a 1 to 64 byte malloc() or free()ing a 1 byte pointer,
		//and free everything when we have allocated 50 times.
		int dLoop;
		
		clock_gettime(CLOCK_MONOTONIC, &tstart);
		for (dLoop = 0; dLoop < 1 ; dLoop ++){
		int randomSelection2; //This is unit digit of a random generated number
		char* check2[150]; // This is the char pointer array that stores all the pointers
		int storeCheck2[150];// This is the int array that stores which position in the char pointer array has been stored 
		int m;
		int n;
		int randomSizeOfMalloc2 = 0; //This is the random number between 1 to 64
		int countMalloc2 = 0;// how many malloc we have allocated
		int freeLocation2 = 0;// This is the randomly generated position that we free
		//build an array to store 
		for (m = 0; m < 150; m++){
			randomSizeOfMalloc2 = rand() % (64 + 1 - 1) + 1;
			randomSelection2 =  random() % 10;
			while(countMalloc2 == 0 && randomSelection2 >= 5){ //the first thing can't be a free, so we regenerate random numbers till it is smaller or equal to 4
				randomSelection2 =  random() % 10;
			}
			if (randomSelection2 <= 4){ // when the random generated number is smaller or equal to 4, we call the malloc
				check2[countMalloc2] = malloc(randomSizeOfMalloc2); //we store a random byte from 1 to 64 byte pointer to check[countMalloc], which starts from check[0],and goes up everytime when malloc is called
				storeCheck2[countMalloc2] = true;//we set true to check[countMalloc2], when malloc is called
				countMalloc2 ++; //countMalloc2 only goes up when malloc is called
			} else if(randomSelection2 >= 5){
				freeLocation2 = rand() % ((countMalloc2 - 1) + 1 - 0) + 0; // set freeLocation2 to be a random number from 0 to the total call of current malloc
					if (freeLocation2 == n && storeCheck2[n] == false){
						//Stop user from freeing something that has already been freed.
					}
					else if(freeLocation2 == n && storeCheck2[n] == true){
						free(check2[n]); //free the spot that has been randomly chosen
						storeCheck2[n] = false; //set this spot to false after freeing.
					}
			}
			if (countMalloc2 == 50){//when countMalloc2 reaches 50, we free all the blocks
				for (n = 0; n < 50; n ++){
					if (storeCheck2[n] == false){
						//This is when user are trying to free something that has already been freed.
					}
					else if(storeCheck2[n] == true){
						free(check2[n]); //free the spot that has been randomly chosen
						storeCheck2[n] = false; //set this spot to false after freeing.
					}
				}
				break;
			}
		}
		clock_gettime(CLOCK_MONOTONIC, &tend);
		timeD += (double)tend.tv_sec + 1.0e-9 * tend.tv_nsec-(double)tstart.tv_sec + 1.0e-9 * tstart.tv_nsec;
		}
	
	

		//Test Case E //Fill up the entire 4096, then make changes in it to see if splitting works
		int eLoop;
		
		clock_gettime(CLOCK_MONOTONIC, &tstart);
		for (eLoop = 0; eLoop < 1 ; eLoop ++){
		char* check3[150];// This is the char pointer array that stores all the pointers
		int storeCheck3[150];
		int q;
		int h;
		int randomSelection3; //This is a random number between 1 to 104(120 - metadata - 1(at least we should be able to offer 1 byte to split
								//)), to see if our free spot will be combined together when two freed spots are next to each other
		int inputMinusRandom3;// This will be 120 - randomSelection3 - metaSize 
		int freeInTotal3 = 0;//This is the number that keeps track of how many bytes have been freed.
		for(q = 0; q < 30; q++){
			check3[q] = malloc(120);
			storeCheck3[q] = true;
		}
		for(q = 0; q < 30; q++){  //free the even ones, so freed memories are not add together.
			if (q == 0 || (q % 2) == 0){
				free (check3[q]);
				storeCheck3[q] = false;
				freeInTotal3 += 120;//After freeing one spot, we increase the total free by 120.
			}
		}
		q = 30;
		while (freeInTotal3 > 0){
			randomSelection3 = rand() % (103 + 1 - 1) + 1;
			inputMinusRandom3 = 120 - randomSelection3 - metaSize ;
			check3[q] = malloc(randomSelection3); //reallocate the first position
			check3[q+1] = malloc(inputMinusRandom3);//reallocate the following position
			storeCheck3[q] = true;//we set true to storeCheck3[q], when malloc is called
			storeCheck3[q+1] = true;//we set true to storeCheck3[q + 1], when malloc is called
			q = q + 2; 
			freeInTotal3 = freeInTotal3 - 120;//we add 120 back to total memory after freeing
		}
		for (q = 0; q < 60; q ++){
			if (storeCheck3[q] == false){
				//This is when user are trying to free something that has already been freed.
			}
			else if(storeCheck3[q] == true){
				free(check3[q]); //free the spot that has been randomly chosen
				storeCheck3[q] = false; //set this spot to false after freeing.
			}
		}
		clock_gettime(CLOCK_MONOTONIC, &tend);
		timeE += (double)tend.tv_sec + 1.0e-9 * tend.tv_nsec-(double)tstart.tv_sec + 1.0e-9 * tstart.tv_nsec;
		}
	

		//Test Case F //Fill up the entire 4096, then make changes in it to see if chuncks combine together
		int fLoop;
		
		clock_gettime(CLOCK_MONOTONIC, &tstart);
		for (fLoop = 0; fLoop < 1 ; fLoop ++){
		char* check4[150]; // This is the char pointer array that stores all the pointers
		int storeCheck4[150];
		int inputMinusRandom4;
		int y;
		int randomSelection4; //This is a big random number between 1500 to 120, to see if our free spot will be combined together when two freed spots are next to each other
		int freeInTotal4 = 0; //This is the number that keeps track of how many bytes have been freed.
		for(y = 0; y < 30; y++){ //First, we exhaust the total memory, which is 4096
			check4[y] = malloc(120);
			storeCheck4[y] = true;
		} 
		for(y = 0; y < 15; y++){//free the consecutive ones
			free (check4[y]);
			storeCheck4[y] = false;
			freeInTotal4 += 120;
		}
		y = 30;
		randomSelection4 = rand() % (1500 + 1 - 120) + 1; 
		inputMinusRandom4 = freeInTotal4 - randomSelection4 - metaSize;

		check4[30] = malloc(randomSelection4); //in check4[30] we store something big in, to see if chunks are combing together
		storeCheck4[30] = true;
		check4[31] = malloc(inputMinusRandom4);
		storeCheck4[31] = true;

		for (y = 0; y < 32; y ++){
			if (storeCheck4[y] == false){
				//This is when user are trying to free something that has already been freed.
			}
			else if(storeCheck4[y] == true){
				free(check4[y]); //free the spot that has been randomly chosen
				storeCheck4[y] = false; //set this spot to false after freeing.
			}
		}
		clock_gettime(CLOCK_MONOTONIC, &tend);
		timeF += (double)tend.tv_sec + 1.0e-9 * tend.tv_nsec-(double)tstart.tv_sec + 1.0e-9 * tstart.tv_nsec;
		}
	
	}
	printf("The mean time for test case A is %.5f\n", timeA / 100);
	printf("The mean time for test case B is %.5f\n", timeB / 100);
	printf("The mean time for test case C is %.5f\n", timeC / 100);
	printf("The mean time for test case D is %.5f\n", timeD / 100);
	printf("The mean time for test case E is %.5f\n", timeE / 100);
	printf("The mean time for test case F is %.5f\n", timeF / 100);






	


   return 0;
	
}

