//------------------------------------------------------------------------------------
// Minimal FTP server program.  Intended for windows to windows file transfers
//
// This program is totally free.  You may re-liscesne it under GPL or BSD if you wish.
//
// Matthias Wandel June 2004
//
// http://www.sentex.net/~mwandel/tech/ftpdmin.html
//------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include "ff.h" 
#include "FTPd.h"
#include "main.h"
#include "..\pmic.h"
#include "slog.h"
#include "..\SPPTask.h"
#include "paths.h"

#define FTPDMIN_VER "1.0"
#define LINK_KEY_HEX_LEN 49
#define ADDR_HEX_LEN 17
#define OVERHEAD_LEN 24
#define LINKKEY_STR OVERHEAD_LEN+ADDR_HEX_LEN+LINK_KEY_HEX_LEN

//in bytes
//static long long int AVAILABLE_SPACE =   1048576L;  //  1Mb
//static long long int AVAILABLE_SPACE = 4194304L;    //  4 Mb
//static long long int AVAILABLE_SPACE = 1073741824L; //  1 Gb
//static long long int AVAILABLE_SPACE = 2147483648L; //  2 Gb
//static long long int AVAILABLE_SPACE = 3221225472L; //  3 Gb
//static long long int AVAILABLE_SPACE = 4294967296L; //  4 Gb
//static long long int AVAILABLE_SPACE = 5368709120L; //  5 Gb
//static long long int AVAILABLE_SPACE = 6442450944L; //  6 Gb
static long long int AVAILABLE_SPACE = 7837614080L; // ~8Gb


void Sleep(int mSec){
  osDelay(mSec);
}

#ifdef BOOT_48K
static int BootUpdate(void);
#endif // BOOT_48K

struct in_addr OurAddr;
char OurAddrStr[20];
int addrlen = sizeof(struct sockaddr_in);
BOOL GetOnly = FALSE;
char PortsUsed[256];

// Address of the data port used to transfer files
// Making this global means we can only handle ONE connection.
typedef struct {
    struct sockaddr_in xfer_addr;
    BOOL PassiveMode;
    int PassiveSocket;
    char XferBuffer[528];
    int CommandSocket;
    int XferPort;
}Inst_t;

extern int my_send(SOCKET s, const char *buf, int len, int flags);
extern int my_recv(SOCKET s, char *buf, int len, int flags);
extern int my_recv2(SOCKET so, char *buf, int len, int flags);

extern void AdvertiseLockStatus(int locked);

// FTP Command tokens
typedef enum {
    USER, PASS, CWD,  PORT, 
    QUIT, PASV, ABOR, DELE,
    RMD,  XRMD, MKD,  XMKD,
    PWD,  XPWD, LIST, NLST, 
    SYST, TYPE, MODE, RETR, 
    STOR, REST, RNFR, RNTO,
    STAT, NOOP, MDTM, xSIZE,
    SRFT, MLST,
    UNKNOWN_COMMAND
}CmdTypes;

struct command_list {
    char *command;
    CmdTypes CmdNum;
};

//------------------------------------------------------------------------------------
// Table of FTP commands
//------------------------------------------------------------------------------------
typedef struct {
    char *command;
    CmdTypes CmdNum;
}Lookup_t;

static const Lookup_t CommandLookup[] = {
    "USER", USER, "PASS", PASS, "CWD",  CWD,  "PORT", PORT, 
    "QUIT", QUIT, "PASV", PASV, "ABOR", ABOR, "DELE", DELE,
    "RMD",  RMD,  "XRMD", XRMD, "MKD",  MKD,  "XMKD", XMKD,
    "PWD",  PWD,  "XPWD", XPWD, "LIST", LIST, "NLST", NLST,  
    "SYST", SYST, "TYPE", TYPE, "MODE", MODE, "RETR", RETR,
    "STOR", STOR, "REST", REST, "RNFR", RNFR, "RNTO", RNTO,
    "STAT", STAT, "NOOP", NOOP, "MDTM", MDTM, "SIZE", xSIZE,
    "SRFT", SRFT, "MLST", MLST,
};

#if 0
void ValidRxData(char* pRxData, int Size)
{
  static uint8_t Data = 'A';
  static int index=0;
  static int loop=0;
  
  if(pRxData == NULL)
  {
    Data = 'A';
    index = 0;
    loop = 0;
    return;
  }
  
  while(Size)
  {
    if(index == 1023)
    {
      if(*pRxData != 0x0A)
        goto bad_data;
      Data++;
      if(Data > 'P')
        Data = 'A';
      index = 0;
      loop++;
    }else if(index==1022)
    {
      if(*pRxData != 0x0D)
        goto bad_data;
      index++;
    }else
    {
      if(*pRxData != Data)
        goto bad_data;
      index++;
    }
    pRxData++;
    Size--;
  }
  return;
bad_data:
  printf ("Unexpected Data. %d != %d\r\n",*pRxData, Data);
  while(1);
}
#endif

