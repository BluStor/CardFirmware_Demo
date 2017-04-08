#ifndef _FTPd
#define _FTPd

#define RBUF_SIZE    2048
typedef struct  ringBufS
{
  unsigned char buf[RBUF_SIZE];
  int head;
  int tail;
  int count;
} ringBufS;

extern ringBufS rBufs1;
extern ringBufS rBufs2;
extern SemaphoreHandle_t gCS;
extern osSemaphoreId FTP_RxDataReady;
extern osSemaphoreId FTP_RxFifoEmpty;

extern void PMICGetDeviceSettings(void);

extern void ringBufS_flush(ringBufS *rBufs, const int clearBuffer);
extern void InitializeCriticalSection(SemaphoreHandle_t *xSemaphore);

extern void FTPThread(void const *argument);

extern FRESULT f_move (const TCHAR* path_source, const TCHAR* path_dest);

#endif