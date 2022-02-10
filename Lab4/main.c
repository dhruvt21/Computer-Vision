
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h>
#include <windows.h>
#include <wingdi.h>
#include <commctrl.h>
#include <winuser.h>
#include <process.h>	/* needed for multithreading */
#include "resource.h"
#include "globals.h"

HWND ret = NULL;

BOOL CALLBACK GetDistance(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_ADD:
			GetDlgItemText(hwnd, IDC_NUMBER_PI, x, 256);
			distance_p = atoi(x);

			EndDialog(hwnd, IDC_ADD);
			break;
		case IDC_REMOVE:
			EndDialog(hwnd, IDC_REMOVE);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
BOOL CALLBACK GetIntensity(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_ADD:
			GetDlgItemText(hwnd, IDC_NUMBER_PI, y, 256);
			Pixel_intensity = atoi(y);

			EndDialog(hwnd, IDC_ADD);
			break;
		case IDC_REMOVE:
			EndDialog(hwnd, IDC_REMOVE);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

BOOL CALLBACK Out_of_Range(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwnd, IDOK);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine, int nCmdShow)

{
	MSG			msg;
	HWND		hWnd;
	WNDCLASS	wc;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, "ID_PLUS_ICON");
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = "ID_MAIN_MENU";
	wc.lpszClassName = "PLUS";

	if (!RegisterClass(&wc))
		return(FALSE);

	hWnd = CreateWindow("PLUS", "plus program",
		WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
		CW_USEDEFAULT, 0, 400, 400, NULL, NULL, hInstance, NULL);
	if (!hWnd)
		return(FALSE);

	ShowScrollBar(hWnd, SB_BOTH, FALSE);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	MainWnd = hWnd;

	ShowPixelCoords = 0;
	BigDots = 0;
	RegionGrow = 0;

	strcpy(filename, "");
	OriginalImage = NULL;
	ROWS = COLS = 0;

	JPress= 0;
	PlayMode = 1;
	StepMode = 1;
	Stop_all_threads = 0;
	count++;
	file_loaded = 0;
	total_threads = 0;

	Pixel_intensity = 0;
	distance_p = 0;

	InvalidateRect(hWnd, NULL, TRUE);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0) > 0)
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
			//PaintImage();
			break;
		case ID_DISPLAY_BIGDOTS:
			BigDots = (BigDots + 1) % 2;
			//PaintImage();
			break;
		case ID_REGIONGROW_ACTIVATE:
		{
			PlayMode = 1;
			Stop_all_threads = 0;
			StepMode = 0;
			RegionGrow = 1;
			PaintImage();
		}
		break;
		case ID_REGIONGROW_CLEAR:
		{
			PlayMode = 0;
			Stop_all_threads = 1;
			StepMode = 0;
			RegionGrow = 0;
			total_threads = 0;
			PaintImage();
		}
		break;
		case ID_REGIONGROW_PLAYMODE:
		{
			PlayMode = 1;
			StepMode = 0;
		}
		break;
		case ID_REGIONGROW_STEPMODE:
		{
			PlayMode = 0;
			StepMode = 1;
		}
		break;

		case ID_FILE_LOAD:
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
			ofn.lpstrFilter = "PPM files\0*.ppm\0All files\0*.*\0\0";
			if (!(GetOpenFileName(&ofn)) || filename[0] == '\0')
				break;		/* user cancelled load */
			if ((fpt = fopen(filename, "rb")) == NULL)
			{
				MessageBox(NULL, "Unable to open file", filename, MB_OK | MB_APPLMODAL);
				break;
			}
			fscanf(fpt, "%s %d %d %d", header, &COLS, &ROWS, &BYTES);
			if (strcmp(header, "P5") != 0 || BYTES != 255)
			{
				MessageBox(NULL, "Not a PPM (P5 greyscale) image", filename, MB_OK | MB_APPLMODAL);
				fclose(fpt);
				break;
			}
			OriginalImage = (unsigned char*)calloc(ROWS * COLS, 1);
			header[0] = fgetc(fpt);	/* whitespace character after header */
			fread(OriginalImage, 1, ROWS * COLS, fpt);
			file_loaded = 1;
			fclose(fpt);
			SetWindowText(hWnd, filename);
			PaintImage();
			break;

		case ID_FILE_QUIT:
			DestroyWindow(hWnd);
			break;
		}
		break;
	case WM_SIZE:		  /* could be used to detect when window size changes */
		PaintImage();
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
		break;
	case WM_PAINT:
		PaintImage();
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
		break;
	case WM_LBUTTONDOWN:case WM_RBUTTONDOWN:
		if (RegionGrow == 1) {
			xPos = LOWORD(lParam);
			yPos = HIWORD(lParam);
			if (xPos >= 0 && xPos < COLS && yPos >= 0 && yPos < ROWS)
			{
				hDC = GetDC(MainWnd);
				CHOOSECOLOR cc;
				static COLORREF acrCustClr[16];
				HBRUSH hbrush;
				static DWORD rgbCurrent;

				// Initialize CHOOSECOLOR 
				ZeroMemory(&cc, sizeof(cc));
				cc.lStructSize = sizeof(cc);
				cc.hwndOwner = MainWnd;
				cc.lpCustColors = (LPDWORD)acrCustClr;
				cc.rgbResult = rgbCurrent;
				cc.Flags = CC_FULLOPEN | CC_RGBINIT;

				if (ChooseColor(&cc) == TRUE)
				{
					hbrush = CreateSolidBrush(cc.rgbResult);
					rgbCurrent = cc.rgbResult;

					r = yPos; c = xPos;
					R1 = GetRValue(rgbCurrent);
					G1 = GetGValue(rgbCurrent);
					B1 = GetBValue(rgbCurrent);

				}

				DialogBox(NULL, MAKEINTRESOURCE(IDD_MAIN), hWnd, GetIntensity);
				DialogBox(NULL, MAKEINTRESOURCE(IDD_MAIN1), hWnd, GetDistance);
				total_threads++;
				_beginthread(region_grow, 0, NULL);
			}
			else {

				DialogBox(NULL, MAKEINTRESOURCE(IDD_MAIN2), hWnd, Out_of_Range);
			}
		}//region grow end
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
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
		if ((TCHAR)wParam == 'j' || (TCHAR)wParam == 'J')
		{
			JPress= 1;
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
	if (file_loaded == 1) {
		EnableMenuItem(hMenu, ID_REGIONGROW_ACTIVATE, MF_ENABLED);

	}
	else {
		EnableMenuItem(hMenu, ID_REGIONGROW_ACTIVATE, MF_DISABLED);
	}

	if (RegionGrow == 1) {
		CheckMenuItem(hMenu, ID_REGIONGROW_ACTIVATE, MF_CHECKED);

		EnableMenuItem(hMenu, ID_REGIONGROW_PLAYMODE, MF_ENABLED);
		EnableMenuItem(hMenu, ID_REGIONGROW_STEPMODE, MF_ENABLED);
		EnableMenuItem(hMenu, ID_REGIONGROW_CLEAR, MF_ENABLED);
	}
	else {
		CheckMenuItem(hMenu, ID_REGIONGROW_ACTIVATE, MF_UNCHECKED);
		EnableMenuItem(hMenu, ID_REGIONGROW_CLEAR, MF_DISABLED);
		EnableMenuItem(hMenu, ID_REGIONGROW_PLAYMODE, MF_DISABLED);
		EnableMenuItem(hMenu, ID_REGIONGROW_STEPMODE, MF_DISABLED);

	}
	if (RegionGrow == 1 && PlayMode == 1) {
		CheckMenuItem(hMenu, ID_REGIONGROW_STEPMODE, MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_REGIONGROW_PLAYMODE, MF_CHECKED);
	}
	else if (RegionGrow == 1 && StepMode == 1) {
		CheckMenuItem(hMenu, ID_REGIONGROW_PLAYMODE, MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_REGIONGROW_STEPMODE, MF_CHECKED);
	}

	DrawMenuBar(hWnd);

	return(0L);
}




