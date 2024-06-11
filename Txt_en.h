

// This is the ENGLISH version of the text messages.

// If you want to translate into a new language
// create a NEW header file based on this one.
//      Example:-  For French, create TXT_FR.h

// Change each of the the "quoted text" values into the new language
// BUT leave the SYMBOLIC_NAMES as they are
//      Example:-  #define FILE_CLOSE   "Fermé."

// BE CAREFUL TO RETAIN THE SAME NUMBER OF "%" SIGNS IN EACH MESSAGE

// When finished, change "TXT.H" to point to your new file, 
// instead of the original English one.
//       Example:-  #include <TXT_FR.h>


#define SAVE_CLIPS_BEFORE_DELETE "CLIPS NEED TO BE SAVED or cleared\n\nBEFORE FILE DELETE"

#define DELETE_ALL_FILES "DELETE ALL FILES"

#define Mpg2Cut2_SORRY   "Mpg2Cut2 - SORRY"
#define Mpg2Cut2_WARNING "Mpg2Cut2 - Warning"
#define Mpg2Cut2_ERROR   "Mpg2Cut2 - ERROR"
#define Mpg2Cut2_FILE_ERROR "Mpg2Cut2 - FILE ERROR"

#define UNMUX_NOT_SUPPORTED_TS "Unmux Not Supported on TS files.\n\nTry PVAStrumento or ProjectX instead."

#define INCLUDE_TO_NOT_VOB "Note: The \"Include TO frame\" option\n may not retain VOB compatibility"

#define IS_NORMAL  "NORMAL"

#define FILE_OPEN   "OPEN"
#define FILE_SEEK   "SEEK"
#define FILE_READ   "READ"
#define FILE_WRITE  "WRITE"
#define FILE_UPDATE "UPDATE"
#define FILE_CLOSE  "CLOSE"

#define FILE_LOCKED             "File currently LOCKED by another task"
#define FILE_BAD_SECTOR         "BAD DISK SECTOR - Permanent Error"
#define FILE_PATH_NOT_FOUND     "Path or File not found"
#define FILE_PERMISSION_DENIED  "Permission Denied"
#define FILE_BAD_NUMBER         " - Bad File Number"
#define FILE_BAD_NAME_FMT       "Illegal File Name format"
#define FILE_RENAME_FAILED      "Rename failed RC(%02d)  ErrNo(%02d)"

#define FILE_SORRY_ERROR "Sorry%s\n%s\n\n%s File %s ERROR\nrc=x%02X - x%02X  Caller=%d\n\n%s"
#define FILE_TOO_SMALL   "FILE TOO SMALL\n\nLength = %d\n\n%s"
#define FILE_TOTAL_COUNT   "Total %d files"
#define FILE_EXCLUDE_COUNT "Excluded %d files"
#define FILE_END_OF_FILE "END OF FILE"

#define FILE_XTN_QRY  "Wrong File Ext: %s  %s\n\nTHIS PROBABLY WON'T WORK\n\nDO YOU REALLY WANT TO TRY THIS ?"

#define ILLEGAL_CHR_TXT "ILLEGAL CHARACTER %c"

#define NON_Mpeg2_PS    "NON-Mpeg2 PS format data\nLoc=%d\n\n%s - NOT FULLY SUPPORTED"
#define NOT_MPEG2_BRIEF "NOT MPEG2 PS"
#define NO_MPEG_PACK_SEQ_AT_START "NO MPEG PACK/SEQ header at start.\n\n%s"

#define PARM_UNKNOWN_KW     "Unknown Parameter Keyword \"%s\" "

#define MEM_ALLOC_ERROR     "Memory Alloc error - Proc #%s"

#define OUTPUT_IN_PROGESS   "OUTPUT IN PROGRESS...  %d,%d"

#define VOLUME_LIMITING     "VOLUME LIMITING  "
#define VOLUME_AT_MAX       "VOLUME AT MAX    "

#define AUDIO_NO_OTHER_TRK  "NO OTHER AUDIO TRK"

#define EXE_PGM "EXECUTABLE PROGRAM"


