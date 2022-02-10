
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h>
#include <windows.h>
#include <wingdi.h>
#include <winuser.h>
#include <process.h>	/* needed for multithreading */
#include "resource.h"
#include "globals.h"


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				LPTSTR lpCmdLine, int nCmdShow)

{
MSG			msg;
HWND		hWnd;
WNDCLASS	wc;

wc.style=CS_HREDRAW | CS_VREDRAW;
wc.lpfnWndProc=(WNDPROC)WndProc;
wc.cbClsExtra=0;
wc.cbWndExtra=0;
wc.hInstance=hInstance;
wc.hIcon=LoadIcon(hInstance,"ID_PLUS_ICON");
wc.hCursor=LoadCursor(NULL,IDC_ARROW);
wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
wc.lpszMenuName="ID_MAIN_MENU";
wc.lpszClassName="PLUS";

if (!RegisterClass(&wc))
  return(FALSE);

hWnd=CreateWindow("PLUS","plus program",
		WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
		CW_USEDEFAULT,0,400,400,NULL,NULL,hInstance,NULL);
if (!hWnd)
  return(FALSE);

ShowScrollBar(hWnd,SB_BOTH,FALSE);
ShowWindow(hWnd,nCmdShow);
UpdateWindow(hWnd);
MainWnd=hWnd;

ShowPixelCoords=0;
BigDots=0;
Lbutton = 0;
stop_all_threads = 0;
file_loaded = 0;
points_drawn = 0;
active_ind = 0;
contour_ind = 0;

strcpy(filename,"");
OriginalImage=NULL;
ROWS=COLS=0;

InvalidateRect(hWnd,NULL,TRUE);
UpdateWindow(hWnd);

while (GetMessage(&msg,NULL,0,0))
  {
  TranslateMessage(&msg);
  DispatchMessage(&msg);
  }
return(msg.wParam);
}




LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg,
	WPARAM wParam, LPARAM lParam)

