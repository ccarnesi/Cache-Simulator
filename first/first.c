#include <stdio.h>
#include "first.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>


int cacheHit = 0;
int cacheMiss = 0;
int memWrites = 0;
int memReads =0;



int checkIfPowerOfTwo(long int x){
        if((x &(x-1))==0){
                return 1;
        }else{
                return 0;
        }
}

int checkInputs(long int cacheSize, char* assoc, long int blockSize, long int* associativity, char* nAssoc){
        if(checkIfPowerOfTwo(cacheSize)==0 || checkIfPowerOfTwo(blockSize)==0){
                return 1;
        }
        if(strlen(assoc)==6){
                //direct
                *associativity = 1;
        }else if(strlen(assoc)== 5){
                //fully associative
                *associativity = -1;
        }else{
                nAssoc = &assoc[6];
                *associativity = atoi(nAssoc);
                if(checkIfPowerOfTwo(*associativity)== 0 ){
                        return 1;
                }
        }


        return 0;
}

void findDetails(long int numOfLines, long int cacheSize, long int* numOfSets, long int* setIndexBits, long int* tagIndexBits, long int* blockIndexBits, long int* associativity, long int blockSize ){

        *numOfSets = cacheSize/ (*associativity * blockSize);
        *blockIndexBits = log(blockSize)/ log(2);
        *setIndexBits = log(*numOfSets) / log(2);
        *tagIndexBits = 48 - *setIndexBits - *blockIndexBits;

}

int countList(Node** head){
        Node* current = *(head);
        int count =0;
        while(current != NULL){
                count +=1;
                current = current->next;
        }
        return count;
}

void clearHash(HashTable* Cache){
        int k;
        for(k=0;k<Cache->capacity;k++){
                Node* newNode = malloc(sizeof(Node));
                newNode->next = NULL;
                newNode->setIndex = 0;
                newNode->wasPrefetch = 0;
                newNode->tag = -10;
                Cache->array[k] = newNode;
        }
}

void freeHash(HashTable* Cache){
        int k;
        for(k=0;k<Cache->capacity;k++){
                Node* current = Cache->array[k];
                Node* prev = current;
                while(current != NULL){
                    prev = current;
                    current = current->next;
                    free(prev);
                }
        }
}



long int generateBinary(int length){
    int i;
    long int answer = 0;
    for(i=0;i<length;i++){
        answer = answer + pow(2,i);
    }
    return answer;
}
Node* createNode(long int hexAddress, long int setIndex, long int tagIndex, long int blockIndex){
        Node* newNode = malloc(sizeof(Node));
        newNode->tag = (hexAddress>>(setIndex + blockIndex)) & generateBinary(tagIndex);
        newNode->setIndex = (hexAddress>>blockIndex) & generateBinary(setIndex);
        newNode->wasPrefetch = 0;
        newNode->next = NULL;
        return newNode;
}

void deleteNode(long int tag, Node** head){
        Node* prev = NULL;
        Node* current = *(head);
        if(current!= NULL && current->tag == tag){
                *(head) = current ->next;
                free(current);
                return;
        }
        while(current->tag != tag){
                prev = current;
                current = current->next;
        }
            prev->next = current->next;
            free(current);
}

void insertToFront(Node** head, Node* newNode){
        //check if head is null;
        newNode->next = *(head);
        *(head) = newNode;
        
}

