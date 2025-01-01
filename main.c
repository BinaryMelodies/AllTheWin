
#include <windows.h>
#if VERSION_WIN5 || VERSION_WIN6
# include <commctrl.h>
#endif
#if TARGET_WINCE
# include <aygshell.h>
#endif

#if !TARGET_WINCE
# define WinMain MyWinMain
# define wWinMain MyWinMain
#endif

#if TARGET_WINCE
# undef MB_TASKMODAL
# define MB_TASKMODAL 0
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if UNICODE
# define stprintf swprintf
# define PRIsT "l"
#else
# define stprintf snprintf
# define PRIsT ""
#endif

#include "main.h"

#if TARGET_WIN16
/* https://learn.microsoft.com/en-us/windows/win32/learnwin32/working-with-strings */
# define TEXT(s) s
# define TCHAR char
#endif

HINSTANCE hCurrentInstance;

#if TARGET_WIN16
BOOL FAR PASCAL _export AboutDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
#elif TARGET_WIN32 || TARGET_WINCE
BOOL CALLBACK AboutDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
#elif TARGET_WIN64
LRESULT CALLBACK AboutDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
#endif
{
	switch(message)
	{
	case WM_COMMAND:
		switch(wParam)
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hWnd, wParam);
			return TRUE;
		}
		break;
	case WM_INITDIALOG:
		return TRUE;
	}
	return FALSE;
}

#if TARGET_WIN16
FARPROC aboutProc; // under Win16, window handler procedure must be bound by MakeProcInstance
#endif

#if TARGET_WINCE
static SHACTIVATEINFO activateInfo;
HWND hwndCommandBar;
#endif

int winver_major, winver_minor;

// some resources: https://learn.microsoft.com/en-us/windows/win32/gdi/drawing-with-the-mouse

// used for drawing
BOOL currently_drawing;
POINT last_point;
int control_position;
int control_count;
HWND hTextEdit;