{
	HMENU				hMenu;
	OPENFILENAME		ofn;
	FILE* fpt;
	HDC					hDC;
	char				header[320], text[320];
	int					BYTES, xPos, yPos, x, y;

	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_SHOWPIXELCOORDS:
			ShowPixelCoords = (ShowPixelCoords + 1) % 2;
			PaintImage();
			break;
		case ID_DISPLAY_BIGDOTS:
			BigDots = (BigDots + 1) % 2;
			PaintImage();
			break;
		case ID_FILE_LOAD:
		{
			if (OriginalImage != NULL)
			{
				free(OriginalImage);
				OriginalImage = NULL;
			}
			memset(&(ofn), 0, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.lpstrFile = filename;
			filename[0] = 0;
			ofn.nMaxFile = MAX_FILENAME_CHARS;
			ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY;
			ofn.lpstrFilter = "PPM files\0*.ppm\0PNM files\0*.pnm\0All files\0*.*\0\0";
			if (!(GetOpenFileName(&ofn)) || filename[0] == '\0')
				break;		/* user cancelled load */
			if ((fpt = fopen(filename, "rb")) == NULL)
			{
				MessageBox(NULL, "Unable to open file", filename, MB_OK | MB_APPLMODAL);
				break;
			}
			fscanf(fpt, "%s %d %d %d", header, &COLS, &ROWS, &BYTES);
			if ((strcmp(header, "P5") != 0 && strcmp(header, "P6") != 0) || BYTES != 255)
			{
				MessageBox(NULL, "Not a PPM (P5 greyscale) image", filename, MB_OK | MB_APPLMODAL);
				fclose(fpt);
				break;
			}
			if (header[1] == '5')
			{
				OriginalImage = (unsigned char*)calloc(ROWS * COLS, 1);
				header[0] = fgetc(fpt);	/* whitespace character after header */
				fread(OriginalImage, 1, ROWS * COLS, fpt);
				file_loaded = 1;
				fclose(fpt);
			}
			else
			{
				OriginalImage = (unsigned char*)calloc(ROWS * COLS, 1);
				image = (unsigned char*)calloc(ROWS * COLS * 3, 1);
				header[0] = fgetc(fpt);
				fread(image, 1, ROWS * COLS * 3, fpt);
				int ind1 = 0;
				for (int i = 0; i < ROWS * COLS * 3; i+=3)
				{
					OriginalImage[ind1] = (image[i] + image[i + 1] + image[i + 2]) / 3;
					ind1++;
				}
				file_loaded = 1;
				fclose(fpt);
			}
			SetWindowText(hWnd, filename);
			PaintImage();
			sobel();
		}
		break;

		case ID_FILE_QUIT:
			DestroyWindow(hWnd);
			break;

		case ID_ACTIVECONTOUR_ACTIVATE:
		{
			active_contour = (active_contour + 1) % 2;
			active_ind = 0;
			stop_all_threads = 0;
			moving = 0;
		}
		break;
		case ID_ACTIVECONTOUR_CLEAR:
		{
			points_drawn = active_ind = contour_ind = contour_ind_b = 0;
			active_contour = (active_contour + 1) % 2;
			X = (int*)calloc(max_points, sizeof(int));
			Y = (int*)calloc(max_points, sizeof(int));
			X_b = (int*)calloc(max_points, sizeof(int));
			Y_b = (int*)calloc(max_points, sizeof(int));
			stop_all_threads = 1;
			moving = 0;
			PaintImage();
		}
		break;
		case ID_ACTIVECONTOUR_RUN:
		{
			_beginthread(rubber_band, 0, NULL);
		}
		break;
		}
		case WM_SIZE:		  /* could be used to detect when window size changes */
			PaintImage();
			return(DefWindowProc(hWnd, uMsg, wParam, lParam));
			break;
		case WM_PAINT:
			PaintImage();
			return(DefWindowProc(hWnd, uMsg, wParam, lParam));
			break;
		case WM_LBUTTONDOWN:
		{
			if (active_contour == 1 && Lbutton == 0 && points_drawn == 0 && moving == 0 && !(GetKeyState(VK_SHIFT) < 0))
			{
				Lbutton = 1;
				active_ind = 0;
				contour_ind = 0;
				temp_x = (int*)calloc(max_points, sizeof(int));
				temp_y = (int*)calloc(max_points, sizeof(int));
				X = (int*)calloc(max_points, sizeof(int));
				Y = (int*)calloc(max_points, sizeof(int));
			}

			if (active_contour == 1 && points_drawn == 1 && moving == 0 && GetKeyState(VK_SHIFT) < 0)
			{
				stop_all_threads = 0;
				xPos = LOWORD(lParam);
				yPos = HIWORD(lParam);
				int minX, minY, maxX, maxY;
				minX = xPos - pixel_width;
				maxX = xPos + pixel_width;
				minY = yPos - pixel_width;
				maxY = yPos + pixel_width;

				moving_ind = -1;
				if (contour_ind != 0)
				{
					for (int i = 0; i < contour_ind; i++)
					{
						if ((X[i] > minX && X[i] < maxX) && (Y[i] > minY && Y[i] < maxY))
						{
							moving_ind = i;
								moving = 1;
								break;
						}
					}
				}

				if(contour_ind_b != 0) 
				{
					for (int i = 0; i < contour_ind_b; i++) 
					{
						if ((X_b[i] > minX && X_b[i] < maxX) && (Y_b[i] > minY && Y_b[i] < maxY)) 
						{
							moving_ind = i;
							moving = 1;
							break;
						}
					}
				}


			}
			return(DefWindowProc(hWnd, uMsg, wParam, lParam));
		}
		break;
		case WM_LBUTTONUP:
		{
			if (Lbutton == 1 && points_drawn == 0 && moving == 0)
			{
				Lbutton = 0;
				down_sample();
			}
			if (active_contour == 1 && points_drawn == 1 && moving == 1)
			{
				moving = 0;
			}
			return(DefWindowProc(hWnd, uMsg, wParam, lParam));
		}
		break;
		case WM_RBUTTONDOWN:
		{
			xPos = LOWORD(lParam);
			yPos = HIWORD(lParam);
			if (active_contour == 1) 
			{
				seedx = xPos;
				seedy = yPos;
				initial_b(seedx, seedy);
				_beginthread(ballon, 1, NULL);
			}
			//_beginthread(ExplosionThread, 0, MainWnd);	/* start up a child thread to do other work while this thread continues GUI */
			return(DefWindowProc(hWnd, uMsg, wParam, lParam));
		}
		break;
		case WM_MOUSEMOVE:
			if (ShowPixelCoords == 1)
			{
				xPos = LOWORD(lParam);
				yPos = HIWORD(lParam);
				if (xPos >= 0 && xPos < COLS && yPos >= 0 && yPos < ROWS)
				{
					sprintf(text, "%d,%d=>%d     ", xPos, yPos, OriginalImage[yPos * COLS + xPos]);
					hDC = GetDC(MainWnd);
					TextOut(hDC, 0, 0, text, strlen(text));		/* draw text on the window */
					if (BigDots == 0)
						SetPixel(hDC, xPos, yPos, RGB(255, 0, 0));	/* color the cursor position red */
					else
					{
						for (x = -2; x <= 2; x++)
							for (y = -2; y <= 2; y++)
								SetPixel(hDC, xPos + x, yPos + y, RGB(255, 0, 0));
					}
					ReleaseDC(MainWnd, hDC);
				}
			}
			if (Lbutton == 1 && points_drawn == 0 && moving == 0)
			{
				HDC hDC;
				hDC = GetDC(MainWnd);
				xPos = LOWORD(lParam);
				yPos = HIWORD(lParam);
				temp_x[active_ind] = xPos;
				temp_y[active_ind] = yPos;

				for (int r = -2; r <= 2; r++)
				{
					for (int c = -2; c <= 2; c++)
					{
						SetPixel(hDC, temp_x[active_ind] + r, temp_y[active_ind] + c, RGB(255, 0, 0));
					}
				}
				sprintf(text, "Point = %d Max Points - %d", active_ind, max_points);
				hDC = GetDC(MainWnd);
				TextOut(hDC, 0, 0, text, strlen(text));
				active_ind++;
			}

			if (active_contour == 1 && points_drawn == 1 && moving == 1)
			{
				PaintImage();
				if (contour_ind != 0)
				{
					X[moving_ind] = LOWORD(lParam);
					Y[moving_ind] = HIWORD(lParam);
					draw_contour(X, Y,contour_ind);
				}
				if (contour_ind_b != 0) {
					X_b[moving_ind] = LOWORD(lParam);
					Y_b[moving_ind] = HIWORD(lParam);
					draw_contour(X_b, Y_b,contour_ind_b);
				}
			}

			
			return(DefWindowProc(hWnd, uMsg, wParam, lParam));
			break;
		case WM_KEYDOWN:
			if (wParam == 's' || wParam == 'S')
				PostMessage(MainWnd, WM_COMMAND, ID_SHOWPIXELCOORDS, 0);	  /* send message to self */
			if ((TCHAR)wParam == '1')
			{
				TimerRow = TimerCol = 0;
				SetTimer(MainWnd, TIMER_SECOND, 10, NULL);	/* start up 10 ms timer */
			}
			if ((TCHAR)wParam == '2')
			{
				KillTimer(MainWnd, TIMER_SECOND);			/* halt timer, stopping generation of WM_TIME events */
				PaintImage();								/* redraw original image, erasing animation */
			}
			if ((TCHAR)wParam == '3')
			{
				ThreadRunning = 1;
				_beginthread(AnimationThread, 0, MainWnd);	/* start up a child thread to do other work while this thread continues GUI */
			}
			if ((TCHAR)wParam == '4')
			{
				ThreadRunning = 0;							/* this is used to stop the child thread (see its code below) */
			}
			return(DefWindowProc(hWnd, uMsg, wParam, lParam));
			break;
		case WM_TIMER:	  /* this event gets triggered every time the timer goes off */
			hDC = GetDC(MainWnd);
			SetPixel(hDC, TimerCol, TimerRow, RGB(0, 0, 255));	/* color the animation pixel blue */
			ReleaseDC(MainWnd, hDC);
			TimerRow++;
			TimerCol += 2;
			break;
		case WM_HSCROLL:	  /* this event could be used to change what part of the image to draw */
			PaintImage();	  /* direct PaintImage calls eliminate flicker; the alternative is InvalidateRect(hWnd,NULL,TRUE); UpdateWindow(hWnd); */
			return(DefWindowProc(hWnd, uMsg, wParam, lParam));
			break;
		case WM_VSCROLL:	  /* this event could be used to change what part of the image to draw */
			PaintImage();
			return(DefWindowProc(hWnd, uMsg, wParam, lParam));
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return(DefWindowProc(hWnd, uMsg, wParam, lParam));
			break;
		
	}
	hMenu = GetMenu(MainWnd);
	if (ShowPixelCoords == 1)
		CheckMenuItem(hMenu, ID_SHOWPIXELCOORDS, MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
	else
		CheckMenuItem(hMenu, ID_SHOWPIXELCOORDS, MF_UNCHECKED);
	if (BigDots == 1)
		CheckMenuItem(hMenu, ID_DISPLAY_BIGDOTS, MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
	else
		CheckMenuItem(hMenu, ID_DISPLAY_BIGDOTS, MF_UNCHECKED);
	if(active_contour == 1)
		CheckMenuItem(hMenu, ID_ACTIVECONTOUR_ACTIVATE, MF_CHECKED);
	else
		CheckMenuItem(hMenu, ID_ACTIVECONTOUR_ACTIVATE, MF_UNCHECKED);
	if(run_contour == 1)
		CheckMenuItem(hMenu, ID_ACTIVECONTOUR_RUN, MF_CHECKED);
	else
		CheckMenuItem(hMenu, ID_ACTIVECONTOUR_RUN, MF_UNCHECKED);
	DrawMenuBar(hWnd);

	return(0L);
}




void PaintImage()

{
PAINTSTRUCT			Painter;
HDC					hDC;
BITMAPINFOHEADER	bm_info_header;
BITMAPINFO			*bm_info;
int					i,r,c,DISPLAY_ROWS,DISPLAY_COLS;
unsigned char		*DisplayImage;

if (OriginalImage == NULL)
  return;		/* no image to draw */

		/* Windows pads to 4-byte boundaries.  We have to round the size up to 4 in each dimension, filling with black. */
DISPLAY_ROWS=ROWS;
DISPLAY_COLS=COLS;
if (DISPLAY_ROWS % 4 != 0)
  DISPLAY_ROWS=(DISPLAY_ROWS/4+1)*4;
if (DISPLAY_COLS % 4 != 0)
  DISPLAY_COLS=(DISPLAY_COLS/4+1)*4;
DisplayImage=(unsigned char *)calloc(DISPLAY_ROWS*DISPLAY_COLS,1);
for (r=0; r<ROWS; r++)
  for (c=0; c<COLS; c++)
	DisplayImage[r*DISPLAY_COLS+c]=OriginalImage[r*COLS+c];

BeginPaint(MainWnd,&Painter);
hDC=GetDC(MainWnd);
bm_info_header.biSize=sizeof(BITMAPINFOHEADER); 
bm_info_header.biWidth=DISPLAY_COLS;
bm_info_header.biHeight=-DISPLAY_ROWS; 
bm_info_header.biPlanes=1;
bm_info_header.biBitCount=8; 
bm_info_header.biCompression=BI_RGB; 
bm_info_header.biSizeImage=0; 
bm_info_header.biXPelsPerMeter=0; 
bm_info_header.biYPelsPerMeter=0;
bm_info_header.biClrUsed=256;
bm_info_header.biClrImportant=256;
bm_info=(BITMAPINFO *)calloc(1,sizeof(BITMAPINFO) + 256*sizeof(RGBQUAD));
bm_info->bmiHeader=bm_info_header;
for (i=0; i<256; i++)
  {
  bm_info->bmiColors[i].rgbBlue=bm_info->bmiColors[i].rgbGreen=bm_info->bmiColors[i].rgbRed=i;
  bm_info->bmiColors[i].rgbReserved=0;
  } 

SetDIBitsToDevice(hDC,0,0,DISPLAY_COLS,DISPLAY_ROWS,0,0,
			  0, /* first scan line */
			  DISPLAY_ROWS, /* number of scan lines */
			  DisplayImage,bm_info,DIB_RGB_COLORS);


if (contour_ind > 2)
	draw_contour(X, Y,contour_ind);
if (contour_ind_b > 2)
	draw_contour(X_b, Y_b, contour_ind_b);
ReleaseDC(MainWnd,hDC);
EndPaint(MainWnd,&Painter);

free(DisplayImage);
free(bm_info);
}




void AnimationThread(HWND AnimationWindowHandle)

{
HDC		hDC;
char	text[300];

ThreadRow=ThreadCol=0;
while (ThreadRunning == 1)
  {
  hDC=GetDC(MainWnd);
  SetPixel(hDC,ThreadCol,ThreadRow,RGB(0,255,0));	/* color the animation pixel green */
  sprintf(text,"%d,%d     ",ThreadRow,ThreadCol);
  TextOut(hDC,300,0,text,strlen(text));		/* draw text on the window */
  ReleaseDC(MainWnd,hDC);
  ThreadRow+=3;
  ThreadCol++;
  Sleep(100);		/* pause 100 ms */
  }
}

void sobel() {
	int sobelx[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1 };
	int sobely[] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };
	int smin = 10000, smax = -9999, r, c, r1, c1, gx, gy, ind;
	external_energy = (double*)calloc(ROWS * COLS, sizeof(double));
	sobl_image = (unsigned char*)calloc(ROWS * COLS, 1);

	for (r = 1; r < ROWS - 1; r++)
	{
		for (c = 1; c < COLS - 1; c++)
		{
			gx = 0;
			gy = 0;
			ind = 0;
			for (r1 = -1; r1 <= 1; r1++)
			{
				for (c1 = -1; c1 <= 1; c1++)
				{
					gx += OriginalImage[((r + r1) * COLS) + c + c1] * sobelx[ind];
					gy += OriginalImage[((r + r1) * COLS) + c + c1] * sobely[ind];
					ind++;
				}
			}
			external_energy[r * COLS + c] = sqrt(pow(gx, 2) + pow(gy, 2));
			if (smin > external_energy[r * COLS + c]) 
				smin = external_energy[r * COLS + c];
			if (smax < external_energy[r * COLS + c]) 
				smax = external_energy[r * COLS + c];
		}
	}

	for (r = 0; r < ROWS * COLS; r++)
	{
		sobl_image[r] = (unsigned char)((external_energy[r] - smin) * 255) / (smax - smin);
		external_energy[r] = -((external_energy[r] - smin) / (smax - smin));
	}
}

