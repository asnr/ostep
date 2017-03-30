#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"

char buf[8192];
char name[3];
char *echoargv[] = { "echo", "ALL", "TESTS", "PASSED", 0 };
int stdout = 1;

// does chdir() call iput(p->cwd) in a transaction?
void
iputtest(void)
{
  printf(stdout, "iput test\n");

  if(mkdir("iputdir") < 0){
    printf(stdout, "mkdir failed\n");
    exit();
  }
  if(chdir("iputdir") < 0){
    printf(stdout, "chdir iputdir failed\n");
    exit();
  }
  if(unlink("../iputdir") < 0){
    printf(stdout, "unlink ../iputdir failed\n");
    exit();
  }
  if(chdir("/") < 0){
    printf(stdout, "chdir / failed\n");
    exit();
  }
  printf(stdout, "iput test ok\n");
}

// does exit() call iput(p->cwd) in a transaction?
void
exitiputtest(void)
{
  int pid;

  printf(stdout, "exitiput test\n");

  pid = fork();
  if(pid < 0){
    printf(stdout, "fork failed\n");
    exit();
  }
  if(pid == 0){
    if(mkdir("iputdir") < 0){
      printf(stdout, "mkdir failed\n");
      exit();
    }
    if(chdir("iputdir") < 0){
      printf(stdout, "child chdir failed\n");
      exit();
    }
    if(unlink("../iputdir") < 0){
      printf(stdout, "unlink ../iputdir failed\n");
      exit();
    }
    exit();
  }
  wait();
  printf(stdout, "exitiput test ok\n");
}

// does the error path in open() for attempt to write a
// directory call iput() in a transaction?
// needs a hacked kernel that pauses just after the namei()
// call in sys_open():
//    if((ip = namei(path)) == 0)
//      return -1;
//    {
//      int i;
//      for(i = 0; i < 10000; i++)
//        yield();
//    }
void
openiputtest(void)
{
  int pid;

  printf(stdout, "openiput test\n");
  if(mkdir("oidir") < 0){
    printf(stdout, "mkdir oidir failed\n");
    exit();
  }
  pid = fork();
  if(pid < 0){
    printf(stdout, "fork failed\n");
    exit();
  }
  if(pid == 0){
    int fd = open("oidir", O_RDWR);
    if(fd >= 0){
      printf(stdout, "open directory for write succeeded\n");
      exit();
    }
    exit();
  }
  sleep(1);
  if(unlink("oidir") != 0){
    printf(stdout, "unlink failed\n");
    exit();
  }
  wait();
  printf(stdout, "openiput test ok\n");
}

// simple file system tests

void
opentest(void)
{
  int fd;

  printf(stdout, "open test\n");
  fd = open("echo", 0);
  if(fd < 0){
    printf(stdout, "open echo failed!\n");
    exit();
  }
  close(fd);
  fd = open("doesnotexist", 0);
  if(fd >= 0){
    printf(stdout, "open doesnotexist succeeded!\n");
    exit();
  }
  printf(stdout, "open test ok\n");
}

void
writetest(void)
{
  int fd;
  int i;

  printf(stdout, "small file test\n");
  fd = open("small", O_CREATE|O_RDWR);
  if(fd >= 0){
    printf(stdout, "creat small succeeded; ok\n");
  } else {
    printf(stdout, "error: creat small failed!\n");
    exit();
  }
  for(i = 0; i < 100; i++){
    if(write(fd, "aaaaaaaaaa", 10) != 10){
      printf(stdout, "error: write aa %d new file failed\n", i);
      exit();
    }
    if(write(fd, "bbbbbbbbbb", 10) != 10){
      printf(stdout, "error: write bb %d new file failed\n", i);
      exit();
    }
  }
  printf(stdout, "writes ok\n");
  close(fd);
  fd = open("small", O_RDONLY);
  if(fd >= 0){
    printf(stdout, "open small succeeded ok\n");
  } else {
    printf(stdout, "error: open small failed!\n");
    exit();
  }
  i = read(fd, buf, 2000);
  if(i == 2000){
    printf(stdout, "read succeeded ok\n");
  } else {
    printf(stdout, "read failed\n");
    exit();
  }
  close(fd);

  if(unlink("small") < 0){
    printf(stdout, "unlink small failed\n");
    exit();
  }
  printf(stdout, "small file test ok\n");
}

void
writetest1(void)
{
  int i, fd, n;

  printf(stdout, "big files test\n");

  fd = open("big", O_CREATE|O_RDWR);
  if(fd < 0){
    printf(stdout, "error: creat big failed!\n");
    exit();
  }

  for(i = 0; i < MAXFILE; i++){
    ((int*)buf)[0] = i;
    if(write(fd, buf, 512) != 512){
      printf(stdout, "error: write big file failed\n", i);
      exit();
    }
  }

  close(fd);

  fd = open("big", O_RDONLY);
  if(fd < 0){
    printf(stdout, "error: open big failed!\n");
    exit();
  }

  n = 0;
  for(;;){
    i = read(fd, buf, 512);
    if(i == 0){
      if(n == MAXFILE - 1){
        printf(stdout, "read only %d blocks from big", n);
        exit();
      }
      break;
    } else if(i != 512){
      printf(stdout, "read failed %d\n", i);
      exit();
    }
    if(((int*)buf)[0] != n){
      printf(stdout, "read content of block %d is %d\n",
             n, ((int*)buf)[0]);
      exit();
    }
    n++;
  }
  close(fd);
  if(unlink("big") < 0){
    printf(stdout, "unlink big failed\n");
    exit();
  }
  printf(stdout, "big files ok\n");
}

void
createtest(void)
{
  int i, fd;

  printf(stdout, "many creates, followed by unlink test\n");

  name[0] = 'a';
  name[2] = '\0';
  for(i = 0; i < 52; i++){
    name[1] = '0' + i;
    fd = open(name, O_CREATE|O_RDWR);
    close(fd);
  }
  name[0] = 'a';
  name[2] = '\0';
  for(i = 0; i < 52; i++){
    name[1] = '0' + i;
    unlink(name);
  }
  printf(stdout, "many creates, followed by unlink; ok\n");
}

void dirtest(void)
{
  printf(stdout, "mkdir test\n");

  if(mkdir("dir0") < 0){
    printf(stdout, "mkdir failed\n");
    exit();
  }

  if(chdir("dir0") < 0){
    printf(stdout, "chdir dir0 failed\n");
    exit();
  }

  if(chdir("..") < 0){
    printf(stdout, "chdir .. failed\n");
    exit();
  }

  if(unlink("dir0") < 0){
    printf(stdout, "unlink dir0 failed\n");
    exit();
  }
  printf(stdout, "mkdir test ok\n");
}

void
exectest(void)
{
  printf(stdout, "exec test\n");
  if(exec("echo", echoargv) < 0){
    printf(stdout, "exec echo failed\n");
    exit();
  }
}

// simple fork and pipe read/write

