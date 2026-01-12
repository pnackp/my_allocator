#include <stdio.h>
#include <unistd.h>

struct mem_block{
  int size;
  int is_free;
  struct mem_block* next;
};

struct mem_block* header = NULL;

void * my_malloc(size_t bytes){
  if(header != NULL){
    struct mem_block* runner = header;
    while(runner != NULL){
      if(runner->is_free && runner->size >= bytes){
        if(runner->size - bytes >= sizeof(struct mem_block) + sizeof(size_t)){
          struct mem_block * temp = (struct mem_block*)( (char*)(runner + 1) + bytes);          
          temp->is_free = 1;
          temp->size = runner->size - bytes - sizeof(struct mem_block);
          temp->next = runner->next;
          runner->next = temp;
          runner->size = bytes;
          runner->is_free = 0;
          return (void*)(runner + 1);
        }
        runner->is_free = 0;
        return (void*)(runner + 1);
      }
      runner = runner->next;
    }
  }
  void* block = sbrk(sizeof(struct mem_block) + bytes);
  if(block == (void *)-1){
    return NULL;
  }
  struct mem_block* buffer = block;
  buffer->size = bytes;
  buffer->is_free = 0;
  buffer->next = header;
  header = buffer;
  return (void*)(buffer + 1);
}