void down_sample()
{
	contour_ind = 0;
	for (int i = 0; i < active_ind; i += 5)
	{
		X[contour_ind] = temp_x[i];
		Y[contour_ind] = temp_y[i];
		contour_ind++;
	}
	points_drawn = 1;
	PaintImage(); 
	temp_x = (int*)calloc(contour_ind, sizeof(int));
	temp_y = (int*)calloc(contour_ind, sizeof(int));
}



void draw_contour(int* X, int* Y, int f)
{
	HDC hDC;
	hDC = GetDC(MainWnd);

	for (int i = 0; i < f; i++)
	{
		for (int x = -pixel_width; x <= pixel_width; x++)
		{
			for (int y = -pixel_width; y <= pixel_width; y++)
			{
				SetPixel(hDC, X[i] + x, Y[i] + y, RGB(255, 0, 0));
			}
		}
	}
}

void rubber_band() {
	HDC hDC;
	int size = floor(WINDOW / 2);
	hDC = GetDC(MainWnd);
	for(int k = 0; k < 400; k++) 
	{
		double* internal_1 = (double*)calloc(WINDOW*WINDOW, sizeof(double));
		double* internal_2 = (double*)calloc(WINDOW*WINDOW, sizeof(double));
		average_distance();
		int iterate = 0;
		while (iterate < contour_ind) 
		{
			int Ccon = X[iterate];
			int Rcon = Y[iterate];
			int Ccon1;
			int Rcon1;
			temp_x[iterate] = X[iterate];
			temp_y[iterate] = Y[iterate];

			if (iterate != (contour_ind - 1)) {
				Ccon1 = X[iterate + 1];
				Rcon1 = Y[iterate + 1];
			}
			else {
				Ccon1 = X[0];
				Rcon1 = Y[0];
			}


			int index = 0;
			for (int r = -size; r <= size; r++) {
				for (int c = -size; c <= size; c++) {
					int cols = Ccon + c;
					int rows = Rcon + r;
					internal_1[index] = pow((cols - Ccon1), 2) + pow((rows - Rcon1), 2);
					internal_2[index] = pow((sqrt(internal_1[index])) - avg_dist, 2);
					index++;
				}
			}


			findminmax(internal_1, WINDOW);
			for (int i = 0; i < WINDOW*WINDOW; i++) {
				internal_1[i] = (internal_1[i] - min) / (max - min);
			}


			findminmax(internal_2, WINDOW);
			for (int i = 0; i < WINDOW*WINDOW; i++) {
				internal_2[i] = (internal_2[i] - min) / (max - min);

			}



			index = 0;
			double min1 = 1000;
			int locr, locc;
			for (int r = -size; r <= size; r++) {
				for (int c = -size; c <= size; c++) {
					double xyz = internal_2[index] + (internal_1[index])
						+ (external_energy[(r + Rcon) * COLS + (c + Ccon)]);
					index++;
					if (min1 > xyz)
					{
						min1 = xyz;
						locr = r + Rcon;
						locc = c + Ccon;
					}
				}
			}
			temp_x[iterate] = locc;
			temp_y[iterate] = locr;
			iterate++;
		}


		for (int i = 0; i < contour_ind; i++) {
			X[i] = temp_x[i];
			Y[i] = temp_y[i];
		}


		Sleep(100);
		if (stop_all_threads == 1) {
			_endthread();
		}

		PaintImage();
	}


}
void findminmax(double* arr, int s) {
	min = arr[0]; max = 0;
	for (int i = 0; i < s*s; i++) {

		if (min > arr[i]) {
			min = arr[i];
		}
		if (max < arr[i]) {
			max = arr[i];
		}

	}
}

