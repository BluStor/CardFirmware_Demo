// ftp_server.cpp : Defines the entry point for the console application.
//

//#define _CRT_SECURE_NO_WARNINGS
//#include <windows.h>

#include "main.h"

#include <FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>
#include <string.h>
#include <task.h>

#include "paths.h"
#include "FTPd.h"

#include "stm32f4xx_hal.h"

#define HANDLE int
#define INVALID_HANDLE_VALUE -1
#define DWORD unsigned int
#define WINAPI
#define LPVOID void *

extern UART_HandleTypeDef huart3;

DWORD GetTickCount(void){
  TickType_t xTimeNow = xTaskGetTickCount();
  return xTimeNow;
}

SemaphoreHandle_t gCS = NULL;
osSemaphoreId FTP_RxDataReady = NULL;
osSemaphoreId FTP_RxFifoEmpty = NULL;

void InitializeCriticalSection(SemaphoreHandle_t *xSemaphore) {
  *xSemaphore = xSemaphoreCreateMutex();
}

void DeleteCriticalSection(SemaphoreHandle_t *xSemaphore) {
  vSemaphoreDelete(*xSemaphore);
}

void EnterCriticalSection(SemaphoreHandle_t *xSemaphore) {
  xSemaphoreTake( *xSemaphore,  portMAX_DELAY);
}

void LeaveCriticalSection(SemaphoreHandle_t *xSemaphore) {
  xSemaphoreGive( *xSemaphore );
}

extern int BT_WriteABuffer(const char * lpBuf, DWORD dwToWrite);
int (*pWriteABuffer)(const char * lpBuf, DWORD dwToWrite) = BT_WriteABuffer;

HANDLE ghComm = INVALID_HANDLE_VALUE;
HANDLE hReaderStatus;

BOOL SetupCommPort(char* szPort);

DWORD WINAPI ReaderAndStatusProc(LPVOID lpV);


ringBufS rBufs1;
ringBufS rBufs2;

void ringBufS_put(ringBufS *rBufs, const unsigned char c);
void ringBufS_put_multi(ringBufS *prBufs, char* buf, int iLen);
int ringBufS_get(ringBufS *rBufs);
int ringBufS_get_multi(ringBufS *prBufs, char* buf, int iLen);
void ringBufS_flush(ringBufS *prBufs, const int clearBuffer);
int my_recv(SOCKET so, char *buf, int len, int flags);
int my_recv2(SOCKET so, char *buf, int len, int flags);

int my_send(SOCKET so, const char *buf, int len, int flags){
	int iRet;
	int iCheck = 0, i;
        char * pbuf = (char *)buf;
	for(;;) {
		if (so == 2){
			pbuf[0] = 0x02;
		}
		else {
			pbuf[0] = 0x01;
		}
		i = len + 5;
		pbuf[1] = (char)(i >> 8);
		pbuf[2] = (char)(i & 0xff);
		pbuf[len + 3] = (char)(iCheck >> 8);
		pbuf[len + 4] = (char)(iCheck & 0xff);
		iRet = pWriteABuffer(buf, len + 5);
		if (iRet >= 0)
			iRet = len;
		break;
	}
	return iRet;
}


int my_recv(SOCKET so, char *buf, int len, int flags){
  DWORD dwTimeout, dwTo; // todo must be tune for baud other than 115200
  int c, iRet = 0, iLen = 0;
  int iStep = 0;
  if (so == 2) {
    dwTo = 500;
  }
  else {
    dwTo = 100;
  }
  dwTimeout = GetTickCount() + dwTo;
  do{
    if ((so == 1) && (FTPAbort)) 
    {
      FTPAborted = TRUE;
      Sleep(1);         // no delay here MDR
      continue;
    } else 
    {
      if (so == 2) 
      {
        if (GetTickCount() > dwTimeout) 
        {
          iRet = 0;
          break;
        }
      }
      if (so ==2)
      {
        c = ringBufS_get(&rBufs2); 
      }
      else
      {
        c = ringBufS_get(&rBufs1); 
      }
      if (c == -1) 
      {
        osSemaphoreWait(FTP_RxDataReady , 10); 
        continue;
      }
    }
    switch (iStep) {
      case 0:
        iLen = (int)c << 8; // len MSB
        //len--;
        iStep++;
        break;
      case 1:
        iLen += c&0xff;	// len LSB
        //len--;
        iStep++;
        if (iLen == 0)iStep++;
        //printf("iLen:%d\n", iLen);
        break;
      case 2:
        if (iLen) {
          //printf("%c", c);
          *buf++ = c;
          len--;
          iRet++;
          iLen--;
        }
        if (iLen == 0){
          iStep++;
        }
        break;
    }
  } while(len && (iStep < 3) && (FTPAbort == FALSE));
   
  osSemaphoreRelease(FTP_RxFifoEmpty);
  osThreadYield();
  return iRet;
}


