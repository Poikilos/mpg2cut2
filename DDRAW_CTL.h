
/* DirectDraw & GDI resources */

#include <ddraw.h>
LPDIRECTDRAWSURFACE lpPrimary, lpOverlay;
DDSURFACEDESC ddsd;


DDPIXELFORMAT ddPixelFormat;

DWORD DDColorMatch(LPDIRECTDRAWSURFACE, COLORREF);
LPDIRECTDRAW	lpDD;
LPDIRECTDRAW2 lpDD2;
DDCAPS halcaps;
DDOVERLAYFX ddofx;