void
pipe1(void)
{
  int fds[2], pid;
  int seq, i, n, cc, total;

  if(pipe(fds) != 0){
    printf(1, "pipe() failed\n");
    exit();
  }
  pid = fork();
  seq = 0;
  if(pid == 0){
    close(fds[0]);
    for(n = 0; n < 5; n++){
      for(i = 0; i < 1033; i++)
        buf[i] = seq++;
      if(write(fds[1], buf, 1033) != 1033){
        printf(1, "pipe1 oops 1\n");
        exit();
      }
    }
    exit();
  } else if(pid > 0){
    close(fds[1]);
    total = 0;
    cc = 1;
    while((n = read(fds[0], buf, cc)) > 0){
      for(i = 0; i < n; i++){
        if((buf[i] & 0xff) != (seq++ & 0xff)){
          printf(1, "pipe1 oops 2\n");
          return;
        }
      }
      total += n;
      cc = cc * 2;
      if(cc > sizeof(buf))
        cc = sizeof(buf);
    }
    if(total != 5 * 1033){
      printf(1, "pipe1 oops 3 total %d\n", total);
      exit();
    }
    close(fds[0]);
    wait();
  } else {
    printf(1, "fork() failed\n");
    exit();
  }
  printf(1, "pipe1 ok\n");
}

// meant to be run w/ at most two CPUs
void
preempt(void)
{
  int pid1, pid2, pid3;
  int pfds[2];

  printf(1, "preempt: ");
  pid1 = fork();
  if(pid1 == 0)
    for(;;)
      ;

  pid2 = fork();
  if(pid2 == 0)
    for(;;)
      ;

  pipe(pfds);
  pid3 = fork();
  if(pid3 == 0){
    close(pfds[0]);
    if(write(pfds[1], "x", 1) != 1)
      printf(1, "preempt write error");
    close(pfds[1]);
    for(;;)
      ;
  }

  close(pfds[1]);
  if(read(pfds[0], buf, sizeof(buf)) != 1){
    printf(1, "preempt read error");
    return;
  }
  close(pfds[0]);
  printf(1, "kill... ");
  kill(pid1);
  kill(pid2);
  kill(pid3);
  printf(1, "wait... ");
  wait();
  wait();
  wait();
  printf(1, "preempt ok\n");
}

// try to find any races between exit and wait
void
exitwait(void)
{
  int i, pid;

  for(i = 0; i < 100; i++){
    pid = fork();
    if(pid < 0){
      printf(1, "fork failed\n");
      return;
    }
    if(pid){
      if(wait() != pid){
        printf(1, "wait wrong pid\n");
        return;
      }
    } else {
      exit();
    }
  }
  printf(1, "exitwait ok\n");
}

void
mem(void)
{
  void *m1, *m2;
  int pid, ppid;

  printf(1, "mem test\n");
  ppid = getpid();
  if((pid = fork()) == 0){
    m1 = 0;
    while((m2 = malloc(10001)) != 0){
      *(char**)m2 = m1;
      m1 = m2;
    }
    while(m1){
      m2 = *(char**)m1;
      free(m1);
      m1 = m2;
    }
    m1 = malloc(1024*20);
    if(m1 == 0){
      printf(1, "couldn't allocate mem?!!\n");
      kill(ppid);
      exit();
    }
    free(m1);
    printf(1, "mem ok\n");
    exit();
  } else {
    wait();
  }
}

// More file system tests

// two processes write to the same file descriptor
// is the offset shared? does inode locking work?
void
sharedfd(void)
{
  int fd, pid, i, n, nc, np;
  char buf[10];

  printf(1, "sharedfd test\n");

  unlink("sharedfd");
  fd = open("sharedfd", O_CREATE|O_RDWR);
  if(fd < 0){
    printf(1, "fstests: cannot open sharedfd for writing");
    return;
  }
  pid = fork();
  memset(buf, pid==0?'c':'p', sizeof(buf));
  for(i = 0; i < 1000; i++){
    if(write(fd, buf, sizeof(buf)) != sizeof(buf)){
      printf(1, "fstests: write sharedfd failed\n");
      break;
    }
  }
  if(pid == 0)
    exit();
  else
    wait();
  close(fd);
  fd = open("sharedfd", 0);
  if(fd < 0){
    printf(1, "fstests: cannot open sharedfd for reading\n");
    return;
  }
  nc = np = 0;
  while((n = read(fd, buf, sizeof(buf))) > 0){
    for(i = 0; i < sizeof(buf); i++){
      if(buf[i] == 'c')
        nc++;
      if(buf[i] == 'p')
        np++;
    }
  }
  close(fd);
  unlink("sharedfd");
  if(nc == 10000 && np == 10000){
    printf(1, "sharedfd ok\n");
  } else {
    printf(1, "sharedfd oops %d %d\n", nc, np);
    exit();
  }
}

// four processes write different files at the same
// time, to test block allocation.
void
fourfiles(void)
{
  int fd, pid, i, j, n, total, pi;
  char *names[] = { "f0", "f1", "f2", "f3" };
  char *fname;

  printf(1, "fourfiles test\n");

  for(pi = 0; pi < 4; pi++){
    fname = names[pi];
    unlink(fname);

    pid = fork();
    if(pid < 0){
      printf(1, "fork failed\n");
      exit();
    }

    if(pid == 0){
      fd = open(fname, O_CREATE | O_RDWR);
      if(fd < 0){
        printf(1, "create failed\n");
        exit();
      }

      memset(buf, '0'+pi, 512);
      for(i = 0; i < 12; i++){
        if((n = write(fd, buf, 500)) != 500){
          printf(1, "write failed %d\n", n);
          exit();
        }
      }
      exit();
    }
  }

  for(pi = 0; pi < 4; pi++){
    wait();
  }

  for(i = 0; i < 2; i++){
    fname = names[i];
    fd = open(fname, 0);
    total = 0;
    while((n = read(fd, buf, sizeof(buf))) > 0){
      for(j = 0; j < n; j++){
        if(buf[j] != '0'+i){
          printf(1, "wrong char\n");
          exit();
        }
      }
      total += n;
    }
    close(fd);
    if(total != 12*500){
      printf(1, "wrong length %d\n", total);
      exit();
    }
    unlink(fname);
  }

  printf(1, "fourfiles ok\n");
}

// four processes create and delete different files in same directory
void
createdelete(void)
{
  enum { N = 20 };
  int pid, i, fd, pi;
  char name[32];

  printf(1, "createdelete test\n");

  for(pi = 0; pi < 4; pi++){
    pid = fork();
    if(pid < 0){
      printf(1, "fork failed\n");
      exit();
    }

    if(pid == 0){
      name[0] = 'p' + pi;
      name[2] = '\0';
      for(i = 0; i < N; i++){
        name[1] = '0' + i;
        fd = open(name, O_CREATE | O_RDWR);
        if(fd < 0){
          printf(1, "create failed\n");
          exit();
        }
        close(fd);
        if(i > 0 && (i % 2 ) == 0){
          name[1] = '0' + (i / 2);
          if(unlink(name) < 0){
            printf(1, "unlink failed\n");
            exit();
          }
        }
      }
      exit();
    }
  }

  for(pi = 0; pi < 4; pi++){
    wait();
  }

  name[0] = name[1] = name[2] = 0;
  for(i = 0; i < N; i++){
    for(pi = 0; pi < 4; pi++){
      name[0] = 'p' + pi;
      name[1] = '0' + i;
      fd = open(name, 0);
      if((i == 0 || i >= N/2) && fd < 0){
        printf(1, "oops createdelete %s didn't exist\n", name);
        exit();
      } else if((i >= 1 && i < N/2) && fd >= 0){
        printf(1, "oops createdelete %s did exist\n", name);
        exit();
      }
      if(fd >= 0)
        close(fd);
    }
  }

  for(i = 0; i < N; i++){
    for(pi = 0; pi < 4; pi++){
      name[0] = 'p' + i;
      name[1] = '0' + i;
      unlink(name);
    }
  }

  printf(1, "createdelete ok\n");
}