int BT_Key_Parser(char *buff, BD_ADDR_t *key){
  unsigned char add[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  if (strncmp(buff, BT_KEY_FILENAME, (sizeof(BT_KEY_FILENAME) -1)) == 0){
    sscanf(buff + sizeof(BT_KEY_FILENAME), " %hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
           &add[0], &add[1], &add[2], &add[3], &add[4], &add[5]);
    ASSIGN_BD_ADDR(*key, add[0], add[1], add[2], add[3], add[4], add[5]);
    return 1;
  }else{
    return 0;
  }
}

void Assign_Link_Key(Link_Key_t *key, unsigned char *list){
  key->Link_Key15 = (list[0]);
  key->Link_Key14 = (list[1]);
  key->Link_Key13 = (list[2]);
  key->Link_Key12 = (list[3]);
  key->Link_Key11 = (list[4]);
  key->Link_Key10 = (list[5]);
  key->Link_Key9 = (list[6]);
  key->Link_Key8 = (list[7]);
  key->Link_Key7 = (list[8]);
  key->Link_Key6 = (list[9]);
  key->Link_Key5 = (list[10]);
  key->Link_Key4 = (list[11]);
  key->Link_Key3 = (list[12]);
  key->Link_Key2 = (list[13]);
  key->Link_Key1 = (list[14]);
  key->Link_Key0 = (list[15]);
}

int Link_Key_Parser(char *buff, Link_Key_t *key){
  unsigned char add[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  if (strncmp(buff, BT_KEY_FILENAME, (sizeof(BT_KEY_FILENAME) -1)) == 0){
    sscanf(buff + sizeof(BT_KEY_FILENAME) + sizeof("_FF:ff:FF:ff:ff:ff"),
           " %hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
           &add[0], &add[1], &add[2], &add[3], &add[4], &add[5], &add[6], &add[7], 
           &add[8], &add[9], &add[10], &add[11], &add[12], &add[13], &add[14], &add[15]);
    Assign_Link_Key(key, add);
    return 1;
  }else{
    return 0;
  }
}

int DeleteAllKeys(void)
{
  FRESULT res;
  BD_ADDR_t BD_ADDR;

  res = f_unlink(BT_KEY_FILENAME);
  /* Delete all Stored Link Keys.                             */
  ASSIGN_BD_ADDR(BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  DeleteLinkKey(BD_ADDR);
  
  return (res);
}

//char *Buffer[17]
void BD_ADDRToStr(BD_ADDR_t addr, char *buffer){
  sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", addr.BD_ADDR5, addr.BD_ADDR4, 
           addr.BD_ADDR3, addr.BD_ADDR2, addr.BD_ADDR1, addr.BD_ADDR0);
}

//char *Buffer[35]
void BD_LinkKeyToStr(Link_Key_t key, char *buffer){
  sprintf(buffer, 
          "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", 
          key.Link_Key15, key.Link_Key14, key.Link_Key13, key.Link_Key12,
          key.Link_Key11, key.Link_Key10, key.Link_Key9, key.Link_Key8,
          key.Link_Key7, key.Link_Key6, key.Link_Key5, key.Link_Key4,
          key.Link_Key3, key.Link_Key2, key.Link_Key1, key.Link_Key0);
}

void LinkKeyInfoToStr(LinkKeyInfo_t btkey, char *buffer){
  sprintf(buffer, "BT address: ");
  BD_ADDRToStr(btkey.BD_ADDR, buffer + 12);
  sprintf(buffer + 12 + ADDR_HEX_LEN, "; Link Key: ");
  BD_LinkKeyToStr(btkey.LinkKey, buffer + ADDR_HEX_LEN + OVERHEAD_LEN);
}

//------------------------------------------------------------------------------------
// Get a ftp command from the command stream, convert to code and an argument string
//------------------------------------------------------------------------------------
static CmdTypes GetCommand(Inst_t * Conn, char *CmdArg) 
{
    char InputString[500+1];
    int  CmdLen;
    char Command[6];
    int  a,b;

    // Read in command string
    CmdLen = 0;
    memset(InputString, 0, sizeof(InputString));
    for (;;){
        int n;
        n = my_recv(Conn->CommandSocket, InputString+CmdLen, sizeof(InputString)-CmdLen,0);
        CmdLen += n;
		if (strstr(InputString, "\r")){
			break;
		}
		if (n <= 0){
			//printf("Conn->CommandSocket:%d\r\n",Conn->CommandSocket);
			return UNKNOWN_COMMAND;
		}
    }

    memset(Command, 0, sizeof(Command));
    for(a=0;a<5;a++) {
		if (!isalpha(InputString[a])){ 
			break;
		}
        Command[a] = toupper(InputString[a]);
    }

    b = 0;
    if (InputString[a++] == ' '){
        for (b=0;b<500-1;b++){
			if (InputString[a+b] < 32){
				break;
			}
            CmdArg[b] = InputString[a+b];
        }
    }
    CmdArg[b] = 0;

    //printf("%s %s\r\n", Command, CmdArg);
    slogf(LOG_DEST_CONSOLE, "");

    // Search through the list of known commands
    for(a=0;a<sizeof(CommandLookup)/sizeof(Lookup_t);a++){
        if(strncmp(Command, CommandLookup[a].command,4)==0){
            return CommandLookup[a].CmdNum;
        }
    }
    // MDR no stderr in system!    fprintf(stderr,"Unknown command '%s'\n",Command);
    return UNKNOWN_COMMAND;
}

//------------------------------------------------------------------------------------
// Wrapper function for more convenient sending of replies.
//------------------------------------------------------------------------------------
static void SendReply(Inst_t * Conn, char *Reply) 
{
    char ReplyStr[MAX_PATH+20];
    slogf(LOG_DEST_BOTH, "[sendReply] %s", Reply);
    sprintf(ReplyStr + 3, "%s\r\n", Reply);
    my_send(Conn->CommandSocket, ReplyStr, strlen(Reply) + 2,0);
}

//------------------------------------------------------------------------------------
// Report general file or I/O error.
//------------------------------------------------------------------------------------
static void Send550Error(Inst_t * Conn)
{
    char ErrString[20];
    slogf(LOG_DEST_BOTH, "[Send550Error] %d", errno);
    sprintf(ErrString, "550 %d",errno);
    SendReply(Conn, ErrString);
}

//------------------------------------------------------------------------------------
// Handle the NLST command (directory)
//------------------------------------------------------------------------------------
static void Cmd_NLST(Inst_t * Conn, char *filename, BOOL Long, BOOL UseCtrlConn)
{
    int xfer_sock;
    char repbuf[500];
    char buffer[LINKKEY_STR];
    BOOL ListAll = FALSE;
    FRESULT res;
    
    slogf(LOG_DEST_BOTH, "[Cmd_NLST] %s", filename);

    if (UseCtrlConn){
        xfer_sock = Conn->CommandSocket;
    }else{
        xfer_sock = 2;
		SendReply(Conn, "150 Opening connection");
		Sleep(200);
    }


    if (strncmp(filename, BT_KEY_FILENAME, (sizeof(BT_KEY_FILENAME) -1)) == 0){
      int keys_len;
      LinkKeyInfo_t *keys = ReturnAllLinkedKey(&keys_len);
      for(int i=0; i<keys_len; i++){
        LinkKeyInfoToStr(keys[i], buffer);
        SendReply(Conn, buffer);
        Sleep(50);
      }
      res = FR_OK;
      free(keys);
    }else{
        FILINFO fno;
        DIR  dp;
        static char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
        char path[MAX_PATH];
        char * tmp_path;
        
        tmp_path = CurrentPhysPath(filename);
        if (!tmp_path) {
          // Substitution not found, use current directory
          res = f_getcwd (path, MAX_PATH);
          tmp_path = path;
        }

        memset(&fno, 0, sizeof(fno));
        fno.lfname = lfn;
        fno.lfsize = sizeof(lfn);

        res = f_opendir(&dp, tmp_path);
        if (res == FR_OK) {
          for(;;) {
            char timestr[20];
            char * strname;
            res = f_readdir(&dp, &fno);
            if ((res != FR_OK) || (fno.fname[0] == 0)) {
              break;
            }
            if (fno.lfname[0] != 0) {
              strname = fno.lfname;
            } else {
              strname = fno.fname;
            }
            if (!strcmp(strname, ".")) continue;
            if (!strcmp(strname, "..")) continue;
            

            if (Long){
                struct tm tm;
                char DirAttr;
                char WriteAttr;

                tm.tm_year = (fno.fdate >> 9) + 80;
                tm.tm_mon = ((fno.fdate >> 5) & 0x0f) - 1;
                tm.tm_mday = fno.fdate  & 0x1f;
                tm.tm_hour = fno.ftime >> 11;
                tm.tm_min = (fno.ftime >> 5) & 0x3f;
                tm.tm_sec = (fno.ftime & 0x1f) << 1;
                strftime(timestr, 20, "%b %d  %Y", &tm);
                
                if (fno.fattrib & (AM_HID | AM_SYS)){
                    if (!ListAll) continue;
                }

                DirAttr = fno.fattrib & AM_DIR ? 'd' : '-';
                WriteAttr = fno.fattrib & AM_RDO ? '-' : 'w';

                sprintf(repbuf + 3,"%cr%c-r%c-r%c-   1 root  root    %7u %s %s\r\n", 
                        DirAttr, WriteAttr, WriteAttr, WriteAttr,
                        fno.fsize,
                        timestr,
                        strname);
            }else{
                sprintf(repbuf + 3, "%s\r\n",strname);
            }
            my_send(xfer_sock, repbuf, strlen(repbuf + 3),0);
          }
          f_closedir(&dp);
      }
    }

    if (!UseCtrlConn){
      if (res == FR_OK) {  
        SendReply(Conn, "226 Transfer Complete");
      } else {
        Send550Error(Conn);
      }
    }
}

//------------------------------------------------------------------------------------
// Handle the RECV command
//------------------------------------------------------------------------------------
static void Cmd_RETR(Inst_t * Conn, char *filename) 
{
    FIL fp;
    FRESULT res;
    BD_ADDR_t key;
    LinkKeyInfo_t linkKeyInfo;
    char buffer[LINKKEY_STR];
    int xfer_sock = 2;
    int size, nbytes = 0;
    
    slogf(LOG_DEST_BOTH, "[Cmd_RETR] %s", filename);
    
    // special treatment for Get Firmware Information only...
    if (strcmp(filename, FTP_DEVICE_FIRMWARE) == 0) {   
      char repbuf[MAX_PATH+10];
      SendReply(Conn, "150 Opening BINARY mode data connection");
      sprintf(repbuf + 3, "BOOT: %d.%d\r\n",
          *pBootloaderMajeur, *pBootloaderMineur);
      my_send(xfer_sock, repbuf, strlen(repbuf + 3),0);
      sprintf(repbuf + 3, "FIRM: %d.%d.%d %s\r\n",
          VERSION_FIRMWARE_MAJEUR, VERSION_FIRMWARE_MINEUR, VERSION_FIRMWARE_PATCH, FIRMWARE_PRODUCT);
      my_send(xfer_sock, repbuf, strlen(repbuf + 3),0);
      SendReply(Conn, "226 Transfer complete.");
      return;
    }
    
    if (BT_Key_Parser(filename, &key)){
      if (GetLinkedKey(key, &linkKeyInfo)){
        SendReply(Conn, "226 Link Key Info: ");
        LinkKeyInfoToStr(linkKeyInfo, buffer);
        SendReply(Conn, buffer);
      }else{
        SendReply(Conn, "550 Key Not Found\r\n");
      }
      return;
    }
    
    // special treatment for battery level 
    if (strcmp(filename, FTP_DEVICE_BATTERY) == 0) {
      char repbuf[MAX_PATH+10];
      SendReply(Conn, "150 Opening BINARY mode data connection");      
      sprintf(repbuf + 3, "BATTERY: %d\r\n", ADC_Bat_GetPercent());  
      my_send(xfer_sock, repbuf, strlen(repbuf + 3),0);
      SendReply(Conn, "226 Transfer complete.");
      return;
    }
    
    // Check to see if the file can be opened for reading
    if (f_open(&fp, filename, FA_READ | FA_OPEN_EXISTING)) {
        Send550Error(Conn);
        return;
    }
    // File opened succesfully, so make the connection
    SendReply(Conn, "150 Opening BINARY mode data connection");
	Sleep(200);

    // Transfer file
    //ValidRxData(NULL,0);    
    for(size=1;size > 0;){
        FTPActivity++;
        res = f_read (&fp, Conn->XferBuffer + 4, 512, (UINT *)&size);

        //ValidRxData(Conn->XferBuffer + 4, size);
        nbytes += size;
        //printf("size:%d\r", nbytes);
        if (FTPAbort) {
          size = 0;
        }
        
        if ((res != FR_OK) || (size == 0)) {
            break;
        }

        // Write buffer to socket.
        if(my_send(xfer_sock, Conn->XferBuffer + 1, size, 0) < 0){
            //perror("send failed");    MDR this function generate an HardFault exeption.
            SendReply(Conn, "426 Broken pipe") ;
            size = -1;
            res = FR_TIMEOUT;
        }
    }
    
    printf("\n");

    if (FTPAbort) {
      SendReply(Conn, "226 Abort");
      FTPAborted = TRUE;
    } else if (res != FR_OK) {
      Send550Error(Conn);
    }else{
      SendReply(Conn, "226 Transfer Complete");
    }

    f_close(&fp);
}

//------------------------------------------------------------------------------------
// Handle the STOR command
//------------------------------------------------------------------------------------

#define eMMC_WRITE_BUF_SIZE     (8*1024)
char eMMC_WriteBuf[eMMC_WRITE_BUF_SIZE];
uint16_t eMMC_Len;

static void Cmd_STOR(Inst_t * Conn, char *filename, DWORD used_b)
{
    FIL fp;
    FRESULT res;
    LinkKeyInfo_t linkKeyInfo;
    BD_ADDR_t key;
    int xfer_sock = 2;
    int size, nbytes = 0;
    long int current_usage = used_b;
    unsigned int written; 
    char buffer[LINKKEY_STR];
    bool NoMoreSpace = FALSE;
    bool ERROR_STOR = FALSE;
    
    eMMC_Len = 0;
    slogf(LOG_DEST_BOTH, "[Cmd_STOR] %s", filename);
    if (BT_Key_Parser(filename, &key)){
      linkKeyInfo.BD_ADDR = key;
      Link_Key_Parser(filename, &(linkKeyInfo.LinkKey));
      if(AddLinkedKey(&linkKeyInfo)==FR_OK){
        LinkKeyInfoToStr(linkKeyInfo, buffer);
        SendReply(Conn, "226 Link Key Info: \r\n");
        SendReply(Conn, buffer);
      }else{
        SendReply(Conn, "550 Link Key Not Stored\r\n");
      }
      return;
    }

    
    // Check if path exists
    res = CheckPath(filename);
    if (res != FR_OK){
      Send550Error(Conn);
      return;
    }

    // Check to see if the file can be opened for writing
    res = f_open(&fp, TEMP_FILE, FA_WRITE | FA_CREATE_ALWAYS);
    if(res != FR_OK){
        Send550Error(Conn);
        return;
    }

    // File opened succesfully, so make the connection
    SendReply(Conn, "150 Opening BINARY mode data connection");
	Sleep(200);

    // Transfer file
    //ValidRxData(NULL,0);    
        
    for(size=1;size >= 0;)
    {
        FTPActivity++;
        // Get from socket.
        do
        {
          size = my_recv2(xfer_sock, &eMMC_WriteBuf[eMMC_Len], eMMC_WRITE_BUF_SIZE-eMMC_Len, 0);
          nbytes += size;
          if(size < 0){
            //perror("read failed");    MDR this function generate an HardFault exeption.
          }else{
            eMMC_Len += size;
            if (eMMC_Len > (AVAILABLE_SPACE - current_usage)){
              NoMoreSpace = TRUE;
              size = -1;
              eMMC_Len = 0;
            }else{
              current_usage += size;
            }
          }
          
          if (FTPAbort) {
            size = -1;
          }

          if (size <= 0) 
            break;

        }while(eMMC_Len < eMMC_WRITE_BUF_SIZE);

        if (eMMC_Len)
        {
          //ValidRxData(eMMC_WriteBuf,eMMC_Len);       
          res = f_write(&fp, eMMC_WriteBuf, eMMC_Len, &written);
          if ((res != FR_OK) || (written != eMMC_Len)) 
          {
              size = -1;
              break;
          }
          if (size <= 0)
            break;
          
          eMMC_Len=0;
        }
        else 
        {
          break;
        }
    }

    if(size < 0){
      if (FTPAbort){
        SendReply(Conn, "226 Abort");
        FTPAborted = TRUE;
        ERROR_STOR = TRUE;
      }else if (NoMoreSpace){
        SendReply(Conn, "552 Disk full, temp file will be deleted");
        ERROR_STOR = TRUE;
      }else if (SPPOpened){
        Send550Error(Conn);
        ERROR_STOR = TRUE;
      }
      
      // Flush receive data to give a chance to unblock FTP client
      //for (nbytes=0;nbytes<32;nbytes++) 
      for (size=1;size >= 0;){
        size = my_recv2(xfer_sock, &eMMC_WriteBuf[eMMC_Len], eMMC_WRITE_BUF_SIZE-eMMC_Len, 0);
        //size = my_recv(xfer_sock, Conn->XferBuffer, sizeof(Conn->XferBuffer), 0);
        if (size == 0) {
          break;
        }
      };
    }else{
        SendReply(Conn, "226 Transfer Complete");
    }

    // For some reason, the file ends up readonly - should fix that.
    f_close(&fp);
    if(ERROR_STOR){
      f_unlink(TEMP_FILE);
    }
}


//------------------------------------------------------------------------------------
// Handle MDTM and SIZE command
//------------------------------------------------------------------------------------
static void StatCmds(Inst_t * Conn, char *filename, /*enum*/ CmdTypes Cmd)
{
    FILINFO fno;
    struct tm tm;
    char RepBuf[50];

    memset(&fno, 0, sizeof(fno));
    if (f_stat(filename, &fno)){
        Send550Error(Conn);
        return;
    }

    if(fno.fattrib & AM_DIR){
        // Its a directory.
        SendReply(Conn, "550 not a plain file.");
        return;
    }

    switch(Cmd){
        case MDTM:
          tm.tm_year = (fno.fdate >> 9) + 1980;
          tm.tm_mon = (fno.fdate >> 5) & 0x0f;
          tm.tm_mday = fno.fdate  & 0x1f;
          tm.tm_hour = fno.ftime >> 11;
          tm.tm_min = (fno.ftime >> 5) & 0x3f;
          tm.tm_sec = (fno.ftime & 0x1f) << 1;

          sprintf(RepBuf, "213 %04d%02d%02d%02d%02d%02d",
                  tm.tm_year, tm.tm_mon, tm.tm_mday, 
                  tm.tm_hour, tm.tm_min, tm.tm_sec);
          break;

        case xSIZE:
          sprintf(RepBuf, "213 %u",fno.fsize);
          break;

        default:
          // Internal screwup!
          strcpy(RepBuf, "550");
          break;
    }
    SendReply(Conn, RepBuf);
}

//------------------------------------------------------------------------------------
// Handle SRFT command
//------------------------------------------------------------------------------------
static void ConvertTimeStamp(FILINFO * pfno, char * TimeStamp)
{
  struct tm tm;
  char buf[5];
  char * Ptr = TimeStamp;
  
  strncpy(buf, Ptr, 4);
  buf[4] = 0;
  tm.tm_year = (int)(strtol(buf, NULL, 10));
  Ptr += 4;
  
  strncpy(buf, Ptr, 2);
  buf[2] = 0;
  tm.tm_mon = (int)(strtol(buf, NULL, 10));
  Ptr += 2;
  
  strncpy(buf, Ptr, 2);
  buf[2] = 0;
  tm.tm_mday = (int)(strtol(buf, NULL, 10));
  Ptr += 2;
  
  strncpy(buf, Ptr, 2);
  buf[2] = 0;
  tm.tm_hour = (int)(strtol(buf, NULL, 10));
  Ptr += 2;

  strncpy(buf, Ptr, 2);
  buf[2] = 0;
  tm.tm_min = (int)(strtol(buf, NULL, 10));
  Ptr += 2;

  strncpy(buf, Ptr, 2);
  buf[2] = 0;
  tm.tm_sec = (int)(strtol(buf, NULL, 10));
  
  pfno->fdate = (WORD)(((tm.tm_year - 1980) * 512U) | tm.tm_mon * 32U | tm.tm_mday);
  pfno->ftime = (WORD)(tm.tm_hour * 2048U | tm.tm_min * 32U | tm.tm_sec / 2U);
}

static void Cmd_SRFT(Inst_t * Conn, char * TimeStamp, char *filename)
{
  FILINFO fno;
  FIL fp;
  FRESULT res = FR_OK;
  char RepBuf[50];
  
  slogf(LOG_DEST_BOTH, "[Cmd_SRFT] %s", TimeStamp);

  res = f_open(&fp, TEMP_FILE, FA_READ | FA_OPEN_EXISTING);
  if (res != FR_OK){
    SendReply(Conn, "550 TEMP_FILE does not exist");
    return;
  }
  f_close(&fp);
  // Delete old file if any
  f_unlink(filename);
  
  // Check for Special Overload commands pre file copy
  if (strcmp(filename, FTP_DEVICE_FIRMWARE) == 0) {
    S_FIRMWARE_HEADER fheader;
    UINT read;
    
    // Firmware update, VUSB must be present (CHDET) or battery must be
    // at ADC_MIN_BAT_FIRMWWARE_UPDATE % or above charge and bootloader must be 2.2 or higher
    
    bool boot22 = ((*pBootloaderMajeur >= 2) && (*pBootloaderMineur >=2));
    int BatteryLevel = 0;
    ADC_Bat_GetVal(&BatteryLevel);          
    slogf(LOG_DEST_BOTH,"Battery level firmware update check = %d.\r\n", BatteryLevel);
    
    if ((PMICStatus & PMIC_STAT_CHDET) || ((BatteryLevel >= ADC_MIN_BAT_FIRMWWARE_UPDATE) && boot22)) {
      res = f_open(&fp, TEMP_FILE, FA_READ | FA_OPEN_EXISTING);
      if (res == FR_OK) {
        res = f_read(&fp, (void *) &fheader, sizeof(fheader), &read);
        f_close(&fp);
        if ((res == FR_OK) && (read == sizeof(fheader))) {
          res = FR_NO_FILE;
#ifdef BOOT_48K
          if ((strcmp(fheader.Signature, FIRMWARE_SIGNATURE_48K) == 0) && (*pBootloaderMajeur >= 2)) {
#else
          if (strcmp(fheader.Signature, FIRMWARE_SIGNATURE_32K) == 0) {
#endif // BOOT_48K
            // Rename temporary file with filename
            res = f_rename(TEMP_FILE, filename);
            if (res == FR_OK) {
              RamParam.BootCmd = CMD_UPDATE_FIRMWARE;
              FLASH_If_SaveParam();
              sprintf(RepBuf, "213 %s",filename);
              SendReply(Conn, RepBuf);
              osDelay(100);
              // Reboot, bootloader will do the update
              HAL_NVIC_SystemReset();
            } else {
              strcpy(RepBuf, "550");
            }
          } else {
            strcpy(RepBuf, "501");
          }
        } else {
          res = FR_NO_FILE;
          strcpy(RepBuf, "501");
        }
      } else {
        strcpy(RepBuf, "550");
      }
    } else {
      res = FR_NO_FILE;
      strcpy(RepBuf, "450 VUSB not detected");
    }
  
#ifdef BOOT_48K  
  } else if (strcmp(filename, FTP_DEVICE_BOOTLOADER) == 0) {
    S_FIRMWARE_HEADER fheader;
    UINT read;
    
    bool boot22 = ((*pBootloaderMajeur >= 2) && (*pBootloaderMineur >=2));
    int BatteryLevel = 0;
    ADC_Bat_GetVal(&BatteryLevel);          
    slogf(LOG_DEST_BOTH,"Battery level bootloader update check = %d.\r\n", BatteryLevel);
    
    // Bootloader update
    if ((PMICStatus & PMIC_STAT_CHDET) || ((BatteryLevel >= ADC_MIN_BAT_FIRMWWARE_UPDATE) && boot22)) {
      res = f_open(&fp, TEMP_FILE, FA_READ | FA_OPEN_EXISTING);
      if (res == FR_OK) {
        res = f_read(&fp, (void *) &fheader, sizeof(fheader), &read);
        f_close(&fp);
        if ((res == FR_OK) && (read == sizeof(fheader))) {
          res = FR_NO_FILE;
          if (strcmp(fheader.Signature, BOOTLOADER_SIGNATURE) == 0) {
            // Rename temporary file with filename
            res = f_rename(TEMP_FILE, filename);
            if (res == FR_OK) {
              res = FR_NO_FILE;
              if(BootUpdate()==0)
              {
                FLASH_If_SaveParam();
                sprintf(RepBuf, "213 %s",filename);
              } else {
                strcpy(RepBuf, "550");
              }
            } else {
              strcpy(RepBuf, "550");
            }
          } else {
            strcpy(RepBuf, "501");
          }
        } else {
          res = FR_NO_FILE;
          strcpy(RepBuf, "501");
        }
      } else {
        strcpy(RepBuf, "550");
      }
    } else {
      res = FR_NO_FILE;
      strcpy(RepBuf, "450 VUSB not detected");
    }
#endif // BOOT_48K
    
#ifdef FACIAL_RECOGNITION_ENABLED
  } else if (strcmp(filename, FTP_AUTH_SIGNIN_FACE_PATH) == 0) {
    printf("Authentication Face\r\n");
    if (AuthCheckFaceForMatch(TEMP_FILE) == NST_MATCH) {
      sprintf(RepBuf, "230 %s",filename);
      
      // Unlock the server and initialize the clock and authentication timer
      FTPLocked = FALSE;
      AdvertiseLockStatus(FTPLocked);
      RTC_InitTime();
      
    } else {
      FTPLocked = TRUE;
      sprintf(RepBuf, "430 %s",filename);
      AdvertiseLockStatus(FTPLocked);
    }
    // To avoid file renaming
    res = FR_NO_FILE;
#endif
    
  } else if (strcmp(filename, FTP_AUTH_SIGNIN_CODE_PATH) == 0) {
    printf("Authentication Code\r\n");
    if (AuthCheckCodeForMatch(TEMP_FILE) == NST_MATCH) {
      sprintf(RepBuf, "230 %s",filename);
      
      // Unlock the server and initialize the clock and authentication timer
      FTPLocked = FALSE;
      AdvertiseLockStatus(FTPLocked);
      RTC_InitTime();
      
    } else {
      FTPLocked = TRUE;
      sprintf(RepBuf, "430 %s",filename);
      AdvertiseLockStatus(FTPLocked);
    }
    // To avoid file renaming
    res = FR_NO_FILE;
    
#ifdef FACIAL_RECOGNITION_ENABLED    
  } else if (strncmp(filename, AUTH_FACE_PATH, (sizeof(AUTH_FACE_PATH) -1)) == 0) {
    enum NStatusCodes nsc = NST_ERROR;
    
    printf("Enroll face\r\n");
    nsc = AuthFaceMatch(TEMP_FILE, TEMP_FILE);
    if (nsc != NST_MATCH) {
      strcpy(RepBuf, "501");
      res = FR_NO_FILE;
    }
#endif
    
  } else if (strncmp(filename, AUTH_RECOVERY_CODE_PATH, (sizeof(AUTH_RECOVERY_CODE_PATH) -1)) == 0) {
    FIL MyFile;
    DWORD size=0;
    
    printf("Enroll Recovery Code\r\n");
    // The only way to validate Recovery code is by the size of the file that
    // must be between RECOVERY_CODE_MIN_SIZE and RECOVERY_CODE_MAX_SIZE.    
    res = f_open(&MyFile, TEMP_FILE, FA_OPEN_EXISTING | FA_READ);
    if (res == FR_OK)
    {
      size = f_size(&MyFile);
      f_close(&MyFile);
      if((size < RECOVERY_CODE_MIN_SIZE) || (size > RECOVERY_CODE_MAX_SIZE))
      {
        strcpy(RepBuf, "501");
        res = FR_NO_FILE;
      }
    }
  }
  
  if (res == FR_OK) {
    ConvertTimeStamp(&fno, TimeStamp);
    if (strncmp(filename, LICENSE_PATH, (sizeof(LICENSE_PATH) -1)) == 0) 
    {
      // destination file is in other partition, must copy file.
      res = f_move(TEMP_FILE, filename);
    }else
    {
      // Rename temporary file with filename
      res = f_rename(TEMP_FILE, filename);
    }
    if (res == FR_OK) {
      // Adjust file timestamp
      res = f_utime(filename, &fno);
      if (res == FR_OK) {
        sprintf(RepBuf, "213 %s",filename);
      } else {
        strcpy(RepBuf, "550");
      }
    } else {
      strcpy(RepBuf, "550");
    }
  }
  SendReply(Conn, RepBuf);
  
  // Check for Special Overload commands - post file copy
  // Check to see if we need to reload device settings
  if (strncmp(filename, FTP_DEVICE_SETTINGS, (sizeof(FTP_DEVICE_SETTINGS) -1)) == 0) {
    // Reload device settings
    PMICGetDeviceSettings();
 }
  
}


//------------------------------------------------------------------------------------
// Handle MLST command
//------------------------------------------------------------------------------------
static void Cmd_MLST(Inst_t * Conn, char * filename)
{
  uint8_t retSD;   
  DWORD fre_clust, fre_sect, tot_sect, used_virtual, free_virtual, total_virtual;
  FATFS *fs;
  char repbuf[MAX_PATH+10];
  int xfer_sock = 2;
  
  slogf(LOG_DEST_BOTH, "[Cmd_MLST] %s", filename);
  
  if (strcmp(filename, FTP_DEVICE_RESET) == 0){
    //reset
    HAL_NVIC_SystemReset();
  }
  
  if (strcmp(filename, FTP_DEVICE_FACTORYRESET) == 0){
    //factoryreset
    PMICStartFactoryReset();
  }
  
  if (strcmp(filename, FTP_DEVICE_POWEROFF) == 0){
    
    // Can only power off board if not conneted to charger
    if (PMICStatus & PMIC_STAT_CHDET) {
       Send550Error(Conn);
    } else {
       //POWEROFF
       PMICSetBoardOff();
    }
  }
        
  // special treatment for selftest request 
  if (strcmp(filename, FTP_DEVICE_SELFTEST) == 0) {            
     // Card will reboot
     RamParam.DoSelfTest = 1;
     FLASH_If_SaveParam();
     HAL_NVIC_SystemReset();
  }
  
  // special treatement for pairing mode
  // force pair command disabled
  /*
  if (strcmp(filename, FTP_DEVICE_PAIR) == 0) { 
      PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_TRIPLE_BLINK;
      BTForcePair = TRUE;
      SendReply(Conn, "226 Pairing Mode Enabled");
      return;
  }
  */
  
  if (strcmp(filename, FTP_VAULT_PATH) == 0) {
    /* Get volume information and free clusters of drive*/
    retSD = f_getfree(SD_Path0, &fre_clust, &fs);
    
    if (retSD != FR_OK) {
        Send550Error(Conn);
        return;
    }
    
    /* Get total sectors and free sectors */
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;
    total_virtual = AVAILABLE_SPACE;
    used_virtual = (tot_sect * 512) - (fre_sect * 512);
    free_virtual = total_virtual - used_virtual;
    sprintf(repbuf + 3, "{'free_B': %10lu, 'used_B': %10lu, 'total_B': %10lu}",
        free_virtual, used_virtual, total_virtual);

    SendReply(Conn, "150 Opening connection");
    my_send(xfer_sock, repbuf, strlen(repbuf + 3),0);
    SendReply(Conn, "226 Transfer Complete");
  } else {
    Send550Error(Conn);
  }
}

FRESULT remove_folder(char* path){
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    res = f_opendir(&dir, path);
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (strncmp(fno.fname, ".", 1) == 0 || strncmp(fno.fname, "..", 2) == 0){
              continue;
            }
            i = strlen(path);
            sprintf(&path[i], "/%s", fno.fname);
            if (fno.fattrib & AM_DIR) { /* is directory ? */
                res = remove_folder(path);   /* recurse in subdirectory */
                if (res != FR_OK) break;
            } else {
                res = f_unlink(path);
            }
            path[i] = 0; /* sets str lenght back to i */
        }
        f_closedir(&dir);
        res = f_unlink(path);
    }else{
      res = f_unlink(path);
    }

    return res;
}

//------------------------------------------------------------------------------------
// Handle DELE command
//------------------------------------------------------------------------------------
static void Cmd_DELE(Inst_t * Conn, char * filename, bool recursive)
{
  FILINFO fno;
  
  slogf(LOG_DEST_BOTH, "[Cmd_DELE] %s", filename);
  BD_ADDR_t key;
  BD_ADDR_t NULL_BD_ADDR; 
  char repbuf[40]; //TODO: remove magic number
  int status;

  if (strcmp(filename, FTP_AUTH_SIGNOUT_PATH) == 0) {
    FTPLocked = TRUE;
    SendReply(Conn, "231 logged out.");
    AdvertiseLockStatus(FTPLocked);
  } else {
    if (BT_Key_Parser(filename, &key)){
      ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
      if (COMPARE_BD_ADDR(NULL_BD_ADDR, key)){
        status = DeleteAllKeys();
      }else{
        status = DeleteLinkKey(key);
      }
      sprintf(repbuf, "%d DELE BT key %s: %d", 
              status ? 550:250, status ? "error":"succesful", status);
      SendReply(Conn, repbuf); 
      return;
    }
    memset(&fno, 0, sizeof(fno));
    if (!f_stat(filename, &fno)){
      if (!(fno.fattrib & AM_DIR)){
        if (!f_unlink(filename)) {
          SendReply(Conn, "250 DELE command successful.");
        }else {
          // Error deleting file  
          Send550Error(Conn);
        }
      } else {
        //file is a directory
        char directory[512];
        sprintf(directory, "%s", filename);
        if (recursive && !remove_folder(directory)) {
          SendReply(Conn, "250 DELE command successful.");
        }else {
          // Error deleting file  
          Send550Error(Conn);
        }
      }
    } else {
      // Error cannot get attributes
      Send550Error(Conn);
    }
  }
}
//------------------------------------------------------------------------------------
// Main loop - handle the FTP commands that are implemented.
//------------------------------------------------------------------------------------
static void ProcessCommands(Inst_t * Conn)
{
    // Data buffer for reading FTP command codes
    char buf[MAX_PATH+10];
    char repbuf[MAX_PATH+10];
    char * NewPath;
    char * Prefix;
    char *Date_Time;
    int a;
    int buf_len;
    bool recursive;
    uint8_t retSD;   
    DWORD fre_clust, fre_sect, tot_sect, used_sect;
    FATFS *fs;
    Conn->PassiveSocket = 1;

    // Indicate ready to accept commands
    SendReply(Conn, "220 Minftpd ready");

    for(;;){
        // Get FTP command from input stream
        CmdTypes FtpCommand;
        int AuthorizedCommand;

        FtpCommand = (CmdTypes)GetCommand(Conn, buf);
        if (FtpCommand == UNKNOWN_COMMAND) {
          continue;
        }
        FTPActivity++;
        if (FTPLocked) {
          // Check if valid templates exist
          if (FindValidTemplate() != FR_OK) {
            // Unlock FTP since no templates
            FTPLocked = FALSE;
            AdvertiseLockStatus(FTPLocked);
            SetRootDir(FTP_ROOT_PATH);
          }
        }
        if (FTPLocked) {
          AuthorizedCommand = FALSE;

          if (FtpCommand == SRFT) {
            NewPath = FindPath(buf);
          }else
          {
            NewPath = buf;
          }
          if (strncmp(NewPath, FTP_AUTH_SIGNIN_PATH, (sizeof(FTP_AUTH_SIGNIN_PATH) -1)) == 0) {
            AuthorizedCommand = TRUE;
          }else if (strncmp(NewPath, LICENSE_PATH, (sizeof(LICENSE_PATH) -1)) == 0) {
            AuthorizedCommand = TRUE;
          }else if (strncmp(NewPath, FTP_LICENSE_PATH, (sizeof(FTP_LICENSE_PATH) -1)) == 0) {
            AuthorizedCommand = TRUE;
          }else if (strncmp(NewPath, FTP_PASSWORD_VAULT_PATH, (sizeof(FTP_PASSWORD_VAULT_PATH) -1)) == 0) {
            AuthorizedCommand = TRUE;
          }
          if (!AuthorizedCommand) {
            SendReply(Conn, "530 not logged");
            continue;
          }
        }
        switch(FtpCommand){
            case SYST:
                slogf(LOG_DEST_BOTH, "[ProcessCommands] SYST command");
                SendReply(Conn, "215 ftpdmin v. "FTPDMIN_VER);
                break;

            case PASV:
                slogf(LOG_DEST_BOTH, "[ProcessCommands] PASV command");
                sprintf(repbuf, "227 Entering Passive Mode (%s,%d,%d)",
                    OurAddrStr, Conn->XferPort >>8, Conn->XferPort & 0xff);
                for (a=0;a<50;a++){
                    if (repbuf[a] == 0) break;
                    if (repbuf[a] == '.') repbuf[a] = ',';
                }
                SendReply(Conn, repbuf);
                Conn->PassiveMode = TRUE;
                break;

            case XPWD:
            case PWD: // Print working directory
                slogf(LOG_DEST_BOTH, "[ProcessCommands] PWD command"); 
                sprintf(repbuf,"257 \"%s\"", MyGetDir());
                SendReply(Conn, repbuf);
                break;

            case NLST: // Request directory, names only.
                Cmd_NLST(Conn, buf, FALSE, FALSE);
                break;

            case LIST: // Request directory, long version.
                Cmd_NLST(Conn, buf, TRUE, FALSE);
                break;

            case STAT: // Just like LIST, but use control connection.
                Cmd_NLST(Conn, buf, TRUE, TRUE);
                break;

            case DELE:
                buf_len = strlen(buf) - 3;
                recursive = false;
                if (strncmp(buf + buf_len, " -r", 2) == 0){
                  recursive = true;
                  buf[buf_len] = 0;
                }
                
                NewPath = TranslatePath(buf);
                if (NewPath == NULL){
                    SendReply(Conn, "550 Path permission error");
                    break;
                }
                Cmd_DELE(Conn, NewPath, recursive);
                break;

            case RMD:
            case MKD:
            case XMKD:
            case XRMD:
                NewPath = TranslatePath(buf);
                slogf(LOG_DEST_BOTH, "[ProcessCommands] RMD/MKD %s", NewPath);
                if (GetOnly){
                    SendReply(Conn, "550 Permission denied");
                    break;
                }
                
                if (NewPath == NULL){
                    SendReply(Conn, "550 Path permission error");
                    break;
                }
                if (FtpCommand == MKD || FtpCommand == XMKD){
                    if (f_mkdir(NewPath)) {
                        Send550Error(Conn);
                    }else{
                        SendReply(Conn, "257 Directory created");
                    }
                }else{
                  FILINFO fno;
                  memset(&fno, 0, sizeof(fno));
                  if (!f_stat(NewPath, &fno)){
                    if (fno.fattrib & AM_DIR){
                      if (!f_unlink(NewPath)) {
                          SendReply(Conn, "250 RMD command successful");
                        }else{
                          // Error deleting file
                          Send550Error(Conn);
                        }
                    } else {
                      // Error file is not a directory
                      Send550Error(Conn);
                    }
                  } else {
                    // Error cannot get attributes
                    Send550Error(Conn);
                  }
                }
                break;

            case RNFR:
                NewPath = TranslatePath(buf);
                slogf(LOG_DEST_BOTH, "[ProcessCommands] RNFR %s", NewPath);
                if (NewPath){
                    strcpy(repbuf, NewPath);
                    SendReply(Conn, "350 File Exists");
                }else{
                    SendReply(Conn, "550 Path permission error");
                }
                break;
                
            case RNTO:
                // Must be immediately preceeded by RNFR!
                NewPath = TranslatePath(buf);
                slogf(LOG_DEST_BOTH, "[ProcessCommands] RNTO %s", NewPath);
                if (GetOnly){
                    SendReply(Conn, "550 Permission denied");
                    break;
                }       
                if (f_rename(repbuf, NewPath)){
                    Send550Error(Conn);
                }else{
                    SendReply(Conn, "250 RNTO command successful");
                }
                break;

            case ABOR:
                SendReply(Conn, "226 Aborted");
                break;

            case xSIZE:
            case MDTM:
                NewPath = TranslatePath(buf);
                slogf(LOG_DEST_BOTH, "[ProcessCommands] MDTM/xSIZE %s", NewPath);
                if (NewPath == NULL){
                    SendReply(Conn, "550 Path permission error");
                    break;
                }
                StatCmds(Conn, NewPath, FtpCommand);
                break;
                
            case SRFT:
              Date_Time = buf;
              NewPath = FindPath(buf);
              if (NewPath == NULL) {
                  SendReply(Conn, "550 Path permission error");
                  break;
              }
              Cmd_SRFT(Conn, Date_Time, NewPath);

              break;
              
            case MLST:
              Cmd_MLST(Conn, buf);
              break;

            case CWD: // Change working directory
                NewPath = TranslatePathAbs(buf);
                slogf(LOG_DEST_BOTH, "[ProcessCommands] CWD %s", NewPath);
                if (NewPath == NULL){
                    SendReply(Conn, "550 Path permission error");
                    break;
                }
                if (!MySetDir(NewPath, FindPrefix(buf))){
                    SendReply(Conn, "550 Could not change directory");
                }else{
                    SendReply(Conn, "250 CWD command successful");
                }
                break;
 
            case TYPE: // Accept file TYPE commands, but ignore.
                slogf(LOG_DEST_BOTH, "[ProcessCommands] TYPE command (ignored)");
                SendReply(Conn, "200 Type set to I");
                break;

            case NOOP:
                slogf(LOG_DEST_BOTH, "[ProcessCommands] NOOP command");
                SendReply(Conn, "200 OK");
                break;

            case PORT: // Set the TCP/IP addres for trasnfers.
                {
                    int h1,h2,h3,h4,p1,p2;
                    char *a, *p;
                    sscanf(buf,"%d,%d,%d,%d,%d,%d",&h1,&h2,&h3,&h4,&p1,&p2);
                    a = (char *) &Conn->xfer_addr.sin_addr;
                    p = (char *) &Conn->xfer_addr.sin_port;
                    a[0] = h1; a[1] = h2; a[2] = h3; a[3] = h4;
                    p[0] = p1; p[1] = p2;
                }
                SendReply(Conn, "200 PORT command successful");
                break;

            case RETR: // Retrieve File and send it
                NewPath = TranslatePath(buf);
                if (NewPath == NULL){
                    SendReply(Conn, "550 Path permission error");
                    break;
                }
                Cmd_RETR(Conn, NewPath);
                break;

            case STOR: // Store the file.
                if (strncmp(buf, BT_KEY_FILENAME, (sizeof(BT_KEY_FILENAME) -1)) == 0){
                  Cmd_STOR(Conn, buf, 0);
                  break;
                }
                if (GetOnly){
                    SendReply(Conn, "553 Permission denied");
                    break;
                }
                NewPath = TranslatePath(buf);
                if (NewPath == NULL){
                    SendReply(Conn, "550 Path permission error");
                    break;
                }
                Prefix = FindPrefix(NewPath);
                
                retSD = f_getfree(SD_Path0, &fre_clust, &fs);
                if (retSD != FR_OK) {
                    Send550Error(Conn);
                    return;
                }
                /* Get total sectors and free sectors */
                tot_sect = (fs->n_fatent - 2) * fs->csize;
                fre_sect = fre_clust * fs->csize;
                used_sect = (tot_sect * 512) - (fre_sect * 512);                
                Cmd_STOR(Conn, Prefix, used_sect);
                break;
            case UNKNOWN_COMMAND:
                slogf(LOG_DEST_BOTH, "[ProcessCommands] unknown command");
                SendReply(Conn, "500 command not recognized");
                break;

            case QUIT: 
                slogf(LOG_DEST_BOTH, "[ProcessCommands] QUIT command");
                SendReply(Conn, "221 goodbye");
                goto EndConnection;


            case -1:
                goto EndConnection;

            
            default: // Any command not implemented, return not recognized response.
                SendReply(Conn, "500 command not implemented");
                break;
        }
        // MDR test power management GotoStop = TRUE;
        //osSemaphoreRelease(PMIC_Semaphore);
        //osThreadYield();
    }
EndConnection:
    printf("Closing control connection\r\n");
    PortsUsed[Conn->XferPort & 255] = 0;
    free(Conn);
}

//--------------------------------------------------------------------------
static void Usage (void)
{
    printf("\nftpdmin is a minimal FTP server program.  ftpdmin is run directly from\n"
           "a windows command prompt.  ftpdmin is used as a quck, temporary ad-hock\n"
           "FTP server for transferring files to and from windows PCs via FTP.\n"
           "ftpdmin is single threaded, single session only, and provides no\n"
           "authentication or security of any sort.\n\n");

    printf("Usage: ftpdmin [options] [root]\n");
    printf("[options] are:\n"
           "  -p <port>  Specify control port number (default 21)\n"
           "  -tp <port> Specify port or port range to use for passive mode (defaults to\n"
           "             abitrary port the OS assigns).  Useful behind firewalls.\n"
           "  -ha        Specifies thru which address ftpdmin is to be accessed\n"
           "             (for NAT firewalled access.  Use only if you know how to set up\n"
           "             port forwarding on your firewall)\n"
           "  -g         Get only (disallows uploads)\n"
           "  root       Directory to consider root for FTP access\n");

    exit(-1);
}

//------------------------------------------------------------------------------------
// Main for ftpdmin program
//------------------------------------------------------------------------------------
int ftp_main(int argc, char **argv)
{
    int ControlPort;
    int argn;
    char * UseRootDir = NULL;
    int XferPort, XferPortStart, XferPortRange; 

    ControlPort = 21;
    XferPortStart = XferPortRange = XferPort = 0;
       
    ringBufS_flush(&rBufs1, 1);
    ringBufS_flush(&rBufs2, 1);

    InitializeCriticalSection(&gCS);
    
    osSemaphoreDef(FTP_SEM_RX_FULL);
    FTP_RxDataReady = osSemaphoreCreate(osSemaphore(FTP_SEM_RX_FULL) , 1);
    
    osSemaphoreDef(FTP_SEM_RX_EMPTY);
    FTP_RxFifoEmpty = osSemaphoreCreate(osSemaphore(FTP_SEM_RX_EMPTY) , 1);
    
    for (argn=1;argn<argc;argn++){
        char * arg;
        arg = argv[argn];
        if (argv[argn][0] == '-'){
            if (!strcmp(arg,"-p")){
                if (argn+2 > argc || sscanf(argv[argn+1],"%d", &ControlPort) != 1){
                    printf("Error!  -p must be followed by port number\n");
                    Usage();
                }
                argn += 1;
            }else if (!strcmp(arg,"-tp")){
                if (argn+2 > argc || sscanf(argv[argn+1],"%d-%d", &XferPortStart, &XferPortRange) < 1){
                    printf("Error!  -tp must be followed by port number\n");
                    Usage();
                }
                argn += 1;
            }else if (!strcmp(arg,"-ha")){
                int a;
                if (argn+2 > argc || sscanf(argv[argn+1],"%d.%d.%d.%d", &a,&a,&a,&a) != 4){
                    printf("Error!  -ha must be followed by an ip address\n");
                    Usage();
                }
                argn += 1;
            }else if (!strcmp(arg,"-g")){
                GetOnly = TRUE;
            }else if (!strcmp(arg,"-h")){
                Usage();
            }else{
                printf("Error!  '%s' not understood\n",argv[argn]);
                Usage();
            }
       
        }else{
            if (UseRootDir){
                printf("Error!  Can only specify one directory\n");
                Usage();
            }
            UseRootDir = argv[argn];
        }
    }

    if (UseRootDir != NULL){
        if (!SetRootDir(UseRootDir)){
            printf("Error!  Unable to set root directory '%s'\n",UseRootDir);
            Usage();
        }
    }else{
        SetRootDir(FTP_ROOT_PATH);
    }

    XferPort = XferPortStart;

    slogf(LOG_DEST_BOTH, "ftpdmin ready to accept connections");
    
    Inst_t * Conn;
    Conn = (Inst_t *)malloc(sizeof(Inst_t));
    if (Conn == 0){
        printf("Out of memory\r\n");
        exit(-1);
    }
    Conn->CommandSocket = 1;
    Conn->XferPort = XferPort;
    
    ProcessCommands(Conn);

	return 0;
}

/**
  * @brief  FTP task
  * @param  pvParameters not used
  * @retval None
  */
void FTPThread(void const *argument)
{

  slogf(LOG_DEST_BOTH, "FTP Thread Started");

  // Init eMMC and FATFS file system
  if(MX_FATFS_Init()==FR_OK)
  {
    // Start FTP server
    ftp_main(0, NULL);    // this function should not return.
  }
  
  /* Infinite Loop */
  for( ;; )
  {
    osDelay(1000);
  }
}

#define READ_BUFF_SIZE 512
uint8_t FileHeader[READ_BUFF_SIZE];
uint8_t ReadBuff[READ_BUFF_SIZE];       /* buffer used to read bootloader update file. */
#ifdef BOOT_48K
static int BootUpdate(void)
{
  uint8_t retSD;          /* Return value for SD */
  FIL MyFile;             /* File object */
  UINT BytesCount;
  unsigned short Crc16;
  unsigned short* pFileCrc;
  //uint32_t x;
  unsigned char zeros[2] = {0, 0};
  static __IO uint32_t FlashWriteAddress; /* Used to track where to write in FLASH */
  int ret=1;  
  int x;
  
retry:
  
  slogf(LOG_DEST_BOTH, "Searching for bootloader file ...");
  retSD = f_open(&MyFile, FTP_DEVICE_BOOTLOADER, FA_READ);
  if(retSD != FR_OK)
  {
    slogf(LOG_DEST_BOTH, "Error opening bootloader file.");
    goto error;
  }
  
  // we must read all file to validate it CRC.
  printf("Calculate bootloader file CRC16...");  
  retSD = f_read(&MyFile, (void*)&FileHeader, sizeof(FileHeader), (UINT*)&BytesCount);
  if((retSD != FR_OK) || (BytesCount != sizeof(FileHeader)))
  {
    slogf(LOG_DEST_BOTH, "Error reading bootloader header!");
    goto error;
  }
  
  Crc16=0;
  x=0;
  do
  {
    retSD = f_read(&MyFile, ReadBuff, READ_BUFF_SIZE, (UINT*)&BytesCount);
    if(retSD != FR_OK)
    {
      slogf(LOG_DEST_BOTH, "Error reading bootloader file!");
      goto error;
    }
    x++;
    if(x==96)
    {
      Crc16 = slow_crc16(Crc16, ReadBuff, BytesCount-2);
    }else
    {
      Crc16 = slow_crc16(Crc16, ReadBuff, BytesCount);
    }
  }while((retSD == FR_OK) && (BytesCount == READ_BUFF_SIZE));
  Crc16 = slow_crc16(Crc16, zeros, 2);
  
  pFileCrc = (unsigned short*) &ReadBuff[510];
  if(Crc16 != *pFileCrc)    
  {
    slogf(LOG_DEST_BOTH, "Error bad bootloader CRC!n");
    goto error;
  }
  
  slogf(LOG_DEST_BOTH, "Bootloader file looks good!");
  
  // replace file pointer at the begin of bootloader... just after the file header.
  retSD = f_lseek(&MyFile, sizeof(FileHeader));
  if(retSD != FR_OK)
  {
    slogf(LOG_DEST_BOTH, "f_lseek() error on bootloader file");
    goto error;
  }

  /* Erase USER FLASH */
  slogf(LOG_DEST_BOTH, "Erasing bootloader flash...");
  FLASH_If_Unlock();
  FLASH_If_Erase(USER_BOOTLOADER_FIRST_PAGE_ADDRESS_48K);
  
  slogf(LOG_DEST_BOTH, "Writing bootloader to flash ...");
  // Don't write Firmware header right now... 
  FlashWriteAddress = USER_BOOTLOADER_FIRST_PAGE_ADDRESS_48K;
  while((FlashWriteAddress < USER_BOOTLOADER_END_ADDRESS)&&(retSD == FR_OK))
  {
    printf(".");
    memset(ReadBuff,0,READ_BUFF_SIZE);
    BytesCount=0;
    retSD = f_read(&MyFile, ReadBuff, READ_BUFF_SIZE, (UINT*)&BytesCount);
    if(retSD != FR_OK)
    {
      slogf(LOG_DEST_BOTH, "Error reading bootloader file");
      goto error;
    }
       
    if(BytesCount > 0)
    {
      if(FLASH_If_Write8(&FlashWriteAddress, (uint8_t*)(ReadBuff),BytesCount))
      {
        // an error occure... retry.
        slogf(LOG_DEST_BOTH, "Flashing bootloader failed, retrying ...");
        retSD = f_close(&MyFile);
        goto retry;
      }
    }else
    {
      printf("\r\nEOF\r\n");
    }
    
    if(BytesCount != READ_BUFF_SIZE )
    {
      // reach end of file!
      break;
    }
  }
    
  if(retSD == FR_OK)
  {
    slogf(LOG_DEST_CONSOLE, "");
    slogf(LOG_DEST_BOTH, "Bootloader update done.");
  }
  retSD = f_close(&MyFile);
  ret=0;
exit:  
  return ret;
  
error:
    slogf(LOG_DEST_BOTH, "Error - bootloader update cancled!");
    goto exit;
}
#endif // BOOT_48K

FRESULT f_move (
	const TCHAR* path_source,
	const TCHAR* path_dest
)
{
  FRESULT res, res_source = FR_NO_FILE, res_dest = FR_NO_FILE;
  FIL Source;
  FIL Dest; 
  UINT NbRead, NbWrite;
  
  res_source = res = f_open(&Source, path_source, FA_READ);
  if(res != FR_OK)
  {
    goto exit;
  }
  
  res_dest = res = f_open(&Dest, path_dest, FA_CREATE_ALWAYS | FA_WRITE);
  if(res != FR_OK)
  {
    goto exit;
  }
  
  do
  {
    res = f_read(&Source, eMMC_WriteBuf, sizeof(eMMC_WriteBuf), (void *)&NbRead);
    if(res != FR_OK)
    {
      goto exit;
    }
    
    res = f_write(&Dest, eMMC_WriteBuf, NbRead, (void *)&NbWrite);
    if(res != FR_OK || NbWrite!=NbRead)
    {
      goto exit;
    }
  }while(NbRead == sizeof(eMMC_WriteBuf));
exit:
  if(res_dest == FR_OK)
  {
    res = f_close(&Dest);
  }
  
  if(res_source == FR_OK)
  {
    res = f_close(&Source);
  }
    
  if(res == FR_OK)
  {
    res = f_unlink(path_source);
  }
  
  
  return res;
}