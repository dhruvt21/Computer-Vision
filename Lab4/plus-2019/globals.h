#define MAX_QUEUE 10000	/* max perimeter size (pixels) of border wavefront */

#define SQR(x) ((x)*(x))	/* macro for square */
#ifndef M_PI			/* in case M_PI not found in math.h */
#define M_PI 3.1415927
#endif
#ifndef M_E
#define M_E 2.718282
#endif

#define MAX_FILENAME_CHARS	320

char 	ext[MAX_FILENAME_CHARS];
char	filename[MAX_FILENAME_CHARS];

HWND	MainWnd;

// Display flags
int		ShowPixelCoords;
int		BigCoords;

int count;
int PlayMode, StepMode;
int Jpress;
//Region grow
int r, c;
int ROWS, COLS;
int	RegionGrow;
int R1, G1, B1;
int pixel_intensity;
int distance_pixel;
char intensity[256];
char dis[256];
int total_threads;
int Stop_all_threads;
int file_loaded;

int max_points = 1000;

//Active Contour rubber band 
int Active_contour;
int Active_index;
int contour_index;
int LButton;
int* Aux_Scalex;
int* Aux_Sacley;
int* Scalex;
int* Sacley;
int window_size;
float Average_Distance;

double* external_energy;
int points_drawn;
int points_drawn_ballon;

int window;

double min, max;
////Active Contour Ballon
int seedx, seedy;
int contour_index_ballon;
int RButton_ballon;
int* Aux_Scalex_ballon;
int* Aux_Sacley_ballon;
int* Scalex_ballon;
int* Sacley_ballon;
int window_size_ballon;
float Average_Distance_ballon;

////Moving 
int Moving;
int Moving_index;
///
int PixelWidth = 2;



// Image data
unsigned char* OriginalImage;
unsigned char* DummyImage;
int				ROWS, COLS;

#define TIMER_SECOND	1			/* ID of timer used for animation */

// Drawing flags
int		TimerRow, TimerCol;
int		ThreadRow, ThreadCol;
int		ThreadRunning;
int distance = 10;
// Function prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void PaintImage();
void AnimationThread(void*);
void region_grow(void*);
void downSample();
void rubberBand();
void Ballon();
void Sobbel();
void LoadPixel(int, int);
void findminmax(double* array, int size);
void drawContour(int*, int*, int);
void CheckTension(float, int, int, int*, int*);
void average();