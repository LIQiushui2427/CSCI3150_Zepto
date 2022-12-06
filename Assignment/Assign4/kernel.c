#include "kernel.h"
//YI Jian 1155157207
//Bonus Asg 4
/*
  1. Check if a free process slot exists and if the there's enough free space (check allocated_pages).
  2. Alloc space for page_table (the size of it depends on how many pages you need) and update allocated_pages.
  3. The mapping to kernel-managed memory is not built up, all the PFN should be set to -1 and present byte to 0.
  4. Return a pid (the index in MMStruct array) which is >= 0 when success, -1 when failure in any above step.
*/
int proc_create_vm(struct Kernel* kernel, int size){
    if(size > VIRTUAL_SPACE_SIZE) return -1;
    	int pages = (size - 1) / PAGE_SIZE + 1;//ceil
	if (kernel->allocated_pages + pages > KERNEL_SPACE_SIZE / PAGE_SIZE) return -1;
    for(int i = 0; i < MAX_PROCESS_NUM; ++i)
        if(!kernel->running[i]){
            kernel->running[i] = 1;
            kernel->mm[i].size = size;
            kernel->mm[i].page_table = malloc(sizeof(struct PageTable));
            kernel->mm[i].page_table->ptes = malloc(sizeof(struct PTE) * ((size - 1) / PAGE_SIZE + 1));
            for(int j = 0; j < ((size - 1) / PAGE_SIZE + 1); ++j){
                kernel->mm[i].page_table->ptes[j].PFN = -1;
                kernel->mm[i].page_table->ptes[j].present = 0;
            }
            return i;
        }
}

/*
  This function will read the range [addr, addr+size) from user space of a specific process to the buf (buf should be >= size).
  1. Check if the reading range is out-of-bounds.
  2. If the pages in the range [addr, addr+size) of the user space of that process are not present,
     you should firstly map them to the free kernel-managed memory pages (first fit policy).
  Return 0 when success, -1 when failure (out of bounds).
*/
int vm_read(struct Kernel* kernel, int pid, char* addr, int size, char* buf) {
    int address = (int)addr;
    if(address + size > kernel->mm[pid].size) return -1;
    int start = address / PAGE_SIZE;
    int end = (address + size - 1) / PAGE_SIZE;
    for(int i = start; i <= end; ++i){
        if(!kernel->mm[pid].page_table->ptes[i].present){//not present
            for(int j = 0; j < VIRTUAL_SPACE_SIZE / PAGE_SIZE; ++j)
                if(!kernel->occupied_pages[j]){
                    kernel->occupied_pages[j] = 1;
                    kernel->allocated_pages++;
                    kernel->mm[pid].page_table->ptes[i].PFN = j;
                    kernel->mm[pid].page_table->ptes[i].present = 1;
                    break;
                }
        }
    }
    for(int i = 0; i < size; ++i)
        buf[i] = kernel->space[address + i];
    return 0;
}

/*
  This function will write the content of buf to user space [addr, addr+size) (buf should be >= size).
  1. Check if the writing range is out-of-bounds.
  2. If the pages in the range [addr, addr+size) of the user space of that process are not present,
     you should firstly map them to the free kernel-managed memory pages (first fit policy).
  Return 0 when success, -1 when failure (out of bounds).
*/
int vm_write(struct Kernel* kernel, int pid, char* addr, int size, char* buf) {
    int address = (int)addr;
    if(address + size > kernel->mm[pid].size) return -1;
    int start = address / PAGE_SIZE;
    int end = (address + size - 1) / PAGE_SIZE;
    for(int i = start; i <= end; ++i){
        if(!kernel->mm[pid].page_table->ptes[i].present){
            for(int j = 0; j < VIRTUAL_SPACE_SIZE / PAGE_SIZE; ++j)
                if(!kernel->occupied_pages[j]){
                    kernel->occupied_pages[j] = 1;
                    kernel->allocated_pages++;
                    kernel->mm[pid].page_table->ptes[i].PFN = j;
                    kernel->mm[pid].page_table->ptes[i].present = 1;
                    break;
                }
        }
    }
    for(int i = 0; i < size; ++i)
        kernel->space[address + i] = buf[i];
    return 0;
}

/*
  This function will free the space of a process.
  1. Reset the corresponding pages in occupied_pages to 0.
  2. Release the page_table in the corresponding MMStruct and set to NULL.
  Return 0 when success, -1 when failure.
*/
int proc_exit_vm(struct Kernel* kernel, int pid) {
    if(pid < 0 || pid >= MAX_PROCESS_NUM) return -1;
    if(!kernel->running[pid]) return -1;
    kernel->running[pid] = 0;
    int pages = (kernel->mm[pid].size - 1) / PAGE_SIZE + 1;
    for(int i = 0; i < pages; i++) {
        if(kernel->mm[pid].page_table->ptes[i].present) {
            kernel->occupied_pages[kernel->mm[pid].page_table->ptes[i].PFN] = 0;
        }
    }
    free(kernel->mm[pid].page_table->ptes);
    free(kernel->mm[pid].page_table);
    kernel->mm[pid].page_table = NULL;
    kernel->allocated_pages -= pages;
    return 0;
}
