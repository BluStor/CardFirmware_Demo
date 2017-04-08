//------------------------------------------------------------------------------------
// Module to handle mapping specified paths onto a relative path.
//------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#include "paths.h"
#include "ff.h" 

static char RootDir[MAX_PATH] = VAULT_DATA_PATH; 
static char VaultCwdDir[MAX_PATH] = VAULT_DATA_PATH;
static char LicensePath[MAX_PATH] = LICENSE_PATH;
const char VaultRootDir[] = VAULT_DATA_PATH;

//------------------------------------------------------------------------------------
// Specify root dir, specified with backslashes.
//------------------------------------------------------------------------------------
BOOL SetRootDir(char * dir) 
{
    if (f_chdir(dir)) return FALSE;
    f_getcwd(RootDir, sizeof(RootDir));
    //printf("Using '%s' as root directory\r\n",RootDir);
    return TRUE;
}

//------------------------------------------------------------------------------------
// Verify if path exists
//------------------------------------------------------------------------------------
FRESULT CheckPath(const char * FilePath)
{
  DIR  dp;
  FRESULT res;
  
  // Check for special paths (not physical paths)
  if (strcmp(FilePath, FTP_AUTH_SIGNIN_PATH) == 0) {
    res = FR_OK;
  } else {
    res = f_opendir(&dp, FilePath);
    if (res == FR_OK) {
      f_closedir(&dp);
    }
  }
  
  return (res);
}

//------------------------------------------------------------------------------------
// Find prefix in command and validate it (prefix/filename)
//------------------------------------------------------------------------------------
char * FindPrefix(const char * FilePath)
{
  char * PrefixPathBegin = (char *)FilePath;
  char * PrefixPathEnd;
  static char Prefix[MAX_PATH];
  
  PrefixPathEnd = strrchr(PrefixPathBegin, '/');
  
  if (PrefixPathEnd != NULL) {
    memset(Prefix, 0, sizeof(Prefix));
    strncpy(Prefix, PrefixPathBegin, (PrefixPathEnd - PrefixPathBegin));
    if (Prefix[0] == 0) {
      PrefixPathBegin = NULL;
    } else {
      PrefixPathBegin = Prefix;
    }
  } else {
    PrefixPathBegin = NULL;
  }
  
  return PrefixPathBegin;
}

//------------------------------------------------------------------------------------
// Find filename in command (/prefix1/prefix.../filename)
//------------------------------------------------------------------------------------
char * FindFilename(char * FilePath)
{
  char * FilenameBegin;
  
  if (strncmp(FilePath, FTP_VAULT_PATH, (sizeof(FTP_VAULT_PATH) -1)) == 0) {
    // If prefix is for File Vault Operations, work with relative path from current working directory
    if (strlen(FilePath) >= sizeof(FTP_VAULT_PATH)) {
      FilenameBegin = FilePath + sizeof(FTP_VAULT_PATH);
    } else {
      FilenameBegin = NULL;
    }
  } else {
    if (FilenameBegin = strrchr(FilePath, '/')) {
      FilenameBegin++;
    } else {
      FilenameBegin = NULL;
    }
  }
  
  return (FilenameBegin);
}

