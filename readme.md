
MPG2CUT2 - CONTROLS
===================

The keyboard is the fastest, most flexible way to navigate,
using the cursor arrow keys, as they have Auto-Repeat.
Auto-Repeat allows fast visual search,
by holding down the appropriate arrow key.

But if you prefer to use the mouse, then you can
navigate along the time line using either the slider
or the toolbar buttons. 

TOOLBAR BUTTONS :-

B = BMP Snapshot of current frame.
+ = Add selection to Clip List
L = Luminance Adjust 
      Alt-L = Toggle Lumniance Adjust on/off
    Shift-L = luminance setting Bold
     Ctrl-L = luminance setting Default
Z = Zoom

�  = Stop
/  = Forward Single Frame
p- = Play Slow
P  = Play
p+ = Play Fast
p* = Play Extra Fast (no sound)



 [ = Mark "IN" point (START of clip)
<< = Jump Back 
 < = Move back 1 GOP
>> = Jump Forward 
 ? = Move forward 1 GOP
 ] = Mark "OUT" point (END of clip)



KEYBOARD COMMANDS :-

The keyboard is the fastest way to use the program.

To allow for people with different habits,
there are 2 different configurations for keyboard commands:

   1) Classic Mpeg2Cut

   2) VirtualDub compatible (well... sorta)

         Not quite, because VDub has frame level navigation,
         while Mpg2cut2 is GOP oriented.



    Arrow keys navigate along time line

	ACTION               CLASSIC       VIRTUALDUB

	Forward one GOP       UP            RIGHT
	Back    one GOP       DOWN          LEFT 
	Back a lot            LEFT          UP
	Forward a lot         RIGHT         DOWN
	Next File Start       Ctrl RIGHT
	Previous File Start   Ctrl LEFT	

	SHIFT - Magnifies each of the above keys

      PAGE UP    = Backward quite a lot
      PAGE DOWN  = Forward  quite a lot


	Forward 1 Frame       /     (NB. NO MARKING in the middle of a GOP)

	GO TO Clip#           0..9

	Mark "IN"             [             Shift [
	Mark "OUT"            ]             Shift ]
	Go to "IN"            Shift [           [
	Go to "OUT"           Shift ]           ]
	ADD clip              +


	Open file         F3 Ctrl-O
	Add file          alt-O       
	Save clips        F4 Ctrl-S
	Save This clip    Ctrl_T
	Play              F5 P  [INS]
      Slow Play         F9
      Fast Play         Shift-F9
      CUE               * [On the numeric keypad]
      Preview SELECTION F8
      Preview END Sel   ALT-F8 (Plays last 3 seconds of current selection.)
      Preview ALL Clips SHIFT-F8
	Pause Play        Space
	Stop Play         ESC

	BMP Snapshot      B or Shift-B  (result dependant on option setting)
      BMP to ClipBoard  C or Ctrl-C

      Buttons & Scrollbar  F11
      Scrollbar Only       F12
      Buttons   Only       Alt-F12

	L = Luminance Boost 
      M = Mute
      K = Karaoke
      V = Volume Boost.  Alt-V = Reduce volume. 
                       Shift-V = Bolder AVC Boost
                        Ctrl-V = Volume Sliders
      A = Audio Track Change (Cycle around Audio Tracks)
      U = Swap UV on display. (Turn Blue People to Red)

Numeric-Pad
      Ctrl + = Volume UP
      Ctrl - = Volume DOWN



IBM 3270 and other extended Keyboards
      Clear = Escape = Pause
      Erase EOF = Mark "OUT" point (END of clip)
      Zoom = Zoom
      Play = Play From Here
      
TOGGLES
-------
      F11 = Toolbar Buttons 
  Alt-F11 = Scrollbar
Shift-F11 = Both Buttons and Scrollbar

      F6  = Stats Screen


NOTE:- PAUSE (space bar) is different to STOP (esc key).
       Pause simply freezes the decoder mid-stream.
       STOP actually shuts down the decoder task.

==========================================================================







	Mpg2Cut2 - http://www.geocities.com/rocketjet4/




MPG2CUT2 - FAULT TOLERANT, GOP LEVEL, BINARY CUTTER 
         - FOR MPEG-2 PROGRAMS STREAMS 
         - ESPECIALLY WITH DIGITAL TELEVISION (DTV) CAPTURES





         *** DEVELOPMENT VERSION ***

         *** USE AT YOUR OWN RISK ***

         Remember to do a virus check


--------------------------------------------------------------------



APPLICATION DESCRIPTION
=======================

Mpg2Cut2 - Enhancement of Mpeg2Cut v 1.6 - based on DVD2AVI 

Copies selected portions of an MPEG-2 Program Stream.

Mpg2Cut2 can only edit Mpeg-2 or Mpeg-1 - NOT Mpeg-4.
Mpeg-1 is only partially supported.

If you have a transport stream, then it is preferable to
convert it to a Program Stream using PVAStrumento.

Ability to correct a small range of errors in the program stream,
such as missing headers or incorrect aspect ratio.

User tailorable reminder messages.



USAGE
=====

The basic approach is to mark the "FROM" and "TO" points 
of the clip that you want to keep, 
and then save that selection to a new file.  

Repeat this for each clip that you want.

If you are working with Digital Television files,
run each of the saved clips through a correction program,
such as ProjectX or PVAStrumento.

Drag and Drop the resulting clips into your DVD Authoring package.
This way each clip becomes a separate scene, 
with it's own thumbnail on the DVD menu.

(Your DVD Authoring package may be different to this.)



ALTERNATE APPROACH
------------------

If you are NOT planning on using the clips as individual DVD scenes,
you can perform multiple clip selections before saving them ALL together
as a SINGLE output file.

BUT, at this stage the joining of clips is very crude,
so this approach MAY NOT work in your environment.
Some players do not like the rough joins.

You will probably need to use either "ProjectX" or "PVAStumento"
to change the output file to be compatible with non-DTV applications.

SO USE THIS FEATURE WITH CAUTION !

See notes below regarding Multi-Clip output.


-----------------------------------------------------------------

FEATURES
--------

Detects and attempts to fix missing Pack and System headers
when creating the output file.


Does *NOT* fully support MPEG-1 format.  NOR Transport Streams.
You MAY be able to view these unsupported formats,
you MAY be able to save a single selection cut,
but DO NOT assume that unsupported formats will be usable.
ALWAYS check the results.


There is NO decryption capability,
it will only work on non-encrypted files.
So if you are trying to use it on a commercial DVD,
and just get rubbish out, don't be surprised.

There is an option to output as Demuxed Elementary Streams,
but there is no recompression.  The output is still essentially the same underlying Mpeg codec, only the container has changed.

Ditto for CDXA RIFF wrapped Mpeg files.  
The RIFF container is stripped away,
but there is no change to the nderlying Mpeg data.

This is NOT a converter.  It cannot output to AVI or other formats.

There are some adjustments to control information,
the internal Mpeg data itself is just grabbed directly from the input.

Naming a file as a VOB will *NOT* convert it a a DVD compatible file.
Use other utilities like "PVASTRUMENTO" or "PROJECT X" for that.



------------------------------------------

Mpg2Cut2 is essentially a GOP level editor.

A GOP is a "Group Of Pictures", which is usually around 13 frames,
lasting around half a second when played. 
This typically corresponds to the minimum playable Video "Sequence".

Selection is generally made at the GOP level,
so accuracy is typically limited to about half a second of time,
in the typical situation of "SEQ/GOP/KEY-FRAME/DELTA-FRAMES".

The program allows for Sequences that do not have a GOP header,
but the sequence must at least begin with a key frame.

The program does NOT check for "Open GOPs",
so you can get spoilage of the first and last GOP in a selection.
The impact of this can be minimized by enabling the menu option:
                 "Include TO frame"
which will extend the clip to include the terminating Key frame.
HOWEVER, this decreases compatibility with set-top DVD players.

The program has only LIMITED support for compensating audio delay 
relative to the video stream, and is a controlled via
Menu Output options:
            - "PARSE ENABLED" 
            - "PARSING OPTIONS" - "AUDIO MATCHING"
You need both these options on for audio adjustment to take place.

I also recommend :-
            - "PARSING OPTIONS" - "DEEP PARSING"
            - "PARSING OPTIONS" - "ALIGN VIDEO"
            - "PARSING OPTIONS" - "ALIGN AUDIO"


As this feature is experimental, 
I suggest you leave some extra fat 
at the start and end of your edit selection, 
to avoid losing some audio.

The Audio delay, which is shown in milliseconds (ms), 
typically will be of the order of 20-500ms for DVDs, 
but over 1 second is not unusal in DTV,
especially on High Definition files.

So add AT LEAST one GOP at the end of the clip selection.
For safety, , especially on HD streams, add 3 GOPs.

Also remember that change of scene is NOT likely to coincide with a GOP 
boundary, so you should also add a padding GOP at the start of the clip.
I.E. Start the selection BEFORE the scene change.

Also, PVAStrumento tends to drop the first GOP of a clip,
so it is safer to have an extra GOP at the start too.

These are rules of thumb - adjust based on your own experience

--------------------------------------------------------------------

**** CURRENT KNOWN PROBLEMS :-