#if TARGET_WIN16
LRESULT FAR PASCAL _export AppWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
#elif TARGET_WIN32 || TARGET_WIN64 || TARGET_WINCE
LRESULT CALLBACK AppWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
#endif
{
	switch(message)
	{
#if TARGET_WINCE
	case WM_CREATE:
		{
			SHMENUBARINFO mbi = { 0 };

			mbi.cbSize = sizeof(SHMENUBARINFO);
			mbi.hwndParent = hWnd;
			mbi.nToolBarId = IDR_MAINMENU; //IDM_MENU;
			mbi.hInstRes = hCurrentInstance;
			mbi.nBmpId = 0;
			mbi.cBmpImages = 0;
			mbi.dwFlags = SHCMBF_HMENU;

			if(!SHCreateMenuBar(&mbi))
				return 0L;

			hwndCommandBar = mbi.hwndMB;
			memset(&activateInfo, 0, sizeof(activateInfo));
			activateInfo.cbSize = sizeof(activateInfo);
		}
		break;
#endif
	case WM_COMMAND:
#if TARGET_WIN16
		switch(wParam)
#else
		// Under Win32, the HIWORD contains 1 for accelerators
		switch(LOWORD(wParam))
#endif
		{
		case ID_FILE_EXIT:
			if(MessageBox(hWnd, TEXT("Are you sure you want to quit?"), TEXT("Question"), MB_YESNO | MB_TASKMODAL) == IDYES)
				DestroyWindow(hWnd);
			break;
		case ID_HELP_ABOUT:
#if TARGET_WIN16
			DialogBox(hCurrentInstance, MAKEINTRESOURCE(IDD_ABOUTDIALOG), hWnd, aboutProc);
#else
			// Win32 does not need MakeProcInstance, the definition simply returns the function
			DialogBox(hCurrentInstance, MAKEINTRESOURCE(IDD_ABOUTDIALOG), hWnd, AboutDialogProc);
#endif
			break;
		/* adding controls */
		// https://zetcode.com/gui/winapi/controls/
		case ID_CONTROL_BUTTON:
			CreateWindow(
				TEXT("Button"),
				TEXT("Button"),
				WS_VISIBLE | WS_CHILD,
				0, control_position, 80, 25,
				hWnd,
				(HMENU) ID_BUTTON,
				NULL,
				NULL);
			control_position += 25;
			break;
		case ID_CONTROL_LABEL:
			CreateWindow(
				TEXT("Static"),
				TEXT("Les sanglots longs\nDes violons\n\tDe l'automne"),
				WS_VISIBLE | WS_CHILD | SS_LEFT,
				0, control_position, 8 * 20, 16 * 3,
				hWnd,
				(HMENU) 0,
				NULL,
				NULL);
			control_position += 16 * 3;
			break;
		case ID_CONTROL_CHECKBOX:
			CreateWindow(
				TEXT("Button"),
				TEXT("Check box"),
				WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
				0, control_position, 80, 25,
				hWnd,
#if !TARGET_WIN64
				(HMENU) control_count++,
#else
				(HMENU) (long long) control_count++,
#endif
				NULL,
				NULL);
			control_position += 25;
			break;
		case ID_CONTROL_TEXTEDIT:
			hTextEdit = CreateWindow(
				TEXT("Edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER, // | ES_MULTILINE
				0, control_position, 150, 25,
				hWnd,
				(HMENU) ID_TEXTEDIT,
				hCurrentInstance,
				NULL);
			control_position += 25;
			break;
		case ID_BUTTON:
			if(hTextEdit == NULL)
			{
				MessageBox(hWnd, TEXT("Command button clicked"), TEXT("Simple Message"), MB_OK | MB_TASKMODAL);
			}
			else
			{
				int length = GetWindowTextLength(hTextEdit) + 1;
				TCHAR * text = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * length);
				GetWindowText(hTextEdit, text, length);
				MessageBox(hWnd, text, TEXT("Simple Message"), MB_OK | MB_TASKMODAL);
				LocalFree((HANDLE)text);
			}
			break;
		case ID_TEXTEDIT:
			break;
		default:
#if TARGET_WIN16
			if(ID_CHECKBOX1 <= wParam && wParam < control_count)
			{
				if(IsDlgButtonChecked(hWnd, wParam))
				{
					CheckDlgButton(hWnd, wParam, FALSE);
				}
				else
				{
					CheckDlgButton(hWnd, wParam, TRUE);
				}
			}
#else
			if(ID_CHECKBOX1 <= LOWORD(wParam) && LOWORD(wParam) < control_count)
			{
				if(IsDlgButtonChecked(hWnd, LOWORD(wParam)))
				{
					CheckDlgButton(hWnd, LOWORD(wParam), BST_UNCHECKED);
				}
				else
				{
					CheckDlgButton(hWnd, LOWORD(wParam), BST_CHECKED);
				}
			}
#endif
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
#if TARGET_WINCE
	case WM_ACTIVATE:
		SHHandleWMActivate(hWnd, wParam, lParam, &activateInfo, FALSE);
		break;
	case WM_SETTINGCHANGE:
		SHHandleWMSettingChange(hWnd, wParam, lParam, &activateInfo);
		break;
#endif
#if !TARGET_WINCE
	case WM_SYSCOMMAND:
		switch(wParam)
		{
		case ID_HELP_ABOUT:
#if TARGET_WIN16
			DialogBox(hCurrentInstance, MAKEINTRESOURCE(IDD_ABOUTDIALOG), hWnd, aboutProc);
#else
			// Win32 does not need MakeProcInstance, the definition simply returns the function
			DialogBox(hCurrentInstance, MAKEINTRESOURCE(IDD_ABOUTDIALOG), hWnd, AboutDialogProc);
#endif
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
#endif

	case WM_LBUTTONDOWN:
		currently_drawing = TRUE;
		last_point.x = LOWORD(lParam);
		last_point.y = HIWORD(lParam);
		break;

	case WM_LBUTTONUP:
		if(currently_drawing)
		{
			HDC hdc;
#if TARGET_WINCE
			POINT points[2];
#endif

			hdc = GetDC(hWnd);
#if !TARGET_WINCE
#if TARGET_WIN16
			MoveTo(hdc, last_point.x, last_point.y);
#else
			MoveToEx(hdc, last_point.x, last_point.y, NULL);
#endif
			LineTo(hdc, LOWORD(lParam), HIWORD(lParam));
#else
			points[0] = last_point;
			points[1].x = LOWORD(lParam);
			points[1].y = HIWORD(lParam);
			Polyline(hdc, points, 2);
#endif
			ReleaseDC(hWnd, hdc);

			currently_drawing = FALSE;
		}
		break;

	case WM_MOUSEMOVE:
		if(currently_drawing)
		{
			HDC hdc;
#if TARGET_WINCE
			POINT points[2];
#endif

			hdc = GetDC(hWnd);
#if !TARGET_WINCE
#if TARGET_WIN16
			MoveTo(hdc, last_point.x, last_point.y);
#else
			MoveToEx(hdc, last_point.x, last_point.y, NULL);
#endif
			last_point.x = LOWORD(lParam);
			last_point.y = HIWORD(lParam);
			LineTo(hdc, LOWORD(lParam), HIWORD(lParam));
#else
			points[0] = last_point;
			points[1].x = LOWORD(lParam);
			points[1].y = HIWORD(lParam);
			last_point = points[1];
			Polyline(hdc, points, 2);
#endif
			ReleaseDC(hWnd, hdc);
		}
		break;

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			RECT rect;
#if TARGET_WINCE
			POINT points[2];
#endif

			GetClientRect(hWnd, &rect);
			hdc = BeginPaint(hWnd, &ps);

#if !TARGET_WINCE
#if TARGET_WIN16
			MoveTo(hdc, rect.left, rect.top);
#else
			MoveToEx(hdc, rect.left, rect.top, NULL);
#endif
			LineTo(hdc, rect.right, rect.bottom);
#else
			points[0].x = rect.left;
			points[0].y = rect.top;
			points[1].x = rect.right;
			points[1].y = rect.bottom;
			Polyline(hdc, points, 2);
#endif

#if !TARGET_WINCE
#if TARGET_WIN16
			MoveTo(hdc, 0, 100);
#else
			MoveToEx(hdc, 0, 100, NULL);
#endif
			LineTo(hdc, 100, 0);
#else
			points[0].x = 0;
			points[0].y = 100;
			points[1].x = 100;
			points[1].y = 0;
			Polyline(hdc, points, 2);
#endif

#define MESSAGE TEXT("Please enjoy this sample application")
#define MESSAGE_LENGTH (sizeof(MESSAGE) / sizeof(TCHAR) - 1)
#if !TARGET_WINCE
			TextOut(hdc, 0, 0, MESSAGE, MESSAGE_LENGTH);
#else
			DrawText(hdc, MESSAGE, MESSAGE_LENGTH, &rect, DT_LEFT | DT_TOP);
#endif
#undef MESSAGE
#undef MESSAGE_LENGTH

			EndPaint(hWnd, &ps);
		}
		break;
#if !TARGET_WINCE
	case WM_CLOSE:
		if(MessageBox(hWnd, TEXT("Are you sure you want to quit?"), TEXT("Question"), MB_YESNO | MB_TASKMODAL) == IDYES)
			DestroyWindow(hWnd);
		break;
#endif
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0L;
}

TCHAR szAppClassName[10];
TCHAR szAppTitle[20];

#if TARGET_WIN16
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#elif TARGET_WINCE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
#elif !UNICODE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
#endif
{
	HWND hWnd;
	MSG msg;
	HACCEL hAccelerators;

#if VERSION_WIN5 || VERSION_WIN6
	INITCOMMONCONTROLSEX icc;
#endif

#if !TARGET_WINCE
	DWORD winver = GetVersion();
	winver_major = LOBYTE(LOWORD(winver));
	winver_minor = HIBYTE(LOWORD(winver));
#else
	OSVERSIONINFO osVersionInfo;
	if(!GetVersionEx(&osVersionInfo))
	{
		MessageBox(NULL, TEXT("Error obtaining version information"), TEXT("Error"), MB_OK | MB_TASKMODAL);
	}
	else
	{
		winver_major = osVersionInfo.dwMajorVersion;
		winver_minor = osVersionInfo.dwMinorVersion;
	}
#endif

#if VERSION_WIN5 || VERSION_WIN6
	if(winver_major >= 5)
	{
		icc.dwSize = sizeof(icc);
		icc.dwICC = ICC_WIN95_CLASSES;
		if(!InitCommonControlsEx(&icc))
		{
			MessageBox(NULL, TEXT("Error initializing the common controls"), TEXT("Error"), MB_OK | MB_TASKMODAL);
		}
	}
#endif

#if VERSION_WIN3
	if(winver_major == 1)
	{
		// Windows 1.x has no checks against version numbers, and the Windows 3.x version has no way to degrade gracefully
		// This dialog box imitates the message displayed on Windows 2.x when attempting to run this
		MessageBox(NULL, "A newer version of Windows is required to run this application", "MS-DOS", MB_OK | MB_TASKMODAL);
		return FALSE;
	}
#endif

	{
		TCHAR * description;
		TCHAR buffer[64];

		switch(winver_major)
		{
#if !TARGET_WINCE
		case 0:
#else
		default:
#endif
			description = TEXT("undefined Windows version");
			break;
#if !TARGET_WINCE
		case 1:
			description = TEXT("Windows 1.x");
			break;
		case 2:
			description = TEXT("Windows 2.x");
			break;
#endif
		case 3:
#if !TARGET_WINCE
			description = TEXT("Windows 3.x");
#else
			description = TEXT("Pocket PC 2000/2002");
#endif
			break;
		case 4:
#if !TARGET_WINCE
			description = TEXT("Windows NT or Windows 95/98/ME");
#else
			description = TEXT("Windows Mobile 2003");
#endif
			break;
		case 5:
#if !TARGET_WINCE
			description = TEXT("Windows 2000 or Windows XP or Windows Server 2003");
#else
			description = TEXT("Windows 5.0/6.0/6.1");
#endif
			break;
		case 6:
#if !TARGET_WINCE
			description = TEXT("Windows Vista/7/8.x or Windows 2008/2012");
#else
			description = TEXT("Windows 6.5 or Windows Phone 7/8");
#endif
			break;
#if !TARGET_WINCE
		default:
			description = TEXT("later Windows");
			break;
#endif
		}

		stprintf(buffer,
#if !TARGET_WINCE
			sizeof buffer / sizeof(TCHAR),
#endif
			TEXT("Version: %d.%02d - %" PRIsT "s"), winver_major, winver_minor, description);

		MessageBox(NULL, buffer, TEXT("printf"), MB_OK | MB_TASKMODAL);
	}

	if(lpCmdLine[0] != '\0')
	{
		MessageBox(NULL, lpCmdLine, TEXT("Command line"), MB_OK | MB_TASKMODAL);
	}

#if TARGET_WIN16
	// Win32 has no hPrevInstance
	if(!hPrevInstance)
	{
#endif

		if(LoadString(hInstance, IDS_APP_CLASS_NAME, szAppClassName, sizeof szAppClassName) == 0)
		{
			MessageBox(NULL, TEXT("Error loading string (class name)"), TEXT("Error"), MB_OK | MB_TASKMODAL);
			return FALSE;
		}
		if(LoadString(hInstance, IDS_APP_TITLE, szAppTitle, sizeof szAppTitle) == 0)
		{
			MessageBox(NULL, TEXT("Error loading string (application title)"), TEXT("Error"), MB_OK | MB_TASKMODAL);
			return FALSE;
		}

#if TARGET_WIN16
	}
	else
	{
		GetInstanceData(hPrevInstance, szAppClassName, sizeof szAppClassName);
		GetInstanceData(hPrevInstance, szAppTitle, sizeof szAppTitle);
	}
#endif

#if TARGET_WINCE || TARGET_WIN16
# if TARGET_WIN16
	if(hPrevInstance)
# elif TARGET_WINCE
	hWnd = FindWindow(szAppClassName, szAppTitle);
	if(hWnd != NULL)
# endif
	{
		MessageBox(NULL, TEXT("Not your first time"), TEXT("Simple Message"), MB_OK | MB_TASKMODAL);
# if TARGET_WINCE
		// set focus to foremost child window and bring any owned child window to the foreground
		SetForegroundWindow((HWND)((ULONG)hWnd | 1));
		return FALSE;
# endif
	}
#endif

#if !TARGET_WIN64
#if TARGET_WIN32
	// Win32s does not support RegisterClassEx
	if(winver_major < 4)
#endif // TARGET_WIN16 || TARGET_WIN32
	{
		WNDCLASS appWndClass = { 0 };

#if !TARGET_WINCE
		appWndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
#else
		appWndClass.hCursor = NULL;
#endif
		appWndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
#if VERSION_WIN1
		appWndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU1);
#elif VERSION_WIN2
		// graceful degradation
		appWndClass.lpszMenuName = winver_major == 1 ? MAKEINTRESOURCE(IDR_MAINMENU1) : MAKEINTRESOURCE(IDR_MAINMENU);
#elif !TARGET_WINCE
		appWndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
#else
		appWndClass.lpszMenuName = NULL;
#endif
		appWndClass.lpszClassName = szAppClassName;
		appWndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		appWndClass.hInstance = hInstance;
		appWndClass.style = CS_HREDRAW | CS_VREDRAW;
		appWndClass.lpfnWndProc = AppWndProc;

		if(!RegisterClass(&appWndClass))
		{
			MessageBox(NULL, TEXT("Error registering window class"), TEXT("Error"), MB_OK | MB_TASKMODAL);
			return FALSE;
		}
	}
#endif // TARGET_WIN64
#if TARGET_WIN32 || TARGET_WIN64
#if TARGET_WIN32
	else // if winver_major >= 4
#endif
	{
		WNDCLASSEX appWndClass = { 0 };

		appWndClass.cbSize = sizeof(WNDCLASSEX); // Win32
		appWndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		appWndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
		appWndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
		appWndClass.lpszClassName = szAppClassName;
		appWndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		appWndClass.hInstance = hInstance;
		appWndClass.style = CS_HREDRAW | CS_VREDRAW;
		appWndClass.lpfnWndProc = AppWndProc;
		appWndClass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON)); // Win32

		if(!RegisterClassEx(&appWndClass))
		{
			MessageBox(NULL, TEXT("Error registering window class"), TEXT("Error"), MB_OK | MB_TASKMODAL);
			return FALSE;
		}
	}