int my_recv2(SOCKET so, char *buf, int len, int flags){
  
  DWORD dwTimeout, dwTo; // todo must be tune for baud other than 115200
  int c, iRet = 0, iLen = 0;
  int iStep = 0;
  
  // so = 1 = command channel
  // so = 2 = data channel
       
  dwTo = Settings.SPP_Receive_Timeout;
  
  dwTimeout = GetTickCount() + dwTo;
  
  do{
    
    // Check to see if FTP is being aborted
    if ((so == 1) && (FTPAbort)) 
    {
      FTPAborted = TRUE;
      Sleep(1);         // no delay here MDR
      continue;
    } else  {   
    
      // Check for data channel timeout, which can also be
      // an indication that there is no more data to be read
      if (so == 2) 
      {
        if (GetTickCount() > dwTimeout) 
        {
          iRet = 0;
          break;
        }
      }
      
      // Read data channel
      if (so ==2)
      {
        if(iStep == 2)
        {
          c = ringBufS_get_multi(&rBufs2, buf, iLen);
        }else
        {
          c = ringBufS_get(&rBufs2); 
        }
        
      }
      
      // Read command channel
      else
      {
        c = ringBufS_get(&rBufs1); 
      }
      
      // c = -1 = no data ready to be read
      if (c == -1) 
      {
        osSemaphoreWait(FTP_RxDataReady , 10);  
        continue;
      }
      
    }
    
    
    switch (iStep) {
      case 0:
        iLen = (int)c << 8; // len MSB
        iStep++;
        break;
        
      case 1:
        iLen += c&0xff;	// len LSB
        iStep++;    
        if (iLen == 0)iStep++; 
        break;
        
      case 2:
        if (iLen) {

          // c contains the number of characters read , the characters are already in the buffer.
          buf += c;
          len -= c;
          iRet += c;
          iLen -= c;
          
        }
        
        // Is there any more data to be read?
        if (iLen == 0){
          iStep++;
        }
        
        break;
    }
    
  } while(len && (iStep < 3) && (FTPAbort == FALSE));
  
  // Did we timeout before all data was read for this packet?
  if (iLen !=0) {
    //printf("[my_recv2] timed out reading data from channel: %d\r\n", so);
    iRet = -1;
  }
  
  osSemaphoreRelease(FTP_RxFifoEmpty);
  osThreadYield();
  return iRet;
}


int HandleASuccessfulRead(char *lpBuf, DWORD dwRead ){
	int c;
        static ringBufS *prBufs;
	static int iStep = 0;
	static int iLen = 0;
	int iCheck = 0, iRet = 0;
	char *s = lpBuf;
        
        //printf("h %d\r\n", dwRead);
        
        if(lpBuf == NULL)       // need to reset state machine?
        {
          iStep = 0;
          return iRet;
        }
        
	while(dwRead && (FTPAbort == FALSE)) 
        {
                c = *s; 
		switch(iStep) 
                {
		case 0:
			if (c == 0x02)
                        {
				prBufs = &rBufs2;
			}else 
                        {
				prBufs = &rBufs1;	
			}
                        iStep++;
                        dwRead--;
                        s++;
			break;
		case 1:
			iLen = (int)c << 8; // len MSB
			iStep++;
                        dwRead--;
                        s++;
			break;
		case 2:
			iLen += c&0xff;	// len LSB
			iLen -= 5;
			ringBufS_put(prBufs, iLen >> 8);
			ringBufS_put(prBufs, iLen & 0xff);
			iStep++;
                        dwRead--;
                        s++;
			break;
		case 3:
                        c=min(iLen,dwRead);
			ringBufS_put_multi(prBufs,s,c);
                        iLen -= c;
                        dwRead -= c;
                        s += c;
			if (iLen == 0) 
                        {
				iStep++;
			}
			break;
		case 4:
			iCheck = (int)c << 8; // checksum MSB
			iStep++;
                        dwRead--;
                        s++;
			break;
		case 5:
			iCheck += c&0xff; // checksum LSB
			iStep = 0;
			iLen = 0;
                        dwRead--;
                        s++;
			break;
		}     
	}
        osSemaphoreRelease(FTP_RxDataReady); 
        osThreadYield();                
	return iRet;
}