void deleteLastNode(Node** head){
        Node* current = *(head);
        Node* prev = NULL;
        if(current->next == NULL){
                *(head) = NULL;
                free(current);
                
        }else{

            while(current->next != NULL){
                    prev = current;
                    current = current ->next;
            }
                free(prev->next);
                prev->next = NULL;
                
        }
}
void insertIntoPrefetch(Node** head, Node* newNode, int numOfLines){
            Node* current = *(head);
            while(current != NULL){
                    if(current->tag == newNode->tag){
                            //already in cache
                            free(newNode);
                            return;
                    }
                    current = current->next;
            }
            int count = countList(head);
            memReads +=1;
            if(count + 1<= numOfLines){
                    insertToFront(head,newNode);
            }else{
                    deleteLastNode(head);
                    insertToFront(head, newNode);
            }
}
int runPrefetchReadCache(Node** head, Node* newNode, int numOfLines){
        Node* current = *(head);
        while(current != NULL){
                if(current->tag == newNode->tag){
                        cacheHit +=1;
                        //now delete current and add newNode to the begining
                        deleteNode(current->tag, head);
                        insertToFront(head, newNode);
                        return 1;
                }else{
                        current = current->next;
                }
        }
        
        cacheMiss +=1;
        memReads += 1;
        int count = countList(head);
        if(count + 1 <= numOfLines){
                //just insert
                insertToFront(head, newNode);
        }else{
                //delete last node and then insert
                deleteLastNode(head);
                insertToFront(head, newNode);
        }
        
        return 0;
        
}
void runReadCache(Node** head, Node* newNode, int numOfLines){
        Node* current = *(head);
        while(current != NULL){
                if(current->tag == newNode->tag){
                        cacheHit +=1;
                        //now delete current and add newNode to the begining
                        deleteNode(current->tag, head);
                        insertToFront(head, newNode);
                        return;
                }else{
                        current = current->next;
                }
        }
        
        cacheMiss +=1;
        memReads += 1;
        int count = 0;
        count = countList(head);
        if(count + 1 <= numOfLines){
                //just insert
                insertToFront(head, newNode);
        }else{
                //delete last node and then insert
                deleteLastNode(head);
                insertToFront(head, newNode);
        }
        
        return;
        
}
int runPrefetchWriteCache(Node**  head, Node* newNode, int numOfLines){
        Node* current = *(head);

        while(current != NULL){
                if(current->tag == newNode->tag){
                        //hit remove old and add new
                        cacheHit += 1;
                        memWrites +=1;
                        deleteNode(current->tag, head);
                        insertToFront(head, newNode);
                        return 1;
                }else{
                        current = current->next;
                }
        }
        cacheMiss +=1;
        memReads +=1;
        memWrites +=1;
        int count = countList(head);
        if(count + 1 <= numOfLines){
                insertToFront(head, newNode);
        }else{
                deleteLastNode(head);
                insertToFront(head, newNode);
        }
        return 0;
}

void runWriteCache(Node**  head, Node* newNode, int numOfLines){
        Node* current = *(head);

        while(current != NULL){
                if(current->tag == newNode->tag){
                        //hit remove old and add new
                        cacheHit += 1;
                        memWrites +=1;
                        deleteNode(current->tag, head);
                        insertToFront(head, newNode);
                        return;
                }else{
                        current = current->next;
                }
        }
        cacheMiss +=1;
        memReads +=1;
        memWrites +=1;
        int count = countList(head);
        if(count + 1 <= numOfLines){
                insertToFront(head, newNode);
        }else{
                deleteLastNode(head);
                insertToFront(head, newNode);
        }
}





