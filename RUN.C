/* run.c
 *
 * Obnoxious Window by Alan Bishop (agb@ecsvax.uncecs.edu)
 *
 * The /h (hard) option turns off the tasklist flag
 *
 */

#define INCL_WIN
#include <os2.h>

#include <stdio.h>
#include <math.h>
#include <string.h>

MRESULT EXPENTRY ClientWndProc (HWND, USHORT, MPARAM, MPARAM);
void move_window(void);
void calculate_push(int x1, int y1, int x2, int y2, double *cx, double *cy, int mult);

#define ID_TIMER	1

HAB hab;
HWND hwndFrame;
static SWP frame;

int
main(int argc, char *argv[])
{
	static CHAR  szClientClass[] = "Welcome";
	static ULONG flFrameFlags = 	FCF_SYSMENU | FCF_BORDER | FCF_TASKLIST;
	HMQ hmq;
	HWND hwndClient;
	QMSG qmsg;

/* Turn on hard option */
	if (argc == 2 && !strcmp(argv[1], "/h")) flFrameFlags &= ~FCF_TASKLIST;

	hab = WinInitialize(0);
	hmq = WinCreateMsgQueue(hab, 0);

	WinRegisterClass(hab, (PCSZ) szClientClass, (PFNWP) ClientWndProc, 0L, 0);

	hwndFrame = WinCreateStdWindow(HWND_DESKTOP, 0L,
		&flFrameFlags, (PCSZ) szClientClass, NULL, 0L, 0, 0, &hwndClient);

	WinSetWindowPos(hwndFrame, 0, 100, 100, 50, 50,
		SWP_MOVE | SWP_SIZE | SWP_SHOW);

	WinSendMsg(hwndFrame, WM_SETICON, (MPARAM) WinQuerySysPointer(HWND_DESKTOP,
		SPTR_APPICON, FALSE), NULL);

	WinStartTimer(hab, hwndFrame, ID_TIMER, 200);

/* Figure out how big the desktop is */
	WinQueryWindowPos(HWND_DESKTOP, &frame);

	while (WinGetMsg(hab, &qmsg, 0, 0, 0)) {

		WinDispatchMsg(hab, &qmsg);

/* Move the window every time we get a message */
		move_window();
	}

	WinDestroyWindow(hwndFrame);
	WinDestroyMsgQueue(hmq);
	WinTerminate(hab);
	return(0);
}

MRESULT EXPENTRY
ClientWndProc(HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
{
	HPS hps;

	switch (msg) {

/* Just keep the screen clear */
		case WM_PAINT :
			hps = WinBeginPaint(hwnd, 0, NULL);

			GpiErase(hps);

			WinEndPaint(hps);
			return(0);

		default :
			return(WinDefWindowProc(hwnd, msg, mp1, mp2));

	}
}

/* Recalculate where the window should be */
void
move_window()
{
	SWP ipos;
	POINTL mpos;
	static double rdx = 0.0, rdy = 0.0;

	WinQueryPointerPos(HWND_DESKTOP, &mpos);
	WinQueryWindowPos(hwndFrame, &ipos);

/* Check bounds and bounce off of edges */
	if (ipos.x < 2) {
		ipos.x = 1;
		rdx = 1.0;
	}
	if (ipos.y < 2) {
		ipos.y = 1;
		rdy = 1.0;
	}
	if (ipos.x > frame.cx - 52) {
		ipos.x = frame.cx - 51;
		rdx = -1.0;
	}
	if (ipos.y > frame.cy - 52) {
		ipos.y = frame.cy - 51;
		rdy = -1.0;
	}

/* Calculate the push from: the four corners, the four points of the
 * edge of the screen where horizontal and vertical lines meet,
 * and the mouse
 */
	calculate_push(0           , 0           , ipos.x, ipos.y, &rdx, &rdy, 1);
	calculate_push(0           , frame.cy    , ipos.x, ipos.y, &rdx, &rdy, 1);
	calculate_push(frame.cx    , 0           , ipos.x, ipos.y, &rdx, &rdy, 1);
	calculate_push(frame.cx    , frame.cy    , ipos.x, ipos.y, &rdx, &rdy, 1);
	calculate_push(ipos.x      , 0           , ipos.x, ipos.y, &rdx, &rdy, 2);
	calculate_push(ipos.x      , frame.cy    , ipos.x, ipos.y, &rdx, &rdy, 2);
	calculate_push(0           , ipos.y      , ipos.x, ipos.y, &rdx, &rdy, 2);
	calculate_push(frame.cx    , ipos.y      , ipos.x, ipos.y, &rdx, &rdy, 2);
	calculate_push((int) mpos.x, (int) mpos.y, ipos.x, ipos.y, &rdx, &rdy, 8);

	ipos.x += (int) rdx;
	ipos.y += (int) rdy;

	WinSetWindowPos(hwndFrame, 0, ipos.x, ipos.y, 0, 0,
		SWP_MOVE);
}

void
calculate_push(int x1, int y1, int x2, int y2, double *cx, double *cy, int mult)
/* Figure out how much we should change the acceleration */
{
#define sign(x) (x > 0 ? 1 : x < 0 ? -1 : 0)
	double dist;
	double chx, chy;
	double mag;
	double dx, dy;

	dx = (double) (x1 - x2);
	dy = (double) (y1 - y2);

	dist = dx*dx + dy*dy;

	if (dist == 0.0) dist = 0.0001;
	if (dx == 0.0) dx = 0.0001;

	mag = 8000/dist*(double)mult;
	chx = mag*sign(x2 - x1);
	chy = mag*sign(y2 - y1);

	if (chx > 10.0) chx = 10.0;
	if (chy > 10.0) chy = 10.0;
	if (chx < -10.0) chx = -10.0;
	if (chy < -10.0) chy = -10.0;
	*cx += chx;
	*cy += chy;
}