//------------------------------------------------------------------------------------
// Search current working directory for given prefix. Return NULL if not found
//------------------------------------------------------------------------------------
char * CurrentPhysPath(const char * FilePath)
{
  char * TmpPath = NULL;
  char * Ptr1;
  char * Ptr2;

  TmpPath = FindPrefix(FilePath);
  if (TmpPath) {
    if (strcmp(TmpPath, FTP_DEVICE_PATH) == 0) {
      // If prefix is for Device Operations, work with device directory
      TmpPath = DEVICE_PATH;
    } else if (strncmp(TmpPath, FTP_VAULT_PATH, (sizeof(FTP_VAULT_PATH) -1)) == 0) {
      // If prefix is for File Vault Operations, work with file vault current working directory
      TmpPath = VaultCwdDir;
    } else if (strcmp(TmpPath, FTP_AUTH_PATH) == 0) {
      // If prefix is for Auth Operations, work with auth directory
      TmpPath = AUTH_PATH;
    } else if (strcmp(TmpPath, FTP_AUTH_ENROLL_FACE_PATH) == 0) {
      // If prefix is for Auth Operations, work with auth/face directory
      TmpPath = AUTH_FACE_PATH;
    } else if (strcmp(TmpPath, FTP_AUTH_ENROLL_CODE_PATH) == 0) {
      // If prefix is for Auth Operations, work with auth/pin directory
      TmpPath = AUTH_RECOVERY_CODE_PATH;
    }else if (strncmp(TmpPath, FTP_LICENSE_PATH, (sizeof(FTP_LICENSE_PATH) -1)) == 0) {
      // If prefix is for Licenses Operations, work with license directory
      strcpy(LicensePath, LICENSE_PATH);
      Ptr1 = strchr(FilePath + 1, '/');
      Ptr2 = strrchr(FilePath, '/');
      if ((Ptr1 != Ptr2) && (Ptr2 != NULL)) {
        strncat(LicensePath, Ptr1, (Ptr2 - Ptr1));
      }
      TmpPath = LicensePath;
    }
  }

  return (TmpPath);
}

//------------------------------------------------------------------------------------
// Search physical root directory for given prefix. Return NULL if not found
//------------------------------------------------------------------------------------
char * LogicalToPhysPath(const char * FilePath)
{
  char * TmpPath = NULL;

  TmpPath = FindPrefix(FilePath);
  if (TmpPath) {
    if (strcmp(TmpPath, FTP_DEVICE_PATH) == 0) {
      // If prefix is for Device Operations, work with device directory
      TmpPath = DEVICE_PATH;
    } else if (strncmp(TmpPath, FTP_VAULT_PATH, (sizeof(FTP_VAULT_PATH) -1)) == 0) {
      // If prefix is for File Vault Operations, work with file vault root directory
      TmpPath = (char *)VaultRootDir;
    } else if (strcmp(TmpPath, FTP_AUTH_PATH) == 0) {
      // If prefix is for Auth Operations, work with auth directory
      TmpPath = AUTH_PATH;
    } else if (strcmp(TmpPath, FTP_AUTH_ENROLL_FACE_PATH) == 0) {
      // If prefix is for Auth Operations, work with auth/face directory
      TmpPath = AUTH_FACE_PATH;
    } else if (strcmp(TmpPath, FTP_AUTH_ENROLL_CODE_PATH) == 0) {
      // If prefix is for Auth Operations, work with auth/pin directory
      TmpPath = AUTH_RECOVERY_CODE_PATH;
    } else if (strcmp(TmpPath, FTP_LICENSE_PATH) == 0) {
      // If prefix is for Auth Operations, work with auth/pin directory
      TmpPath = LICENSE_PATH;
    }
  }

  return (TmpPath);
}

//------------------------------------------------------------------------------------
// Check for path bits that are all '.'  These are illegal filenames under
// dos, or would allow us to go outside of the 'root' specified.
//------------------------------------------------------------------------------------
char * TranslatePath(const char * FilePath)
{
    int a,b;
    BOOL NonDot;
    static char NewPath[MAX_PATH];
    char * TmpPath;

    NonDot = FALSE;

    if (FilePath[0] == '\\' || FilePath[0] == '/'){
      // Absolute file path.  Substitute logical path with physical path
      TmpPath = CurrentPhysPath(FilePath);
      if (TmpPath) {
        strcpy(NewPath, TmpPath);
        b = strlen(NewPath);
        TmpPath = FindFilename((char *)FilePath);
        if (TmpPath[0] != 0) {
          // if filename follows prefix, add '/' separator
          NewPath[b++] = '/';
        }
      } else {
        TmpPath = (char *)FilePath;
        b = 0;
      }
    }else{
      // Relative path. 
      b = 0;
      TmpPath = (char *)FilePath;
    }
    
    if (!TmpPath) {
      return NULL;
    }

    NonDot = FALSE;
    for (a=0;;a++){
        if (TmpPath[a] == '/' || TmpPath[a] == '\\'){
            NewPath[b] = '/';
            if (!NonDot && a != 0) return NULL;
            NonDot = FALSE;
        }else{
            if (TmpPath[a] != '.'){
                NonDot = TRUE;
            }
            NewPath[b] = TmpPath[a];
        }
        if (TmpPath[a] == 0) break;
        b++;
    }
    if (!NonDot) return NULL;

    return NewPath;
}