void PaintImage()

{
	PAINTSTRUCT			Painter;
	HDC					hDC;
	BITMAPINFOHEADER	bm_info_header;
	BITMAPINFO* bm_info;
	int					i, r, c, DISPLAY_ROWS, DISPLAY_COLS;
	unsigned char* DisplayImage;

	if (OriginalImage == NULL)
		return;		//no image to draw 

			  /* Windows pads to 4-byte boundaries.  We have to round the size up to 4 in each dimension, filling with black. */
	DISPLAY_ROWS = ROWS;
	DISPLAY_COLS = COLS;
	if (DISPLAY_ROWS % 4 != 0)
		DISPLAY_ROWS = (DISPLAY_ROWS / 4 + 1) * 4;
	if (DISPLAY_COLS % 4 != 0)
		DISPLAY_COLS = (DISPLAY_COLS / 4 + 1) * 4;
	DisplayImage = (unsigned char*)calloc(DISPLAY_ROWS * DISPLAY_COLS, 1);
	for (r = 0; r < ROWS; r++)
		for (c = 0; c < COLS; c++)
			DisplayImage[r * DISPLAY_COLS + c] = OriginalImage[r * COLS + c];

	BeginPaint(MainWnd, &Painter);
	hDC = GetDC(MainWnd);
	bm_info_header.biSize = sizeof(BITMAPINFOHEADER);
	bm_info_header.biWidth = DISPLAY_COLS;
	bm_info_header.biHeight = -DISPLAY_ROWS;
	bm_info_header.biPlanes = 1;
	bm_info_header.biBitCount = 8;
	bm_info_header.biCompression = BI_RGB;
	bm_info_header.biSizeImage = 0;
	bm_info_header.biXPelsPerMeter = 0;
	bm_info_header.biYPelsPerMeter = 0;
	bm_info_header.biClrUsed = 256;
	bm_info_header.biClrImportant = 256;
	bm_info = (BITMAPINFO*)calloc(1, sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD));
	bm_info->bmiHeader = bm_info_header;
	for (i = 0; i < 256; i++)
	{
		bm_info->bmiColors[i].rgbBlue = bm_info->bmiColors[i].rgbGreen = bm_info->bmiColors[i].rgbRed = i;
		bm_info->bmiColors[i].rgbReserved = 0;
	}

	SetDIBitsToDevice(hDC, 0, 0, DISPLAY_COLS, DISPLAY_ROWS, 0, 0,
		0, /* first scan line */
		DISPLAY_ROWS, /* number of scan lines */
		DisplayImage, bm_info, DIB_RGB_COLORS);
	ReleaseDC(MainWnd, hDC);
	EndPaint(MainWnd, &Painter);

	free(DisplayImage);
	free(bm_info);
}



