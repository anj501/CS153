#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {

struct proc *curproc = myproc(); //current process

uint sz = PGROUNDUP(curproc->sz); //rounds the page size up
int i;
int j = 64;
int k = 0;
acquire(&(shm_table.lock));
for (i = 0; i < 64; i++) {
    if (j == 64 && shm_table.shm_pages[i].id == 0) {
      j = i;
    }
  if (shm_table.shm_pages[i].id == id) {
    k = 1;
    mappages(curproc->pgdir, (char *)sz, PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);
    shm_table.shm_pages[i].refcnt++;
    break;
  }
}

if (k ==0 && j < 64) {
  shm_table.shm_pages[j].id = id;
    shm_table.shm_pages[j].frame = kalloc();
  memset(shm_table.shm_pages[j].frame, 0, PGSIZE);
  mappages(curproc->pgdir, (char *)sz, PGSIZE, V2P(shm_table.shm_pages[j].frame), PTE_W|PTE_U);
  shm_table.shm_pages[j].refcnt++;
}

curproc->sz = sz + PGSIZE;
*pointer = (char *)sz;
release(&(shm_table.lock));

return 0; //added to remove compiler warning -- you should decide what to return
}


static pte_t *
walkpgdir(pde_t *pgdir, const void *va, int alloc)
{
  pde_t *pde;
  pte_t *pgtab;

  pde = &pgdir[PDX(va)];
  if(*pde & PTE_P){
    pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
  } else {
    if(!alloc || (pgtab = (pte_t*)kalloc()) == 0)
      return 0;
    // Make sure all those PTE_P bits are zero.
    memset(pgtab, 0, PGSIZE);
    // The permissions here are overly generous, but they can
    // be further restricted by the permissions in the page table
    // entries, if necessary.
    *pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
  }
  return &pgtab[PTX(va)];
}

int shm_close(int id) {

int i;
struct proc *curproc = myproc();
uint va = PGROUNDUP(curproc->sz - PGSIZE);
acquire(&(shm_table.lock));

for (i = 0; i < 64; i++) {
  if (shm_table.shm_pages[i].id == id) {
    shm_table.shm_pages[i].refcnt--;
    if (shm_table.shm_pages[i].refcnt > 0) {
      break;
    }
      shm_table.shm_pages[i].id = 0;
      shm_table.shm_pages[i].frame = 0;
      pte_t * pte = walkpgdir(curproc->pgdir, (void *) va, 0);
      *pte = 0;
      release(&(shm_table.lock));
      return 0;
  }
}

release(&(shm_table.lock));

return 1; //added to remove compiler warning -- you should decide what to return
}

