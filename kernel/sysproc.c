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
  uint64 start_addr;
  int n; 
  uint64 return_addr;

  argaddr(0, &start_addr);
  argint(1, &n);
  argaddr(2, &return_addr);

  unsigned int bit_mask = 0;

  if (n > MAX_PTE){
    return -1;
  }

  struct proc *p = myproc();
  pte_t *current_pte;

  for (int i = 0; i < n; i++){
    current_pte = walk(p->pagetable, start_addr + i*PGSIZE, 0);
    // check if the page is valid
    if (current_pte == 0 || (*current_pte & PTE_V) == 0){
      return -1;
    }

    if ((*current_pte & PTE_A)){
      bit_mask |= 1 << i;
    }

    *current_pte &= ~PTE_A;
  }

  if (copyout(p->pagetable, return_addr, (char *)&bit_mask, sizeof(bit_mask)) < 0) {
    return -1;
  }
  
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