void AnimationThread(HWND AnimationWindowHandle)

{
	HDC		hDC;
	char	text[300];

	ThreadRow = ThreadCol = 0;
	while (ThreadRunning == 1)
	{
		hDC = GetDC(MainWnd);
		SetPixel(hDC, ThreadCol, ThreadRow, RGB(0, 255, 0));	/* color the animation pixel green */
		sprintf(text, "%d,%d     ", ThreadRow, ThreadCol);
		TextOut(hDC, 300, 0, text, strlen(text));		/* draw text on the window */
		ReleaseDC(MainWnd, hDC);
		ThreadRow += 3;
		ThreadCol++;
		Sleep(100);		/* pause 100 ms */
	}
}


void region_grow()
{

	HDC hDC;
	int r1 = r;
	int c1 = c;
	int R = R1;
	int G = G1;
	int B = B1;
	int	r2, c2;
	int	average, total;
	int	q[MAX_QUEUE], qh, qt;
	int distance = distance_p;
	int intensity = Pixel_intensity;
	unsigned char* labels;
	int thread_num = total_threads;

	labels = (unsigned char*)calloc(ROWS * COLS, sizeof(unsigned char));
	labels[r1 * COLS + c1] = 255;

	average = total = (int)OriginalImage[r1 * COLS + c1];

	int paint_over_label = 0;
	q[0] = r1 * COLS + c1;
	qh = 1;	/* queue of  head */
	qt = 0;	/* queue of  tail */
	while (qt != qh)
	{

		hDC = GetDC(MainWnd);
		if ((count) % 50 == 0)
		{
			average = total / (count);

		}
		if (Stop_all_threads == 1) {
			_endthread();

		}
		for (r2 = -1; r2 <= 1; r2++)
		{
			for (c2 = -1; c2 <= 1; c2++)
			{
				if (Stop_all_threads == 1) {
					_endthread();

				}

				if (r2 == 0 && c2 == 0) //Current Pixel value
				{
					continue;
				}
				if (((q[qt] / COLS + r2) * COLS + q[qt] % COLS + c2) > (ROWS * COLS)) //case if index is  not in the range 
				{
					continue;
				}

				if ((q[qt] / COLS + r2) < 0 || (q[qt] / COLS + r2) >= ROWS || (q[qt] % COLS + c2) < 0 || (q[qt] % COLS + c2) >= COLS) //Case if pixel is out of the image range
				{
					continue;
				}

				if (labels[(q[qt] / COLS + r2) * COLS + q[qt] % COLS + c2] != paint_over_label)  //Case if pixel has already been painted
				{
					continue;
				}
				if (abs((int)(OriginalImage[(q[qt] / COLS + r2) * COLS + q[qt] % COLS + c2]) - average) > intensity)  //the diff btw the pixel and the average is greate than the intensity
				{
					continue;
				}
				int dist = sqrt(pow(c1 - q[qt] % COLS + c2, 2) + pow(r1 - q[qt] / COLS + r2, 2));

				if (dist > distance) //Case if the distance of the centroid is greater than the input
				{
					continue;
				}

				labels[(q[qt] / COLS + r2) * COLS + q[qt] % COLS + c2] = 255;
				SetPixel(hDC, q[qt] % COLS + c2, q[qt] / COLS + r2, RGB(R, G, B));
				total += (int)OriginalImage[(q[qt] / COLS + r2) * COLS + q[qt] % COLS + c2];
				count++;
				q[qh] = (q[qt] / COLS + r2) * COLS + q[qt] % COLS + c2;
				qh = (qh + 1) % MAX_QUEUE;
				if (qh == qt)
				{
					printf("Max queue size exceeded\n");
					exit(0);
				}
				if (PlayMode == 1)
				{
					Sleep(1);
				}
				else
				{
					while ((JPress== 0) && (StepMode == 1))
					{
						Sleep(1);
						if (Stop_all_threads == 1) {
							_endthread();

						}
					}
					JPress= 0;
				}
				if (Stop_all_threads == 1) {
					_endthread();

				}

			}

		}
		qt = (qt + 1) % MAX_QUEUE;
		ReleaseDC(MainWnd, hDC);
	}
	_endthread();
}