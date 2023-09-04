#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  uint64 page_adr;
  int n_of_pages;
  uint64 amask_adr;
  char amask[512/8];

  argaddr(0, &page_adr);
  argint(1, &n_of_pages);
  argaddr(2, &amask_adr);
  
  if (n_of_pages > 512) {
    return -1;
  }

  uint max_mask_cell = n_of_pages/8+1;
  memset(amask, 0, sizeof(char)*max_mask_cell);

  struct proc* p = myproc();
  uint64 va = PGROUNDDOWN(page_adr);

  for(int poffset = 0; poffset < n_of_pages; poffset++) {
    pte_t* pte = walk(p->pagetable, va+PGSIZE*poffset, 0);
    uint cell = poffset / 8;
    uint cellbit = poffset % 8;
    if(*pte & PTE_A) {
      // TODO: Segna bitmask 1 e pulisci
      amask[cell] |= (1 << cellbit);
      *pte &= ~PTE_A;
    } else {
      amask[cell] |= (0 << cellbit);
    }
  }

  copyout(p->pagetable, amask_adr, amask, max_mask_cell);
  
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