*1:  Joins are very rough, when saving multiple clips into a single file.

    To allow for the possibility of rough joins,
    I STRONGLY suggest adding an extra GOP or two
    at the start and end of each clip.

    PVAStrumento tends to drop GOPs at the start and end,
    so all the more reason to leave on some extra. 

    Options are available to make the join smoother.
    (See "Output Controls" below).
   
    Audio sync problems after joins, on some players.
    Try using the Parsing option: "Match Audio".

    Multiple selection ranges basically works, at least in MY environment.
    It MAY NOT be acceptable to YOUR system.
    If you have problems with the output, pass it through a clean-up
    utility such as PVAStrumento or ProjectX.
    
    Those clean-up utilities adjust Mpeg files into a format required for
    DVD standard VOB files, or convert to elementary streams for input to
    DVD authoring software.

    If this doesn't fix problems arising from multiple selections,
    then save each selection to a SEPARATE file, 
    and use a different utility to perform the joins. 

    As mentioned above, in some DVD Authoring packages,
    you may find the the easiest way to create chapters,
    is to save each clip SEPARATELY.
    This way each clip becomes a separate chapter.

    Another alternative is to output as Un-muxed elementary files.
    This is for software that expects video and audio to be stored
    in separate files.  [.M2V=Video .M2A=Audio]

    [Techical note: Smoother joins are more likely with files encoded with 
    Closed GOPs and with Sequence Headers aligned to Packets.]





LESSER ISSUES:

  - Multiple input files - not fully tested.
    It is quite usable if you are not too fussy.

  - Preview function is a bit rough.

    Mpg2Cut2 is not intended to be used as a player.
    No attempt is made to provide audio lip-sync during preview.
       (Maybe one day though....)
      
    Also the preview of Private Stream 2 Audio is unreliable.
    All I am doing is chucking the raw data at the audio interface.
    The fact that ANY PS2 sound sometimes comes out during preview 
    is just fortuitious.

  - "BACK" command sometimes stops.
    In this situation, either use the key combination Shift+Back,
    or use the Scrollbar to scroll back.
    This problem has been reduced, but may still occur.

  - "BACK" command sometimes skips an extra GOP.
    Trade-off against the problem, still working on this.


------------------------------------------------------------------

MINIMUM REQUIREMENTS
====================

Windows 98 SE or later

	Win98SE is what I am running on. 
      Win2K - seems to run OK (maybe faster ?).

	I expect it would work on other modern versions of Windows...


Celeron or better CPU

	Editing:
		The basic editing function does not use much CPU,
		so you could get away with using a very basic machine.

		I use a P-III 800 Mhz and this is plenty fast enough
            for editing Standard Definition files. 

            For High Definition, 800 Mhz is OK for editing, 
            but maybe a little slow.


	Previewing:
		The preview function chews up a lot more CPU than editing does.

            600Mhz P3 would probably be minimum for previewing a 
            Standard Definition (SD) file without dropping frames.

            If your video card does not support YUV/YUY2 Overlay,
            then zoom out to reduce the amount of video traffic.

            For smooth High Definition preview, probably need at least 
            2 Ghz even for a moderate bit rate HD file. 
            But for the seriously high bit rates that some countries have, 
            probably need more than 3Ghz.
            YUV/YUY2 Video Overlay is even more important with HD.

            HD Preview will run OK on more humble machines,
            but the display will be jerky due to skipping frames.


	Slow Machines:

            Turn on the Menu-Preview option "Skip Frames If Behind", 
            which allows HD preview on a much more humble machine,
            albeit with somewhat jerky video.

            For HD preview on Pentium II or earlier CPU, 
            try turning on ALL the Drop options,
            to get more aggresive performance.
            See below for more info.


      NOTE - Frame dropping does NOT impact the output file.
             It only controls the preview display.

      There is NO special exploitation for other CPUs (eg 3dNow).


Remember to DEFRAG your hard disks regularly for optimum speed.
Norton SpeedDisk is better than Micro$oft defrag.

*IMPORTANT*  DO NOT RUN NERO AT THE SAME TIME.
Nero tends to take-over the system,
leaving insufficent reources for other tasks.
Solution - Don't start Nero until AFTER Mpg2Cut2 finished.

If sequential acccess performance on DVD drives is very, very poor,
check that the relevant IDE ATA/ATAPI Controller is in UDMA mode.
The intructions below are for an IDE drive on the Secondary Channel.
Control Panel - System - Hardware - Device Manager;
Expand IDE ATA/ATAPI Controllers;
double click on Secondary IDE Channel
click on Advanced Properties.
Ensure that Transfer Mode is set to "DMA if available".
If not change it to that.
If it was already set to DMA, but Current Transfer mode is PIO mode,
then Windows may have gotten itself confused.
Restart the system, and recheck the Current Transfer Mode.

To unconfuse Windows, you may need to do the following :-
Uninstall the Secondary Controller.
Restart Windows.
Wait for Windows to detect and reinstall the drivers.
Change the desired transfer mode to "DMA if available",
as described above.
Restart the system, and recheck the Current Transfer Mode.




------------------------------------------------------------------------

MULTIPLE INPUT FILES.

   Reinstated the ability to concatenate multiple input files.
	   Menu - File - Add. 
   Multiple files can be selected during File Open or Add,
   but be patient as it takes time to open a number of files.

   NOTE
   ====
   This is a primitive binary join,
   with NO analysis of the Mpeg attributes.
   It is up to you to ENSURE that the files are COMPATIBLE.
   Otherwise, expect strange results.

   EG Does *NOT* detect:-
       - Broken GOP at join boundary (maybe fix in a future release)
       - Different bitrates
       - Different Height or Width
       - Different chroma sub-sampling.
         HEALTH WARNING: - Mixing 4:2:0 with 4:2:2 may trigger Epilepsy,
                           due to psychedlic flashing patterns.

   *** ESPECIALLY do *NOT* intermix ELEMENTARY streams with PROGRAM streams!

   You can choose to sort the file list by :-
             -  File Name,
             -  File Creation Date,
       *OR*  -  Mpeg Time Stamp (System Clock Reference - SCR).

   Each approach has it benefits and limitations.
   Mpeg Time Stamp is good when joining segments created on the same day,
   but may have been edited on different days.

   Otherwise, File Creation Date would probably be better.


   Do any sorting *BEFORE* you start selecting edit points, 
   because the Edit Decision List is *NOT* updated after the sort.
                (May fix this in a later release.)

   For files split with HJSPLIT, or similar non-mpeg utility,
   Use "Sort by NAME" in this case.
   Do *NOT* sort by Time Stamp, 
   as the segments may not be split on exact MPEG PACK boundaries.  

   Multiple input files are treated as if they were one big file.
   But to help navigate, keyboard commands allow skip to next/prev file.
   Also, a beep will sound whenever a file boundary is reached.

   Files can be added either through the File Menu,
   or using Drag-And-Drop from Windows Explorer.  

   By default the program will ask whether dropped files are to added
   to those already opened.  This pop-up can be controlled via :-
   MENU - Misc - PopUps - Dropped Files.



-------------------------------------------------------


MULTI-CLIP selection (NEW FEATURE IN VERSION 2)
====================

You can select multiple clips to be written to a single output file,
but the resulting file is LESS COMPATIBLE with other software.

Select one clip at a time to be included in the output.

IE - MARK the "FROM" point, 
     MARK the "TO"   point,
     ADD the selection to the list using the  + symbol.

As mentioned above, leave some extra fat 
at the start and end of your edit selection,
to protect against open Gops and audio offsets, etc.

If you just change you mind about the "FROM" or "TO" position,
of the most recently added clip,
then just MARK the new point and do another ADD.
It will recognize that the other is the same,
and optionally ask you if you want to replace the previous selection.

To REMOVE the last clip added,
use MENU - EDIT - REMOVE CLIP.

The ADD action can be automated,
so that as soon as you mark the "OUT" point,
it will add the clip to the list.

An intermediate level of automation is "REMIND",
where it will remind you if you forget to manually add a clip.

However, the Automated function assumes that successive clips are always
progressing forward.  If you change your mind about the previous clip,
or want clips out of chronological order, turn OFF the automation.



At the end - REMEMBER TO *SAVE* THE CLIPS:
 
    MENU - FILE - SAVE ALL   (or press F4).

LIMITATION
==========

Some players may pause or freeze at joins,
or you have other player problems.

and try using one of the following utulities to reset time stamps:-
     - "ProjectX"
     - "PVAStrumento".

Alternatively, you could try an experimental option :-

   MENU - OUTPUT - PARSING - ADJUST TIME STAMPS

If this does not fix the problem, turn it off.

Another option is to save the clips using the "FILE-UNMUX" command.
This saves as separate Video and Audio files,
with no Mpeg Time Stamps.
BUT do NOT bother trying this on raw DTV captures,
as they are prone to intermittent signal loss,
leading to audio sync issues.

----

The Edit Decision list can be saved and later reloaded.
The EDL includes both the list of input FILES and the
list of selected CLIPS.

At this stage the EDL Save and Load options are very primitive.

     
Mpeg "Sequence End" codes can be added at the end of each clip,
if this menu option has been enabled. However this is experimental.


