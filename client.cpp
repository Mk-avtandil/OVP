#include <Windows.h>
#pragma warning(disable: 4996)

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
char szProcName[] = "MapClient";
HWND hStatic, hEdit, hEdit2, hBtn, hList; //Дескрипторы управляющих элементов
HPEN hPen; //создаём перо

int pascal WinMain(HINSTANCE hInst, HINSTANCE hPreInst, LPSTR lpszCmdLine, int nCmdShow) {
	HWND hWnd;
	MSG lpMsg;
	WNDCLASS wcApp;

	wcApp.lpszClassName = szProcName;
	wcApp.hInstance = hInst;
	wcApp.lpfnWndProc = WndProc;
	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcApp.lpszMenuName = 0;
	wcApp.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcApp.style = CS_HREDRAW | CS_VREDRAW;
	wcApp.cbClsExtra = 0;
	wcApp.cbWndExtra = 0;
	if (!RegisterClass(&wcApp)) return 0;

	hWnd = CreateWindow(szProcName, "Клиент", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, (HWND)NULL, (HMENU)NULL, (HINSTANCE)hInst, (LPSTR)NULL);
	ShowWindow(hWnd, nCmdShow);

	hStatic = CreateWindow("static", "Введите число:", WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER, 10, 10, 300, 25, (HWND)hWnd, (HMENU)200, (HINSTANCE)hInst, (LPSTR)NULL);

	hEdit = CreateWindow("edit", "", WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER, 320, 10, 160, 25, (HWND)hWnd, (HMENU)301, (HINSTANCE)hInst, (LPSTR)NULL);
	hList = CreateWindowEx(WS_EX_CLIENTEDGE, "listbox", "", WS_CHILDWINDOW | WS_VISIBLE | WS_HSCROLL | ES_AUTOHSCROLL, 10, 40, 471, 60, (HWND)hWnd, (HMENU)306, (HINSTANCE)hInst, (LPSTR)NULL);
	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)"Математическое ожидание");
	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)"Дисперсия");
	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)"Среднеквадричное отклонение");

	hBtn = CreateWindow("BUTTON", "Запуск", BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 490, 10, 130, 80, (HWND)hWnd, (HMENU)100, (HINSTANCE)hInst, (LPSTR)NULL);

	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg);
		DispatchMessage(&lpMsg);
	}
	return(lpMsg.wParam);
}

void drawLines(HDC hdc, float x1, float y1, float x2, float y2) {
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
}

LRESULT	CALLBACK WndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	static char str[20];
	static char *buf, *buf2;
	static HANDLE hMappingFromServer, hMappingForServer, hFileForServer;

	//Создаем файл с именем “filemapping”, который будет отображаться на адресные пространства процессов 
	hFileForServer = CreateFile("filemapping1", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//	Открываем или создаем файл на диске 
	hMappingFromServer = OpenFileMapping(FILE_MAP_WRITE, FALSE, "Mapping");
	//Создаем объект отображения файла в память с именем “Mapping2” 
	hMappingForServer = CreateFileMapping(hFileForServer, NULL, PAGE_READWRITE, 0, 20, "Mapping2");
	//Если открыть объект отображения удалось – отображаем его на виртуальное адресное пространство процесса. Адрес начала отображения записываем в указатель buf
	buf = (char*)MapViewOfFile(hMappingFromServer, FILE_MAP_WRITE, 0, 0, 20);
	buf2 = (char*)MapViewOfFile(hMappingForServer, FILE_MAP_WRITE, 0, 0, 20);

	switch (messg) {
	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) { // и эта команда связана с изменением содержимого строки ввода
			if (!SendDlgItemMessage(hWnd, 100, BM_GETCHECK, 1, 0)) {
				GetDlgItemText(hWnd, 301, str, 20);  //считываем  содержимое  строки ввода...
				strcpy(buf, str); //и записываем его в по адресу buf – в объект отображения
			} 
			if (!SendMessage(hList, LB_GETSEL, 1, 0) > 0) strcpy(buf2, "Математическое ожидание");
			if (!SendMessage(hList, LB_GETSEL, 2, 0) > 0) strcpy(buf2, "Дисперсия");
			if (!SendMessage(hList, LB_GETSEL, 3, 0) > 0) strcpy(buf2, "Среднеквадричное отклонение");
		}
		break;
	/*case WM_TIMER:
		SetDlgItemText(hWnd, 301, buf); //через каждые 5 минут обновляем содержимое статического элемента из объекта отображения
		break;*/
	case WM_DESTROY:
		//при уничтожении окна приложения: 
		UnmapViewOfFile(buf); //отменяем ранее сделанное отображение 
		UnmapViewOfFile(buf2); 
		CloseHandle(hMappingFromServer); //закрываем объект отображения
		CloseHandle(hMappingForServer);
		KillTimer(hWnd, 1);	//уничтожаем таймер
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

		drawLines(hdc, 10, 400, 500, 400); // Ось х
		drawLines(hdc, 10, 100, 10, 400); // Ось y

		for (int i = 40; i < 500; i += 40)
			drawLines(hdc, i, 405, i, 395);

		for (int i = 100; i < 400; i+= 40)
			drawLines(hdc, 5, i, 15, i);

		/*hPen = CreatePen(1, 4, RGB(255, 25, 0));
		SelectObject(hdc, hPen);*/

		EndPaint(hWnd, &ps);
		break;
	}
	return(DefWindowProc(hWnd, messg, wParam, lParam));
}