// can I unlink a file and still read it?
void
unlinkread(void)
{
  int fd, fd1;

  printf(1, "unlinkread test\n");
  fd = open("unlinkread", O_CREATE | O_RDWR);
  if(fd < 0){
    printf(1, "create unlinkread failed\n");
    exit();
  }
  write(fd, "hello", 5);
  close(fd);

  fd = open("unlinkread", O_RDWR);
  if(fd < 0){
    printf(1, "open unlinkread failed\n");
    exit();
  }
  if(unlink("unlinkread") != 0){
    printf(1, "unlink unlinkread failed\n");
    exit();
  }

  fd1 = open("unlinkread", O_CREATE | O_RDWR);
  write(fd1, "yyy", 3);
  close(fd1);

  if(read(fd, buf, sizeof(buf)) != 5){
    printf(1, "unlinkread read failed");
    exit();
  }
  if(buf[0] != 'h'){
    printf(1, "unlinkread wrong data\n");
    exit();
  }
  if(write(fd, buf, 10) != 10){
    printf(1, "unlinkread write failed\n");
    exit();
  }
  close(fd);
  unlink("unlinkread");
  printf(1, "unlinkread ok\n");
}

void
linktest(void)
{
  int fd;

  printf(1, "linktest\n");

  unlink("lf1");
  unlink("lf2");

  fd = open("lf1", O_CREATE|O_RDWR);
  if(fd < 0){
    printf(1, "create lf1 failed\n");
    exit();
  }
  if(write(fd, "hello", 5) != 5){
    printf(1, "write lf1 failed\n");
    exit();
  }
  close(fd);

  if(link("lf1", "lf2") < 0){
    printf(1, "link lf1 lf2 failed\n");
    exit();
  }
  unlink("lf1");

  if(open("lf1", 0) >= 0){
    printf(1, "unlinked lf1 but it is still there!\n");
    exit();
  }

  fd = open("lf2", 0);
  if(fd < 0){
    printf(1, "open lf2 failed\n");
    exit();
  }
  if(read(fd, buf, sizeof(buf)) != 5){
    printf(1, "read lf2 failed\n");
    exit();
  }
  close(fd);

  if(link("lf2", "lf2") >= 0){
    printf(1, "link lf2 lf2 succeeded! oops\n");
    exit();
  }

  unlink("lf2");
  if(link("lf2", "lf1") >= 0){
    printf(1, "link non-existant succeeded! oops\n");
    exit();
  }

  if(link(".", "lf1") >= 0){
    printf(1, "link . lf1 succeeded! oops\n");
    exit();
  }

  printf(1, "linktest ok\n");
}

// test concurrent create/link/unlink of the same file
void
concreate(void)
{
  char file[3];
  int i, pid, n, fd;
  char fa[40];
  struct {
    ushort inum;
    char name[14];
  } de;

  printf(1, "concreate test\n");
  file[0] = 'C';
  file[2] = '\0';
  for(i = 0; i < 40; i++){
    file[1] = '0' + i;
    unlink(file);
    pid = fork();
    if(pid && (i % 3) == 1){
      link("C0", file);
    } else if(pid == 0 && (i % 5) == 1){
      link("C0", file);
    } else {
      fd = open(file, O_CREATE | O_RDWR);
      if(fd < 0){
        printf(1, "concreate create %s failed\n", file);
        exit();
      }
      close(fd);
    }
    if(pid == 0)
      exit();
    else
      wait();
  }

  memset(fa, 0, sizeof(fa));
  fd = open(".", 0);
  n = 0;
  while(read(fd, &de, sizeof(de)) > 0){
    if(de.inum == 0)
      continue;
    if(de.name[0] == 'C' && de.name[2] == '\0'){
      i = de.name[1] - '0';
      if(i < 0 || i >= sizeof(fa)){
        printf(1, "concreate weird file %s\n", de.name);
        exit();
      }
      if(fa[i]){
        printf(1, "concreate duplicate file %s\n", de.name);
        exit();
      }
      fa[i] = 1;
      n++;
    }
  }
  close(fd);

  if(n != 40){
    printf(1, "concreate not enough files in directory listing\n");
    exit();
  }

  for(i = 0; i < 40; i++){
    file[1] = '0' + i;
    pid = fork();
    if(pid < 0){
      printf(1, "fork failed\n");
      exit();
    }
    if(((i % 3) == 0 && pid == 0) ||
       ((i % 3) == 1 && pid != 0)){
      close(open(file, 0));
      close(open(file, 0));
      close(open(file, 0));
      close(open(file, 0));
    } else {
      unlink(file);
      unlink(file);
      unlink(file);
      unlink(file);
    }
    if(pid == 0)
      exit();
    else
      wait();
  }

  printf(1, "concreate ok\n");
}

// another concurrent link/unlink/create test,
// to look for deadlocks.
void
linkunlink()
{
  int pid, i;

  printf(1, "linkunlink test\n");

  unlink("x");
  pid = fork();
  if(pid < 0){
    printf(1, "fork failed\n");
    exit();
  }

  unsigned int x = (pid ? 1 : 97);
  for(i = 0; i < 100; i++){
    x = x * 1103515245 + 12345;
    if((x % 3) == 0){
      close(open("x", O_RDWR | O_CREATE));
    } else if((x % 3) == 1){
      link("cat", "x");
    } else {
      unlink("x");
    }
  }

  if(pid)
    wait();
  else
    exit();

  printf(1, "linkunlink ok\n");
}

// directory that uses indirect blocks
void
bigdir(void)
{
  int i, fd;
  char name[10];

  printf(1, "bigdir test\n");
  unlink("bd");

  fd = open("bd", O_CREATE);
  if(fd < 0){
    printf(1, "bigdir create failed\n");
    exit();
  }
  close(fd);

  for(i = 0; i < 500; i++){
    name[0] = 'x';
    name[1] = '0' + (i / 64);
    name[2] = '0' + (i % 64);
    name[3] = '\0';
    if(link("bd", name) != 0){
      printf(1, "bigdir link failed\n");
      exit();
    }
  }

  unlink("bd");
  for(i = 0; i < 500; i++){
    name[0] = 'x';
    name[1] = '0' + (i / 64);
    name[2] = '0' + (i % 64);
    name[3] = '\0';
    if(unlink(name) != 0){
      printf(1, "bigdir unlink failed");
      exit();
    }
  }

  printf(1, "bigdir ok\n");
}

