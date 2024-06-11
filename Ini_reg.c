
// System Register 

#include "global.h" 


// File Asociation code supplied by WeWantWidescreen

// Replaces .reg.txt file
// TODO: might need quotes around szEXE, confirmation msgbox, rename function

void Ini_Associate()
{

  const char szEDLfile[]="EDLfile";

  HKEY mru;
  DWORD uDisp;
  char cTST;
  char szEXE[MAX_PATH];

  int iRC, iLen, iLen2;


  int iType_ix, iVerb_ix, iVerb_Start_ix;

  char *lsFileXTN[]={  // Mpeg Streams
                             ".MPG", ".MPEG",
                             ".VOB", 
                             ".M2P",  ".M2V", 
                             ".M1P",  ".M1V",
                             ".TS",   ".M2T",
                             "MPEGfile",    "Mpeg_Auto_File", 
                             "VOBFILE",     "VOB_Auto_File",
                             "PDVDmpgfile",
                             "-",
                       // Other things
                             ".EDL",
                             "EDLfile",      "EDL_Auto_File",
                             ".001",  ".002", ".003", ".004", 
                             ".GetRight", 
                             ".PART",
                             0};
  /*
  char *lsFileType[]={
                      0};
  */
  char *lsVerb[]={"Edit",     "Play_Mpg2Cut2",  "Mpg2Cut2", 0};
  char *lsQual[]={"",         "/play",          "",         0};



    //char MPGStr[32];
    DWORD uLen3 = 0;
 //   DWORD uType;


  GetModuleFileName(NULL,  szTMPname, sizeof(szTMPname));
  // Wrap quotes around it - allows for embedded spaces
  sprintf(szEXE, "\"%s\"", szTMPname);  

  //lstrcat(szEXE, " %1");

  iLen = sprintf(szBuffer, "%s    \"%%1\" ",  szEXE);
  iLen++;

  sprintf(szMsgTxt, "This will register Mpg2Cut2 with Windows Explorer\nas the EDIT function for MPEG file types\n\n%s",
                    szBuffer);

  iRC = MessageBox(hWnd_MAIN, szMsgTxt,  szAppName, MB_OKCANCEL);
  if (iRC != IDOK)
      return;           

  
/*
  // filetype for .mpg
  if(RegCreateKeyEx(HKEY_CLASSES_ROOT, ".mpg", 0, "", 0,
     KEY_ALL_ACCESS, NULL, &mru, &uDisp) == ERROR_SUCCESS)
  {
     if(uDisp == REG_CREATED_NEW_KEY)
     {
         RegSetValueEx(mru,"",0,REG_SZ, (BYTE*)lsFileType[0], 9); 
     }
     else
     if(uDisp == REG_OPENED_EXISTING_KEY)
     {
         uLen3=sizeof(szTmp80);
         iRC = RegQueryValueEx(mru,"",NULL,&uType,(LPBYTE)&szTmp80,&uLen3);
         MessageBox(0,szTmp80,"",0); //dbg
         if (iRC == ERROR_SUCCESS)
             lsFileType[4] = &szTmp80[0];

     }

     RegCloseKey(mru);
  }
*/

  iVerb_Start_ix = 0;
  
  for (iType_ix = 0; lsFileXTN[iType_ix] != 0; iType_ix++)
  {
    cTST = *lsFileXTN[iType_ix];
    if (cTST == '-')
        iVerb_Start_ix = 2; // Skip "Edit" and "Play" for non-mpeg types
    else
    {
      iVerb_ix = iVerb_Start_ix;

      for (; lsVerb[iVerb_ix] != 0; iVerb_ix++)
      {
        sprintf(szTemp, "%s\\shell\\%s\\command",
                    lsFileXTN[iType_ix], lsVerb[iVerb_ix]);

        iLen2 = sprintf(szMsgTxt, "%s %s ", szBuffer, lsQual[iVerb_ix]);
        iLen2++;

        
        if(RegCreateKeyEx(HKEY_CLASSES_ROOT, 
                          szTemp
                          ,0, "", 0, KEY_ALL_ACCESS, NULL, &mru, &uDisp)
           == ERROR_SUCCESS)
        {
            if (DBGflag)
                MessageBox(0,szTemp,"",0); //dbg

            RegSetValueEx(mru,"",0,REG_SZ, (BYTE*)&szMsgTxt[0], iLen2); 
            RegCloseKey(mru);
        }
        
      }
    }

  }


  
/*    
  if(RegCreateKeyEx(HKEY_CLASSES_ROOT, 
      "MPEGFILE\\shell\\Edit\\command",0, "", 0, 
      KEY_ALL_ACCESS, NULL, &mru, &uDisp) == ERROR_SUCCESS)
  {
      RegSetValueEx(mru,"",0,REG_SZ,(BYTE*)&szBuffer,lstrlen(szBuffer)+1);    
      RegCloseKey(mru);
  }

  if(RegCreateKeyEx(HKEY_CLASSES_ROOT, 
      "MPEGFILE\\shell\\Play_Mpg2Cut2\\command",0, "", 0,
      KEY_ALL_ACCESS, NULL, &mru, &uDisp) == ERROR_SUCCESS)
  {
      iLen2 = sprintf(szMsgTxt, "%s /play ", szBuffer);
      iLen2++;
      RegSetValueEx(mru,"",0,REG_SZ, (BYTE*)&szMsgTxt[0], iLen2); 
      RegCloseKey(mru);
  }


  if(RegCreateKeyEx(HKEY_CLASSES_ROOT, 
      "VOBFILE\\shell\\Mpg2Cut2\\command",0, "", 0, 
      KEY_ALL_ACCESS, NULL, &mru, &uDisp) == ERROR_SUCCESS)
  {
      RegSetValueEx(mru,"",0,REG_SZ,(BYTE*)&szBuffer,lstrlen(szBuffer)+1);    
      RegCloseKey(mru);
  }

  // POWER DVD
  if(RegCreateKeyEx(HKEY_CLASSES_ROOT, 
      "PDVDmpgfile\\shell\\Edit\\command",0, "", 0, 
      KEY_ALL_ACCESS, NULL, &mru, &uDisp) == ERROR_SUCCESS)
  {
      RegSetValueEx(mru,"",0,REG_SZ,(BYTE*)&szBuffer,lstrlen(szBuffer)+1);    
      RegCloseKey(mru);
  }

  if(RegCreateKeyEx(HKEY_CLASSES_ROOT, 
      "PDVDmpgfile\\shell\\Play_Mpg2Cut2\\command",0, "", 0, 
      KEY_ALL_ACCESS, NULL, &mru, &uDisp) == ERROR_SUCCESS)
  {
      iLen2 = sprintf(szMsgTxt, "%s /play ", szBuffer);
      iLen2++;
      RegSetValueEx(mru,"",0,REG_SZ, (BYTE*)&szMsgTxt[0], iLen2); 
      RegCloseKey(mru);
  }
  

  //-----------

  if(RegCreateKeyEx(HKEY_CLASSES_ROOT, ".EDL",0, "", 0, KEY_ALL_ACCESS, NULL, &mru, &uDisp) == ERROR_SUCCESS)
  {
     RegSetValueEx(mru,"",0,REG_SZ,(BYTE*)&szEDLfile,lstrlen(szEDLfile)+1);    
     RegCloseKey(mru);
  }

  if(RegCreateKeyEx(HKEY_CLASSES_ROOT, "EDLfile",0, "", 0, KEY_ALL_ACCESS, NULL, &mru, &uDisp) == ERROR_SUCCESS)
  {
     DWORD dwRegValue=0;
     RegSetValueEx(mru,"EditFlags",0,REG_BINARY,(BYTE*)&dwRegValue,4);    
     RegCloseKey(mru);
  }


  if(RegCreateKeyEx(HKEY_CLASSES_ROOT, 
                    "EDLfile\\shell\\Mpg2Cut2",0, "", 0, 
                    KEY_ALL_ACCESS, NULL, &mru, &uDisp) == ERROR_SUCCESS)
  {
     DWORD dwRegValue=1;
     RegSetValueEx(mru,"EditFlags",0,REG_BINARY,(BYTE*)&dwRegValue,4);    
     RegCloseKey(mru);
  }

  if(RegCreateKeyEx(HKEY_CLASSES_ROOT, 
                    "EDLfile\\shell\\Mpg2Cut2\\command",0, "", 0, 
                    KEY_ALL_ACCESS, NULL, &mru, &uDisp) == ERROR_SUCCESS)
  {
     RegSetValueEx(mru,"",0,REG_SZ,(BYTE*)&szBuffer,lstrlen(szBuffer)+1);    
     RegCloseKey(mru);
  }


  // DefaultIcon
  //GetModuleFileName(NULL, szEXE, sizeof(szEXE));
  lstrcat(szEXE,",0");
*/



  /*
  if(RegCreateKeyEx(HKEY_CLASSES_ROOT, 
                    "MPEGFILE\\DefaultIcon",0, "", 0, 
                    KEY_ALL_ACCESS, NULL, &mru, &uDisp) == ERROR_SUCCESS)
  {
        RegSetValueEx(mru,"",0,REG_SZ,(BYTE*)&szEXE,lstrlen(szEXE)+1);    
        RegCloseKey(mru);
  }
  */
    
    
  if(RegCreateKeyEx(HKEY_CLASSES_ROOT, 
                    ".EDL\\DefaultIcon",0, "", 0, 
                    KEY_ALL_ACCESS, NULL, &mru, &uDisp) == ERROR_SUCCESS)
  {
        RegSetValueEx(mru,"",0,REG_SZ, (BYTE*)&szEXE, (lstrlen(szEXE)+1));    
        RegCloseKey(mru);
  }

  if(RegCreateKeyEx(HKEY_CLASSES_ROOT, 
                    "EDLfile\\DefaultIcon",0, "", 0, 
                    KEY_ALL_ACCESS, NULL, &mru, &uDisp) == ERROR_SUCCESS)
  {
        RegSetValueEx(mru,"",0,REG_SZ, (BYTE*)&szEXE, (lstrlen(szEXE)+1));    
        RegCloseKey(mru);
  }
}