void average_distance() {
	double dist = 0;
	for (int i = 0; i < contour_ind; i++) {
		if (i != (contour_ind - 1)) {
			dist = dist + sqrt(pow((X[i] - X[i + 1]), 2) + pow(Y[i] - Y[i + 1], 2));
		}
		else {
			dist = dist + sqrt(pow((X[i] - X[0]), 2) + pow(Y[i] - Y[0], 2));
		}
	}

	avg_dist = dist / contour_ind;
}

void average_distance_b() {
	double dist = 0;
	for (int i = 0; i < contour_ind_b; i++) {
		if (i != (contour_ind_b - 1)) {
			dist = dist + sqrt(pow((X_b[i] - X_b[i + 1]), 2) + pow(Y_b[i] - Y_b[i + 1], 2));
		}
		else {
			dist = dist + sqrt(pow((X_b[i] - X_b[0]), 2) + pow(Y_b[i] - Y_b[0], 2));
		}
	}

	avg_dist_b = dist / contour_ind_b;
}


void initial_b(int x, int y) {

	HDC hDC;
	hDC = GetDC(MainWnd);
	int radius = 10;
	temp_x_b = (int*)calloc(50, sizeof(int));
	temp_y_b = (int*)calloc(50, sizeof(int));
	X_b = (int*)calloc(50, sizeof(int));
	Y_b = (int*)calloc(50, sizeof(int));
	int x1 = x - radius, x2 = x + radius;
	int y1 = y - radius, y2 = y + radius;
	int index = 0;
	contour_ind_b = 0;
	for (int i = 0; i < radius; i++) {
		temp_x_b[contour_ind_b] = x1 + i;
		temp_y_b[contour_ind_b] = y - i;
		contour_ind_b++;
	}

	for (int i = 0; i < radius; i++) {
		temp_x_b[contour_ind_b] = x + i;
		temp_y_b[contour_ind_b] = y1 + i;
		contour_ind_b++;
	}

	for (int i = 0; i < radius; i++) {
		temp_x_b[contour_ind_b] = x2 - i;
		temp_y_b[contour_ind_b] = y + i;
		contour_ind_b++;
	}
	for (int i = 0; i < radius; i++) {
		temp_x_b[contour_ind_b] = x - i;
		temp_y_b[contour_ind_b] = y2 - i;
		contour_ind_b++;
	}

	for (int i = 0; i < contour_ind_b; i += 3) {
		X_b[index] = temp_x_b[i];
		Y_b[index] = temp_y_b[i];
		SetPixel(hDC, X_b[index], Y_b[index], RGB(255, 0, 0));

		index++;
	}

	draw_contour(X_b, Y_b,index);
	contour_ind_b = index;
	points_drawn = 1;
}