void
subdir(void)
{
  int fd, cc;

  printf(1, "subdir test\n");

  unlink("ff");
  if(mkdir("dd") != 0){
    printf(1, "subdir mkdir dd failed\n");
    exit();
  }

  fd = open("dd/ff", O_CREATE | O_RDWR);
  if(fd < 0){
    printf(1, "create dd/ff failed\n");
    exit();
  }
  write(fd, "ff", 2);
  close(fd);

  if(unlink("dd") >= 0){
    printf(1, "unlink dd (non-empty dir) succeeded!\n");
    exit();
  }

  if(mkdir("/dd/dd") != 0){
    printf(1, "subdir mkdir dd/dd failed\n");
    exit();
  }

  fd = open("dd/dd/ff", O_CREATE | O_RDWR);
  if(fd < 0){
    printf(1, "create dd/dd/ff failed\n");
    exit();
  }
  write(fd, "FF", 2);
  close(fd);

  fd = open("dd/dd/../ff", 0);
  if(fd < 0){
    printf(1, "open dd/dd/../ff failed\n");
    exit();
  }
  cc = read(fd, buf, sizeof(buf));
  if(cc != 2 || buf[0] != 'f'){
    printf(1, "dd/dd/../ff wrong content\n");
    exit();
  }
  close(fd);

  if(link("dd/dd/ff", "dd/dd/ffff") != 0){
    printf(1, "link dd/dd/ff dd/dd/ffff failed\n");
    exit();
  }

  if(unlink("dd/dd/ff") != 0){
    printf(1, "unlink dd/dd/ff failed\n");
    exit();
  }
  if(open("dd/dd/ff", O_RDONLY) >= 0){
    printf(1, "open (unlinked) dd/dd/ff succeeded\n");
    exit();
  }

  if(chdir("dd") != 0){
    printf(1, "chdir dd failed\n");
    exit();
  }
  if(chdir("dd/../../dd") != 0){
    printf(1, "chdir dd/../../dd failed\n");
    exit();
  }
  if(chdir("dd/../../../dd") != 0){
    printf(1, "chdir dd/../../dd failed\n");
    exit();
  }
  if(chdir("./..") != 0){
    printf(1, "chdir ./.. failed\n");
    exit();
  }

  fd = open("dd/dd/ffff", 0);
  if(fd < 0){
    printf(1, "open dd/dd/ffff failed\n");
    exit();
  }
  if(read(fd, buf, sizeof(buf)) != 2){
    printf(1, "read dd/dd/ffff wrong len\n");
    exit();
  }
  close(fd);

  if(open("dd/dd/ff", O_RDONLY) >= 0){
    printf(1, "open (unlinked) dd/dd/ff succeeded!\n");
    exit();
  }

  if(open("dd/ff/ff", O_CREATE|O_RDWR) >= 0){
    printf(1, "create dd/ff/ff succeeded!\n");
    exit();
  }
  if(open("dd/xx/ff", O_CREATE|O_RDWR) >= 0){
    printf(1, "create dd/xx/ff succeeded!\n");
    exit();
  }
  if(open("dd", O_CREATE) >= 0){
    printf(1, "create dd succeeded!\n");
    exit();
  }
  if(open("dd", O_RDWR) >= 0){
    printf(1, "open dd rdwr succeeded!\n");
    exit();
  }
  if(open("dd", O_WRONLY) >= 0){
    printf(1, "open dd wronly succeeded!\n");
    exit();
  }
  if(link("dd/ff/ff", "dd/dd/xx") == 0){
    printf(1, "link dd/ff/ff dd/dd/xx succeeded!\n");
    exit();
  }
  if(link("dd/xx/ff", "dd/dd/xx") == 0){
    printf(1, "link dd/xx/ff dd/dd/xx succeeded!\n");
    exit();
  }
  if(link("dd/ff", "dd/dd/ffff") == 0){
    printf(1, "link dd/ff dd/dd/ffff succeeded!\n");
    exit();
  }
  if(mkdir("dd/ff/ff") == 0){
    printf(1, "mkdir dd/ff/ff succeeded!\n");
    exit();
  }
  if(mkdir("dd/xx/ff") == 0){
    printf(1, "mkdir dd/xx/ff succeeded!\n");
    exit();
  }
  if(mkdir("dd/dd/ffff") == 0){
    printf(1, "mkdir dd/dd/ffff succeeded!\n");
    exit();
  }
  if(unlink("dd/xx/ff") == 0){
    printf(1, "unlink dd/xx/ff succeeded!\n");
    exit();
  }
  if(unlink("dd/ff/ff") == 0){
    printf(1, "unlink dd/ff/ff succeeded!\n");
    exit();
  }
  if(chdir("dd/ff") == 0){
    printf(1, "chdir dd/ff succeeded!\n");
    exit();
  }
  if(chdir("dd/xx") == 0){
    printf(1, "chdir dd/xx succeeded!\n");
    exit();
  }

  if(unlink("dd/dd/ffff") != 0){
    printf(1, "unlink dd/dd/ff failed\n");
    exit();
  }
  if(unlink("dd/ff") != 0){
    printf(1, "unlink dd/ff failed\n");
    exit();
  }
  if(unlink("dd") == 0){
    printf(1, "unlink non-empty dd succeeded!\n");
    exit();
  }
  if(unlink("dd/dd") < 0){
    printf(1, "unlink dd/dd failed\n");
    exit();
  }
  if(unlink("dd") < 0){
    printf(1, "unlink dd failed\n");
    exit();
  }

  printf(1, "subdir ok\n");
}

// test writes that are larger than the log.
void
bigwrite(void)
{
  int fd, sz;

  printf(1, "bigwrite test\n");

  unlink("bigwrite");
  for(sz = 499; sz < 12*512; sz += 471){
    fd = open("bigwrite", O_CREATE | O_RDWR);
    if(fd < 0){
      printf(1, "cannot create bigwrite\n");
      exit();
    }
    int i;
    for(i = 0; i < 2; i++){
      int cc = write(fd, buf, sz);
      if(cc != sz){
        printf(1, "write(%d) ret %d\n", sz, cc);
        exit();
      }
    }
    close(fd);
    unlink("bigwrite");
  }

  printf(1, "bigwrite ok\n");
}

void
bigfile(void)
{
  int fd, i, total, cc;

  printf(1, "bigfile test\n");

  unlink("bigfile");
  fd = open("bigfile", O_CREATE | O_RDWR);
  if(fd < 0){
    printf(1, "cannot create bigfile");
    exit();
  }
  for(i = 0; i < 20; i++){
    memset(buf, i, 600);
    if(write(fd, buf, 600) != 600){
      printf(1, "write bigfile failed\n");
      exit();
    }
  }
  close(fd);

  fd = open("bigfile", 0);
  if(fd < 0){
    printf(1, "cannot open bigfile\n");
    exit();
  }
  total = 0;
  for(i = 0; ; i++){
    cc = read(fd, buf, 300);
    if(cc < 0){
      printf(1, "read bigfile failed\n");
      exit();
    }
    if(cc == 0)
      break;
    if(cc != 300){
      printf(1, "short read bigfile\n");
      exit();
    }
    if(buf[0] != i/2 || buf[299] != i/2){
      printf(1, "read bigfile wrong data\n");
      exit();
    }
    total += cc;
  }
  close(fd);
  if(total != 20*600){
    printf(1, "read bigfile wrong total\n");
    exit();
  }
  unlink("bigfile");

  printf(1, "bigfile test ok\n");
}

void
fourteen(void)
{
  int fd;

  // DIRSIZ is 14.
  printf(1, "fourteen test\n");

  if(mkdir("12345678901234") != 0){
    printf(1, "mkdir 12345678901234 failed\n");
    exit();
  }
  if(mkdir("12345678901234/123456789012345") != 0){
    printf(1, "mkdir 12345678901234/123456789012345 failed\n");
    exit();
  }
  fd = open("123456789012345/123456789012345/123456789012345", O_CREATE);
  if(fd < 0){
    printf(1, "create 123456789012345/123456789012345/123456789012345 failed\n");
    exit();
  }
  close(fd);
  fd = open("12345678901234/12345678901234/12345678901234", 0);
  if(fd < 0){
    printf(1, "open 12345678901234/12345678901234/12345678901234 failed\n");
    exit();
  }
  close(fd);

  if(mkdir("12345678901234/12345678901234") == 0){
    printf(1, "mkdir 12345678901234/12345678901234 succeeded!\n");
    exit();
  }
  if(mkdir("123456789012345/12345678901234") == 0){
    printf(1, "mkdir 12345678901234/123456789012345 succeeded!\n");
    exit();
  }

  printf(1, "fourteen ok\n");
}

