
int DVD_PlugIn_Flag;
int KeyOp_Flag;
int lfsr0, lfsr1;

#define   KEY_OFF   0
#define   KEY_INPUT 1
#define   KEY_OP    2

typedef __int64 (WINAPI *pfnKeyOp) (int, char*[], HWND);

pfnKeyOp KeyOp;

typedef void (WINAPI *pfnBufferOp) (unsigned char*, int, int);

pfnBufferOp BufferOp;