----------------------------------------------------------------------


EXPLANATION OF PROGRAM MESSAGES
===============================

NOTE: If you have ticked a box to stop a message from popping up,
      but later you have changed your mind,
      you can reset it via MENU - Misc - Pop-up Msg



MESSAGE: "OUTPUT filename same as INPUT."
         You cannot save to the same name as your input file,
         as the program needs to read the input while writing the output.


 
MESSAGE: "MISSING FIRST %s header on input file
          Input file starts PART WAY THROUGH GOP/SEQ.
          SKIP TO BETTER START POINT ?"

   Common with DTV capture files. 

   OK - Positions to the first "clean" start point :-
        MPG - Sequence header Pack
        VOB - PrivateStream2 NAV Pack

   CANCEL - Accepts messy start of file ASIS,
            and temporarily turns off VOB Preservation.

   If you are editing a SEQUENCE of files that are actually
   a single stream, then ONLY click "OK" on the FIRST one.

   Best approach for editing split files is to use the "File-Add" command
   to concatenate fragments in a single edit session,
   to avoid losing frames from GOPs that have been split across files.  

   For ease of finding files when you have a lot in the same folder,
   use "File-Add-Like", which restricts the File display
   to those with a similar prefix. (Not supported on DBCS)





MESSAGE: "No VOB NAV PACK" and no picture is showing.

   Maybe your VOB file is NOT TRULY a DVD VOB file ?

   If this happens on a lot of files, 
   TURN OFF the Output option "VOB preservation",
   to stop it looking for VOB Navigation Packs.




MESSAGE: "DISK PROBLEM" 

   The length of time taken for the system to deliver data from the input
   file was suspiciously long.

   Could be any of the following causes :- 

   1) Drive had powered down since last used.  Not much to worry about.
   2) Other tasks are slowing performance.     Not much to worry about.
   3) The disk may need Defragmenting.
   4) Hard Drive may be failing, causing poor I/O performance thru retries.
                                               Be afraid - be very afraid.


MESSAGE: "NON-MPEG2 FORMAT DATA"

If you try to use the program on an UNSUPPORTED file type,
such as Mpeg-1 or a Transport Stream,
it will likely crash at some stage, maybe lock up your system,
and even if it does not crash, any saved files will probably be rubbish.

But if you are lucky, you MIGHT be able to see part of the picture.

If you insist on using it in this situation, NOTE THESE MAJOR LIMITATIONS:
- Single selection ONLY. NOT multiple clips.
- Cutting is binary mode ONLY. NO output parsing.
- MONO audio will not preview properly (pre-existing bug).



MESSAGE: MISSING PTS   
     or: NO PTS

Program Streams are expected to have a Presentation Time Stamp at 
the start of each GOP or video Sequence,
but none was found at this point.
Such a file may cause problems with some players.


MESSAGE:  CDXA RIFF file.  Format not fully supported,

Files created by CDXA, have a non-mpeg wrapper around the mpeg data.
The wrapper will be stripped off, to create a standard mpeg file.


MESSAGE: "SEQ HDR coming from Preamble"

The input file does not follow the usual structure of SEQ/GOP/I-Frame.
The program needs to using the output parsing capabilities
to copy SEQ HDR (and maybe EXTN HDRs) from the Preamble.

Recommend the following options to be activated :-
   OUTPUT - PREAMBLE - SMALL
   OUTPUT - PARSING
   OUTPUT - PARSING - ALIGN VIDEO PACKETS


---------------------------------------------------------------------

OUTPUT CONTROLS
===============


VOB Preservation.
-----------------

   This feature is still primitive, as it needs a LOT more work !

   The menu option VOB Chunks allows Mpg2Cut2 to do special processing when
   handling files with the name extension ".VOB".

   VOBs are a specially styled form of MPEG file.
   They should conform to a number of extra restrictions :-

   Private Stream 2 NAV pack entries at frequent intervals.
   The program will lock to these points when navigating,
   effectively looking past intervening Key-Frames between PS2 entries.

   2k boundary required on VOB Mpeg Pack. 

   At this stage of development, this option will pad the data at specific
   points, but this is not very thorough.  

   It pads out any EXTRA packs arising when control headers are created,
   and will retain existing pack sizes in the audio PTS matching area. 

   Turning on this option may allow some testing of interoperability.

   *WARNING*
          This option should ONLY be turned on for a FINAL compilation VOB.
          NOT for intermediate edits.
          IT IS *NOT* TRUE DVD CONFORMANCE
          only a help along the way.

   For proper DVD conformance, use "PVAStrumento" by Offeryn
   or the open-source "ProjectX" to reblock *ALL* the Mpeg packs to 2k.





Include TO frame  (previously known as Pad OUT Position).
----------------

   Minimizes the impact of picture break-up at the end of a clip,
   by adding an extra key-frame at the end of the selection.

   If you don't like manually adding an extra GOP at the end of the clip,
   then this is a compromise position that MAY be of use. 

   May also be useful for "I-frame only" files, 
   which some capture cards produce.

   If Output Parsing is enabled, then it can also add
   some succeeding 'b' frames with appropriate Presentation Time Stamps 
   as might be found in "Open GOP" files, which are common.

   The main drawback is that it creates a GOP with only a single frame.
   This is not an illegal structure in itelf, 
   but some players may cough up at the join, 
   because of the bitrate hit, 
   which is likely to be non-conforming.

   Also it can dilute the effect of the AUDIO PTS Matching function.

   If Timestamps are not present and DIFFERENT on EVERY frame, 
   then "TO" frame padding should NOT be combined with Audio PTS matching.
   This is because some Mpeg encoders only update the PTS info at GOP
   boundaries, thus making partial GOP functions very haphazrd.

   "Include TO Frame" is NOT recommended when trying to preserve VOB format,
   NOR if Audio PTS matching is desired.





PARSING OPTIONS.
----------------

    Parsing ONLY applies to Program and Elemntary Streams, and only Mpeg-2.
    NOT Transmission Streams. Not PVA.  Not Mpeg-1.

    Work is underway to make the joins smoother,
    by parsing (analysing) the underlying Packet headers,
    but this is not yet complete, nor widely tested.

    Normally you will want Output Parsing turned on,
    but you can turn it off if you want to retain internal controls.
    With parsing turned off, only the headers at the start of the file 
    will be corrected. 
 
    If you are not sure whether your Mpeg streams are Aligned,
    then activate Deep Parsing. 

   
   Audio Matching - Compares the Video and Audio Presentation Time Stamps
                    to trim excess audio from the start of each clip,
                    and include trailing audio at end of clip.
                    This is instead of doing a basic binary cut.
                    Insignificant overhead.
   
   Broken Link flag - Sets a flag at the start of each clip to indicate 
                      that some data before it has been edited out.

   Deep Parse - Will look for headers THROUGHOUT the relevant packets,
                rather than just the start of the packets.
                Not required on files that are "Data Aligned".
                
   Parse All Packets - The program will analyse the headers of every packet,
                       not just the start and end of the clip.
                       This is more challenging though.

   NOTE:  Turning on BOTH the DEEP PARSE and PARSE ALL program
          means EVERY video byte is examined.


   Fix Errors In Headers - Fixes bad Width, Height or Chroma format.
                           Useful on DTV files.
                           Checks Width & Height are multiples of 8.
                           Chroma format must be 4:2:0 or 4:2:2 or 4:4:4.

   
   Align Video Packets -
                    Intended for streams where sequences are not aligned.

                    First video packet in clip will be aligned to start at
                    the first Mpeg SEQ/GOP/PIC Header.

                    Trims the last Video packet of each clip
                    in case the last pic encroaches onto the next packet.

                    Only boundary packs are corrected, 
                    NOT the entire file.

   Align Audio Packets -
                    Adjusts the first Mpeg Audio packet of each clip
                    to ensure alignment to a syncword.

                    Intended for streams where Audio frames are not aligned
                    to a pack boundary.
                    Only boundary packs are corrected, 
                    NOT the entire file.
 
                    Mpeg Audio header detection is not always accurate.

   Drop Padding pkts & PS2-NAV  -
         For use with outputing Non-VOB variable bitrate Program Streams.
         Wider effect when VOB preservation turned OFF.
         Can also be extended using Unlock option in Sys hdr.
         Implicitly turns on "Parse All Pkts".
         Inhibited when saving to a VOB or using Vob Preservation.

   Adjust Time Stamps - Still under development - VERY PRIMITIVE.
                        The aim is to smooth joins by adjusting
                        internal Mpeg Time Stamps at each join.

                        Currently does *NOT* handle overlapping clips.
                        *EXPERIMENTAL*  *NOT WIDELY TESTED*.
                                

SEQUENCE HEADER CORRECTION
--------------------------

There are various types of video sequence header correction :-
        a) BitRate specification
        b) Aspect Ratio code.
        c) Width, Height and Chroma formatl.
        d) Progressive/Interlaced display indicator.

The details of these 2 options are discuessed below.

NOTE:-  To correct ALL headers,
        you *MUST* also enable BOTH :-
             - "PARSING" 
        AND use one of the following :- 
             - "DEEP PARSE"  (If you are not sure about alignment)
          OR - "PARSE ALL PACKETS" (If you know video packets are aligned)

       OTHERWISE it will only fix some of the headers.

