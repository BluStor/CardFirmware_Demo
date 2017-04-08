/**
  ******************************************************************************
  * @file    Src/slog.h
  * @author  Mark W. Bennett
  * @version V1.0.0
  * @date    07-August-2016
  * @brief   CyberGate log file manager
  ******************************************************************************
**/

#define LOG_FILENAME            "log"           // log file name
#define LOG_PATH                "/device"       // path where log files are written
#define LOG_MAXSIZE             1024 * 1024     // bytes

#define LOG_MAXROTATION         5               // maximum number of log files
#define LOG_MAXLINESIZE         256             // max size of individual line
#define LOG_MAXPATH             64              // max size of fill log file path
#define LOG_MAXBUFFER           1024 * 1        // max size of buffer to use when eMMC is not available

#define LOG_DEST_FILE          0
#define LOG_DEST_CONSOLE       1
#define LOG_DEST_BOTH          2

#define CONSOLE_SUPPORT        1

#define Display(_x)                                do { BTPS_OutputMessage _x; } while(0)

void slog_init(void);
void slogf(int logDest, const char* format, ...);