void ballon()
{

	HDC hDC;
	int size = 5;
	int window_size = 121;
	hDC = GetDC(MainWnd);

	for (int k1 = 0; k1 < 70; k1++)
	{
		double* internal_1 = (double*)calloc(window_size, sizeof(double));
		double* internal_2 = (double*)calloc(window_size, sizeof(double));

		average_distance_b();
		int iterate = 0;
		while (iterate < contour_ind_b)
		{
			int Ccon = X_b[iterate];
			int Rcon = Y_b[iterate];
			int Ccon1;
			int Rcon1;
			temp_x_b[iterate] = X_b[iterate];
			temp_y_b[iterate] = Y_b[iterate];

			if (iterate != (contour_ind_b - 1))
			{
				Ccon1 = X_b[iterate + 1];
				Rcon1 = Y_b[iterate + 1];
			}
			else
			{
				Ccon1 = X_b[0];
				Rcon1 = Y_b[0];
			}

			double tot1 = 0, tot2 = 0;
			for (int i = 0; i < contour_ind_b; i++)
			{
				tot1 += X_b[i];
				tot2 += Y_b[i];
			}
			tot1 /= contour_ind_b;
			tot2 /= contour_ind_b;

			int index = 0;
			for (int r = -size; r <= size; r++) {
				for (int c = -size; c <= size; c++) {
					int cols = Ccon + c;
					int rows = Rcon + r;
					internal_1[index] = pow((cols - tot1), 2) + pow((rows - tot2), 2);
					internal_2[index] = pow((sqrt(pow((cols - Ccon1), 2) + pow((rows - Rcon1), 2))) - avg_dist_b, 2);
					index++;
				}
			}

			findminmax(internal_1, 11);
			for (int i = 0; i < window_size; i++)
			{
				internal_1[i] = (internal_1[i] - min) / (max - min);
			}

			findminmax(internal_2, 11);
			for (int i = 0; i < window_size; i++)
			{
				internal_2[i] = (internal_2[i] - min) / (max - min);


			}

			index = 0;
			double min1 = 1000;
			int locr = temp_y_b[iterate];
			int	locc = temp_x_b[iterate];
			for (int r = -size; r <= size; r++)
			{
				for (int c = -size; c <= size; c++)
				{
					double x = (-internal_1[index]) + (internal_2[index]) + (5 * external_energy[(r + Rcon) * COLS + (c + Ccon)]);

					index++;

					if (min1 > x)
					{
						min1 = x;
						locr = r + Rcon;
						locc = c + Ccon;
					}
				}

			}

			temp_x_b[iterate] = locc;
			temp_y_b[iterate] = locr;
			iterate++;

		}

		for (int i = 0; i < contour_ind_b; i++)
		{
			X_b[i] = temp_x_b[i];
			Y_b[i] = temp_y_b[i];
		}

		Sleep(100);
		if (stop_all_threads == 1)
		{
			_endthread();
		}

		PaintImage();
	}
}