Header correction is best used with Elementary Video Streams,
or "Data Aligned" Program Streams, 
as it does not handle headers that are split across packets.

                   

Limit SD Headers
----------------

Can be used to patch the bitrate headers at the start of each clip.

For files with a canvas wide within Standard Definition limits,
the bit rate in the headers at the start of the output file
will be check, and kept within the SD limit.

This is useful with Nebula DTV capture cards, 
where the rate field contains the TS rate rather than the PS rate.

This function will also fix missing bitrate info in pack headers,
by inserting the same arbitrary value used when limiting.


ASPECT RATIO (in VIEW menu)
---------------------------

Some Mpeg Streams contain the WRONG aspect ratio specification.

For Mpeg-1 flies, you may like to turn on the Mpeg-1 option
"Infer Aspect", as there are a lot of non-standard Mpeg-1 files around.
This will make a best guess as to the aspect ratio for viewing.


For Mpeg2 files, if you override the Aspect Ratio in the VIEW menu,
then you will also be asked if you want to force the output
to match the new Aspect Ratio that you have chosen.

"70mm  Non-anamorphic"
The setting labelled as 70mm is actually the MPEG definition (2.21)
rather than the true cinema 70mm ratio (2.20).
I suspect someone at MPEG HQ transcribed the standard wrongly,
and now it is set in cement.


"Narrow Cropped"
The aspect setting called "Narrow Cropped" is a NON-standard one.
It is used for videos that were created with the edges cropped out.
This was sometimes used in the bad old days by people who did not like
the irregular frame sides coming out of home videos.
You also see this on some amateur telecine.
Cards like MiroVideo allowed them to crop off the edges,
but the resultant videos are narrower than the standard 4:3 aspect ratio.
Mpg2Cut2 allows you to manually select a narrow aspect ratio for previewing.

There are a whole bunch of theatrical film aspect ratios
that are to numerous to mention.

If standard aspect ratios ALWAYS look wrong on your monitor,
there is an INI file parameter to apply a consistent correction.
On the INI line that begins with "N3=",
the first parameter is a percentage to adjust the output image.
EG If images are always 5% too wide,
then set "N3=-5," to reduce all image widths by 5%.


d) Progressive/Interlaced display indicator.

Some encoders do not give sufficient control of the 
Progressive/Interlaced indicators.

This option allows you to reflag a Progressive encoded file
for interlaced display, which is required by older DVD players.

However, it is best used with Elementary Video Streams,
or "Data Aligned" Program Streams, 
as it does not handle headers that are split across PS packets.

                   

SYSTEM HEADER OPTIONS
=====================

PREAMBLE.
--------

   Can copy control info from the start of the input file.
   This is very crude, but may help some people, sometimes.
   Previous version did not copy control info from start of file.

   PREAMBLE = NONE  

      The program will create its own dummy header,
      rather than trying to use what is on the input file.
      Good for Twinhan captures from Australian Network TEN.   

   PREAMBLE = SMALL  (RECOMMENDED)

      Good for NEBULA DigiTV files to remove crud from start of file,
      thus improving compatibility with other utilites such as TMPGENC.   

   PREAMBLE = MAXIMUM

      This is a more dramatic setting that allows for files
      with complex control information at the start.
      Try it if you have trouble editing VOB files.
      Personally, I no longer use this setting.

   PREAMBLE = NONE
      This is basically the old approach used in version 1 of MPEG2CUT,
      except that the System Header is slightly more general.
      Retained for those who may still want it,
      but has been moved into the "obsolete" preferences sub-menu.




----------------------------------------------------------
EXPERIMENTAL SETTINGS
=====================

HIDE AUDIO

Mutes the sound on a VOB file,
while preserving the overall block structure.



Seq End codes - HIGHLY EXPERIMENTAL.

   At the end of each clip, this inserts an MPEG SEQUENCE END code (B7) 
   to indicate the end of a video sequence.  
   This may aid some players, at very little overhead.




DEFAULT OUTPUT FOLDER
===================== 

  Allows you to set what will be the default folder for the "SAVE" dialog.

  The default folder is used for the first save of a session,
  or after you change input files.

  There are 3 options -

     Always Most Recent = Remembers the last used Output folder each time.

     Same as Input = Defaults to same folder as the input file,
                     for the first save from that file.

     Static default for First Save 
          - Sets a specific folder as the default 
            for the first save of each file.

  ---

  "Show Both In Win98"

  For Windows 98 users, the facility is more sophisticated still :-
  The INPUT folder name is still provided in the filename,
  to allow easy override of the default folder.

  Windows 2000 does not seem to allow this flexibility, 
  so think *BEFORE* the save, what Default Folder setting you want.
  


DEFAULT EXTENSION
-----------------

This control the 3 letter file type on the end of the file name.
For use with Premier Pro, you will want the default to be MPG.





TIDY NAME
---------

  NO BLANKS = Change imbedded spaces in file name to underline "_"
              Allows for utlities that cannot tolerate imbedded spaces.
              EG  ALL GOOD THINGS.mpg  becomes ALL_GOOD_THINGS.mpg

  MIXED CASE = Converts file name to mixed case,
               based on word separation.
              EG  ALL_GOOD_THINGS.mpg  becomes All_Good_Things.mpg

NOTE: The Tidy Name function occurs at when files are loaded,
      so when first turned on, it will begin form the NEXT file opened.


UNMUX
=====

The selected clips will be demuxed into Elementary Streams,
rather than copied as a Program Stream.

Some authoring tools use Elementary Streams as input.

*NOT* recommended for DTV files,
since they need PVAStrumento or ProjectX 
to replace missing data and fix time stamps.

Only the Video, Audio and Subtitle streams are demuxed.
Any other streams are skipped.

Q. Why did I call it UNmux rather than DEmux ?
A. To avoid confusion in keyboard shortcuts.
   D=DELETE  U=UNMUX



FILE NAMES DISPLAY
------------------

Menu - File - Names

Shows all the Mpeg files currently open for input,
along with their creation date.

You can adjust the column sizes by dragging the borders.

[+] = Add a file to the list.  
[-] = Remove the currently highlighted file from the list. 
[1] = Keep the currently selected one, and remove all the others.

[REN] = Rename the currently highlighted file. 

By default, the font size in this panel is large for readability.
But you can suppress this by turning off the Readability option
Menu - Misc - Pop Up Msgs - Readability



POST-PROCESS
------------

Allows you to automatically run another program whenever an output file
is saved.  The name of the output file is passed as a parameter.
You can specify whether the file name needs to be in quotes,
to allow for names with imbedded spaces.

As well as entering the FULL NAME and PATH of the program to be executed,
you should also tick the ENABLE box.  

The Path to the program can be omitted if it resides in the system path.


-------------------------------------------------------------------------



OUTPUT SPEED THROTTLING
=======================

The output speed can be throttled back,
to avoid swamping other programs running on the PC.

The "Slow" check box on the output progress panel,
will force Mpg2Cut2 to "sleep" more during output.
This allows other applications to breathe.
It also reduces the size of the buffer used for copying,
to further reduce the demands on the I/O system.

If the Slow button is not effective enough,
then you can adjust the "Breathe" parameters
by editing the INI file :-

Breathe=(256,128,32),Pkts=(1,16,256)

The parameters are in 2 groups of 3.

Within each group, the numbers refer to the 3 possible priority settings :-
       Low, Normmal and High Priority.

The first group of 3 numbers set how many milliseconds to sleep per buffer.
The larger the number, the SLOWER it runs. 

Parm 1 applies to Low Priority (Slow).
Parm 2 applies to Normal Priority.
Parm 3 applies to High Priority.

The last group of 3 numbers set how many packets are processed 
before the program sleeps for 1 millisecond.
The larger the number, the FASTER it runs. 

Parm 4 applies to Low Priority (Slow).
Parm 5 applies to Normal Priority.
Parm 6 applies to High Priority.



-------------------------------------------------------------------------


VIEW OPTIONS
============

View options do NOT impact the output Mpeg file.
Some settings may impact BMP snapshots, where noted.



LUMINANCE CONTROLS
------------------

If the image is too dark on your monitor,
you can boost the display using this facility.

This does NOT impact the output MPEG file,
but can optionally be applied to BMP snapshots as well.
The choice is yours.

GAMMA - Increases brightness and contrast in dark areas.

        This is the main difference between TV and PC systems.
        Adjust this first, as it is the most common discepancy.
        130 is a good starting point,
        but different systems may need different values.

        This is a logarithmic control.
        Values below 100 darken.
        Values above 100 brighten.
        

CONTRAST   - Same as an analogue TV contrast control.
BRIGHTNESS - Same as an analogue TV brightness control.


LOCK - Automatically adjust contrast and brightness together.

BMP - The changes will also be applied to BMP snapshots.
      Be careful, as some video cards (like mine) 
      have DIFFERENCES in the Gamma settings
      between Overlay and BMP (DIB-RGB) interfaces.

[-] - Negative image

[D] [C] [B] [A] = Luminance preset buttons.

