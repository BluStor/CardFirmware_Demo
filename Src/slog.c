/**
  ******************************************************************************
  * @file    Src/slog.c
  * @author  Mark W. Bennett
  * @version V1.0.0
  * @date    07-August-2016
  * @brief   CyberGate log file manager
  ******************************************************************************
**/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "ff.h" 
#include "slog.h"
#include "pmic.h"
#include "stm32f4xx_hal.h"

static osSemaphoreId SLOG_Semaphore = NULL;
static char buffer[LOG_MAXBUFFER];
static int bufferLength = 0;

// External references required to get the system data/time
extern void get_date(RTC_DateTypeDef * sDate, RTC_TimeTypeDef * sTime);

void slog_init() {
  
  if (SLOG_Semaphore != NULL) return;
  
  osSemaphoreDef(SLOG_Semaphore);
  SLOG_Semaphore = osSemaphoreCreate(osSemaphore(SLOG_Semaphore), 1);
  
  memset(&buffer, 0, sizeof(buffer));
  
  slogf(LOG_DEST_BOTH, "[slog_init] logging system initialized.");
  
}

void slog_writefile(char *dtPrefix, char *msg) {
  
  FRESULT res;
  FILINFO fno;
  char filename[LOG_MAXPATH];
  char newfilename[LOG_MAXPATH];
  char output[LOG_MAXLINESIZE];
  FIL fp;
  unsigned int written; 
  
  memset(&output, 0, sizeof(output));
  snprintf(output, sizeof(output), "%s%s\r\n", dtPrefix, msg); 
  
  // Make sure eMMC is ready for read/write opeations
  if ((eMMC_Ready)  && (eMMC_Powered)) {
  
    // Build filename with fullpath and extension
    snprintf(filename, sizeof(filename), "%s/%s.0", LOG_PATH, LOG_FILENAME);
    
    // Open log file
    res = f_open(&fp, filename,  FA_WRITE | FA_OPEN_ALWAYS);
    if(res != FR_OK){
      Display(("[slog_writefile] ERROR - unable to open logfile: %s", filename));
      return;
    }
    
    // Move to end of the file to append data
    res = f_lseek(&fp, f_size(&fp));
     if(res != FR_OK){
      Display(("[slog_writefile] ERROR - unable to append logfile: %s", filename));
      return;
    }
    
    // Do we have any buffered data to process first?
    if (bufferLength > 0) {
      
      res = f_write(&fp, buffer, strlen(buffer), &written);
      if ((res != FR_OK) || (written != strlen(output)))  {
         Display(("[slog_writefile] ERROR - writing buffered data to logfile: %s", filename));   
      }
      
      // clear the buffer
      memset(&buffer, 0, sizeof(buffer));
      bufferLength = 0;
    }
    
    res = f_write(&fp, output, strlen(output), &written);
    if ((res != FR_OK) || (written != strlen(output)))  {
      Display(("[slog_writefile] ERROR - writing to logfile: %s", filename));   
    }
    
    
    // Check log rotation
    if (f_size(&fp) < LOG_MAXSIZE) {
        f_close(&fp);
        return;
    }
    
    f_close(&fp);
                
    //slog_rotate();
     
    // Delete oldest possible file if it exists
    memset(&fno, 0, sizeof(fno));
    memset(&filename, 0, sizeof(filename));
    snprintf(filename, sizeof(filename), "%s/%s.%d", LOG_PATH, LOG_FILENAME, LOG_MAXROTATION - 1);
    if (!f_stat(filename, &fno)) f_unlink(filename);  
        
    // Loop through all possible remaining log files
    for (int ext = LOG_MAXROTATION - 2; ext >=0; ext--) {  
      
      // Zero everything out between each file rename
      memset(&fno, 0, sizeof(fno));  
      memset(&filename, 0, sizeof(filename));
      memset(&newfilename, 0, sizeof(newfilename));
      
      // Build filename with fullpath and extension
      snprintf(filename, sizeof(filename), "%s/%s.%d", LOG_PATH, LOG_FILENAME, ext);
      snprintf(newfilename, sizeof(newfilename), "%s/%s.%d", LOG_PATH, LOG_FILENAME, ext + 1);
      
      // Rename file if it exists
      if (!f_stat(filename, &fno)) {
        res = f_rename(filename, newfilename);
        if ((res != FR_OK) && (res != FR_NO_FILE)) {
          Display(("[slog_rotate] ERROR - unable to rotate logfile: %s", filename)); 
          break;
        }  
      }         
    }
    
  // eMMC not available, so buffer the data if there is room
  } else {
    
    if (bufferLength + strlen(output) <= LOG_MAXBUFFER) {   
        strcat(buffer, output);
        bufferLength = bufferLength + strlen(output);
    } else {
        Display(("[slog_rotate] ERROR - offline log buffer full\r\n"));
    }
    
  }
}


void slogf(int logDest, const char* format, ...)
{
  
  static char msg[LOG_MAXLINESIZE];
  char tmp[LOG_MAXLINESIZE];
  va_list args;  
  RTC_DateTypeDef sDate;
  RTC_TimeTypeDef sTime;
  char dtPrefix[20];
  int32_t sem;
   
  sem = osSemaphoreWait(SLOG_Semaphore , 1);
  if (sem != osOK) {
    return;
  }
  
  // If we don't use this approach of building the
  // string in tmp and then copying it to msg, the
  // va_ macros modify the passed arguments for 
  // some reason!
  memset(msg, 0, sizeof(msg));
  memset(tmp, 0, sizeof(tmp));

  va_start(args, format);
  vsnprintf(tmp, sizeof(tmp), format, args); 
  va_end(args);
  
  strcpy(msg, tmp);
  
  
#ifdef CONSOLE_SUPPORT
   if ((logDest == LOG_DEST_CONSOLE) || (logDest == LOG_DEST_BOTH)) {
     
     // Display output without timestamp when logging to console
     if (BTActivity) {
       Display(("%s\r\n", msg)); // Use BTSP driver once available
     } else {
       printf("%s\r\n", msg);    // Otherwise use standard i/o
     }
     
   }
#endif

   if ((logDest == LOG_DEST_FILE) || (logDest == LOG_DEST_BOTH)) {
     
     // Build date/time prefix string
     get_date(&sDate, &sTime);
     int subSeconds = (int)(((sTime.SecondFraction-sTime.SubSeconds) / (sTime.SecondFraction+1.0)) * 1000.0);
     sprintf(dtPrefix, "%02d.%02d.%02d-%02d:%02d:%02d.%04d - ",
             sDate.Year, sDate.Month, sDate.Date,
             sTime.Hours, sTime.Minutes, sTime.Seconds, subSeconds);
        
     slog_writefile(dtPrefix, msg); 
     
   }
   
  osSemaphoreRelease(SLOG_Semaphore);
    
}

                       
                       