void
rmdot(void)
{
  printf(1, "rmdot test\n");
  if(mkdir("dots") != 0){
    printf(1, "mkdir dots failed\n");
    exit();
  }
  if(chdir("dots") != 0){
    printf(1, "chdir dots failed\n");
    exit();
  }
  if(unlink(".") == 0){
    printf(1, "rm . worked!\n");
    exit();
  }
  if(unlink("..") == 0){
    printf(1, "rm .. worked!\n");
    exit();
  }
  if(chdir("/") != 0){
    printf(1, "chdir / failed\n");
    exit();
  }
  if(unlink("dots/.") == 0){
    printf(1, "unlink dots/. worked!\n");
    exit();
  }
  if(unlink("dots/..") == 0){
    printf(1, "unlink dots/.. worked!\n");
    exit();
  }
  if(unlink("dots") != 0){
    printf(1, "unlink dots failed!\n");
    exit();
  }
  printf(1, "rmdot ok\n");
}

void
dirfile(void)
{
  int fd;

  printf(1, "dir vs file\n");

  fd = open("dirfile", O_CREATE);
  if(fd < 0){
    printf(1, "create dirfile failed\n");
    exit();
  }
  close(fd);
  if(chdir("dirfile") == 0){
    printf(1, "chdir dirfile succeeded!\n");
    exit();
  }
  fd = open("dirfile/xx", 0);
  if(fd >= 0){
    printf(1, "create dirfile/xx succeeded!\n");
    exit();
  }
  fd = open("dirfile/xx", O_CREATE);
  if(fd >= 0){
    printf(1, "create dirfile/xx succeeded!\n");
    exit();
  }
  if(mkdir("dirfile/xx") == 0){
    printf(1, "mkdir dirfile/xx succeeded!\n");
    exit();
  }
  if(unlink("dirfile/xx") == 0){
    printf(1, "unlink dirfile/xx succeeded!\n");
    exit();
  }
  if(link("README", "dirfile/xx") == 0){
    printf(1, "link to dirfile/xx succeeded!\n");
    exit();
  }
  if(unlink("dirfile") != 0){
    printf(1, "unlink dirfile failed!\n");
    exit();
  }

  fd = open(".", O_RDWR);
  if(fd >= 0){
    printf(1, "open . for writing succeeded!\n");
    exit();
  }
  fd = open(".", 0);
  if(write(fd, "x", 1) > 0){
    printf(1, "write . succeeded!\n");
    exit();
  }
  close(fd);

  printf(1, "dir vs file OK\n");
}

// test that iput() is called at the end of _namei()
void
iref(void)
{
  int i, fd;

  printf(1, "empty file name\n");

  // the 50 is NINODE
  for(i = 0; i < 50 + 1; i++){
    if(mkdir("irefd") != 0){
      printf(1, "mkdir irefd failed\n");
      exit();
    }
    if(chdir("irefd") != 0){
      printf(1, "chdir irefd failed\n");
      exit();
    }

    mkdir("");
    link("README", "");
    fd = open("", O_CREATE);
    if(fd >= 0)
      close(fd);
    fd = open("xx", O_CREATE);
    if(fd >= 0)
      close(fd);
    unlink("xx");
  }

  chdir("/");
  printf(1, "empty file name OK\n");
}

// test that fork fails gracefully
// the forktest binary also does this, but it runs out of proc entries first.
// inside the bigger usertests binary, we run out of memory first.
void
forktest(void)
{
  int n, pid;

  printf(1, "fork test\n");

  for(n=0; n<1000; n++){
    pid = fork();
    if(pid < 0)
      break;
    if(pid == 0)
      exit();
  }

  if(n == 1000){
    printf(1, "fork claimed to work 1000 times!\n");
    exit();
  }

  for(; n > 0; n--){
    if(wait() < 0){
      printf(1, "wait stopped early\n");
      exit();
    }
  }

  if(wait() != -1){
    printf(1, "wait got too many\n");
    exit();
  }

  printf(1, "fork test OK\n");
}

void
sbrktest(void)
{
  int fds[2], pid, pids[10], ppid;
  char *a, *b, *c, *lastaddr, *oldbrk, *p, scratch;
  uint amt;

  printf(stdout, "sbrk test\n");
  oldbrk = sbrk(0);

  // can one sbrk() less than a page?
  a = sbrk(0);
  int i;
  for(i = 0; i < 5000; i++){
    b = sbrk(1);
    if(b != a){
      printf(stdout, "sbrk test failed %d %x %x\n", i, a, b);
      exit();
    }
    *b = 1;
    a = b + 1;
  }
  pid = fork();
  if(pid < 0){
    printf(stdout, "sbrk test fork failed\n");
    exit();
  }
  c = sbrk(1);
  c = sbrk(1);
  if(c != a + 1){
    printf(stdout, "sbrk test failed post-fork\n");
    exit();
  }
  if(pid == 0)
    exit();
  wait();

  // can one grow address space to something big?
#define BIG (100*1024*1024)
  a = sbrk(0);
  amt = (BIG) - (uint)a;
  p = sbrk(amt);
  if (p != a) {
    printf(stdout, "sbrk test failed to grow big address space; enough phys mem?\n");
    exit();
  }
  lastaddr = (char*) (BIG-1);
  *lastaddr = 99;

  // can one de-allocate?
  a = sbrk(0);
  c = sbrk(-4096);
  if(c == (char*)0xffffffff){
    printf(stdout, "sbrk could not deallocate\n");
    exit();
  }
  c = sbrk(0);
  if(c != a - 4096){
    printf(stdout, "sbrk deallocation produced wrong address, a %x c %x\n", a, c);
    exit();
  }

  // can one re-allocate that page?
  a = sbrk(0);
  c = sbrk(4096);
  if(c != a || sbrk(0) != a + 4096){
    printf(stdout, "sbrk re-allocation failed, a %x c %x\n", a, c);
    exit();
  }
  if(*lastaddr == 99){
    // should be zero
    printf(stdout, "sbrk de-allocation didn't really deallocate\n");
    exit();
  }

  a = sbrk(0);
  c = sbrk(-(sbrk(0) - oldbrk));
  if(c != a){
    printf(stdout, "sbrk downsize failed, a %x c %x\n", a, c);
    exit();
  }

  // can we read the kernel's memory?
  for(a = (char*)(KERNBASE); a < (char*) (KERNBASE+2000000); a += 50000){
    ppid = getpid();
    pid = fork();
    if(pid < 0){
      printf(stdout, "fork failed\n");
      exit();
    }
    if(pid == 0){
      printf(stdout, "oops could read %x = %x\n", a, *a);
      kill(ppid);
      exit();
    }
    wait();
  }

  // if we run the system out of memory, does it clean up the last
  // failed allocation?
  if(pipe(fds) != 0){
    printf(1, "pipe() failed\n");
    exit();
  }
  for(i = 0; i < sizeof(pids)/sizeof(pids[0]); i++){
    if((pids[i] = fork()) == 0){
      // allocate a lot of memory
      sbrk(BIG - (uint)sbrk(0));
      write(fds[1], "x", 1);
      // sit around until killed
      for(;;) sleep(1000);
    }
    if(pids[i] != -1)
      read(fds[0], &scratch, 1);
  }
  // if those failed allocations freed up the pages they did allocate,
  // we'll be able to allocate here
  c = sbrk(4096);
  for(i = 0; i < sizeof(pids)/sizeof(pids[0]); i++){
    if(pids[i] == -1)
      continue;
    kill(pids[i]);
    wait();
  }
  if(c == (char*)0xffffffff){
    printf(stdout, "failed sbrk leaked memory\n");
    exit();
  }

  if(sbrk(0) > oldbrk)
    sbrk(-(sbrk(0) - oldbrk));

  printf(stdout, "sbrk test OK\n");
}

