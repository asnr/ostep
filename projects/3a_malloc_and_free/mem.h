#ifndef __mem_h__
#define __mem_h__

#define E_NO_SPACE            (1)
#define E_CORRUPT_FREESPACE   (2)
#define E_PADDING_OVERWRITTEN (3)
#define E_BAD_ARGS            (4)
#define E_BAD_POINTER         (5)

extern int m_error;

int Mem_Init(int sizeOfRegion, int debug);
void *Mem_Alloc(int size);
int Mem_Free(void *ptr);
void Mem_Dump();


#endif // __mem_h__


