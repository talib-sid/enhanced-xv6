#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
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

uint64 sys_trace(void)
{
  int mask;

  int f = argint(0,&mask);

  if(f < 0)
  {
    return -1;
  }

  struct proc *p = myproc();
  p->mask = mask;

  return 0; 
}

uint64 sys_sigalarm(void)
{
  int ticks;
  uint64 handler;

 argint(0,&ticks);
 argaddr(1,&handler);
  
  struct proc* p = myproc();

  p->ticks = ticks;
  p->handler = handler;
  p->elapsed_ticks = 0;

  return 0;
}

uint64 sys_sigreturn(void)
{
  struct proc* p = myproc();

  // Recover saved trapframe.
  //*p->trapframe = p->saved_tf;    //was failing test 2
  // if trapframe not recovered , test 0,2,3 were passing
  memmove(p->trapframe,&(p->saved_tf),sizeof(struct trapframe)); // currently passing tests 0,1,2

  // p->trapframe->a0 = p->saved_tf.a0;

  p->elapsed_ticks = 0;

  return p->saved_tf.a0;    // This is the return value of sigreturn, the state of a0 reg in the saved trapframe
}