//------------------------------------------------------------------------------------
// Check for path bits that are all '.'  These are illegal filenames under
// dos, or would allow us to go outside of the 'root' specified.
//------------------------------------------------------------------------------------
char * TranslatePathAbs(const char * FilePath)
{
    int a,b;
    BOOL NonDot;
    static char NewPath[MAX_PATH];
    char * TmpPath;

    NonDot = FALSE;

    if (FilePath[0] == '\\' || FilePath[0] == '/'){
      // Absolute file path.  Substitute logical path with physical path
      TmpPath = LogicalToPhysPath((char *)FilePath);
      if (TmpPath) {
        strcpy(NewPath, TmpPath);
        b = strlen(NewPath);
        TmpPath = FindFilename((char *)FilePath);
        if (TmpPath[0] != 0) {
          // if filename follows prefix, add '/' separator
          NewPath[b++] = '/';
        }
      } else {
        TmpPath = (char *)FilePath;
        b = 0;
      }
    }else{
      // Relative path. 
      b = 0;
      TmpPath = (char *)FilePath;
    }
    
    if (!TmpPath) {
      return NULL;
    }

    NonDot = FALSE;
    for (a=0;;a++){
        if (TmpPath[a] == '/' || TmpPath[a] == '\\'){
            NewPath[b] = '/';
            if (!NonDot && a != 0) return NULL;
            NonDot = FALSE;
        }else{
            if (TmpPath[a] != '.'){
                NonDot = TRUE;
            }
            NewPath[b] = TmpPath[a];
        }
        if (TmpPath[a] == 0) break;
        b++;
    }
    if (!NonDot) return NULL;

    return NewPath;
}

//------------------------------------------------------------------------------------
// Find path in SFTR command and validate it (TIMESTAMP FILENAME)
//------------------------------------------------------------------------------------
char * FindPath(const char * CmdPath)
{
  char * NewPath = (char *)CmdPath;
  
  NewPath = strstr(NewPath, " ");
  // Didn't found 'space' between TIMESTAMP ans FILENAME
  if (NewPath == NULL) {
    return NewPath;
  }
  // Pass 'space' 
  NewPath++;
    
  return TranslatePath(NewPath);
}

//------------------------------------------------------------------------------------
// Specify directory change.  Specified with forward slashes.
//------------------------------------------------------------------------------------
BOOL MySetDir(char * dir, char *Prefix)
{
    int a;
    char DirWas[MAX_PATH];
    char DirNow[MAX_PATH];

    if (dir[0] == 0) return TRUE;

    f_getcwd(DirWas, sizeof(DirWas));
    for (a=0;dir[a];a++){
        if (dir[a] == '/') dir[a] = '\\';
    }
    if (dir[0] == '\\'){
        if (strncmp(Prefix, FTP_VAULT_PATH, (sizeof(FTP_VAULT_PATH) -1)) == 0) {
          dir += (sizeof(VAULT_DATA_PATH) -1);
          if (dir[0] == '\\') {
            dir +=1;
          } else if (dir[0] == 0) {
            // No path specified. Assume root is wanted
            f_chdir(RootDir);
            // Keep a copy of working directory
            strcpy(VaultCwdDir, RootDir);
          }
        } else {
          // Set absolute directory.  Start from our root.
          f_chdir(RootDir);
          dir += 1;
        }
    }

    if (dir[0] == 0) return TRUE;
    
    // Set relative.
    if (f_chdir(dir)){
        // Chdir failed.
        f_chdir(DirWas); // Go back to previous directory.
        return FALSE;
    }

    f_getcwd(DirNow, sizeof(DirNow));

    if (memcmp(DirNow, RootDir, strlen(RootDir))){
        // We are no longer under 'root'.
        f_chdir(DirWas);
        return FALSE;
    } else {
      if (Prefix) {
        if (strncmp(Prefix, FTP_VAULT_PATH, (sizeof(FTP_VAULT_PATH) -1)) == 0) {
          // Keep a copy of working directory
          strcpy(VaultCwdDir, DirNow);
        }
      }
    }
    return TRUE;
}