#endif

	currently_drawing = FALSE;
	control_position = 0;
	control_count = ID_CHECKBOX1;
	hTextEdit = NULL;

#if VERSION_WIN1
	hWnd = CreateWindow(
		szAppClassName,
		szAppTitle,
		WS_TILED | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX,
		0,
		0,
		0,
		0,
		NULL,
		NULL,
		hInstance,
		NULL);
#elif VERSION_WIN2
	hWnd = CreateWindow(
		szAppClassName,
		szAppTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		200,
		200,
		NULL,
		NULL,
		hInstance,
		NULL);
#elif VERSION_WIN3
	hWnd = CreateWindowEx(
		0,
		szAppClassName,
		szAppTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		200,
		200,
		NULL,
		NULL,
		hInstance,
		NULL);
#elif !TARGET_WINCE
	hWnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		szAppClassName,
		szAppTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		200,
		200,
		NULL,
		NULL,
		hInstance,
		NULL);
#else
	hWnd = CreateWindowEx(
		0,
		szAppClassName,
		szAppTitle,
		WS_VISIBLE, // fill the full screen
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);
#endif

	if(hWnd == NULL)
	{
		MessageBox(NULL, TEXT("Error creating window"), TEXT("Error"), MB_OK | MB_TASKMODAL);
		return FALSE;
	}

	hCurrentInstance = hInstance;

