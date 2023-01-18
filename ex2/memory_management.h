#include "kernel/types.h"
#include "user/user.h"

struct Header {
  struct Header *next;
  struct Header *prev;
  uint size;
  int isFree;
};

typedef struct Header header;

void _free(void *ptr);

void moreMem(uint size);

void* _malloc(uint size);

void addLink(header* toAdd);

void mergeLinks();

void splitMem(header* ptr, uint size);

void removeLink(header* toRemove);
