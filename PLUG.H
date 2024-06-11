
// PLUGH - Y2

char szRenamePlugIn_Name[_MAX_PATH];
char cRenamePlugIn_MultiMode;
char cRenamePlugIn_AsyncMode;
char szFileListPlugIn_Name[_MAX_PATH];
char cFileListPlugIn_MultiMode;
char cFileListPlugIn_AsyncMode;


typedef struct
{
  char    szPathFile[_MAX_PATH];   
  HMODULE hDll;   
  int     iStatus, iActive;
  FARPROC fpInit;
  FARPROC fpProc;
  FARPROC fpFinal;
} PLUGIN;

PLUGIN  PlugFileList;   
PLUGIN  PlugFileRename;

FARPROC Plug81_Load_DLL(HMODULE *, FARPROC *, char *);
int Plug89_Free_DLL(HMODULE *);