#if TARGET_WIN16
	// Win32 does not need MakeProcInstance, the definition simply returns the function
	aboutProc = MakeProcInstance((FARPROC)AboutDialogProc, hCurrentInstance);
#endif

#if !TARGET_WINCE
	{
		HMENU hSysMenu = GetSystemMenu(hWnd, FALSE);
		ChangeMenu(hSysMenu, 0, NULL, 999, MF_APPEND | MF_SEPARATOR);
#if VERSION_WIN1
		ChangeMenu(hSysMenu, 0, "About\tF1", ID_HELP_ABOUT, MF_APPEND | MF_STRING);
#elif VERSION_WIN2
		// graceful degradation
		ChangeMenu(hSysMenu, 0, winver_major == 1 ? "About\tF1" : "A&bout\tF1", ID_HELP_ABOUT, MF_APPEND | MF_STRING);
#else
		ChangeMenu(hSysMenu, 0, TEXT("A&bout\tF1"), ID_HELP_ABOUT, MF_APPEND | MF_STRING);
#endif
	}
#endif

	hAccelerators = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

	if(hAccelerators == NULL)
	{
		MessageBox(NULL, TEXT("Error loading accelerators"), TEXT("Error"), MB_OK | MB_TASKMODAL);
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while(GetMessage(&msg, NULL, 0, 0) > 0)
	{
		if(!TranslateAccelerator(msg.hwnd, hAccelerators, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

#if TARGET_WIN16
	FreeProcInstance(aboutProc);
#endif

	MessageBox(NULL, TEXT("Leaving already?"), TEXT("Simple Message"), MB_OK | MB_TASKMODAL);

	return msg.wParam;
}

