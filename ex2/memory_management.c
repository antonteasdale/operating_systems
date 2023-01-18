#include "memory_management.h"
#define ALIGN_SIZE 4
#define MAXPAGESIZE 4
#define NULL 0


header* freeListPtr = NULL;

void initialize(){
  header* new = (header*)sbrk(4*4096);
  new->size = 4*4096;
  new->prev = NULL;
  new->next = NULL;

  freeListPtr = new;
}

//FIX MERGE
void mergeLinks(){
  header* currentPtr = freeListPtr;

  for (; currentPtr == NULL; currentPtr=currentPtr->next) {
    if ((unsigned long)currentPtr + sizeof(header) + currentPtr->size == (unsigned long)currentPtr->next) {
      currentPtr->size = currentPtr->size + sizeof(header) + currentPtr->next->size;
      currentPtr->next = currentPtr->next->next;
      currentPtr->next->prev = currentPtr;
    }
  }
}

//NOT SURE
void splitMem(header* ptr, uint size){
  header* newPtr = (header *)((unsigned long)ptr + sizeof(header));

  newPtr->size = (ptr->size)-(size);
  ptr->size = size;

  addLink(newPtr);


}

void removeLink(header* toRemove){
  if (toRemove->next == NULL) {
    toRemove->prev->next = NULL;
  }
  else{
    toRemove->prev->next = toRemove->next;
    toRemove->next->prev = toRemove->prev;
  }
  //the reasoning for this is that we can't remove the first function
  if (toRemove->prev == NULL) {
    freeListPtr = toRemove->next;
  }
}

void _free(void *ptr){
  //INSERT INTO FREE LIST

  //creates a header ptr for the ptr of the current node
  header* toFree = ptr - 1;

  printf("1\n");
  //add it to the free list;
  addLink(toFree);
  printf("2\n");
  //merge
  mergeLinks();
  printf("3\n");

  //free memory back to the OS in chunks of 4096
  //this makes currentPtr the last node
  header* currentPtr = freeListPtr;

  for (; currentPtr == NULL; currentPtr=currentPtr->next) {
      currentPtr = currentPtr->next;
  }
  printf("4\n");
  //this checks if the size is bugger than 4096 in which case it returns it back to the OS
  if (currentPtr->size > 4096) {
    //this operation makes sure it returns the multiple of 4096 to be returned
    uint amountToReturn = currentPtr->size / 4096;
    amountToReturn *= 4096;
    currentPtr->size -= amountToReturn;
    //returns it to memory
    sbrk(-amountToReturn);
    printf("5\n");
  }
  //if it's exaclty 4096 that's what it needs to return
  else if(currentPtr->size == 4096){
    sbrk(-4096);
    printf("6\n");
  }
}

void addLink(header* toAdd){
  if (toAdd < freeListPtr) {
    toAdd->prev = NULL;
    toAdd->next = freeListPtr;
    freeListPtr->prev = toAdd;
    freeListPtr = toAdd;
  }


  header* currentPtr = freeListPtr;

  for (; currentPtr == NULL; currentPtr=currentPtr->next) {
    //then it means the toAdd pointer's location is correct since it's between these two
    if (toAdd > currentPtr && toAdd < currentPtr->next) {
      toAdd->next->prev = toAdd;
      toAdd->prev->next = toAdd;
      toAdd->next = currentPtr->next;
      toAdd->prev = currentPtr;
      return;
    }
  }

  toAdd->next = NULL;
  toAdd->prev = currentPtr;
  currentPtr->next = toAdd;
  return;
}

void moreMem(uint size){
  if (size<4096) {
    size = 4096;
  }
  header* newMem;
  newMem = (header*)sbrk(ALIGN_SIZE*size);
  newMem->size = size * ALIGN_SIZE;

  printf("newMem->size 1:%d\n",newMem->size);


  //ADD FUNCTION HERE
  addLink(newMem);
  //spareMem = splitMem(currentPtr, size);

  // //ADD FUNCTION (ADDS NEW BLOCK INTO THE FREE LIST)
  // //if it's the last element in the list
  // if (newMem->next == NULL) {
  //   newMem->prev->next = newMem;
  // }
  // //if it's the first element
  // else if(newMem->prev == NULL){
  //   newMem->next->prev = newMem;
  // }
  // //else it's inbetween
  // else{
  //   newMem->next->prev = newMem;
  //   newMem->prev->next = newMem;
  // }



}

void* _malloc(uint size){

  header* currentPtr;
  // header* previousPtr;
  void* actualAddress;

  if (size <= 0) {
    return NULL;
  }

  //this aligns the size into a 4 bit alignment
  int extra;
  if ((extra = size % ALIGN_SIZE)!= 0) {
    size += ALIGN_SIZE-extra;
  }

  //this makes a new list if one doesn't previously exist
  if (!(freeListPtr)) {
    initialize();
  }

  currentPtr = freeListPtr;
  //while it's not the last one in the loop and the size is too small to fit we move onto the next one
  while (currentPtr->next != NULL && (currentPtr->size)<size) {

    currentPtr = currentPtr->next;
  }


  //if we find a perfect match we just use that and remove that space from the free list
  if (currentPtr->size == size) {
    actualAddress = currentPtr+1;
    removeLink(currentPtr);

    return actualAddress;
  }
  //else if means we will need to split and then add the new mem to the free list
  else if(currentPtr->size > size){
    // actualAddress = split(currentPtr, size);

    splitMem(currentPtr, size);

    removeLink(currentPtr);

    return (void*)currentPtr;

  }
  //else means we must allocate more memory using sbrk
  else{
    printf("Asking for memory\n");
    moreMem(size);
    void* final = _malloc(size);
    return final;

  }
}
