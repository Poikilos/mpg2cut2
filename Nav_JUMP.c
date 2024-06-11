

#include "global.h"
#include "Nav_JUMP.h"


void  Nav_Jump_Fwd(JUMPTBL *BwdTbl)
{
  
  if  (BwdTbl->i64Loc[BwdTbl->ix]  <  process.CurrLoc
  ||   BwdTbl->iFile [BwdTbl->ix]  <  process.CurrFile)
  {
      //  Push  onto  circular  Location  LIFO  stack

          if (BwdTbl->ix  <  JUMPTBL_MAX)
              BwdTbl->ix++;
          else
              BwdTbl->ix = 0;

          BwdTbl->i64Loc[BwdTbl->ix] =  process.CurrLoc;
          BwdTbl->iFile [BwdTbl->ix] =  process.CurrFile;

          if  (BwdTbl->ix  ==  BwdTbl->iOrg)
          {
              if  (BwdTbl->iOrg  <  JUMPTBL_MAX)
                   BwdTbl->iOrg++;
              else
              {
                   BwdTbl->iOrg = 0;
              }
          }
          else
          {
          }



  }  //  ENDIF  Location  has  progressed
  else
  {
            // BUG IF WE ARRIVE HERE ?
  }
}



//-----------------------------------------------------



void Nav_Jump_BWD(JUMPTBL *BwdTbl)
{
    int   iFile1;
  __int64 i64Loc1;

          //  Pop  from  circular  Location  LIFO  stack  if  OK

          if  (BwdTbl->ix  !=  BwdTbl->iOrg)
          {
            i64Loc1 = BwdTbl->i64Loc[BwdTbl->ix];
            iFile1  = BwdTbl->iFile [BwdTbl->ix];

            if (i64Loc1 < 1 && iFile1 < 1)
            {
                //  breaks  the  chain  -  reset  LIFO  stack;
                BwdTbl->ix  =  0; BwdTbl->iOrg  =  0;
            }
            else
            {
              if  (iCTL_FastBack)
              {
                 //  If  we  have  been  single  stepping,
                 //  then  may  need  to  return  to  start  of  CURRENT  gop
                 //
                 //  if  (MPEG_Pic_Type  >  I_TYPE  &&  MPEG_Pic_Structure  ==  FULL_FRAME_PIC
                 //                            &&  iCTL_FastBack)
                 //          process.Action  =  ACTION_NEW_CURRLOC;

                 process.CurrLoc   =  i64Loc1;
                 process.CurrFile  =  iFile1;
                 process.Action    =  ACTION_NEW_CURRLOC;
              }

              if (BwdTbl->ix  >  0)
                  BwdTbl->ix--;
              else
                  BwdTbl->ix  =  JUMPTBL_MAX;
            }
          }
}