The Luminance preset buttons can be changed using the [@] button.
The next Luminance button clicked will then take the current setting.
EG - To change button [D] to match the current setting,
     click [@], then click [D].



COLOR
-----

BLUE / RED sliders
      - Turn green people a bit redder during preview.
        Primarily for bad NTSC videos.
        Only works with YUV Video overlay, at this time.

Swap - Swap U-V colour channels.  Use this if people have blue skin.
           

VHS - Some VHS tapes may turn strong red into an exaggerated glowing red.
      This will help stop Red from over saturating strong reds.
      This is NOT the same as using the Red slider,
      as VHS only effects strong red, not pale red.

      Also consider adjusting VHS convergence,
      under View - Special FX.


SPECIAL FX   [Only work if Fast Decode option is in effect.]
----------

VHS convergence - Use when either Red or Blue components, or both,
                  are either too far right, or too far down.

Retain Saturation settings across session
         - Useful when your color adjustments are due to monitor problems
           rather than input file quality problems.

Lum adj Deselect between clips
    - Useful when viewing program material that is much darker than ads.



ASPECT RATIO
------------ 

The display will normally use the aspect ratio specified in the MPEG file.
This is needed for most Mpeg files, as they are compressed more
in one direction than the other.

Overlay (YUV/YUY2) gives the best results, as the system does the work.

In RGB mode, aspect ratio conversion is quick and dirty,
so a non-overlay image can look a bit rough.

You can override the default Aspect Ratio, if the file is coded wrong.
If you do, then when saving the output you will be asked if you want to 
force the output to the new Aspect Ratio.

If you are using a Widescreen monitor 
with an old adapter that has NO Widescreen support,
you can correct the distortion by ticking :-

Menu - View - Aspect Ratio - Adapter Mismatch



DE-INTERLACE
------------

De-interlace option is useful on non-cine material,
or traditional NTSC telecine (29.97fps),
and also for poor PAL telecine.

But for standard PAL telecine, it should not be needed.

Sadly some PAL material contains both PAL and NTSC interlacing,
due to very poor conversion.  I cannot remove double interlacing.

De-interlacing is performed by dropping the 2nd field,
and then adjusting to the correct aspect ratio.

In YUV overlay mode, the upscaling is done by the video card.
If the video card is optimized for DVD playback,
then the de-intelaced picture will be smooth.
However, if your video card is only optimized for games,
then the deinterlaced picture will look jagged.

My old ATI All-In-Wonder card built around 1996 (I think)
gives an excellent picture, because it has good DVD support.
However my 2005 vintage SiS video card gives a jaggy picture,
because it is only optimized for gaming, not video.
However, ATI software is crap, and tends to forget Gamma settings.
This is why I resurrected Gamma setting within Mpg2Cut2.
Other brands have similar software problems.

In summary, buy a video card with good upscaling/DVD support.
If you can also get one with good software support,
then that's a blessing !


CROP TOP EDGE
-------------

Some video streams have annoying dots on the top or bottom of the picture.
Turn on this option to suppress display of the top and bottom edges.
Menu - Auto-Deinterlace - Crop Top Edge



ZOOM
----

If the picture is wider than the screen,
you can ZOOM OUT to see the whole picture.

Otherwise, it will display the middle of the picture,
and you can scroll by pointing and clicking with the mouse
on the area of interest to centre it. 

There are 4 zoom settings, which the Zoom button can cycle through.

Zoom OFF      = Width is stretched to create correct aspect ratio. 
Zoom CompactF = Height is squeezed to create correct aspect ratio. 
Zoom / 2      = Half the size of "Compact".
Zoom / 3      = Third the size of "Compact".

The Zoom button will skip "Zoom Off" when it is wider than the screen,
but it can still be selected on the View Menu.

If the window is Maximized while in Compact mode,
then the zoom factor will be set to match the width of the screen.


CENTRE CROP
-----------

Some files have 4:3 material with black borders added at the side,
to fill out a widescreen (16:9) frame.

This option crops out the black borders at the sides,
mainly for use in full screen viewing.



MULTI-ANGLE   (EXPERIMENTAL)
---------------------------

This rather crude and NOT very thoroughly tested,
so the setting is not retained across sessions.

Turning on this option should allow you to play a multi-angle video.
The angle selected will be the one that was currently displayed
when you pressed play.

         ** ANGLE SETTING DOES NOT IMPACT THE OUTPUT FILE ***



MULTI-STREAM   (EXPERIMENTAL)
-----------------------------

This is untested, as I have not come across a multi-stream file yet.
Theoretically, it should allow you to choose which Video stream to view.

The default is the video track zero (internally hex E0).

"AUTO" means that it will play the first angle that it finds.

         MULTI-STREAM SETTING DOES NOT IMPACT THE OUTPUT FILE.




PID   (EXPERIMENTAL)
----

This only applies to viewing, and it is very crude.
At this stage ALL PIDS are still copied when saving.

PID only applies to TRANSPORT STREAMS,
which are NOT fully supported.

PID = ALL (default)

      When searching, this shows ANY I-Frame with a Video PES Header, 
      so you can see what is on each PID.
      This is disabled when playing.

PID = CURRENT
      Shows only the PID that you are currently seeing.
      Tick this once you have found the PID of interest to you,
      and you no longer want to see the other PIDs.




TECHNICAL 
---------

RGB/YUV - Allows you to manually select whether the video display will
          use RGB mode(slow but works on most video cards
          or YUV mode (fast but requires reasonably modern card).

DirectDraw Overlay OK - indicates whether program is using DirectDraw overlay.
 
Auto-Release - Program will release the Overlay when not required.
                  - During File Save
                  - When Minimized
                  - When file/s closed.
               Does not work with all video drivers.

SIMD Technology - Stop/Allow the program to use cetain CPU functions.


DRIVER CONFLICTS
----------------

ATI Sloping - Allow for bug in ATI driver.
      Some ATI drivers sometimes give sloping video on first display. 
      This should overcome the problem.



Leadtek OVL Key - Grey 
           If Leadtek Winfast is running in the background,
           it stops Mpg2Cut2 from using it's own Color Key for the overlay.
           This option forces Mpg2Cut2 to use the Leadtek Color Key.
           *BUT* It has the side effect of turning menus TRANSPARENT !

           Furthermore, in some situations, when you minimize and restore, 
           sometimes the overlay key has changed to black in the interim.
           Tick the Leadtek OVL - Black option to overcome this.
           As this situation is usually transient,        
           Black mode is reset to Grey at the end of the session.
           Thanks for the annoying software Mr LeadTek. Grrr... Argh....

           If the LeadTek conflict is simply too annoying,
           then tick "Never Use Overlay" to avoid the situation.



*** EXPERIMENTAL OPTIONS FOR WINDOWS VISTA *****

OVL Transparency  - Vista users may need to turn this on.
                    Shows Overlay through Black screen.
                    Only takes effect from the NEXT session.
                    [So shutdown Mpg2Cut2 after you change this.]

Overlay Notification - Vista Users may need to experiment with this,
                       to get overlay to show.
      - Notify Frame Change  - Avery Lee method.
      - Notify Update Window - Alternative method.
      - Notify GUI           - Redirects OVL update to the GUI thread.
                               Click on black screen to show Overlay.

Never Use Overlay - If overlay driver is incompatible,
                    you can tell the program to never use it.
                    VISTA users may need to turn this on.
                    Ditto with MSI on-board video !
 


DECODER OPTIONS
---------------

These should not normally make much difference,
but are included just in case.


Field Correction - Alters display to allow for strange encodes.
                   Try fiddling with this if De-Interlace does not help.

PC/TV Scale    - Minor technical change in display, 
                 because TV black is not identical to PC black.

iDCT Algorithm - Minor technical change in display,
                 with some performance trade-off.



FAST 4:2:0 DECODE
-----------------

For most files, this offers more efficient conversion
into the format required for display on the video card.

4:2:0 is probably the most common Mpeg format for domestic use.

The professional format 4:2:2 can also be done fast.

Other formats, such as 4:4:4 continue to use the unaccelerated decode. 

If the input file is Mpeg-1 or a Transport Stream,
then Fast Decode is temporarily DISABLED,
to reduce the chance of crashing.
The program really is only rugged on Mpeg-2 PS files.




---------------------------------------------------------------------------

PREVIEW
=======


Summary - When previewing the clips, only shows start and end of each clip.


Slower - Playback speed is reduced [F9].
         Repeated clicks slows motr.

Faster - Playback speed is increased [Shift-F9].
         One click    = 1.5x with audio
         Two clicks   =  2x with audio
         Three clicks = 3x - with indecipherable audio
         Four clicks  = 4x - if you have a fast PC.
         Five clicks  = enters CUE mode - no audio.


The program will attempt to match the audio speed appropriately,
where it can.  The adjusted sound playback is a bit choppy,
sounding a bit like the film is loose on the sound drum.

To reduce RSI - CUE mode can be toggled using numeric pad [*] .
Instead of holding down the Fast Fwd arrow continuously,
single press the [*] key on the numeric key pad to enter Cue mode.
Press again to return to normal speed, or press any arrow to stop.



TIMING METHOD
-------------

Controls how the preview function plays the file.

It has a number of sub-options :-


LOCK TO AUDIO
--------------

Because preview does not attempt to buffer all the video frames
required for the audio delay, there is a trade-off between
having jerky audio versus jerky video.

This is relevant on files with "chunky" interleave of audio and video.
It is not so important on files that are "finely" interleaved.
(Planned future enhancements to buffer handling may supersede this.)

Earlier versions played the video smoothly, but audio was choppy
on "chunky" files. 

This new option allows the program to play audio smoothly,
and adjusts the video frame rate to keep up with the audio.

The choice is yours.  Smooth Audio or smooth Video, but not both.

Locks frame rate to audio by default.

You can use the "Statistics" panel to determine whether the file
has "chunky" or "fine" interleaving.  
See description of Stats panel for more info.


SKIP FRAMES WHEN BEHIND
-----------------------

If preview is slow, try turning this on.

This should help when previewing High Definition on a slow machine, 
but it's a bit ugly, so only use it if you need it.

Typically this will drop delta frames if PLAY mode is too slow.
Conservatively set, so that Key frames always show, even if behind.

If your PC is too slow even for SD, then turn SD skipping on too.

Dropping options can be adjusted to suit your environment :-



PTS BASED SKIPPING (Aggressive Experimental setting)
-------------------

If you have najor performance problems,
maybe you could also try this.

If you are working with files that are FINELY interleaved
or finely time-stamped, then you can try turning on this
option for more efficient skipping when frames are dropped.

*BUT* beware of using this on "chunky" files, 
as it will lead to excessive frame dropping.
See description of Stats panel for more info.

It also decreases the amount of statistics shown,
as more control information is skipped.



FIELD DROPPING  (Experimental)
--------------

If you are working with Field-Encoded pictures,
then this will allow more efficient de-interlacing.

NOTE:- Surprisingly, it is RARE to find files suitable for this feature,
       as a lot of interlaced material is NOT field-encoded !
       I don't know why they encode like that.


-------------------------------------------------------------------

BMP snapshot of current frame.

By default, BMPs will be created in the current input folder.
You can override this using MENU - OUTPUT - BMP Folder.

You also have the option of copying to the Windows ClipBoard. 
 
There are 2 different capture styles:
	1) Grab FRAME asis
	2) Grab FIELDS separately 
	    Use this where interlacing artefacts are a problem.

