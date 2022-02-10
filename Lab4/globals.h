
#define SQR(x) ((x)*(x))	/* macro for square */
#ifndef M_PI			/* in case M_PI not found in math.h */
#define M_PI 3.1415927
#endif
#ifndef M_E
#define M_E 2.718282
#endif
#define MAX_QUEUE 1000
#define MAX_FILENAME_CHARS	320

char	filename[MAX_FILENAME_CHARS];

HWND	MainWnd;

// Display flags
int		ShowPixelCoords;

// Image data
unsigned char* OriginalImage;
int				ROWS, COLS;

#define TIMER_SECOND	1			/* ID of timer used for animation */

// Drawing flags
int		TimerRow, TimerCol;
int		ThreadRow, ThreadCol;
int		ThreadRunning;
int		BigDots;
int		mouse_x, mouse_y;
int		Pixel_intensity, distance_p;
char	x[255], y[255];
int		RegionGrow;
int		R1, G1, B1;
int		pixel_intensity;
int		distance_pixel, r, c, JPress, count, PlayMode, StepMode, Stop_all_threads, file_loaded, total_threads;

// Function prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void PaintImage();
void AnimationThread(void*);		/* passes address of window */
void ExplosionThread(void*);		/* passes address of window */
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
void region_grow();