unsigned int modulo_inc(const unsigned int value, const unsigned int modulus)
{
	unsigned int my_value = value + 1;
	if (my_value >= modulus)
	{
		my_value  = 0;
	}
	return my_value;
}

int ringBufS_get(ringBufS *prBufs)
{
    int c;
    EnterCriticalSection(&gCS);
    
    if (prBufs->count>0)
    {
      c  = prBufs->buf[prBufs->tail];
      prBufs->tail = modulo_inc (prBufs->tail, RBUF_SIZE);
      --prBufs->count;
    }
    else
    {
      c = -1;
    }
    LeaveCriticalSection(&gCS);
    return (c);
}


int ringBufS_get_multi(ringBufS *prBufs, char* buf, int iLen)
{
  int c;
  int r=0;
  EnterCriticalSection(&gCS);
  if (prBufs->count>0)
  {
    c = min(iLen, prBufs->count);
    if((prBufs->tail + c) > RBUF_SIZE)
    { 
      r = RBUF_SIZE-prBufs->tail;
      assert_param(r>0 && r<RBUF_SIZE);
      memcpy(buf, &prBufs->buf[prBufs->tail],r);
      buf += r;
      prBufs->tail=0;
    }
    memcpy(buf, &prBufs->buf[prBufs->tail],c-r);
    prBufs->tail += c-r;
    if(prBufs->tail>=RBUF_SIZE)
    {
      prBufs->tail=0;
    }
    prBufs->count -= c;
    
  }else
  {
    c = -1;
  }
  
  LeaveCriticalSection(&gCS);
  return(c);
}


void ringBufS_put(ringBufS *prBufs, const unsigned char c)
{
    int iWait = 1;
    do {
	  EnterCriticalSection(&gCS);
          if (prBufs->count < RBUF_SIZE)
          {
            prBufs->buf[prBufs->head] = c;
            prBufs->head = modulo_inc (prBufs->head, RBUF_SIZE);
            ++prBufs->count;
            iWait = 0;
          }
	  LeaveCriticalSection(&gCS);
          if (iWait && (FTPAbort == FALSE)) {
            osSemaphoreWait(FTP_RxFifoEmpty , 1);      // no delay while transferring MDR
          }
    } while (iWait && (FTPAbort == FALSE));
}


void ringBufS_put_multi(ringBufS *prBufs, char* buf, int iLen)
{
    int iWait = 1;
    int PutNb;
    int c;

   
    do 
    {
	  EnterCriticalSection(&gCS);
          if (prBufs->count < RBUF_SIZE)
          {
            PutNb = min((RBUF_SIZE-prBufs->count),iLen);
            if((prBufs->head + PutNb) > RBUF_SIZE)
            {
              c=RBUF_SIZE - prBufs->head;
              memcpy(&prBufs->buf[prBufs->head],buf,c);
              PutNb -= c;
              iLen -= c;
              buf += c;
              prBufs->head=0;
              prBufs->count += c;
            }
            
            memcpy(&prBufs->buf[prBufs->head],buf,PutNb);
            iLen -= PutNb;
            buf += PutNb;
            prBufs->head += PutNb;
            if(prBufs->head >= RBUF_SIZE)
            {
              prBufs->head = 0;
            }
            prBufs->count += PutNb;
            
            if(iLen == 0)
            {
              iWait = 0;
            }
          }
	  LeaveCriticalSection(&gCS);
          if (iWait && (FTPAbort == FALSE)) 
          {
            //osDelay(1);
            osSemaphoreWait(FTP_RxFifoEmpty , 1);      // no delay while transferring MDR
          }
    } while (iWait && (FTPAbort == FALSE));
}

void ringBufS_flush(ringBufS *prBufs, const int clearBuffer)
{
	prBufs->count  = 0;
	prBufs->head   = 0;
	prBufs->tail   = 0;
	if (clearBuffer){
		memset(prBufs->buf, 0, sizeof (prBufs->buf));
	}
}

void FtpServerReset(void)
{
  HandleASuccessfulRead(NULL,0);
  ringBufS_flush(&rBufs1, TRUE);
  ringBufS_flush(&rBufs2, TRUE);
  printf("FTP Server Reset\r\n");
}