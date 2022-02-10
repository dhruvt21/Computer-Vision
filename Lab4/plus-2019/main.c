#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include <windows.h>
#include <wingdi.h>
#include <winuser.h>
#include <process.h>	/* needed for multithreading */
#include "resource.h"
#include "globals.h"
BOOL CALLBACK getpixelIntensity(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetDlgItemText(hwnd, IDC_EDIT1, intensity, 256);
			pixel_intensity = atoi(intensity);

			EndDialog(hwnd, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
BOOL CALLBACK getDistance(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetDlgItemText(hwnd, IDC_EDIT1, dis, 256);
			distance_pixel = atoi(dis);

			EndDialog(hwnd, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

BOOL CALLBACK getWindowLoops(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetDlgItemText(hwnd, IDC_EDIT1, dis, 256);
			window = atoi(dis);
			EndDialog(hwnd, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}


BOOL CALLBACK Error(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
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
	BigCoords = 0;
	RegionGrow = 0;
	strcpy(filename, "");
	OriginalImage = NULL;
	ROWS = COLS = 0;
	Jpress = 0;
	PlayMode = 1;
	StepMode = 1;
	Stop_all_threads = 0;
	////TO default playpixel
	///regiongorw
	count++;;
	file_loaded = 0;
	total_threads = 0;
	points_drawn = 0;
	points_drawn_ballon = 0;
	pixel_intensity = 0;
	distance_pixel = 0;

	/////
	LButton = 0;
	Active_index = 0;
	contour_index = 0;

	InvalidateRect(hWnd, NULL, TRUE);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0))
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
	int					BYTES, xPos, yPos;

	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_SHOWPIXELCOORDS:
			ShowPixelCoords = (ShowPixelCoords + 1) % 2;
			PaintImage();
			break;
		case ID_DISPLAY_BIGDOT:
			BigCoords = (BigCoords + 1) % 2;
			PaintImage();
			break;
		case ID_REGIONGROWING_ACTIVATE:
		{
			PlayMode = 1;
			Stop_all_threads = 0;
			StepMode = 0;
			RegionGrow = 1;
			PaintImage();
		}
		break;
		case ID_REGIONGROWING_CLEAR:    //Clear and Stop Growing
		{
			PlayMode = 0;
			StepMode = 0;
			RegionGrow = 0;
			Stop_all_threads = 1;
			total_threads = 0;
			PaintImage();
		}
		break;
		case ID_REGIONGROWING_PLAYMODE:
		{
			PlayMode = 1;
			StepMode = 0;
		}
		break;
		case ID_REGIONGROWING_STEPMODE:
			PlayMode = 0;
			StepMode = 1;
			break;
		case ID_ACTIVECONTOUR_STOP:
		{
			Stop_all_threads = 1;
			points_drawn = 1;
			Moving = 0;

		}
		break;
		case ID_ACTIVECONTOUR_RESUME:
		{
			if (Active_contour == 1 && points_drawn == 1) {
				if (contour_index != 0)_beginthread(rubberBand, 0, NULL);
				if (contour_index_ballon != 0)_beginthread(Ballon, 0, NULL);
			}
		}

		break;
		case ID_ACTIVECONTOUR_CLEAR:
		{
			points_drawn = 0;
			Active_index = 0;
			contour_index = 0;
			contour_index_ballon = 0;
			Active_contour = (Active_contour + 1) % 2;;
			Scalex = (int*)calloc(max_points, sizeof(int));
			Sacley = (int*)calloc(max_points, sizeof(int));
			Scalex_ballon = (int*)calloc(max_points, sizeof(int));
			Sacley_ballon = (int*)calloc(max_points, sizeof(int));
			Stop_all_threads = 1;
			Moving = 0;
			PaintImage();
		}
		break;

		case ID_ACTIVECONTOUR_ACTIVATE:
		{
			Active_contour = (Active_contour + 1) % 2;;
			Active_index = 0;
			Stop_all_threads = 0;
			Moving = 0;
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

			if (header[1] == '5') {
				OriginalImage = (unsigned char*)calloc(ROWS * COLS, 1);
				header[0] = fgetc(fpt);	/* whitespace character after header */
				fread(OriginalImage, 1, ROWS * COLS, fpt);
				file_loaded = 1;
				fclose(fpt);
			}
			else
			{


				OriginalImage = (unsigned char*)calloc(ROWS * COLS, 1);
				DummyImage = (unsigned char*)calloc((ROWS * COLS) * 3, 1);
				header[0] = fgetc(fpt);	/* whitespace character after header */
				fread(DummyImage, 1, (ROWS * COLS) * 3, fpt);
				int index = 0;
				for (int i = 0; i < (ROWS * COLS) * 3; i += 3) {
					OriginalImage[index] = (DummyImage[i] + DummyImage[i + 1] + DummyImage[i + 2]) / 3;
					index++;
				}

				file_loaded = 1;
				fclose(fpt);
			}

			SetWindowText(hWnd, filename);
			PaintImage();
			Sobbel();
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
	case WM_LBUTTONDOWN:
	{
		if (RegionGrow == 1) {
			xPos = LOWORD(lParam);
			yPos = HIWORD(lParam);
			if (xPos >= 0 && xPos < COLS && yPos >= 0 && yPos < ROWS)
			{
				hDC = GetDC(MainWnd);
				CHOOSECOLOR cc;
				static COLORREF acrCustClr[16]; // array of custom colors 
				HBRUSH hbrush;                  // brush handle
				static DWORD rgbCurrent;        // initial color selection

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

				DialogBox(NULL, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, getDistance);
				DialogBox(NULL, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, getpixelIntensity);
				total_threads++;
				_beginthread(region_grow, 0, NULL);
			}

		}//EndRegionGrow
		if (Active_contour == 1 && LButton == 0 && points_drawn == 0 && Moving == 0) {
			LButton = 1;
			Active_index = 0;
			contour_index = 0;
			Aux_Scalex = (int*)calloc(max_points, sizeof(int));
			Aux_Sacley = (int*)calloc(max_points, sizeof(int));

			Scalex = (int*)calloc(max_points, sizeof(int));
			Sacley = (int*)calloc(max_points, sizeof(int));
			OutputDebugString("asd");
		}
		if (Active_contour == 1 && points_drawn == 1 && Moving == 0) {
			Stop_all_threads = 0;

			xPos = LOWORD(lParam);
			yPos = HIWORD(lParam);
			int minx = xPos - PixelWidth, maxx = xPos + PixelWidth;
			int miny = yPos - PixelWidth, maxy = yPos + PixelWidth;
			Moving_index = -1;
			if (contour_index != 0) {
				for (int i = 0; i < contour_index; i++) {
					if ((Scalex[i] > minx && Scalex[i] < maxx) && (Sacley[i] > miny && Sacley[i] < maxy)) {
						Moving_index = i;
						Moving = 1;
						break;
					}
				}
			}
			if (contour_index_ballon != 0) {
				for (int i = 0; i < contour_index_ballon; i++) {
					if ((Scalex_ballon[i] > minx && Scalex_ballon[i] < maxx) && (Sacley_ballon[i] > miny && Sacley_ballon[i] < maxy)) {
						Moving_index = i;
						Moving = 1;
						break;
					}
				}
			}
		}
	}////MOuse Left Down
	return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	break;
	case WM_RBUTTONDOWN:
	{
		if (Active_contour == 1) {
			xPos = LOWORD(lParam);
			yPos = HIWORD(lParam);
			seedx = xPos;
			seedy = yPos;
			LoadPixel(seedx, seedy);
			_beginthread(Ballon, 1, NULL);
		}
	}
	return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	break;
	case WM_LBUTTONUP:
	{
		if (LButton == 1 && points_drawn == 0 && Moving == 0) {
			LButton = 0;
			OutputDebugString(" dasdsasd ");
			DialogBox(NULL, MAKEINTRESOURCE(IDD_DIALOG4), hWnd, getWindowLoops);
			downSample();
			_beginthread(rubberBand, 0, NULL);
		}
		if (Active_contour == 1 && points_drawn == 1 && Moving == 1) {
			Moving = 0;
		}
	}
	return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	break;

	case WM_MOUSEMOVE:
	{
		if (ShowPixelCoords == 1)
		{
			xPos = LOWORD(lParam);
			yPos = HIWORD(lParam);

			if (xPos >= 0 && xPos < COLS && yPos >= 0 && yPos < ROWS)
			{
				sprintf(text, "%d,%d=>%d %d    ", xPos, yPos, OriginalImage[yPos * COLS + xPos], Active_index);
				hDC = GetDC(MainWnd);
				TextOut(hDC, 0, 0, text, strlen(text));		/* draw text on the window */
				int x, y;
				if (BigCoords == 1) {
					for (x = -4; x <= 4; x++) {
						for (y = -4; y <= 4; y++) {
							SetPixel(hDC, xPos + x, yPos + y, RGB(255, 0, 0));	/* color the cursor position red */
						}
					}
				}
				else {
					SetPixel(hDC, xPos, yPos, RGB(255, 0, 0));	/* color the cursor position red */
				}
				ReleaseDC(MainWnd, hDC);
			}
		}
		if (LButton == 1 && points_drawn == 0 && Moving == 0)  /////Drawing and capturing
		{
			HDC hDC;
			hDC = GetDC(MainWnd);
			xPos = LOWORD(lParam);
			yPos = HIWORD(lParam);
			Aux_Scalex[Active_index] = xPos;
			Aux_Sacley[Active_index] = yPos;

			for (int x = -3; x <= 3; x++) {
				for (int y = -3; y <= 3; y++) {
					SetPixel(hDC, Aux_Scalex[Active_index] + x, Aux_Sacley[Active_index] + y, RGB(255, 0, 0));
				}
			}
			sprintf(text, "Point - %d  Max Points -%d ", Active_index, max_points);
			hDC = GetDC(MainWnd);
			TextOut(hDC, 0, 0, text, strlen(text));

			Active_index++;
		}
		if (Active_contour == 1 && points_drawn == 1 && Moving == 1) {
			PaintImage();
			if (contour_index != 0) {
				Scalex[Moving_index] = LOWORD(lParam);
				Sacley[Moving_index] = HIWORD(lParam);
				average();
				CheckTension(Average_Distance, Moving_index, 0, Scalex, Sacley);
				drawContour(Scalex, Sacley, contour_index);
			}
			if (contour_index_ballon != 0) {
				Scalex_ballon[Moving_index] = LOWORD(lParam);
				Sacley_ballon[Moving_index] = HIWORD(lParam);
				average();
				CheckTension(Average_Distance, Moving_index, 0, Scalex_ballon, Sacley_ballon);
				drawContour(Scalex_ballon, Sacley_ballon, contour_index_ballon);
			}
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
			Jpress = 1;
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


	if (BigCoords == 1)
		CheckMenuItem(hMenu, ID_DISPLAY_BIGDOT, MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
	else
		CheckMenuItem(hMenu, ID_DISPLAY_BIGDOT, MF_UNCHECKED);

	if (file_loaded == 1) {
		EnableMenuItem(hMenu, ID_REGIONGROWING_ACTIVATE, MF_ENABLED);

	}
	else {
		EnableMenuItem(hMenu, ID_REGIONGROWING_ACTIVATE, MF_DISABLED);
	}

	if (RegionGrow == 1) {
		CheckMenuItem(hMenu, ID_REGIONGROWING_ACTIVATE, MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */

		EnableMenuItem(hMenu, ID_REGIONGROWING_PLAYMODE, MF_ENABLED);
		EnableMenuItem(hMenu, ID_REGIONGROWING_STEPMODE, MF_ENABLED);
		EnableMenuItem(hMenu, ID_REGIONGROWING_CLEAR, MF_ENABLED);
	}
	else {
		CheckMenuItem(hMenu, ID_REGIONGROWING_ACTIVATE, MF_UNCHECKED);
		EnableMenuItem(hMenu, ID_REGIONGROWING_CLEAR, MF_DISABLED);
		EnableMenuItem(hMenu, ID_REGIONGROWING_PLAYMODE, MF_DISABLED);
		EnableMenuItem(hMenu, ID_REGIONGROWING_STEPMODE, MF_DISABLED);

	}
	if (RegionGrow == 1 && PlayMode == 1) {
		CheckMenuItem(hMenu, ID_REGIONGROWING_STEPMODE, MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_REGIONGROWING_PLAYMODE, MF_CHECKED);
	}
	else if (RegionGrow == 1 && StepMode == 1) {
		CheckMenuItem(hMenu, ID_REGIONGROWING_PLAYMODE, MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_REGIONGROWING_STEPMODE, MF_CHECKED);
	}
	else if (Active_contour == 1) {
		CheckMenuItem(hMenu, ID_ACTIVECONTOUR_ACTIVATE, MF_CHECKED);
	}
	else {
		CheckMenuItem(hMenu, ID_ACTIVECONTOUR_ACTIVATE, MF_UNCHECKED);
	}

	DrawMenuBar(hWnd);

	return(0L);
}



void PaintImage() {
	PAINTSTRUCT			Painter;
	HDC					hDC;
	BITMAPINFOHEADER	bm_info_header;
	BITMAPINFO* bm_info;
	int					i, r, c, DISPLAY_ROWS, DISPLAY_COLS;
	unsigned char* DisplayImage;

	if (OriginalImage == NULL)
		return;		/* no image to draw */
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
	if (contour_index > 2)drawContour(Scalex, Sacley, contour_index);
	if (contour_index_ballon > 2)drawContour(Scalex_ballon, Sacley_ballon, contour_index_ballon);
	ReleaseDC(MainWnd, hDC);
	EndPaint(MainWnd, &Painter);



	free(DisplayImage);
	free(bm_info);
}

void CheckTension(float avg, int index, int level, int* x, int* y)
{
	if (level < 4) {
		int before, after;

		if (index == 0) {
			before = contour_index - 1;
		}
		else {
			before = index - 1;
		}
		if (index == (contour_index - 1)) {
			after = 0;
		}
		else {
			after = index + 1;
		}

		float	Bdist = sqrt(pow((x[before] - x[index]), 2) + pow(y[before] - y[index], 2));
		float	Adist = sqrt(pow((x[after] - x[index]), 2) + pow(y[after] - y[index], 2));



		if (fabsf(avg - Bdist) > distance) {
			int getCcon = x[before];
			int getRcon = y[before];
			int min = pow((getCcon - x[index]), 2) + pow((getRcon - y[index]), 2);
			min = min + pow((sqrt(min)) - avg, 2);
			int r1 = y[before], c1 = x[before];

			int Flag = 0;
			for (int r = -1; r <= 1; r++) {
				for (int c = -1; c <= 1; c++) {
					int cols = getCcon + c;
					int rows = getRcon + r;
					int value = pow((cols - x[index]), 2) + pow((rows - y[index]), 2);
					value = value + pow((sqrt(value)) - avg, 2);
					if (min > value) {
						min = value;
						c1 = cols;
						r1 = rows;
						Flag = 1;
					}
				}
			}

			if (Flag == 1) {
				x[before] = c1;
				y[before] = r1;
				average();
				CheckTension(Average_Distance, before, level + 1, x, y);
			}
		}///Before End Loop
		if (fabsf(avg - Adist) > distance) {
			int getCcon = x[after];
			int getRcon = y[after];
			int min = pow((getCcon - x[index]), 2) + pow((getRcon - y[index]), 2);
			min = min + pow((sqrt(min)) - avg, 2);
			int r1 = y[after], c1 = x[after];
			int Flag = 0;
			for (int r = -1; r <= 1; r++) {
				for (int c = -1; c <= 1; c++) {
					int cols = getCcon + c;
					int rows = getRcon + r;
					int value = pow((cols - x[index]), 2) + pow((rows - y[index]), 2);
					value = value + pow((sqrt(value)) - avg, 2);
					if (min > value) {
						min = value;
						c1 = cols;
						r1 = rows;
						Flag = 1;
					}
				}
			}

			if (Flag == 1) {
				x[after] = c1;
				y[after] = r1;
				average();
				CheckTension(Average_Distance, after, level + 1, x, y);
			}
		}////After End Loop
	}

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



/* REGION GROW FUNCTION */
void region_grow()
{
	int r1 = r;
	int c1 = c;
	int R = R1;
	int G = G1;
	int B = B1;
	int distance = distance_pixel;
	int intensity = pixel_intensity;

	HDC hDC;
	int	r2, c2;
	int	queue[MAX_QUEUE], qh, qt;
	int	average, total;	/* average and total intensity in growing region */
	unsigned char* labels;
	int thread_num = total_threads;
	labels = (unsigned char*)calloc(ROWS * COLS, sizeof(unsigned char));
	labels[r1 * COLS + c1] = 255;
	average = total = (int)OriginalImage[r1 * COLS + c1];
	int paint_over_label = 0;
	queue[0] = r1 * COLS + c1;
	qh = 1;	/* queue head */
	qt = 0;	/* queue tail */
	int maxd = 0;
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

				if (r2 == 0 && c2 == 0) { continue; }
				if (((queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2) > (ROWS * COLS)) { continue; }

				if ((queue[qt] / COLS + r2) < 0 || (queue[qt] / COLS + r2) >= ROWS ||
					(queue[qt] % COLS + c2) < 0 || (queue[qt] % COLS + c2) >= COLS)
				{
					continue;
				}

				if (labels[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2] != paint_over_label)
				{
					continue;
				}
				if (abs((int)(OriginalImage[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2]) - average) > intensity)
				{
					continue;
				}
				int dis = sqrt(pow(c1 - queue[qt] % COLS + c2, 2) + pow(r1 - queue[qt] / COLS + r2, 2));

				if (dis > distance) {
					continue;
				}
				if (maxd < dis) {
					maxd = dis;
				}

				labels[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2] = 255;
				SetPixel(hDC, queue[qt] % COLS + c2, queue[qt] / COLS + r2, RGB(R, G, B));
				total += (int)OriginalImage[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2];
				count++;
				queue[qh] = (queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2;
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
					while ((Jpress == 0) && (StepMode == 1))
					{
						Sleep(1);
						if (Stop_all_threads == 1) {
							_endthread();

						}
					}
					Jpress = 0;
				}
				if (Stop_all_threads == 1) {
					_endthread();

				}
				/* draw text on the window */
			}
		}
		qt = (qt + 1) % MAX_QUEUE;
	}
	_endthread();
}


/////////--------------------------------------/////////////
void downSample() {

	contour_index = 0;
	for (int i = 0; i < Active_index; i += 20) {
		Scalex[contour_index] = Aux_Scalex[i];
		Sacley[contour_index] = Aux_Sacley[i];

		contour_index++;
	}
	points_drawn = 1;
	PaintImage();


	Aux_Scalex = (int*)calloc(contour_index, sizeof(int));
	Aux_Sacley = (int*)calloc(contour_index, sizeof(int));

}

void drawContour(int* X, int* Y, int s) {
	HDC hDC;
	hDC = GetDC(MainWnd);
	int size = 2;


	for (int i = 0; i < s; i++) {
		for (int x = -PixelWidth; x <= PixelWidth; x++) {
			for (int y = -PixelWidth; y <= PixelWidth; y++) {
				SetPixel(hDC, X[i] + x, Y[i] + y, RGB(255, 0, 0));
			}
		}

		if (i != (s - 1)) {
			MoveToEx(hDC, X[i], Y[i], (LPPOINT)NULL);
			LineTo(hDC, X[i + 1], Y[i + 1]);
		}
		else
		{
			MoveToEx(hDC, X[i], Y[i], (LPPOINT)NULL);
			LineTo(hDC, X[0], Y[0]);
		}
	}
}
void average() {
	//Average Distance of the point
	double dist = 0;
	for (int i = 0; i < contour_index; i++) {
		if (i != (contour_index - 1)) {
			dist = dist + sqrt(pow((Scalex[i] - Scalex[i + 1]), 2) + pow(Sacley[i] - Sacley[i + 1], 2));
		}
		else {
			dist = dist + sqrt(pow((Scalex[i] - Scalex[0]), 2) + pow(Sacley[i] - Sacley[0], 2));
		}
	}

	Average_Distance = dist / contour_index;
}

//////////////---------------------------------////////////////
void rubberBand() {
	HDC hDC;
	int size = floor(window / 2);
	int window_size = pow((size * 2) + 1, 2);
	hDC = GetDC(MainWnd);
	while (1) {



		double* internal_enegry = (double*)calloc(window_size, sizeof(double));
		double* internal_enegry2 = (double*)calloc(window_size, sizeof(double));


		average();



		int iterate = 0;
		while (iterate < contour_index) {
			int getCcon = Scalex[iterate];
			int getRcon = Sacley[iterate];
			int getCcon1;
			int getRcon1;
			Aux_Scalex[iterate] = Scalex[iterate];
			Aux_Sacley[iterate] = Sacley[iterate];

			if (iterate != (contour_index - 1)) {
				getCcon1 = Scalex[iterate + 1];
				getRcon1 = Sacley[iterate + 1];
			}
			else {
				getCcon1 = Scalex[0];
				getRcon1 = Sacley[0];
			}
			int distance = sqrt(pow((getCcon - getCcon1), 2) + pow(getRcon - getRcon1, 2));
			int index = 0;
			for (int r = -size; r <= size; r++) {
				for (int c = -size; c <= size; c++) {
					int cols = getCcon + c;
					int rows = getRcon + r;
					internal_enegry[index] = pow((cols - getCcon1), 2) + pow((rows - getRcon1), 2);
					internal_enegry2[index] = pow((sqrt(internal_enegry[index])) - Average_Distance, 2);
					index++;
				}
			}

			findminmax(internal_enegry, window_size);
			//normallize
			for (int i = 0; i < window_size; i++) {
				internal_enegry[i] = (internal_enegry[i] - min) / (max - min);
				//	printf("%lf ",internal_enegry[i]);
			}
			findminmax(internal_enegry2, window_size);
			//normallize
			for (int i = 0; i < window_size; i++) {
				internal_enegry2[i] = (internal_enegry2[i] - min) / (max - min);

			}
			index = 0;
			double min1 = 1000;
			int locr, locc;
			for (int r = -size; r <= size; r++) {
				for (int c = -size; c <= size; c++) {
					double dummy = internal_enegry2[index] + internal_enegry[index]
						+ external_energy[(r + getRcon) * COLS + (c + getCcon)];
					index++;
					if (min1 > dummy)
					{
						min1 = dummy;
						locr = r + getRcon;
						locc = c + getCcon;
					}
				}
			}
			Aux_Scalex[iterate] = locc;
			Aux_Sacley[iterate] = locr;
			iterate++;

		}//end contour


		for (int i = 0; i < contour_index; i++) {
			Scalex[i] = Aux_Scalex[i];
			Sacley[i] = Aux_Sacley[i];
		}


		Sleep(100);
		if (Stop_all_threads == 1) {
			_endthread();
		}

		PaintImage();
	}


}


void findminmax(double* array, int size) {
	min = array[0]; max = 0;
	for (int i = 0; i < size; i++) {

		if (min > array[i]) {
			min = array[i];
		}
		if (max < array[i]) {
			max = array[i];
		}

	}
}

///////////////////Baloon
void Ballon() {

	HDC hDC;
	int size = 3;
	int window_size = pow((size * 2) + 1, 2);
	hDC = GetDC(MainWnd);

	while (1) {



		double* internal_enegry = (double*)calloc(window_size, sizeof(double));
		double* internal_enegry2 = (double*)calloc(window_size, sizeof(double));

		//Average Distance of the point
		double dist = 0;
		for (int i = 0; i < contour_index_ballon; i++) {
			if (i != (contour_index_ballon - 1)) {
				dist = dist + sqrt(pow((Scalex_ballon[i] - Scalex_ballon[i + 1]), 2) + pow(Sacley_ballon[i] - Sacley_ballon[i + 1], 2));
			}
			else {
				dist = dist + sqrt(pow((Scalex_ballon[i] - Scalex_ballon[0]), 2) + pow(Sacley_ballon[i] - Sacley_ballon[0], 2));
			}
		}

		Average_Distance = dist / contour_index_ballon;



		int iterate = 0;
		while (iterate < contour_index_ballon) {
			int getCcon = Scalex_ballon[iterate];
			int getRcon = Sacley_ballon[iterate];
			int getCcon1;
			int getRcon1;
			Aux_Scalex_ballon[iterate] = Scalex_ballon[iterate];
			Aux_Sacley_ballon[iterate] = Sacley_ballon[iterate];

			if (iterate != (contour_index_ballon - 1)) {
				getCcon1 = Scalex_ballon[iterate + 1];
				getRcon1 = Sacley_ballon[iterate + 1];
			}
			else {
				getCcon1 = Scalex_ballon[0];
				getRcon1 = Sacley_ballon[0];
			}
			int distance = sqrt(pow((getCcon - getCcon1), 2) + pow(getRcon - getRcon1, 2));
			int index = 0;
			for (int r = -size; r <= size; r++) {
				for (int c = -size; c <= size; c++) {
					int cols = getCcon + c;
					int rows = getRcon + r;
					internal_enegry[index] = pow((cols - seedx), 2) + pow((rows - seedy), 2);
					internal_enegry2[index] = pow((sqrt(pow((cols - getCcon1), 2) + pow((rows - getRcon1), 2))) - Average_Distance, 2);
					index++;
				}
			}

			findminmax(internal_enegry, window_size);
			//normallize
			for (int i = 0; i < window_size; i++) {
				internal_enegry[i] = (internal_enegry[i] - min) / (max - min);
				//	printf("%lf ",internal_enegry[i]);
			}
			findminmax(internal_enegry2, window_size);
			//normallize
			for (int i = 0; i < window_size; i++) {
				internal_enegry2[i] = (internal_enegry2[i] - min) / (max - min);


			}


			index = 0;
			double min1 = 0;
			int locr = Aux_Sacley_ballon[iterate], locc = Aux_Scalex_ballon[iterate];
			for (int r = -size; r <= size; r++) {
				for (int c = -size; c <= size; c++) {
					double dummy = (-internal_enegry[index]) + (internal_enegry2[index]) + (external_energy[(r + getRcon) * COLS + (c + getCcon)] * 5);
					if (index == 0)min1 = dummy;

					index++;

					if (min1 > dummy)
					{
						min1 = dummy;
						locr = r + getRcon;
						locc = c + getCcon;
					}
				}

			}

			Aux_Scalex_ballon[iterate] = locc;
			Aux_Sacley_ballon[iterate] = locr;
			iterate++;

		}//end contour

		for (int i = 0; i < contour_index_ballon; i++) {
			Scalex_ballon[i] = Aux_Scalex_ballon[i];
			Sacley_ballon[i] = Aux_Sacley_ballon[i];
		}

		Sleep(100);
		if (Stop_all_threads == 1) {

			_endthread();

		}

		PaintImage();
	}



}////////////////////////

void Sobbel() {
	int sobelx[] = { -1,0,1,
			  -2,0,2,
			  -1,0,1 };

	int sobely[] = { -1,-2,-1,
				   0,0,0,
				   1,2,1 };

	unsigned char* sobe;
	int Smin = 1000, Smax = 0;
	external_energy = (double*)calloc(ROWS * COLS, sizeof(double));

	sobe = (unsigned char*)calloc(ROWS * COLS, 1);
	for (int r = 1; r < (ROWS - 1); r++) {
		for (int c = 1; c < (COLS - 1); c++) {
			int gx = 0, gy = 0;
			int index = 0;
			for (int r1 = -1; r1 <= 1; r1++) {
				for (int c1 = -1; c1 <= 1; c1++) {
					gx += OriginalImage[((r + r1) * COLS) + (c + c1)] * sobelx[index];
					gy += OriginalImage[((r + r1) * COLS) + (c + c1)] * sobely[index];
					index++;
				}
			}//sobel;
			external_energy[r * COLS + c] = sqrt(pow(gx, 2) + pow(gy, 2));
			if (Smax < external_energy[(r * COLS) + c]) Smax = external_energy[(r * COLS) + c];
			if (Smin > external_energy[(r * COLS) + c]) Smin = external_energy[(r * COLS) + c];
		}
	}

	for (int r = 1; r < ROWS * COLS; r++) {
		sobe[r] = (unsigned char)(((external_energy[r] - Smin) * 255) / (Smax - Smin));
		external_energy[r] = -((external_energy[r] - Smin) / (Smax - Smin));
	}
	FILE* fpt;
	fpt = fopen("D:\\sobel.ppm", "wb");
	fprintf(fpt, "P5 %d %d 255\n", COLS, ROWS);
	fwrite(sobe, 1, ROWS * COLS, fpt);
	fclose(fpt);
}





void LoadPixel(int x, int y) {

	HDC hDC;
	hDC = GetDC(MainWnd);
	int wind = 10;
	Aux_Scalex_ballon = (int*)calloc(50, sizeof(int));
	Aux_Sacley_ballon = (int*)calloc(50, sizeof(int));
	Scalex_ballon = (int*)calloc(50, sizeof(int));
	Sacley_ballon = (int*)calloc(50, sizeof(int));
	int x1 = x - wind, x2 = x + wind;
	int y1 = y - wind, y2 = y + wind;
	int index = 0;
	contour_index_ballon = 0;
	for (int i = 0; i < wind; i++) {
		Aux_Scalex_ballon[contour_index_ballon] = x1 + i;
		Aux_Sacley_ballon[contour_index_ballon] = y - i;
		contour_index_ballon++;
	}

	for (int i = 0; i < wind; i++) {
		Aux_Scalex_ballon[contour_index_ballon] = x + i;
		Aux_Sacley_ballon[contour_index_ballon] = y1 + i;
		contour_index_ballon++;
	}

	for (int i = 0; i < wind; i++) {
		Aux_Scalex_ballon[contour_index_ballon] = x2 - i;
		Aux_Sacley_ballon[contour_index_ballon] = y + i;
		contour_index_ballon++;
	}
	for (int i = 0; i < wind; i++) {
		Aux_Scalex_ballon[contour_index_ballon] = x - i;
		Aux_Sacley_ballon[contour_index_ballon] = y2 - i;
		contour_index_ballon++;
	}

	for (int i = 0; i < contour_index_ballon; i += 5) {
		Scalex_ballon[index] = Aux_Scalex_ballon[i];
		Sacley_ballon[index] = Aux_Sacley_ballon[i];
		SetPixel(hDC, Scalex_ballon[index], Sacley_ballon[index], RGB(255, 0, 0));

		index++;
	}



	drawContour(Scalex_ballon, Sacley_ballon, index);
	contour_index_ballon = index;
}