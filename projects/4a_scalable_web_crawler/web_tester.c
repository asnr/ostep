#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "crawler.h"
#include "cs537.h"
#include "cs537.h"

void *Malloc(size_t size) {
  void *r = malloc(size);
  assert(r);
  return r;
}

char *Strdup(const char *s) {
  void *r = strdup(s);
  assert(r);
  return r;
}

/*
 * Send an HTTP request for the specified file 
 */
void clientSend(int fd, char *filename)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "GET %s HTTP/1.1\n", filename);
  sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
  Rio_writen(fd, buf, strlen(buf));
}

char *grab_page(int fd)
{
  rio_t rio;
  char buf[MAXBUF];  
  int length = 0;
  int n;
  
  Rio_readinitb(&rio, fd);

  /* Read and display the HTTP Header */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (strcmp(buf, "\r\n") && (n > 0)) {
    n = Rio_readlineb(&rio, buf, MAXBUF);

    /* If you want to look for certain HTTP tags... */
    if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
      // pass
    }
  }

  /* Read and display the HTTP Body */
  int page_len = 1;
  char *page = Malloc(page_len);
  int pos = 0;
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    if (&page[pos]+n+1 > page+page_len) {
      page_len += n;
      page = realloc(page, page_len);
      assert(page);
    }
    pos += sprintf(&page[pos], "%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }

  return page;
}

char *fetch(char *link) {
  char url[256];
  snprintf(url, 256, "/~harter/537/p4/%s", link);
  int clientfd = Open_clientfd("pages.cs.wisc.edu", 80);
  ///~harter/537/  clientSend(clientfd, "/~harter/537/slides.html");
  clientSend(clientfd, url);
  char *page = grab_page(clientfd);
  Close(clientfd);
  return page;
}

void edge(char *from, char *to) {
  printf("%s -> %s\n", from, to);
}

int main(int argc, char *argv[]) {
  assert(argc == 2);
  int rc = crawl(argv[1], 1, 1, 1, fetch, edge);
  assert(rc == 0);
  return 0;
}