//------------------------------------------------------------


/*

void Reg_GetValues()
{
  unsigned char *REG_TOP_KEY = &"Software\\RocketJet4\\Mpg2Cut2";


LONG RegCreateKeyEx(

    HKEY  hKey,	// handle of an open key 
    LPCTSTR  lpszSubKey,	// address of subkey name 
    DWORD  dwReserved,	// reserved 
    LPTSTR  lpszClass,	// address of class string 
    DWORD  fdwOptions,	// special options flag 
    REGSAM  samDesired,	// desired security access 
    LPSECURITY_ATTRIBUTES  lpSecurityAttributes,	// address of key security structure 
    PHKEY  phkResult,	// address of buffer for opened handle  
    LPDWORD  lpdwuDisp 	// address of uDisp value buffer 
   );

/*
	
Parameters

hKey

   Identifies a currently open SUPERIOR key 
   OR any of the following predefined reserved handle values: 

        HKEY_CLASSES_ROOT
        HKEY_CURRENT_USER
        HKEY_LOCAL_MACHINE
        HKEY_USERS

   The key opened or created by the RegCreateKeyEx function 
   is a subkey of the key identified by the hKey parameter.
 

lpszSubKey

   Points to a null-terminated string 
   specifying the name of a subkey 
   that this function opens or creates. 
   The subkey specified must be a subkey 
   of the key identified by the hKey parameter. 
   This subkey must not begin with the backslash character ('\'). 
   This parameter cannot be NULL. 


dwReserved

   Reserved; must be zero. 

lpszClass

   Points to a null-terminated string 
   that specifies the class (object type) of this key.
   This parameter is ignored if the key already exists. 

fdwOptions

   Specifies special options for the key. 
   Currently, this parameter can be one of the following values: 

   Value	Meaning

   REG_OPTION_VOLATILE
   
          WinNT: This key is volatile; 
                 the information is stored in memory 
                 and is NOT preserved when the system is restarted. 
                 The RegSaveKey function does not save volatile keys.
          Win95: This value is ignored in Windows 95. 
                 Even if REG_OPTION_VOLATILE is specified, 
                 the RegCreateKeyEx function creates a nonvolatile key 
                 and returns ERROR_SUCCESS.

   REG_OPTION_NON_VOLATILE
   
         This key is not volatile; 
         the information is stored in a file 
         and IS preserved when the system is restarted. 
         The RegSaveKey function saves keys that are not volatile.

   By default, keys are not volatile. 
   This option is ignored if the key already exists. 

   The RegSaveKey function only saves nonvolatile keys. 
   It does not save volatile keys. 


samDesired

   Specifies an access mask that specifies the desired security access for the new key. This parameter can be a combination of the following values: 

   Value	                Meaning

   KEY_CREATE_LINK	      Permission to  Create a symbolic link.
   KEY_CREATE_SUB_KEY	    Permission to  Create subkeys.
   KEY_ENUMERATE_SUB_KEYS	Permission to  Enumerate subkeys.
   KEY_EXECUTE           	Permission for Read access.
   KEY_NOTIFY             Permission for Change Notification.
   KEY_QUERY_VALUE       	Permission to  Query Subkey Data.
   KEY_READ	              Combination of KEY_QUERY_VALUE, 
                                         KEY_ENUMERATE_SUB_KEYS, and 
                                         KEY_NOTIFY access.
   KEY_SET_VALUE	        Permission to set subkey data.
   KEY_WRITE	            Combination of KEY_SET_VALUE and 
                                         KEY_CREATE_SUB_KEY access.
   KEY_ALL_ACCESS	        Combination of KEY_QUERY_VALUE, 
                                         KEY_ENUMERATE_SUB_KEYS, 
                                         KEY_NOTIFY, 
                                         KEY_CREATE_SUB_KEY, 
                                         KEY_CREATE_LINK, 
                                         KEY_SET_VALUE access.



lpSecurityAttributes

   Points to a SECURITY_ATTRIBUTES structure 
   that specifies the security attributes for the new key.
   
   If this parameter is NULL, 
   the key receives a default security descriptor. 

   If the operating system does not support security, 
   this parameter is ignored. 


phkResult

   Points to a variable that receives the handle 
   of the opened or created key.
   

lpdwuDisp

   Points to a variable that receives 
   one of the following uDisp values: 

   Value	Meaning
   REG_CREATED_NEW_KEY	    The key did not exist and was created.
   REG_OPENED_EXISTING_KEY	The key existed and was simply opened 
                            without being changed.

Return Value

   If the function succeeds, the return value is ERROR_SUCCESS.
   If the function fails, the return value is an error value.


Remarks

   The key that the RegCreateKeyEx function creates has no values. 
   Use the RegSetValue or RegSetValueEx function to set key values. 

   The key identified by the hKey parameter must have been opened 
   with KEY_CREATE_SUB_KEY access. 
   
   To open the key, use the RegCreateKeyEx or RegOpenKeyEx function.
   
   An application cannot create a key under HKEY_USERS or HKEY_MACHINE. 

   An application can use RegCreateKeyEx to temporarily lock 
   a portion of the registry. 
   When the locking process creates a new key, 
   it receives the uDisp value REG_CREATED_NEW_KEY, 
   indicating that it "owns" the lock. 
   Another process attempting to create the same key 
   receives the uDisp value REG_OPENED_EXISTING_KEY, 
   indicating that another process already owns the lock. 

See Also

   RegCreateKey, RegDeleteKey, RegOpenKey, RegOpenKeyEx, 
   RegSaveKey, SECURITY_ATTRIBUTES 

*/



//  The RegOpenKeyEx function opens the specified key. 

/*
LONG RegOpenKeyEx(

    HKEY  hKey,	// handle of open key 
    LPCTSTR  lpszSubKey,	// address of name of subkey to open 
    DWORD  dwReserved,	// reserved 
    REGSAM  samDesired,	// security access mask 
    PHKEY  phkResult 	// address of handle of open key 
   );	


  // RUN_CTR
  // If this is the first time that we have run in Windows VISTA
  // put up a warning message suggesting 
  //    View - Technical - Never Use Overlay

  // If there are values for video settings,
  //    then use them
  // Else save the ones from the INI file into the Registry.

  // YUV \ADJUST, 
  //     \LOCKBC, \GAMMA, \BRIGHTNESS, \CONTRAST,
  //     \LOCKUV, \SAT, \U, \V,  
  //     \NOTIFY, \NEVER
  // RGB \ADJUST,
  //     \LOCKBC, \GAMMA, \BRIGHTNESS, \CONTRAST,
  //     \LOCKUV, \SAT, \U, \V,  



  RegCloseKey(HKEY  hKey);

}

*/

