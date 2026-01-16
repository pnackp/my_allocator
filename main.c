#include <unistd.h>
#include <stdint.h>


#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))
#define BLOCK_META_SIZE ALIGN(sizeof(struct mem_block))

struct mem_block {
    size_t size;
    int is_free;
    struct mem_block* next;
    struct mem_block* prev;
};

struct mem_block * first = NULL;
struct mem_block * last = NULL;

void* align_ptr(void* p, size_t align) {
    uintptr_t addr = (uintptr_t)p;
    addr = (addr + align - 1) & ~(align - 1);
    return (void*)addr;
}

void* my_alloc(size_t bytes) {
  size_t align_bytes = ALIGN(bytes);
  if(first != NULL){
    struct mem_block* runner = first;
    while(runner){
      if(runner->is_free && runner->size >= align_bytes){
        if(runner->size - align_bytes >= BLOCK_META_SIZE + ALIGNMENT){ 
          struct mem_block* new_node = (struct mem_block*)((char*)runner + BLOCK_META_SIZE + align_bytes);
          new_node->size = runner->size - align_bytes - BLOCK_META_SIZE;
          new_node->is_free = 1;
          new_node->next = runner->next;
          new_node->prev = runner;
          
          if(new_node->next) new_node->next->prev = new_node;
          runner->next = new_node;
          runner->size = align_bytes;
          
          if (runner == last) {
              last = new_node; 
          }
        }
        runner->is_free = 0;
        return (void*)((char*)runner + BLOCK_META_SIZE);
      }
      runner = runner->next;
    }
  }
  
  void* buffer = sbrk(align_bytes + BLOCK_META_SIZE + ALIGNMENT);
  if (buffer == (void*)-1) return NULL;
  
  struct mem_block* node = align_ptr(buffer, ALIGNMENT);
  node->size = align_bytes;
  node->is_free = 0;
  node->next = NULL;         
  node->prev = last;          
  
  if (last != NULL) {
      last->next = node;      
  }
  if (first == NULL) {
      first = node;        
  }
  last = node;
  return (void*)((char*)node + BLOCK_META_SIZE);
}

void my_free(void* ptr){
    if (!ptr) return;
    
    struct mem_block* node = (struct mem_block*)((char*)ptr - BLOCK_META_SIZE);
    node->is_free = 1;

    if (node->next && node->next->is_free) {
        node->size += BLOCK_META_SIZE + node->next->size;
        node->next = node->next->next;
        if (node->next) {
            node->next->prev = node;
        } else {
            last = node;  
        }
    }

    if (node->prev && node->prev->is_free) {
        node->prev->size += BLOCK_META_SIZE + node->size;
        node->prev->next = node->next;
        if (node->next) {
            node->next->prev = node->prev;
        } else {
            last = node->prev; 
        }
        
    }
}