void
validateint(int *p)
{
  int res;
  asm("mov %%esp, %%ebx\n\t"
      "mov %3, %%esp\n\t"
      "int %2\n\t"
      "mov %%ebx, %%esp" :
      "=a" (res) :
      "a" (SYS_sleep), "n" (T_SYSCALL), "c" (p) :
      "ebx");
}

void
validatetest(void)
{
  int hi, pid;
  uint p;

  printf(stdout, "validate test\n");
  hi = 1100*1024;

  for(p = 0; p <= (uint)hi; p += 4096){
    if((pid = fork()) == 0){
      // try to crash the kernel by passing in a badly placed integer
      validateint((int*)p);
      exit();
    }
    sleep(0);
    sleep(0);
    kill(pid);
    wait();

    // try to crash the kernel by passing in a bad string pointer
    if(link("nosuchfile", (char*)p) != -1){
      printf(stdout, "link should not succeed\n");
      exit();
    }
  }

  printf(stdout, "validate ok\n");
}

// does unintialized data start out zero?
char uninit[10000];
void
bsstest(void)
{
  int i;

  printf(stdout, "bss test\n");
  for(i = 0; i < sizeof(uninit); i++){
    if(uninit[i] != '\0'){
      printf(stdout, "bss test failed\n");
      exit();
    }
  }
  printf(stdout, "bss test ok\n");
}

// does exec return an error if the arguments
// are larger than a page? or does it write
// below the stack and wreck the instructions/data?
void
bigargtest(void)
{
  int pid, fd;

  unlink("bigarg-ok");
  pid = fork();
  if(pid == 0){
    static char *args[MAXARG];
    int i;
    for(i = 0; i < MAXARG-1; i++)
      args[i] = "bigargs test: failed\n                                                                                                                                                                                                       ";
    args[MAXARG-1] = 0;
    printf(stdout, "bigarg test\n");
    exec("echo", args);
    printf(stdout, "bigarg test ok\n");
    fd = open("bigarg-ok", O_CREATE);
    close(fd);
    exit();
  } else if(pid < 0){
    printf(stdout, "bigargtest: fork failed\n");
    exit();
  }
  wait();
  fd = open("bigarg-ok", 0);
  if(fd < 0){
    printf(stdout, "bigarg test failed!\n");
    exit();
  }
  close(fd);
  unlink("bigarg-ok");
}

// what happens when the file system runs out of blocks?
// answer: balloc panics, so this test is not useful.
void
fsfull()
{
  int nfiles;
  int fsblocks = 0;

  printf(1, "fsfull test\n");

  for(nfiles = 0; ; nfiles++){
    char name[64];
    name[0] = 'f';
    name[1] = '0' + nfiles / 1000;
    name[2] = '0' + (nfiles % 1000) / 100;
    name[3] = '0' + (nfiles % 100) / 10;
    name[4] = '0' + (nfiles % 10);
    name[5] = '\0';
    printf(1, "writing %s\n", name);
    int fd = open(name, O_CREATE|O_RDWR);
    if(fd < 0){
      printf(1, "open %s failed\n", name);
      break;
    }
    int total = 0;
    while(1){
      int cc = write(fd, buf, 512);
      if(cc < 512)
        break;
      total += cc;
      fsblocks++;
    }
    printf(1, "wrote %d bytes\n", total);
    close(fd);
    if(total == 0)
      break;
  }

  while(nfiles >= 0){
    char name[64];
    name[0] = 'f';
    name[1] = '0' + nfiles / 1000;
    name[2] = '0' + (nfiles % 1000) / 100;
    name[3] = '0' + (nfiles % 100) / 10;
    name[4] = '0' + (nfiles % 10);
    name[5] = '\0';
    unlink(name);
    nfiles--;
  }

  printf(1, "fsfull test finished\n");
}

void
uio()
{
  #define RTC_ADDR 0x70
  #define RTC_DATA 0x71

  ushort port = 0;
  uchar val = 0;
  int pid;

  printf(1, "uio test\n");
  pid = fork();
  if(pid == 0){
    port = RTC_ADDR;
    val = 0x09;  /* year */
    /* http://wiki.osdev.org/Inline_Assembly/Examples */
    asm volatile("outb %0,%1"::"a"(val), "d" (port));
    port = RTC_DATA;
    asm volatile("inb %1,%0" : "=a" (val) : "d" (port));
    printf(1, "uio: uio succeeded; test FAILED\n");
    exit();
  } else if(pid < 0){
    printf (1, "fork failed\n");
    exit();
  }
  wait();
  printf(1, "uio test done\n");
}

void argptest()
{
  int fd;
  fd = open("init", O_RDONLY);
  if (fd < 0) {
    printf(2, "open failed\n");
    exit();
  }
  read(fd, sbrk(0) - 1, -1);
  close(fd);
  printf(1, "arg test passed\n");
}


void getprocstest_block_on_read(char *from, int fd, int read_num)
{
  char msg = 'G';
  int msg_len = 1;

  if (read(fd, &msg, msg_len) != msg_len) {
    printf(1, "getprocstest: read() #%d from %s failed\n", read_num, from);
    exit();
  }
}

void getprocstest_write_release(char *to, int fd, int write_num)
{
  char msg = 'G';
  int msg_len = 1;

  if (write(fd, &msg, msg_len) != msg_len) {
    printf(1, "getprocstest: write() #%d to %s failed\n", write_num, to);
    exit();
  }
}

void
getprocstest()
{
  // This test has race conditions! If this process is
  // interrupted midway through and another process is created or
  // dies then this will fail even if getprocs() is correct.
  int numprocs_start = getprocs();
  int numprocs_fst_child, numprocs_snd_child;

  int root_read_fds[2], fst_child_fds[2], snd_child_fds[2];
  if (pipe(root_read_fds) != 0 ||
      pipe(fst_child_fds) != 0 || pipe(snd_child_fds) != 0) {
    printf(1, "pipe() failed\n");
    exit();
  }

  
  int forkrc = fork();
  if (forkrc > 0) {
    close(root_read_fds[1]);
    close(fst_child_fds[0]);
    close(snd_child_fds[0]);

    numprocs_fst_child = getprocs();
    
    getprocstest_write_release("first child", fst_child_fds[1], 1);

    getprocstest_block_on_read("first child", root_read_fds[0], 1);

    numprocs_snd_child = getprocs();
    
    getprocstest_write_release("second child", snd_child_fds[1], 1);
    getprocstest_write_release("first child", fst_child_fds[1], 2);

    wait();

    close(fst_child_fds[1]);
    close(snd_child_fds[1]);

  } else if (forkrc == 0) {
    close(root_read_fds[0]);
    close(fst_child_fds[1]);
    close(snd_child_fds[1]);

    getprocstest_block_on_read("root", fst_child_fds[0], 1);
    
    int forkrc2 = fork();
    if (forkrc2 > 0) {
      getprocstest_block_on_read("root", fst_child_fds[0], 2);
      wait();
      exit();
    } else if (forkrc2 == 0) {
      getprocstest_write_release("root", root_read_fds[1], 1);
      getprocstest_block_on_read("root", snd_child_fds[0], 1);
      exit();
    } else {
      printf(1, "getprocstest: second fork() failed\n");
      exit();
    }
  } else {
    printf(1, "getprocstest: fork() failed\n");
    exit();
  }

  if (numprocs_fst_child != numprocs_start + 1) {
    printf(1, "getprocstest failed: numprocs_start %d, numprocs_fst_child %d\n",
           numprocs_start, numprocs_fst_child);
    exit();
  }

  if (numprocs_snd_child != numprocs_fst_child + 1) {
    printf(1, "getprocstest failed: numprocs_fst_child %d, numprocs_snd_child %d\n",
           numprocs_fst_child, numprocs_snd_child);
    exit();
  }

  printf(1, "getprocs test passed\n");
}

