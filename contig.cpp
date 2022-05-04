#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<sys/types.h>
#include<unistd.h>
#include<sys/wait.h>
#include<pthread.h>

using namespace std;
const int NUMBER_OF_CHILDREN = 10;

int arraySize = 1000;
int array1[1000];
int array1Free=1000;
int array2[1000];
int array2Free=1000;
int array3[1000];
int array3Free=1000;



void* thread(void *child_to_parent){
    int *ctp = (int*) child_to_parent;
    //close(ctp[0]);
    int request = 50* (1+rand()%4);
    write(ctp[1], &request, sizeof(int));
    //close(ctp[1]);
}

void printFree(int *arr){
    int startFree=-1, i, countFree=0;
    for(i=0; i<arraySize; i++){
        if(arr[i]==1){
            if(countFree){
                printf("%d-%d, ", startFree+1, i-1);
                countFree=0;
            }
            startFree=i;
            continue;
        }else
            countFree++;
    }

    if(i>startFree && countFree)
        printf("%d-%d, ", startFree+1, i-1);
    printf("\n");
}

int main() {
    int startPos=50; //50+rand()%100;
    for(int i=startPos; i<startPos+100; i++){
        array1[i]=1;
        array1Free--;
        array2[i]=1;
        array2Free--;
        array3[i]=1;
        array3Free--;
    }
    startPos=400; //400+rand()%120;
    for(int i=startPos; i<startPos+100; i++){
        array1[i]=1;
        array1Free--;
        array2[i]=1;
        array2Free--;
        array3[i]=1;
        array3Free--;
    }

    printf("Initial Array Free space: ");
    printFree(array1);

    int child_to_parent[2];
    pipe(child_to_parent);
    
    pthread_t tid[NUMBER_OF_CHILDREN];
    for(int i=0; i<NUMBER_OF_CHILDREN; i++) {
        pthread_create(&tid[i], NULL, thread, &child_to_parent);
    } //end child creation for loop
  
    close(child_to_parent[1]);


    int i;
    int request;
    int startingFree=0, allocation=0;
    for(int x=0; x<NUMBER_OF_CHILDREN; x++) {
        read(child_to_parent[0], &request, sizeof(int));
        printf("GOT REQUEST FOR %d bytes\n", request);
        
        sleep(1);

        // first fit
        if(array1Free<request){
            printf("  First Fit: No more free space\n");
        }else{
            startingFree=0; allocation = 0;
            for(i=0; i<arraySize; i++){
                if(array1[i]==1){
                    startingFree=i+1;
                    continue;
                }

                if(i-startingFree>=request-1){
                    for(int j=startingFree; j<startingFree+request; j++){
                        array1[j] = 1;
                    }
                    array1Free -= request;
                    allocation = 1;
                    printf("  [FF] Allocating space at %d, New free size = %d = ", startingFree, array1Free);
                    printFree(array1);
                    break;
                }
            }

            if(!allocation){
                printf("  [FF] No contiguos free space, New free size = %d = ", array1Free);
                printFree(array1);
            }
        }

        // worst fit
        if(array2Free<request){
            printf("  Worst Fit: No more free space\n");
        }else{
            startingFree=0; allocation = 0;
            int largestSpaceAt=0, largestSpaceSize=0;
            for(i=0; i<arraySize; i++){
                if(array2[i]==1){


                    if(i-startingFree>=request-1 && i-startingFree>largestSpaceSize){
                        largestSpaceSize = i-startingFree;
                        largestSpaceAt = startingFree;
                        startingFree = arraySize;
                    }

                    startingFree=i+1;
                    continue;
                }
            }

            if(i-startingFree>=request-1 && i-startingFree>largestSpaceSize){
                largestSpaceSize = i-startingFree;
                largestSpaceAt = startingFree;
                startingFree = arraySize;
            }

            if(largestSpaceSize){
                for(int j=largestSpaceAt; j<largestSpaceAt+request; j++){
                    array2[j] = 1;
                }
                array2Free -= request;
                allocation = 1;
                printf("  [WF] Allocating space at %d, New free size = %d = ", largestSpaceAt, array2Free);
                printFree(array2);
            }else{
                printf("  [WF] No contiguos free space, New free size = %d = ", array2Free);
                printFree(array2);
            }
        }

        // best fit
        if(array3Free<request){
            printf("  Best Fit: No more free space\n");
        }else{
            startingFree = 0; allocation = 0;
            int spaceAt=-1, spaceSize=1000;
            for(i=0; i<arraySize; i++){
                if(array3[i]==1){
                    if(i-startingFree>=request-1 && i-startingFree<spaceSize){
                        spaceSize = i-startingFree;
                        spaceAt = startingFree;
                        startingFree = arraySize;
                        allocation = 1;
                    }

                    startingFree=i+1;
                    continue;
                }
            }

            if(i-startingFree>=request-1 && i-startingFree<spaceSize){
                spaceSize = i-startingFree;
                spaceAt = startingFree;
                startingFree = arraySize;
                allocation = 1;
            }

            if(allocation){
                for(int j=spaceAt; j<spaceAt+request; j++){
                    array3[j] = 1;
                }
                array3Free -= request;
                printf("  [BF] Allocating space at %d, New free size = %d = ", spaceAt, array3Free);
                printFree(array3);
            }else{
                printf("  [BF] No contiguos free space, New free size = %d = ", array3Free);
                printFree(array3);
            }
        }
    }

    printf("First Fit Free = %d = ", array1Free); printFree(array1);
    printf("Worst Fit Free = %d = ", array2Free); printFree(array2);
    printf("Best Fit Free = %d = ", array3Free); printFree(array3);

    if(array1Free<array2Free && array1Free<array3Free)
        printf("First fit is best in this case\n");
    else if(array2Free<array3Free && array2Free<array1Free)
        printf("Worst fit is best in this case\n");
    else if(array3Free<array2Free && array3Free<array1Free)
        printf("Best fit is best in this case\n");
    else
	printf("Not possible to determine the best algorithm in this case\n");
    
    return 0;
}