By default, if the overlay display is de-interlaced,
then BMP snapshots will be too.

Override this with the SHIFT key when you press B,
or change the view to turn it off for that session.

Overlay de-interlacing can be turned off in MENU - VIEW
and this will change the Snapshot default at the same time.

By default, the program uses BiCubic resampling to stretch
the image to the correct Aspect Ratio.

If you prefer to use your own imaging program 
like irFanView or Paint Shop Pro to do resampling
of the snapshot to correct the aspect ratio,
then set the Output BMP format to NO Aspect Ratio Correction,

Any good imaging program can be used to resample the image
to give the correct Aspect ratio:

PAL EXAMPLES:
    720x576 16:9 
             Full FRAME Grab - Resample WIDTH  from 720 to 1024
             FIELD Separated - Resample HEIGHT from 288 to  405. (Alive !)

    720x576 4:3 
             Full FRAME Grab - Resample WIDTH  from 720 to 768
             FIELD Separated - Resample HEIGHT from 288 to 540.


========================================================================


AUDIO preview controls
----------------------

Audio controls do NOT affect the output file.
They are ONLY for help in monitoring the preview.

The [V] button brings up a Volume Control window
that has 2 sliders and a number of tick boxes.
The 1st slider is a conventional volume control.
The 2nd slider is for the audio limiter.

On the Keyboard, Ctrl-V brings up this window.
However, this can be changed in MENU - MISC - KEYBOARD,
to allow the V key to bring up the window, without the Ctrl key.

Volume Boost - Audio boost during preview, for files that are too quiet.
               If overflow occurs, boost is decreased.

Automatic Volume Control (AVC)
             - Booster will try to keep preview volume high,
               for files where some parts are much louder than others.
               This is a a form of volume normalization.

Limit Loud Sounds 
            - Set a maximum for the overall volume,
              for closer matching to other PC applications.
              If activated, default limit is one quarter of max volume.
              Adjustable via 2nd slider on Volume window.

              If you have the Volume set quite HIGH,
              but the Limiter set quite LOW,
              then they will be acting against each other,
              and the sound may become choppy,
              or background sounds may become annoying.
                

Karaoke      - Attempts to remove vocals from STEREO music track.
               Only basic function, no lag compensation.

Audio Track  
       - Allows for selection of different language tracks,
         on files that have parallel concurent audio.

Audio Track - Auto 
       - Defaults to first track found when play begins. 

Audio Track - Auto Default 
       - Resets Audio track to Auto on each launch.
         Otherwise remembers last specific audio track number used. 

You can cycle around the audio tracks by typing "A".
If there is only 1 track, then the Boost will be reset to the default.

Mute - Turn audio on or off.  Keyboard shortcut = "M".


TO IMPROVE PLAYBACK ON MUDDY AUDIO
FROM VHS TAPES RECORDED IN STERO FROM A MONO SOURCE.

     Anti-Phase   - Adjustment for bad audio equipment
               that inverts the singnal on one channel.
               Try this if your audio sounds muffled.
               Only basic correction -  
               Does not compensate for lag between channels.
               If this does not help, maybe try Karaoke instead.

     Karaoke can also SOMETIMES be used to improve playback of poor audio, 
        making muddy vocals CLEARER, by emphasising any channel lag, 
        which can improve treble and reduce bass.


MISC AUDIO OPTIONS

Slow Attack - Slow down the AVC reboosting effect.

PS2 Audio    - Needed for Australian DTV HD with Nebula Card
               which uses Private Stream 2 for Audio.
               Different to DVD VOBs, which use PS2 for navigation info.
               This feature is very crude and unreliable,
               but better than nothing. 

48kHz = 44.1kHz - MAY be useful if an annoying whistle comes from your 
                  sound card gives when playing some files.  
                  Slows preview by about 10%,
                  and changed annoyance to a different note,
                  so that a different part of your ear is annoyed..


Decoder - If the default Audio decoder does not work on your CPU,
          you can override it.

CRC Checking - Can be turned off to allow for cruddy files.

Advanced Users ONLY :-
       If Boost, AVC and Limit do not give exactly what you want,
       the controls can be tweaked in the INI file. 
       On the INI line with the keyword "VOLUME" :-
            - parm1 = reserved
            - parm2 = Audio Boost for Mpeg Audio. 
            - parm3 - Audio Boost for basic AC3 files. 
                      For DTS or DD+, this value is doubled.
            - parm4 - Audio Limiter. 1=on. 0=off.
            - parm5 - Audio Ceiling for limiter.  Max=32767. 
                      Smaller is quieter. 0=Quarter Max Volume.
            - parm6 - AVC Slow Attack Delay period
                      0=Off 16=Moderate. 
                      Negative is temporarily off.
            Other parms reserved for future use.
               
-----------------------------------------------------------------

If a crash occurs in a module name starting with "MPA",
then turn the audio off.
The Mpeg Audio Modules are: MPA_SSE1.dll, MPA_SSE2.dll, 
                            MPA_MMX.dll,  MPALIB.dll.

I am not in a position to support the Dark Avenger mpeg audio modules,
since I cannot get them to compile on my system.

Any volunteers ?
 


=======================================================================

EXTERNAL ACTIONS
----------------

The menu allows you to invoke facilities that are external to this program,
such as 3rd party players or diagnostc utilities.


=======================================================================

NAME LIST

+ = Add another file to the list.
- = Exclude selected file from the list.
    Same as "File-Close-Current".
1 = Retain thus one file, exclude all the others.
    Same as "File-Close-Others".

REN = Rename the selected file

INFO = Show file details.
       Creation Date is in "International" format (YEAR-MMMDD_HHMM.SS)



=======================================================================
MISCELLANEOUS INFORMATION
=========================


AUDIO DELAY
-----------

This is shown in the main window's sttaus line
on the extreme right hand side, after the time co-ordinates.

Eg A-500ms     Indicates an Audio delay of 500ms.

This is the "raw" difference between the Audio and Video PTS
for the current GOP.   Thus it may not match the value shown in the
utility DGIndex which compensates for orphan frames at the start.

When outputing with the parsing option "Audio Matching" turned on,
excessive audio delay should be reduced somewhat in the output file.
EG Audio delay of 1000ms could be reduced to 200ms.



JUMP SIZES
----------

The amount by which the navigation "Jump" commands will jump,
can be overridden.  

Eventually a dialog will be written to allow this to be changed easily,
but for now, the settings are available in the INI file line :-

             JumpSpan=2,20,50,-2,-20,-50

The 6 values correspond to the following Jump commands:-
              Forward - Small  (2), Big  (20), VeryBig  (50)
             Backward - Small (-2), Big (-20), VeryBig (-50) 


If you are using the VirtualDub key layout option,
then these correcpond to  :-

Small jump = Shift-RightArrow
Big Jump = DownArrow
VeryBig Jump = Shift-DownArrow

Notice that the backward jumps are NEGATIVE numbers.
Make sure that you use commas, not dots, not semi-colons.
Do *NOT* reposition the line higher or lower,
as the INI file scanner is very, very fragile.