void
deref_null_ptr_test()
{
  int parentpid = getpid();

  int forkrc = fork();
  if (forkrc < 0) {
    printf(1, "deref_null_ptr_test: fork() failed\n");
    exit();
  } else if (forkrc == 0) {
    int *x = 0;
    int y = *x;
    printf(1,
      "deref_null_ptr_test failed: dereferenced null ptr, val at 0: %d\n", y);
    kill(parentpid);
    exit();
  } else {
    wait();
  }

  printf(1, "deref_null_ptr_test passed\n");
}

void
bad_ptr_to_syscall_test()
{
  int parentpid = getpid();

  int forkrc = fork();
  if (forkrc < 0) {
    printf(1, "bad_ptr_to_syscall_test: fork() failed.\n");
    exit();
  } else if (forkrc == 0) {
    int *illegalptr = (int*) 2;
    int piperc = pipe(illegalptr);
    if (piperc != -1) {
      printf(1,
             "bad_ptr_to_syscall_test failed: syscall accepted invalid ptr.\n");
      kill(parentpid);
    }
    exit();
  } else {
    wait();
  }

  printf(1, "bad_ptr_to_syscall_test passed\n");
}

void
exit_if_thread_create_failed(int thread_create_rc, char *testname) {
  if (thread_create_rc < 0) {
    printf(1, "%s: thread_create() failed, it returned %d.\n",
           testname, thread_create_rc);
    exit();
  }
}

void
exit_if_joined_unexpected(int thread_join_rc, int expected, char *testname) {
  if (thread_join_rc != expected) {
    printf(1, "%s: thread_join() expected return code %d, instead got %d\n",
           testname, expected, thread_join_rc);
    exit();
  }
}

void
exit_if_thread_join_error(int thread_join_rc, char *testname) {
  if (thread_join_rc < 0) {
    printf(1, "%s: thread_join() error, returned %d\n",
           testname, thread_join_rc);
    exit();
  }
}

void
exit_immediately(void *unused) {
  exit();
}

void basic_thread_test() {
  char testname[] = "basic_thread_test";
  int thread_create_rc = thread_create(exit_immediately, 0);
  if (thread_create_rc < 0) {
    printf(1, "%s: thread_create failed, it returned %d.\n", testname, thread_create_rc);
    exit();
  }

  int thread_join_rc = thread_join();
  if (thread_join_rc != thread_create_rc) {
    printf(1, "%s: thread_join expected return code %d, got %d.\n", testname, thread_create_rc, thread_join_rc);
    exit();
  }

  printf(1, "%s passed\n", testname);
}

int global_for_thread_test = -189;

void modify_global(void *unused) {
  global_for_thread_test = 45;
  exit();
}

void thread_shares_address_space_test() {
  char testname[] = "thread_shares_address_space_test";

  int thread_create_rc = thread_create(modify_global, 0);
  exit_if_thread_create_failed(thread_create_rc, testname);

  int thread_join_rc = thread_join();
  exit_if_joined_unexpected(thread_join_rc, thread_create_rc, testname);

  if (global_for_thread_test != 45) {
    printf(1, "%s: global was not modified correctly. Expected %d, got %d\n",
           testname, 45, global_for_thread_test);
    exit();
  }

  printf(1, "%s passed\n", testname);
}

void exit_if_pipe_fails(int piperc, char *testname) {
  if (piperc == -1) {
    printf(1, "%s: pipe() failed, return code = %d\n", testname, piperc);
    exit();
  }
}

int childreadfds_global[2];
int childwritefds_global[2];

void pipe_read_and_write(void *unused) {
  close(childreadfds_global[1]);
  close(childwritefds_global[0]);

  int msg;
  read(childreadfds_global[0], &msg, sizeof(int));
  write(childwritefds_global[1], &msg, sizeof(int));

  exit();
}

void thread_starts_with_same_file_descriptors_as_parent() {
  char testname[] = "thread_starts_with_same_file_descriptors_as_parent";

  int piperc1 = pipe(childreadfds_global);
  exit_if_pipe_fails(piperc1, testname);
  int piperc2 = pipe(childwritefds_global);
  exit_if_pipe_fails(piperc2, testname);

  int thread_create_rc = thread_create(pipe_read_and_write, 0);
  exit_if_thread_create_failed(thread_create_rc, testname);

  close(childreadfds_global[0]);
  close(childwritefds_global[1]);

  int sendmsg = 842;
  write(childreadfds_global[1], &sendmsg, sizeof(int));
  int recvmsg = 0;
  read(childwritefds_global[0], &recvmsg, sizeof(int));

  int thread_join_rc = thread_join();
  exit_if_joined_unexpected(thread_join_rc, thread_create_rc, testname);

  if (recvmsg != sendmsg) {
    printf(1, "%s: did not recieve expected message: got %d, expected %d\n",
           testname, recvmsg, sendmsg);
    exit();
  }

  printf(1, "%s passed\n", testname);
}

struct thread_arg_test {
  int x;
  char y;
};

void modify_arg_struct(void *raw_arg) {
  struct thread_arg_test *arg = (struct thread_arg_test *) raw_arg;
  arg->x = -7;
  arg->y = 'c';
  exit();
  return;
}

void pass_argument_to_child_thread_test() {
  char testname[] = "pass_argument_to_child_thread_test";

  struct thread_arg_test arg;
  arg.x = 77;
  arg.y = '!';

  int thread_create_rc = thread_create(modify_arg_struct, &arg);
  exit_if_thread_create_failed(thread_create_rc, testname);

  int thread_join_rc = thread_join();
  if (thread_join_rc != thread_create_rc) {
    printf(1,"%s: thread_join() returned %d, expected %d.\n", testname, thread_join_rc, thread_create_rc);
    exit();
  }

  if (arg.x != -7 || arg.y != 'c') {
    printf(1, "%s: thread failed to modify args, arg.x = %d, arg.y = %c\n",
           testname, arg.x, arg.y);
    exit();
  }

  printf(1, "%s passed\n", testname);
}

int
fib(int n) {
  if (n == 1 || n == 2) return 1;
  return fib(n - 1) + fib(n - 2);
}

void calculate_fibonacci_numbers(void *fail_flag_arg) {
  int *fail_flag = (int *) fail_flag_arg;
  if (fib(6) != 8 || fib(10) != 55 || fib(20) != 6765) {
    *fail_flag = 1;
  }
  exit();
}

void each_thread_gets_its_own_stack_test() {
  char testname[] = "each_thread_gets_its_own_stack_test";

  const int NUM_THREADS = 50;
  int fail_flag = 0;
  int thread_create_rc;
  int i;
  for (i = 0; i < NUM_THREADS; i++) {
    thread_create_rc = thread_create(calculate_fibonacci_numbers, &fail_flag);
    exit_if_thread_create_failed(thread_create_rc, testname);
  }

  int thread_join_rc;
  int j;
  for (j = 0; j < NUM_THREADS; j++) {
    thread_join_rc = thread_join();
    if (thread_join_rc < 0) {
      printf(1, "%s: thread_join() failed, returned %d\n",
             testname, thread_join_rc);
      exit();
    }
  }

  if (fail_flag) {
    printf(1, "%s: one of the fibanocci number was calculated incorrectly.\n",
           testname);
    exit();
  }

  printf(1, "%s passed\n", testname);
}

