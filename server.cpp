#include <Windows.h>
#pragma warning(disable: 4996)

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
char szProcName[] = "MapServer";
HWND hEdit, hEdit2, hStatic, hButton; //Дескриптор управляющего элемента

int	pascal	WinMain(HINSTANCE	hInst, HINSTANCE hPreInst, LPSTR lpszCmdLine, int nCmdShow) {
	HWND hWnd;
	MSG lpMsg;
	WNDCLASS wcApp;
	//Регистрируем класс окна 
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
	
	//Создаем главное окно приложения
	hWnd = CreateWindow(szProcName, "Сервер", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, (HWND)NULL, (HMENU)NULL, (HINSTANCE)hInst, (LPSTR)NULL);
	ShowWindow(hWnd, nCmdShow);

	hStatic = CreateWindow("static", "Сообщения клиенту", WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER, 10, 10, 300, 25, (HWND)hWnd, (HMENU)200, (HINSTANCE)hInst, (LPSTR)NULL);
	hStatic = CreateWindow("static", "Сообщения от клиента", WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER, 10, 40, 300, 25, (HWND)hWnd, (HMENU)200, (HINSTANCE)hInst, (LPSTR)NULL);

	//Создаем управляющий элемент “строка ввода” с идентификатором 300
	hEdit = CreateWindow("edit", "", WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER, 320, 10, 160, 25, (HWND)hWnd, (HMENU)300, (HINSTANCE)hInst, (LPSTR)NULL);
	hEdit2 = CreateWindow("edit", "", WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER, 320, 40, 160, 25, (HWND)hWnd, (HMENU)306, (HINSTANCE)hInst, (LPSTR)NULL);

	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg);
		DispatchMessage(&lpMsg);
	}
	return(lpMsg.wParam);
}

LRESULT	CALLBACK WndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	static char str[20];
	static char *buf, *buf2;
	static HANDLE hFileForClient, hMappingForClient, hMappingFromClient;
	static int typeOfOperation = 0;
	
	//Создаем файл с именем “filemapping”, который будет отображаться на адресные пространства процессов 
	hFileForClient = CreateFile("filemapping2", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//Открываем или создаем файл на диске 
	hMappingFromClient = OpenFileMapping(FILE_MAP_WRITE, FALSE, "Mapping2");
	//Создаем объект отображения файла в память с именем “Mapping” 
	hMappingForClient = CreateFileMapping(hFileForClient, NULL, PAGE_READWRITE, 0, 20, "Mapping");
	//Если открыть объект отображения удалось – отображаем его на виртуальное адресное пространство процесса. Адрес начала отображения записываем в указатель buf
	buf = (char*)MapViewOfFile(hMappingForClient, FILE_MAP_WRITE, 0, 0, 20);
	buf2 = (char*)MapViewOfFile(hMappingFromClient, FILE_MAP_WRITE, 0, 0, 20);
	SetTimer(hWnd, 1, 100, NULL); //создаем таймер с периодом 1сек 

	switch (messg) {
	/*case WM_COMMAND:	//если	пришла	команда	от	управляющего элемента
		if (HIWORD(wParam) == EN_CHANGE) { // и эта команда связана с изменением содержимого строки ввода
			GetDlgItemText(hWnd, 300, str, 20);  //считываем  содержимое  строки ввода...
			//strcpy(buf3, str); //и записываем его в по адресу buf – в объект отображения
		}
		break;*/
	case WM_TIMER:
		if (!strcmp(buf2, "Математическое ожидание") == 0) typeOfOperation = 1;
		if (!strcmp(buf2, "Дисперсия") == 0) typeOfOperation = 2;
		if (!strcmp(buf2, "Среднеквадричное отклонение") == 0) typeOfOperation = 3;
		SetDlgItemText(hWnd, 300, buf);
		SetDlgItemText(hWnd, 306, buf2);
		break;
	case WM_DESTROY: 
		// при уничтожении окна приложения 
		UnmapViewOfFile(buf); //отменяем ранее сделанное отображение 
		UnmapViewOfFile(buf2); //отменяем ранее сделанное отображение 
		CloseHandle(hMappingForClient); //закрываем объект отображения 
		CloseHandle(hMappingFromClient);
		KillTimer(hWnd, 1);	//уничтожаем таймер
		PostQuitMessage(0);
		break;
	default:
		return(DefWindowProc(hWnd, messg, wParam, lParam));
	}
	return 0;
}