//------------------------------------------------------------------------------------
// get current directory.  Directory is returned with forward slashes.
//------------------------------------------------------------------------------------
char * MyGetDir(void)
{
    int a;
    int RootLen;
    static char DirNow[MAX_PATH];

    RootLen = strlen(RootDir);
    if (RootLen ==3) RootLen = 2;

    f_getcwd(DirNow, sizeof(DirNow));
    strcpy(DirNow, DirNow+RootLen);
    for (a=0;DirNow[a];a++){
        if (DirNow[a] == '\\') DirNow[a] = '/';
    }
    
    // If dir length is zero, we are at root, but that is indicated by '/'
    if (DirNow[0] == 0){
        strcpy(DirNow, "/");
    }

    return DirNow;
}

//------------------------------------------------------------------------------------
// authentication
//------------------------------------------------------------------------------------
FRESULT FindFaceTemplate(int index, char *filename)
{
  FRESULT res;
  FILINFO fno;
  DIR  dp;
  int i;
  
  if (index < 0) {
    res = FR_INVALID_PARAMETER;
    return (res);
  }
  
  memset(&fno, 0, sizeof(fno));
  res = f_opendir(&dp, AUTH_FACE_PATH);
  if (res == FR_OK) {
    for (i=0; i<= index;) {
      res = f_readdir(&dp, &fno);
      if ((res != FR_OK) || (fno.fname[0] == 0)) {
        res = FR_NO_FILE;
        break;
      }
      if ((!strcmp(fno.fname, ".")) ||  (!strcmp(fno.fname, ".."))) {
       continue;
      }
      if (i == index) {
        if (filename) {
          strcpy(filename, AUTH_FACE_PATH);
          strcat(filename, "/");
          strcat(filename, fno.fname);
        }
      }
      i++;
    }
    f_closedir(&dp);
  }
  return (res);
}

FRESULT FindCodeTemplate(int index, char *filename)
{
  FRESULT res;
  FILINFO fno;
  DIR  dp;
  int i;
  
  if (index < 0) {
    res = FR_INVALID_PARAMETER;
    return (res);
  }
  
  memset(&fno, 0, sizeof(fno));
  res = f_opendir(&dp, AUTH_RECOVERY_CODE_PATH);
  if (res == FR_OK) {
    for (i=0; i<= index;) {
      res = f_readdir(&dp, &fno);
      if ((res != FR_OK) || (fno.fname[0] == 0)) {
        res = FR_NO_FILE;
        break;
      }
      if ((!strcmp(fno.fname, ".")) ||  (!strcmp(fno.fname, ".."))) {
       continue;
      }
      if (i == index) {
        if (filename) {
          strcpy(filename, AUTH_RECOVERY_CODE_PATH);
          strcat(filename, "/");
          strcat(filename, fno.fname);
        }
      }
      i++;
    }
    f_closedir(&dp);
  }
  return (res);
}

enum NStatusCodes AuthFaceMatch(const char * probefilename, const char * templfilename) 
{
  enum NStatusCodes nsc = NST_ERROR;
  FILINFO fno;
  FIL fp;
  FRESULT res;
  void * probe = NULL;
  int32_t probe_size;
  void * tmpl = NULL;
  int32_t tmpl_size;
  
  do {
    // Verify probe file presence and get size
    memset(&fno, 0, sizeof(fno));
    res = f_stat(probefilename, &fno);
    if (res != FR_OK) {
      break;
    }
    
    // Alloc memory for probe data
    probe = malloc(fno.fsize);
    if (probe == NULL) {
      break;
    }

    // load probe data
    res = f_open(&fp, probefilename, FA_READ | FA_OPEN_EXISTING);
    if (res != FR_OK) {
      break;
    }
    res = f_read(&fp, probe, fno.fsize, (UINT *)&probe_size);
    if ((res != FR_OK) || (probe_size != fno.fsize)) {
      f_close(&fp);
      break;
    }
    f_close(&fp);
    
    // Verify template file presence and get size
    res = f_stat(templfilename, &fno);
    if (res != FR_OK) {
      break;
    }
    
    // Alloc memory for template data
    tmpl = malloc(fno.fsize);
    if (tmpl == NULL) {
      break;
    }

    // load template data
    res = f_open(&fp, templfilename, FA_READ | FA_OPEN_EXISTING);
    if (res != FR_OK) {
      break;
    }
    res = f_read(&fp, tmpl, fno.fsize, (UINT *)&tmpl_size);
    if ((res != FR_OK) || (tmpl_size != fno.fsize)) {
      f_close(&fp);
      break;
    }
    f_close(&fp);
    
    nsc = NFaceMatch(probe, probe_size, tmpl, tmpl_size, Settings.Face_MatchThreshold);
  } while(0);
  