void thread_join_doesnt_leak_memory_test() {
  // To be implemented
  // Apparently a maximum of 150 pages?
  // Can use sbrk(0) to see how much memory we've been using?
}

char *increased_parent_sz, *decreased_parent_sz;

lock_t incr_addr_space_lk;
lock_t decr_addr_space_lk;
uint address_space_thread_errors;
uint threads_done_incr_addr_check;

void
check_address_size_matches_parent()
{
  char *my_sz = sbrk(0);

  lock_acquire(&incr_addr_space_lk);
  if (my_sz != increased_parent_sz) {
    printf(1, "Expected thread address size %d, got %d\n",
           increased_parent_sz, my_sz);
    address_space_thread_errors++;
  }
  threads_done_incr_addr_check++;
  lock_release(&incr_addr_space_lk);

  lock_acquire(&decr_addr_space_lk);
  my_sz = sbrk(0);
  if (my_sz != decreased_parent_sz) {
    printf(1, "Expected thread address size %d, got %d\n",
           decreased_parent_sz, my_sz);
    address_space_thread_errors++;
  }
  lock_release(&decr_addr_space_lk);

  exit();
}

void
modify_address_space_across_threads_test()
{
  char testname[] = "modify_address_space_across_threads_test";

  address_space_thread_errors = 0;

  lock_init(&incr_addr_space_lk);
  lock_init(&decr_addr_space_lk);
  lock_acquire(&incr_addr_space_lk);
  lock_acquire(&decr_addr_space_lk);

  const int NUM_THREADS = 5;
  int thread_create_rc;
  int i;
  for (i = 0; i < NUM_THREADS; i++) {
    thread_create_rc = thread_create(check_address_size_matches_parent, 0);
    exit_if_thread_create_failed(thread_create_rc, testname);
  }

  sbrk(4096);
  increased_parent_sz = sbrk(0);

  lock_release(&incr_addr_space_lk);

  uint num_threads_done = 0;
  while (num_threads_done < NUM_THREADS) {
    sleep(0);  // don't want to release it and immediately acquire it again
    lock_acquire(&incr_addr_space_lk);
    num_threads_done = threads_done_incr_addr_check;
    lock_release(&incr_addr_space_lk);
  }

  // Now we know for sure that all threads have finished
  // increased address size check.

  sbrk(-113);
  decreased_parent_sz = sbrk(0);

  lock_release(&decr_addr_space_lk);

  int j, thread_join_rc;
  for (j = 0; j < NUM_THREADS; j++) {
    thread_join_rc = thread_join();
    exit_if_thread_join_error(thread_join_rc, testname);
  }

  if (address_space_thread_errors) {
    printf(1, "%s: child threads reported %d address size mismatches.\n",
           testname, address_space_thread_errors);
    exit();
  }

  printf(1, "%s passed\n", testname);
}

void join_ignores_forked_children_test() {
  char testname[] = "join_ignores_forked_children_test";

  int forkrc = fork();
  if (forkrc == 0) {
    exit();
  } else if (forkrc < 0) {
    printf(1, "%s: fork() failed.\n", testname);
    exit();
  }

  void *stack;
  int joinrc = join(&stack);
  if (joinrc != -1) {
    printf(1, "%s: join() should have failed. It returned %d.\n",
           testname, joinrc);
    exit();
  }

  int waitrc = wait();
  if (waitrc != forkrc) {
    printf(1, "%s: wait() should have returned %d, got %d.\n",
           testname, forkrc, waitrc);
    exit();
  }

  printf(1, "%s passed\n", testname);
}

void wait_ignores_child_threads_test() {
  char testname[] = "wait_ignores_child_threads_test";

  int thread_create_rc = thread_create(exit_immediately, 0);
  exit_if_thread_create_failed(thread_create_rc, testname);

  int waitrc = wait();
  if (waitrc != -1) {
    printf(1, "%s: wait() should have failed, but it returned %d\n",
           testname, waitrc);
    exit();
  }

  int thread_join_rc = thread_join();
  exit_if_joined_unexpected(thread_join_rc, thread_create_rc, testname);

  printf(1, "%s passed\n", testname);
}

void join_with_no_child_thread_test() {
  char testname[] = "join_with_no_child_thread_test";
  int thread_join_rc = thread_join();
  if (thread_join_rc != -1) {
    printf(1, "%s: thread_join() expected return code -1, got %d\n", thread_join_rc);
    exit();
  }

  printf(1, "%s passed\n", testname);
}

int counter_for_spinlock_test;
lock_t lock_for_spinlock_counter_test;

void
increment_global_sleepily(void *unused)
{
  int temp;

  lock_acquire(&lock_for_spinlock_counter_test);
  temp = counter_for_spinlock_test;
  sleep(1);
  temp += 1;
  counter_for_spinlock_test = temp;
  lock_release(&lock_for_spinlock_counter_test);

  exit();
}

void
spinlock_counter_test()
{
  char testname[] = "spinlock_counter_test";

  counter_for_spinlock_test = 0;
  lock_init(&lock_for_spinlock_counter_test);

  const int NUM_THREADS = 10;
  int thread_create_rc;
  int i;
  for (i = 0; i < NUM_THREADS; i++) {
    thread_create_rc = thread_create(increment_global_sleepily, 0);
    exit_if_thread_create_failed(thread_create_rc, testname);
  }

  int thread_join_rc;
  int j;
  for (j = 0; j < NUM_THREADS; j++) {
    thread_join_rc = thread_join();
    exit_if_thread_join_error(thread_join_rc, testname);
  }

  int expected_counter_val = NUM_THREADS;
  if (counter_for_spinlock_test != expected_counter_val) {
    printf(1, "%s: expected counter value %d, got %d\n",
           testname, expected_counter_val, counter_for_spinlock_test);
    exit();
  }

  printf(1, "%s passed\n", testname);
}

unsigned long randstate = 1;
unsigned int
rand()
{
  randstate = randstate * 1664525 + 1013904223;
  return randstate;
}

int
main(int argc, char *argv[])
{
  printf(1, "usertests starting\n");

  if(open("usertests.ran", 0) >= 0){
    printf(1, "already ran user tests -- rebuild fs.img\n");
    exit();
  }
  close(open("usertests.ran", O_CREATE));

  basic_thread_test();
  thread_shares_address_space_test();
  thread_starts_with_same_file_descriptors_as_parent();
  pass_argument_to_child_thread_test();
  each_thread_gets_its_own_stack_test();
  modify_address_space_across_threads_test();

  join_ignores_forked_children_test();
  wait_ignores_child_threads_test();
  join_with_no_child_thread_test();

  spinlock_counter_test();

  bad_ptr_to_syscall_test();
  deref_null_ptr_test();

  getprocstest();

  argptest();
  createdelete();
  linkunlink();
  concreate();
  fourfiles();
  sharedfd();

  bigargtest();
  bigwrite();
  bigargtest();
  bsstest();
  sbrktest();
  validatetest();

  opentest();
  writetest();
  writetest1();
  createtest();

  openiputtest();
  exitiputtest();
  iputtest();

  mem();
  pipe1();
  preempt();
  exitwait();

  rmdot();
  fourteen();
  bigfile();
  subdir();
  linktest();
  unlinkread();
  dirfile();
  iref();
  forktest();
  bigdir(); // slow

  uio();

  exectest();

  exit();
}
