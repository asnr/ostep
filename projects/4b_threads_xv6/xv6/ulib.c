#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"
#include "mmu.h"

void lock_init(lock_t *lock)
{
  lock->locked = 0;
}

void lock_acquire(lock_t *lock)
{
  while (xchg(&(lock->locked), 1) != 0)
    ;

  __sync_synchronize();
}

void lock_release(lock_t *lock)
{
  __sync_synchronize();

  // Atomic set lock->locked = 0
  asm volatile("movl $0, %0" : "+m" (lock->locked) : );
}

int
thread_create(void (*start_routine)(void*), void *arg)
{
  // The stack needs to be page aligned. The only alignment guarantee
  // malloc gives us is alignment to sizeof(long) (we will make our
  // implementation independent of this, however).
  //
  // To get around this, we will request more memory than we need and
  // align the stack ourselves. Later on, join() will only give us the
  // address of the stack, so we will store the starting address of the
  // malloc'd block immediately before the stack so that thread_join()
  // can free() the entire malloc'd block.
  //
  // We will request 2*PGSIZE + sizeof(void *) bytes, which will
  // guarantee we have enough space to correctly align the stack.
  //
  // Note that this strategy wastes around a page of memory.
  const uint HEADER_SZ = sizeof(uint);
  const uint MEM_SZ = 2 * (uint)PGSIZE + HEADER_SZ;
  uint mem = (uint) malloc(MEM_SZ);
  if (mem == 0) {
    return -1;
  }

  uint fst_possible_pgstart = mem + HEADER_SZ;
  uint stack = PGROUNDUP(fst_possible_pgstart);

  if (stack - HEADER_SZ < mem || stack + PGSIZE > mem + MEM_SZ) {
    printf(2, "WARNING in thread_create: stack does not fit in malloc'd block");
  }

  uint *header = (uint *) (stack - HEADER_SZ);
  *header = mem;

  int clonerc = clone(start_routine, arg, (void *) stack);
  if (clonerc == -1) {
    free((void *) mem);
  }

  return clonerc;
}

int
thread_join()
{
  void *stack;
  int joinrc = join(&stack);
  if (joinrc < 0) {
    return -1;
  }

  uint *header = ((uint *) stack) - 1;
  void *malloc_block_start = (void *) *header;
  free(malloc_block_start);

  return joinrc;
}

char*
strcpy(char *s, char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  stosb(dst, c, n);
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, void *vsrc, int n)
{
  char *dst, *src;

  dst = vdst;
  src = vsrc;
  while(n-- > 0)
    *dst++ = *src++;
  return vdst;
}