int main(int argc, char* argv[]){
        if(argc != 6){
                printf("error");
                exit(0);
        }
        long int cacheSize = atoi(argv[1]);
        char *assoc = argv[2];
        long int blockSize = atoi(argv[4]);
        FILE *fp;
        long int associativity = 0;
        char* nAssoc = NULL;
         
        
        int errorCheck = checkInputs(cacheSize, assoc, blockSize, &associativity, nAssoc);
        if(errorCheck == 1){
                printf("error");
                exit(0);
        }
        if(errorCheck== 0){
                //run program
                //now find index lengths
                long int numOfLines = 0;
                if(associativity == -1){
                    //fully associative
                    numOfLines = cacheSize / blockSize;
                    associativity = numOfLines;

                }else{
                    numOfLines = associativity;

                }
            
            long int numOfSets = 0;
            long int setIndexBits = 0;
            long int tagIndexBits = 0;
            long int blockIndexBits = 0;
            findDetails(numOfLines, cacheSize, &numOfSets, &setIndexBits, &tagIndexBits, &blockIndexBits, &associativity, blockSize);
         // printf("NumOfSets: %ld, NumOfSetIndex: %ld, numOfTagIndex: %ld, numOfBlockIndex: %ld numOfLines:%ld\n", numOfSets, setIndexBits, tagIndexBits, blockIndexBits, numOfLines);
            fp = fopen(argv[5], "r");
            if(fp == NULL){
                printf("error");
            }else{
                    //char* garbage;
                    char readOrWrite;
                    long int hexAddress;
                    HashTable* Cache = malloc(sizeof(HashTable));
                    Cache->capacity = numOfSets;
                    Cache->array = malloc(Cache->capacity* sizeof(Node*));
                    clearHash(Cache);
                    long int j = fscanf(fp, "%*s\t%c\t%lx",&readOrWrite, &hexAddress);
                    while(j!= EOF){
                            if(j== 2){
                                Node* newNode = createNode(hexAddress, setIndexBits, tagIndexBits, blockIndexBits);
                                if(readOrWrite == 'R'){
                                        //read
                                            runReadCache(&(Cache->array[newNode->setIndex]), newNode, numOfLines);
                                }else{
                                        //write
                                            runWriteCache(&(Cache->array[newNode->setIndex]), newNode, numOfLines);
                                }
                            }
                            j = fscanf(fp, "%*s\t%c\t%lx", &readOrWrite, &hexAddress);

                    }
                    printf("no-prefetch\n");
                    printf("Memory reads: %i\n", memReads);
                    printf("Memory writes: %i\n", memWrites);
                    printf("Cache hits: %i\n", cacheHit);
                    printf("Cache misses: %i\n", cacheMiss);
                    freeHash(Cache);
                    free(Cache->array);
                    free(Cache);
                    memReads = 0;
                    memWrites = 0;
                    cacheHit = 0;
                    cacheMiss = 0;
                    //start Prefetch cache
                    HashTable* Cache2 = malloc(sizeof(HashTable));
                    Cache2->capacity = numOfSets;
                    Cache2->array = malloc(Cache2->capacity * sizeof(Node*));
                    clearHash(Cache2);
                    fseek(fp, 0, SEEK_SET);
                    long int k = fscanf(fp, "%*s\t%c\t%lx", &readOrWrite, &hexAddress);
                    while(k!= EOF){
                            if(k==2){
                                Node* newNode = createNode(hexAddress,setIndexBits, tagIndexBits, blockIndexBits);
                                if(readOrWrite == 'R'){
                                    //run read prefetch cache
                                    int hitOrMiss = runPrefetchReadCache(&(Cache2->array[newNode->setIndex]), newNode, numOfLines);
                                    if(hitOrMiss == 0){
                                        Node* prefetchNode = createNode(hexAddress + blockSize, setIndexBits, tagIndexBits, blockIndexBits);
                                        prefetchNode -> wasPrefetch = 1;
                                        insertIntoPrefetch(&(Cache2->array[prefetchNode->setIndex]), prefetchNode, numOfLines);
                                    }
                                }else{
                                   //run write prefetch cache
                                   int hitOrMiss = runPrefetchWriteCache(&(Cache2->array[newNode->setIndex]), newNode, numOfLines);
                                   if(hitOrMiss == 0){
                                           Node* prefetchNode = createNode(hexAddress + blockSize, setIndexBits, tagIndexBits, blockIndexBits);
                                           prefetchNode->wasPrefetch = 1;
                                           insertIntoPrefetch(&(Cache2->array[prefetchNode->setIndex]), prefetchNode, numOfLines);
                                   }
                                }
                            }
                            k = fscanf(fp,"%*s\t%c\t%lx", &readOrWrite, &hexAddress);
                    }
                    printf("with-prefetch\n");
                    printf("Memory reads: %i\n", memReads);
                    printf("Memory writes: %i\n", memWrites);
                    printf("Cache hits: %i\n", cacheHit);
                    printf("Cache misses: %i\n", cacheMiss);

                    freeHash(Cache2);
                    free(Cache2->array);
                    free(Cache2);
                    fclose(fp);
                    

                    

            }
            

        }

        return 0;
}