Keep a backup copy of the original INI file,
in case something goes wrong.

 
=======================================================================

STATISTICS PANEL
================

This panel is getting very crowded, 
so many fields are not self-explanatory :-


Avg and Nom Bitrates are for the VIDEO stream only. 
Mux rate is the Maximum for the combined Video+Audio.



NON-FIELD PIC - Although the file may be marked as interlaced,
                the pictures are NOT encoded as separate fields.

FINE AV - Smooth interleaving of Video and Audio data, 
          *AND* unique Presentation Time Stamps per frame.

FINE A  - Smooth interleaving of Video and Audio data.
          *BUT* PTS is NOT updated for every frame. (See "@" below)

ROUGH - Many video frames before audio is interleaved.

CHUNKY - File interleaving and PTS resolution are BOTH chnky
         IE Many video frames before audio is interleaved.
            Many video frames before PTS changes.

Mpeg-2 PS    - Mpeg-2 Program Stream. 
Mpeg-2 ES    - Mpeg-2 Elementary Stream.
Vob n Cell m - DVD VOB. 

TS PID nnn - Transport Stream.          *NOT FULLY SUPPORTED*
Mpeg-1 PS  - Mpeg-1 Program Stream.     *NOT FULLY SUPPORTED*
Mpeg-1 ES  - Mpeg-1 Elementary Stream.  *NOT FULLY SUPPORTED*
Mpeg-1+2 PS- Mpeg-1 Videon wrapped in Mpeg-2 packets. *NOT FULLY SUPPORTED*
Mpeg-3 PS  - Decoder is confused.       *NOT SUPPORTED*

If you try to use the program on an UNSUPPORTED file type,
it will likely crash at some stage, maybe lock up your system,
and even if it does not crash, any saved files will probably be rubbish.

4:2:2 = Professional level chroma format
4:4:4 = VERY Professional level chroma format



"Timers" section
----------------

Note that PTS, DTS and GOP timecodes can be all different !
This section shows each in detail.
Values of time stamps are shown in the format "hh:mm:ss.ff".

"V" line is Video information. 
"A" line is Audio. 
"-" line is the relationship between Video and Audio.


In the second column of the timers section:

  "@" = Video PTS Granularity 
      = how many frames between increment of Video PTS. 
          @ 1 = Frame level PTS increment. Very fine.
          @12-17 = GOP level PTS increment. Normal.

  "i" = Audio Interleave 
      = number of video frames before an audio interleave.
          i 1 = very fine.
          i 12-17 = interleave about once per GOP. 





==========================================================================

INSTALLATION
============

1)   Download TWO (2) Zip files from http://www.geocities.com/rocketjet4/

     You will need BOTH the main MPG2CUT2 zip file
     AND the AUDIO DLL zip file.


2)  Unzip the downloaded files and do a VIRUS SCAN !

3)  Copy the Mpg2Cut2.EXE file and this TXT file to an appropriate folder.
    The AUDIO sub-routines should be copied into the SAME folder.

    They MAY also need to be copied into the Windows folder,
    or into a System PATH folder :-

            libmmd.dll,
            mpalib.dll,   mpa_mmx.dll,   mpa_sse1.dll,   mpa_sse2.dll


    The DLLs can be in any folder included in the System Path.
    The easiest way to see the list is to 
    open a dos window [Command Prompt]
    and type in the word PATH followed by the enter key.

4)  Create shortcuts to both the EXE and this TXT file
    and put the shortcuts either on the desktop or in your Start Menu.

3)  Under Windows 2000 or later, set security to allow dumb users
    to update the file "Mpg2Cut2.INI".



WINDOWS EXPLORER CONTEXT MENU
=============================

You can add Mpg2Cut2 as a context command for Windows Explorer.
Menu - Misc - Associate.

------------------------------------------------------------------

HOME PAGE: 

     http://www.geocities.com/rocketjet4/


------------------------------------------------------------------

COMPATIBILITY WITH OTHER UTILITIES.

Check that the following OUTPUT options are turned on in Mpg2Cut2 :-

 - Parse Enabled
 - Parse options - Audio Matching  
 - Parse options - Align Audio start packets
 - VOB Preservation (If you are using true DVD vobs.)
 - Default Extension - MPG  (If you are using Adobe.)
 - Preamble - Small   (for most files.)
            - Maximum (if needed for some VOB files).



ADOBE PREMIERE - "Unsupported Audio Format".

As I understand it, Adobe Premiere support for AC3 format
is handled by a plug-in.  If you do not have that plug-in,
or it is not installed correctly, Adobe may issue a message :-
"Unsupported Audio Format", or such like.

To determine whether your clip uses AC3 format for audio,
perform the following in Mpg2Cut2 :- 

Bring up the stats panel by pressing F6,
play the clip for a few seconds.
The stats panel will describe what format the audio is.
EG   mp3, mp2, mp1, DD.
     mp2.5  = mp2 with mpeg 2.5 features.
     mp3.5  = mp3 with mpeg 2.5 features.
     mp3.5L = mp3 with BOTH mpeg 2.5 features and Low sampling extension.



If the type is "DD"or "DTS", then for direct import to Premiere 
you will need an AC3 plug-in.

If you do NOT have that plug-in, then do the following :-

- Unmux the clip to separate the audio and video.
  (Menu - File - Unmux)
  The Video is created in an "M2V' file.
  The Audio is put into an "AC3" file.

- Use HeadAc3he to convert the AC3 file to WAV format.
- Drop the M2V and WAV files into the Premiere timeline,
  at matching positions. (Starting at the SAME time.)


------------------------------------------------------------------

COMMAND LINE INTERFACE 

This was primarily developed for use with IGCUTTER.

The parameter interface is very basic and limited.

Because the new parameter format is not 100% backwards compatible,
the new mode must first be enabled via the Mpg2Cut2 menu.
MENU - MISC - EXPERIMENTAL - Parm With Time Codes.

Quotes around file names are then REQUIRED,
to allow for imbedded spaces.
EG  "C:\My Documents\FRED.MPG"

At this stage, proper functioning is limited to Mpeg2 files that
fall within the following restrictions :-

- Only 1 input file at a time.
- Mpeg-2 format only, NOT Mpeg-1.
- Progam Stream only. NO Transport Streams, No Elementary Streams.
- PTS must be locked to SCR at SEQ/GOP boundaries,
  as is normal for DVD format.
  This means some DTV captures may NOT be acceptable.
- SCR must not reset within the file.
  IE It must always increase, never decrease.
  This meams PANASONIC DVD recordings are NOT acceptable at this time.


MPG2CUT2 does NOT check that the restrictons are being adhered to,
as the interface is still very experimental.

Parameter format :-

"input-file" [...] FROM=hh:mm:ss.ff TO=hh:mm:ss.ff  O="output-file" xxx

All parameters are optional, except for the input-file-name,
which MUST be the FIRST parameter.

FROM= defaults to start of file
TO=   defaults to end of file

The user will be presented with a File-SaveAs dialog,
which allows the output name to be overridden.

xxx = Time Code Style
      ABS = Absolute
      REL = Relative (default)

Time codes can omit the hour, minute and frame numbers,
where they are zero. Seconds MUST be pesent, even when zero.

EG  FROM=30 is the same as FROM=00:00:30.00  (30 seconds)

Please note the the frame number portion is preceeded
with a DOT, not a Colon, as it specifies a fraction of a second.
BUT it is NOT a decimal fraction - it is a frame number.

EG:   30.5 = 30.05 = 30 seconds plus 5 frames.
 
Time Codes are by default expected to be RELATIVE SCR values,
a common convention in players that show time codes.

EG  FROM=30 means 30 seconds from the start of the file,
even if the first SCR on the file is NOT ZERO.

If you want to eliminate the conversion to relative time,
specify the keyword parameter ABS (Absoloute).
Time codes must then match the actual SCR time stamps.


------------------------------------------------------------------

API INTERFACE FOR CONTROL BY EXTERNAL PROGRAMS.

Yes, There is a very primitive one.
No, I have not documented it.
See the source modules: 
       - Mpg2Cut2_API.h
       - GUI.cpp

------------------------------------------------------------------

PLUG-INS FOR MPG2CUT2

If you are planning on writing a plug-in for Mpg2Cut2,
here is a list of the current exit points.

0) Mpeg Audio Decoder.

1) File List.


2) File Rename.

   3 Parameters are passed to the plug-in.
      - Pointer to start of Table of pointers to File_Name/s
      - Total Number of File Names 
      - Occurence number of File currently being viewed.
     
   The plug-in must update the File_Name table with the new name,
   so that Mpg2Cut2 can re-open the file/s.

   Supports 2 styles of Renamer :-

   2.1 Single File Rename.  (MODE "S")
       Only renames a single supplied file,
       so only the first parameter is relevant.

   2.2 Multi-File Rename.   (MODE "M")
       Potentially can rename ALL the input file names in the table.

------------------------------------------------------------------


CHANGE NOTES (since Version 1.5 of Mpeg2Cut)
============

