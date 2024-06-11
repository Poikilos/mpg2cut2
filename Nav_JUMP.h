
// FastBack Jump Tables

#define JUMPTBL_MAX 127

typedef struct {
  __int64  i64Loc[JUMPTBL_MAX+1];  // Save previous location for skip back
    int     iFile[JUMPTBL_MAX+1];  // Save previous loc's file for skip back
    int    ix, iOrg;
} JUMPTBL;    // FastBack controls

JUMPTBL BwdGop;
JUMPTBL BwdFast1;

void  Nav_Jump_Fwd(JUMPTBL *);
void  Nav_Jump_BWD(JUMPTBL *);


