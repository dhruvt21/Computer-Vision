
#define SQR(x) ((x)*(x))	/* macro for square */
#ifndef M_PI			/* in case M_PI not found in math.h */
#define M_PI 3.1415927
#endif
#ifndef M_E
#define M_E 2.718282
#endif

#define MAX_FILENAME_CHARS	320
#define WINDOW 7

char	filename[MAX_FILENAME_CHARS];

HWND	MainWnd;

		// Display flags
int		ShowPixelCoords;

		// Image data
unsigned char	*OriginalImage, *image;
int				ROWS,COLS;

#define TIMER_SECOND	1			/* ID of timer used for animation */

		// Drawing flags
int		TimerRow,TimerCol;
int		ThreadRow,ThreadCol;
int		ThreadRunning;
int		BigDots;
int		mouse_x,mouse_y;
int		file_loaded;
int ROWS, COLS;

int pixel_width = 2;

		//drawing the contour
int Lbutton;
int max_points = 1000, active_ind;
int* temp_x, * temp_y, * X, * Y;
int stop_all_threads, active_ind, moving, active_contour, run_contour, contour_ind, points_drawn, moving_ind;

		//rubber band model
int w_size;
char itr[20];
unsigned char* sobl_image;
double* external_energy, avg_dist;
int *temp_row, *temp_col;
double min, max;

		//baloon model
int contour_ind_b, *X_b, *Y_b, *temp_x_b, *temp_y_b, contour_index_ballon;
int seedx, seedy;
double avg_dist_b;

		// Function prototypes
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
void PaintImage();
void AnimationThread(void *);		/* passes address of window */
void ExplosionThread(void*);		/* passes address of window */
void rubber_band(void*);
void sobel();
void down_sample();
void draw_contour(int*, int*, int);
void findminmax(double*);
void average_distance();
void initial_b(int, int);
void LoadPixel(int, int);
void average_distance_b();
void ballon();