1) Multiple clips into a single file. 
   After making a selection, click the "+" button
   to add the clip into the Edit selection List.

   When you have selected all your clips,
        Menu - File - Save ALL   to save all the clips.

   If you just want to save ONLY the current selection,
        Menu - File - Save This   to save just the current selection.

   Players may pause at joins - try hitting PLAY.
   This is because I am not rebuilding the internal Time Stamps.

   Preview function of multiple clips (SHIFT-F8)
    is a bit rough around the edges.


2) Multiple Input files.

   Reinstated the ability to concatenate multiple input files.
   Added a menu command to DELETE all the input files.
   It will attempt to send to the recycle bin,
   but large files will be deleted without going there.


3) Compatibility improvements.

   Overcomes most of the problems reported by users regarding
   Mpeg2Cut incompatibility with other software:

   Output Format controls to standardise start and end of file.
   Preamble options to copy headers from original file.
   Padding option to pad out end of clip or file.

   Clip scoping changed from I-Frame start block number
   to instead use SEQ/GOP header start address.

4) More Fault Tolerance.
   Much less vulnerable to transmission errors.
   This is especially important when working with cruddy DTV captures.

5) Interface with Windows Explorer 
   Now accepts file name via parm area.
   Output file names are now based on input file name.

6) Toolbar changed.
   Moved to top to allow for picture height > display height
   Extra buttons for ergonomic reasons.
   Button size scaled according to screen resolution. 
   Extra buttons and info on progress window.

7) BMP snapshot no longer requires RGB24 display.
   Option to separate interlaced fields.
   Name now based on input file name and frame time stamp.

8) Display Enhancements.

      - Aspect Ratio Correction 
      - Fast De-Interlace
	- Align view window based on primary mouse click within image area. 
      - Zoom-Out through toolbar button or secondary mouse click;
	// not yet:- Zoom-In at double-click position (Max=1:1)
		     - Release overlay when minimized 
      - Attempt to reacquire overlay after DisplayChange 
                or Mpeg canvas size increase.

   Most MPEG files have anamorphic (squeezed) pixels.
   Program now allows for this,
   so even Cinemascope looks almost sensible.

   HDTV (1080) viewed on low-res screen (640) will ZOOM OUT by default.
   Amount of Zoom can be controlled through "Z" button on toolbar.

   To allow for previewing HDTV on a small processor,
   option to skip frames when behind, during play/preview processing.
   This is rather rough, so only turn on when really needed.

   RGB mode support for view enhancements is very basic. YUY2 is better.

   Gamma control added to Luminance dialog.

   This allows you to adjust for the difference in luminal linearity
   that exists between TV standards and PC video cards.
   Also, the gain control now acts more like classic TV contrast control.

   This slows down "PLAY" mode, as it is less efficient than the basic
   controls that existed in the previous version, which used MMX.

9) Navigation Keys.
   SHIFT key speeds up the arrow keys. Other new keys defined.
   Because I am impatient.
   "Fast Back" option to speed up backward stepping.

10) Intelligent reset of OUT point
    on subsequent IN selection.
    Now resets OUT to END.

11) Shows how many MegaBytes are selected out of total MB.
    Also checks amount of free space on output FAT drive.
    Estimated time left when outputing large file.

12) Default Output folder and Extension.

13) Visual EOF Indicator.
    Display now shows a visual indicator of EOF reached,
    to clearly distinguish it from the last image shown. (YUY2 mode).

13) Frame Rate Controls.
    Basic frame rate calculation is more accurate.
    Special playback modes added.

14) Sped up Mpeg decoder a little bit.
    Still scope for trimming fat from the processing of component buffers.
    Also installed the new version of IDCTMMX containing SSE2 code by
    Dmitry Rozhdestvensky.

15) Reduced tendency to swamp the system when copying large selections.
    Can be adjusted using the "Slow" option or buffer size Menu control.

16) Shows Time Co-ordinates.
    Current position is shown as time relative to start of file.
    Assumes timestamp sequence is unbroken.
    
    Alternatively co-ordinates can be shown as the actually internal value
    of the Mpeg time stamps, or just a block number within the file.
    Swap formats by clicking on the time display.

    Preference is given to GOP based time codes,
    but can fall back to PTS basis when GOP time missing.

17) MPAlib interface more intelligent.
    Hardware specific versions chosen autiomatically.
    The user no longer needs to understand the difference in hardware
    between models of CPU with different hardware features.

    Repackaged the "Dark Avenger" Audio DLLs into a single ZIP file,
    downloadable from the same page as Mpg2Cut.
    People did not like having to search 2 different sites for downloads.
    However, the actual content of the DLLs has not changed.

    Detect aand correct out-range values returned from MPAlib 
    when in audio format is 16bit.

18) Cleaned up code for grabbing Private Streams.
    but there is more to do on this, especially on Stream 2.
    More info displayed about mpeg audio layer.

19) More info in Title bar and Stats panel.
    Progessive/Interlaced indicators (p/i/Top/Bot) after vertical line count

    3 indicators are shown :-
    - original source progressive frame indicator from Pic Coding Extension
    - progressive_sequence indicator from Sequence Extension
    - Field encode order (Top first/Bottom first/p=progressive encode)

    EXAMPLES :-

        576iiBot means - interlaced original, such as SD TV camera.
                       - Sequence default is interlaced.
                       - encoded with Bottom field first
        576iiTop means - interlaced original, such as SD TV camera.
                       - Sequence default is interlaced.
                       - encoded with Top field first
        576ipp   means - interlaced original, such as SD TV camera.
                       - Sequence default is interlaced.
                       - encoded with as a single frame (stupid)
        576pip   means - noninterlaced original, such as Cine film.
                       - Sequence default is interlaced (stupid).
                       - encoded as a single frame (makes sense)
    
    I wonder why so many files from interlaced field sources
    are encoded as NON-FIELD (Full Frame) pictures, 
    as if they were from a progressive source ?

20) Garbage Collection Command.
    Windows 9x ONLY.
 
    So you've deleted a file and it ain't in the recycle bin.
    You've even looked in the Norton Protected Bin 
    and it ain't there either.
    The File-Garbage-Reclaim command converts free space on disk 
    into a file, to allow browsing of very deleted data.  

    All very wobbly though.  Works best on frequently defragged disks.
 
21) TULEBOX - New Frame by Frame Stepping
    Sep 04 2005 - Tulebox supplied a better idea for frame stepping.
    See EZBOARD posting.

22) Status Line Text Colour can be changed via INI file.
    Colors are specified as 6 hex digits = BBGGRR.
    Notice that Blue (BB) PRECEEDS Green (GG) and Red(RR).
    This is the OPPOSITE of the HTML convention.
 
+ Miscellaneous little changes.


--------------------

ACKNOWLEDGEMENTS

"WeWantWideScreen" for various code.
"TuleBox" for single frame advance.

Many thanks to Brent Beyeler for his "BBtools" package.
This has been very helpful in figuring what the heck is going on !

--------------------




OTHER KNOWN BUGS & LIMITATIONS
===============-==============

0) Files larger than 2GB may *NOT* be accepted by the host file system.
   Although Mpg2Cut2 can create large files,
   the File System may not accept it.
   ISO DVD-ROM files must NOT exceed 2GB,
   but you won't find out UNTIL you try to burn it.

1) Timestamps are not corrected at joins.
   Some players will pause at the join,
   or become confused in other ways.
   
2) Buffer Bitrate limits are not checked.
   Players with limited capabilities may not like the joins.

3) Bad MPEG data MAY still cause crashes,
   BUT it's a LOT less frequent in this version.

   Since the macro-block decoder is a mystery to me,
   there are still problems that I haven't allowed for.

4) Always puts an MPEG header at the front,
   even if  you don't really want it to.
   However, you can set "preamble" to SMALL
   to reduce the amount of crud.

   Setting preamble to NONE corresponds to ver 1.15 of Mpeg2Cut,
   except that the System Header is slightly more general.

5) After applying SP5 service pack, EXE grew by 20% overnight,
   and I started getting more crashes. Don't like this.

6) In DirectX9, when overlay unavailable, 
   not automatically falling back to RGB. 
   Use the Hardware menu to manually select RGB mode.

7) Luminance control might benefit from being rewritten in MMX.

8) Some Mpeg audio tracks don't play properly.
   EG low-bit-rate mono.

9) Navigation - Slow response.
   Side-effect of the way audio is buffered during PLAY mode.

10) Black screen during luminance adjust.
    Not sure why this happens sometimes.
    Just use the arrow keys to refresh the screen.


MPEG DECODE ERROR CODES
=======================

Fault_Flag = 1;   // Invalid DCT table code
Fault_Flag = 2;   // Invalid Macro Block type for this Picture type
Fault_Flag = 3;   // Invalid Coded Block Pattern
Fault_Flag = 4;   // Invalid macroblock_address_increment
Fault_Flag = 5;   // Invalid motion_type
Fault_Flag = 10;  // Invalid motion_code

Fault_Flag = 11;  // Invalid start code within Picture data
                     BUG - Triggered when STOP PLAYING 

Fault_Flag = 42;  // Unknown Pic type - not I,P,B

AC3 in MPA pkt.   // The input is muxed incorrectly.
                  // It uses AC3 audio encoding,
                  // been muxed inside Mpeg Audio packets.

