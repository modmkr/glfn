
#define TEXWNDCLASS "TexWndControl"
bool InitTexWnd(HINSTANCE hInst);

#define TW_SETDC (WM_USER + 1)
// lParam == HDC
#define TW_SETSIZE (WM_USER + 2)
// LOWORD(lParam) == iWidth, HIWORD(lParam) == iHeight