  // free memory
  if (probe) {
    free(probe);
  }
  if (tmpl) {
    free(tmpl);
  }
  
  return (nsc);
}

enum NStatusCodes AuthCodeMatch(const char * probefilename, const char * templfilename) 
{
  enum NStatusCodes nsc = NST_ERROR;
  FILINFO fno;
  FIL fp;
  FRESULT res;
  void * probe = NULL;
  int32_t probe_size;
  void * tmpl = NULL;
  int32_t tmpl_size;
  
  do {
    // Verify probe file presence and get size
    memset(&fno, 0, sizeof(fno));
    res = f_stat(probefilename, &fno);
    if (res != FR_OK) {
      break;
    }
    
    // Alloc memory for probe data
    probe = malloc(fno.fsize);
    if (probe == NULL) {
      break;
    }

    // load probe data
    res = f_open(&fp, probefilename, FA_READ | FA_OPEN_EXISTING);
    if (res != FR_OK) {
      break;
    }
    res = f_read(&fp, probe, fno.fsize, (UINT *)&probe_size);
    if ((res != FR_OK) || (probe_size != fno.fsize)) {
      f_close(&fp);
      break;
    }
    f_close(&fp);
    
    // Verify template file presence and get size
    res = f_stat(templfilename, &fno);
    if (res != FR_OK) {
      break;
    }
    
    // Alloc memory for template data
    tmpl = malloc(fno.fsize);
    if (tmpl == NULL) {
      break;
    }

    // load template data
    res = f_open(&fp, templfilename, FA_READ | FA_OPEN_EXISTING);
    if (res != FR_OK) {
      break;
    }
    res = f_read(&fp, tmpl, fno.fsize, (UINT *)&tmpl_size);
    if ((res != FR_OK) || (tmpl_size != fno.fsize)) {
      f_close(&fp);
      break;
    }
    f_close(&fp);
    
    //nsc = NFaceMatch(probe, probe_size, tmpl, tmpl_size, Settings.Face_MatchThreshold);
    if(probe_size != tmpl_size){
      break;
    }
    
    if(memcmp(probe, tmpl, tmpl_size)==0){
      nsc = NST_MATCH;
    } 
    
  } while(0);
  
  // free memory
  if (probe) {
    free(probe);
  }
  if (tmpl) {
    free(tmpl);
  }
  
  return (nsc);
}


enum NStatusCodes AuthCheckFaceForMatch(const char * probefilename) 
{
  enum NStatusCodes nsc = NST_ERROR;
  char templfilename[MAX_PATH];
  int i = 0;
  
  while (FindFaceTemplate(i, templfilename) == FR_OK) {
    nsc = AuthFaceMatch(probefilename, templfilename);
    if (nsc == NST_MATCH) {
      break;
    }
    i++;
  }
  return (nsc);
}

enum NStatusCodes AuthCheckCodeForMatch(const char * probefilename) 
{
  enum NStatusCodes nsc = NST_ERROR;
  char templfilename[MAX_PATH];
  int i = 0;
  
  while (FindCodeTemplate(i, templfilename) == FR_OK) {
    nsc = AuthCodeMatch(probefilename, templfilename);
    if (nsc == NST_MATCH) {
      break;
    }
    i++;
  }
  return (nsc);
}

FRESULT FindValidTemplate(void)
{
  FRESULT res;
  char templfilename[MAX_PATH];
  
  res = FindFaceTemplate(0, templfilename);
  
  if(res != FR_OK)      // no face template found... but maybe a Recovery code is present.
  {
    res = FindCodeTemplate(0, templfilename);
  }
  
  return (res);
}


