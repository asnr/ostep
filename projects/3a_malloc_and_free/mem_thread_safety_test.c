#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include "mem.h"

const size_t NUM_ALLOCS_PER_THREAD = 100;
const size_t MEM_ALLOC_SIZE = 16;

struct thread_args {
  char fill_value;
  char **buffers;
};

void
fill_buffer_with_value(char *buffer, size_t size, char value)
{
  char *buffer_end = buffer + size;
  for (char *curr_char = buffer; curr_char < buffer_end; curr_char++) {
    *curr_char = value;
  }
}

void*
allocate_and_fill_memory(void *thread_args)
{
  struct thread_args *args = (struct thread_args *) thread_args;
  char *buffer;
  for (size_t buffer_idx = 0; buffer_idx < NUM_ALLOCS_PER_THREAD; buffer_idx++) {
    // TODO: error handling
    buffer = (char *) Mem_Alloc(MEM_ALLOC_SIZE);
    (args->buffers)[buffer_idx] = buffer;
    fill_buffer_with_value(buffer, MEM_ALLOC_SIZE, args->fill_value);
  }

  return NULL;
}

void check_buffers_for_corruption(struct thread_args *thread_args);

int
main ()
{
  printf(">> Thread Safety Test >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

  int upper_bound_on_required_heap_memory =
    2 * NUM_ALLOCS_PER_THREAD * (MEM_ALLOC_SIZE + 32) + 128;
  Mem_Init(upper_bound_on_required_heap_memory);

  pthread_t thread_1, thread_2;
  struct thread_args thread_args_1, thread_args_2;

  thread_args_1.fill_value = 'a';
  thread_args_2.fill_value = 'Z';

  int buffer_array_size = NUM_ALLOCS_PER_THREAD * sizeof(char *);
  thread_args_1.buffers = (char **) Mem_Alloc(buffer_array_size);
  thread_args_2.buffers = (char **) Mem_Alloc(buffer_array_size);
  assert(thread_args_1.buffers != NULL && thread_args_2.buffers != NULL);

  int create_1_rc =
    pthread_create(&thread_1, NULL, allocate_and_fill_memory, &thread_args_1);
  int create_2_rc =
    pthread_create(&thread_2, NULL, allocate_and_fill_memory, &thread_args_2);
  assert(create_1_rc == 0 && create_2_rc == 0);

  int join_1_rc = pthread_join(thread_1, NULL);
  int join_2_rc = pthread_join(thread_2, NULL);
  assert(join_1_rc == 0 && join_2_rc == 0);

  check_buffers_for_corruption(&thread_args_1);
  check_buffers_for_corruption(&thread_args_2);

  printf("Thread safety test passed!\n");
  return 0;
}

void
check_buffer_filled_with_value(char *buffer, size_t size, char value)
{
  char *buffer_end = buffer + size;
  for (char *curr_char = buffer; curr_char < buffer_end; curr_char++) {
    if (*curr_char != value) {
      printf("Expected to see value %c, but got %c\n", value, *curr_char);
    }
    assert(*curr_char == value);
  }
}

void
check_buffers_for_corruption(struct thread_args *thread_args)
{
  char **buffer = thread_args->buffers;
  char **buffers_end = buffer + NUM_ALLOCS_PER_THREAD;
  for (; buffer < buffers_end; buffer++) {
    check_buffer_filled_with_value(*buffer, MEM_ALLOC_SIZE, thread_args->fill_value);
  }
}
