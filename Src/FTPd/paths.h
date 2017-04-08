
#include <time.h>
#include "main.h"

#define BOOL int
//#define FALSE 0
//#define TRUE 1

#define SOCKET int
#define MAX_PATH 256

struct in_addr {
    unsigned long s_addr;  // load with inet_aton()
};

struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
};



BOOL SetRootDir(char * dir);
BOOL MySetDir(char * dir, char * Prefix);
char * MyGetDir(void);
char * TranslatePath(const char * FilePath);
char * TranslatePathAbs(const char * FilePath);
char * FindPath(const char * CmdPath);
char * FindPrefix(const char * CmdPath);
char * LogicalToPhysPath(const char * FilePath);
char * CurrentPhysPath(const char * FilePath);
FRESULT CheckPath(const char * FilePath);
enum NStatusCodes AuthCheckFaceForMatch(const char * probefilename);
enum NStatusCodes AuthCheckCodeForMatch(const char * probefilename);
FRESULT FindValidTemplate(void);
enum NStatusCodes AuthFaceMatch(const char * probefilename, const char * templfilename);

void Sleep(int mSec);


