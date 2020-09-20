

// i just ported this from another my project
// so it may be a bit messy...

#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <stdio.h>

#include "texwnd.h"
#include "rect.h"


class CTexWnd
{
public:
	CTexWnd(void);
	~CTexWnd();
	void WM_Create(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_Destroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_Paint(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_Size(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_MouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_LButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_LButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_MouseWheel(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_KeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_KeyUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_HScroll(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_VScroll(HWND hWnd, WPARAM wParam, LPARAM lParam);
	//void WM_LButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void SetTextureDC(HDC hDC);
	void SetTextureSize(int iWidth, int iHeight);
private:
	void UpdateScroll(void);
	void SetViewportRect(RECT* prect);
	void GetViewportRect(RECT* prect);
	void GetViewportPos(POINT* pptSize, POINT* pptOrigin);
	void SetImageRect(RECT* prectImage);
	void GetImageRect(RECT* prectImage);
	void SetImagePos(POINT* pptSize, POINT* pptOrigin);
	void GetImagePos(POINT* pptSize, POINT* pptOrigin);
	void UpdateZoom(void);
	void ChangeZoom(int iChange);
	void InvalidateImage(void);
	void InvalidateViewport(void);
	HWND m_hWndParent;
	HWND m_hWnd;
	RECT m_rectViewport;
	RECT m_rectImage;
	int m_iZoom;
	bool m_bScrolling;
	POINT m_ptScrollOrigin;
	HDC m_hTextureDC;
	int m_iTextureWidth;
	int m_iTextureHeight;
};


CTexWnd::CTexWnd(void)
{
	m_hTextureDC = NULL;
	m_iTextureWidth = 0;
	m_iTextureHeight = 0;
}


CTexWnd::~CTexWnd()
{
	//
}


void CTexWnd::WM_Create(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	CREATESTRUCT* pcs = (CREATESTRUCT*)lParam;

	m_hWndParent = pcs->hwndParent;
	m_hWnd = hWnd;

	RECT rect;
	GetClientRect(hWnd, &rect);
	CopyRect(&m_rectViewport, &rect);
	CopyRect(&m_rectImage, &rect);

	m_bScrolling = false;
	m_iZoom = 0;
}


void CTexWnd::WM_Destroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	//
}


void CTexWnd::WM_Paint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT rect;

	hDC = BeginPaint(m_hWnd, &ps);

	if (m_hTextureDC != NULL)
	{
		GetImageRect(&rect);
		if (m_iZoom < 0)
		{
			SetStretchBltMode(hDC, HALFTONE);
		}
		else
		{
			SetStretchBltMode(hDC, COLORONCOLOR);
		}
		StretchBlt(hDC,rect.left,rect.top,RectWidth(&rect),RectHeight(&rect),m_hTextureDC,0,0,m_iTextureWidth,m_iTextureHeight,SRCCOPY);
	}

	EndPaint(m_hWnd, &ps);
}


void CTexWnd::WM_Size(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	SetViewportRect(&rect);
}


void CTexWnd::WM_MouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    int x, y;

	x = (short)LOWORD(lParam);
	y = (short)HIWORD(lParam);

    if (m_bScrolling)
	{
		POINT ptOrigin;
		
		GetImagePos(NULL, &ptOrigin);
		ptOrigin.x += x - (m_ptScrollOrigin.x + ptOrigin.x);
		ptOrigin.y += y - (m_ptScrollOrigin.y + ptOrigin.y);
		SetImagePos(NULL, &ptOrigin);

		GetImagePos(NULL, &ptOrigin); // need to get it again, because it can be clipped
        m_ptScrollOrigin.x = x - ptOrigin.x;
        m_ptScrollOrigin.y = y - ptOrigin.y;
    }
}


void CTexWnd::WM_LButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	SetFocus(hWnd);

    RECT rect;
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(m_hWnd, &pt);
	int x = pt.x;
	int y = pt.y;

    if (/*m_bReady*/ true)
	{
		GetViewportRect(&rect);

        if ((x >= rect.left) && (x < rect.right) && 
            (y >= rect.top) && (y < rect.bottom))
		{
			//GetImageRect(&rect);
            //m_ptScrollOrigin.x = x - rect.left;
            //m_ptScrollOrigin.y = y - rect.top;
			POINT ptOrigin;
			GetImagePos(NULL, &ptOrigin);
            m_ptScrollOrigin.x = x - ptOrigin.x;
            m_ptScrollOrigin.y = y - ptOrigin.y;

            SetCapture(m_hWnd);
            m_bScrolling = true;
        }
    }
}


void CTexWnd::WM_LButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    if (m_bScrolling)
	{
        ReleaseCapture();
        m_bScrolling = false;
    }
}


void CTexWnd::WM_MouseWheel(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int iDelta = (short)HIWORD(wParam) / 120;
	ChangeZoom(iDelta);
}


void CTexWnd::WM_KeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch ((DWORD)wParam)
	{
	case VK_ADD:
		ChangeZoom(1);
		break;
	case VK_SUBTRACT:
		ChangeZoom(-1);
		break;
	}
}


void CTexWnd::WM_KeyUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	//
}


void CTexWnd::SetViewportRect(RECT* prect)
{
    RECT rectImage;
    int iOldWidth;
    int iOldHeight;
    int iWidth;
    int iHeight;
    int x;
    int y;

    iOldWidth = RectWidth(&m_rectViewport);
    iOldHeight = RectHeight(&m_rectViewport);
    iWidth = RectWidth(prect);
    iHeight = RectHeight(prect);

    // todo: check this
    CopyRect(&m_rectViewport, prect);

    GetImageRect(&rectImage);
    x = (iWidth / 2) - (iOldWidth / 2);
    y = (iHeight / 2) - (iOldHeight / 2);
    OffsetRect(&rectImage, x, y);

    SetImageRect(&rectImage);
}


void CTexWnd::GetViewportRect(RECT* prect)
{
	CopyRect(prect, &m_rectViewport);
}


void CTexWnd::GetViewportPos(POINT* pptSize, POINT* pptOrigin)
{
    RECT rectViewport;

    GetViewportRect(&rectViewport);
    RectToPos(&rectViewport, pptSize, pptOrigin);
}


void CTexWnd::SetImageRect(RECT* prectImage)
{
	int iWidth = RectWidth(prectImage);
	int iHeight = RectHeight(prectImage);
    RECT rectViewport;
	GetViewportRect(&rectViewport);

    if (iWidth <= RectWidth(&rectViewport))
	{
        prectImage->left = (rectViewport.left + (RectWidth(&rectViewport) / 2)) - (iWidth / 2);
        prectImage->right = prectImage->left + iWidth;
    }
	else
	{
        if (prectImage->left > rectViewport.left)
		{
            prectImage->left = rectViewport.left;
            prectImage->right = prectImage->left + iWidth;
        }
		else if (prectImage->right < rectViewport.right)
		{
            prectImage->right = rectViewport.right;
            prectImage->left = prectImage->right - iWidth;
        }
    }

    if (iHeight <= RectHeight(&rectViewport))
	{
        prectImage->top = (rectViewport.top + (RectHeight(&rectViewport) / 2)) - (iHeight / 2);
        prectImage->bottom = prectImage->top + iHeight;
    }
	else
	{
        if (prectImage->top > rectViewport.top)
		{
            prectImage->top = rectViewport.top;
            prectImage->bottom = prectImage->top + iHeight;
        }
		else if (prectImage->bottom < rectViewport.bottom)
		{
            prectImage->bottom = rectViewport.bottom;
            prectImage->top = prectImage->bottom - iHeight;
        }
    }

	if (!EqualRect(prectImage, &m_rectImage))
	{
		if (EqualRectSize(prectImage, &m_rectImage))
		{
			ScrollWindowEx(
				m_hWnd, 
				prectImage->left - m_rectImage.left,
				prectImage->top - m_rectImage.top,
				&m_rectViewport, 
				NULL, 
				NULL, //m_hrgnUpdate, 
				NULL, 
				SW_INVALIDATE
				);

			CopyRect(&m_rectImage, prectImage);
		}
		else
		{
			// XXX
			//   InvalidateImage();
			//   ValidateRect(m_hwnd, prectImage); // XXX flicker?
			//   InvalidateRect(m_hwnd, &m_rectImage, FALSE);
			//InvalidateWindow();
			InvalidateImage();
			CopyRect(&m_rectImage, prectImage);
			InvalidateImage();
		}
	}

	UpdateScroll();

}


void CTexWnd::GetImageRect(RECT* prect)
{
    CopyRect(prect, &m_rectImage);
}


void CTexWnd::SetImagePos(POINT* pptSize, POINT* pptOrigin)
{
    RECT rectImage;
    POINT ptSize;
    POINT ptOrigin;

    GetImagePos(&ptSize, &ptOrigin);

    if (pptSize != NULL)
	{
        ptSize.x = pptSize->x;
        ptSize.y = pptSize->y;
    }

    if (pptOrigin != NULL)
	{
        ptOrigin.x = pptOrigin->x;
        ptOrigin.y = pptOrigin->y;
    }

    rectImage.left = ptOrigin.x - (ptSize.x / 2);
    rectImage.top = ptOrigin.y - (ptSize.y / 2);
    rectImage.right = rectImage.left + ptSize.x;
    rectImage.bottom = rectImage.top + ptSize.y;

    SetImageRect(&rectImage);
}


void CTexWnd::GetImagePos(POINT* pptSize, POINT* pptOrigin)
{
    RECT rectImage;

    GetImageRect(&rectImage);
    RectToPos(&rectImage, pptSize, pptOrigin);
}


void CTexWnd::InvalidateImage(void)
{
	InvalidateRect(m_hWnd, &m_rectImage, TRUE);
}


void CTexWnd::InvalidateViewport(void)
{
	InvalidateRect(m_hWnd, NULL, FALSE);
}


int RaiseToLevel(int iValue, int iLevel)
{
	int iResult;

	if (iLevel > 0)
	{
		iResult = iValue << iLevel;
	}
	else if (iLevel < 0)
	{
		iResult = iValue >> (-iLevel);
	}
	else
	{
		iResult = iValue;
	}

	return iResult;
}


void CTexWnd::UpdateZoom(void)
{
	POINT ptSize;
	POINT ptOrigin;

	//InvalidateImage();

	GetImagePos(&ptSize, &ptOrigin);
	ptSize.x = RaiseToLevel(m_iTextureWidth, m_iZoom);
	ptSize.y = RaiseToLevel(m_iTextureHeight, m_iZoom);
	SetImagePos(&ptSize, &ptOrigin);

	//InvalidateImage();
}


#define ZOOM_MAX 3
#define ZOOM_MIN -3
void CTexWnd::ChangeZoom(int iChange)
{
	m_iZoom += iChange;

	if (m_iZoom > ZOOM_MAX)
	{
		m_iZoom = ZOOM_MAX;
	}

	if (m_iZoom < ZOOM_MIN)
	{
		m_iZoom = ZOOM_MIN;
	}

	UpdateZoom();

	//InvalidateViewport();
}


void CTexWnd::UpdateScroll(void)
{
	SCROLLINFO si;
	int iViewportWidth;
	int iViewportHeight;
	int iImageWidth;
	int iImageHeight;
	//int iPageWidth;
	//int iPageHeight;
	//int iScrollWidth;
	//int iScrollHeight;
	//int iPosX;
	//int iPosY;

	iViewportWidth = RectWidth(&m_rectViewport);
	iViewportHeight = RectHeight(&m_rectViewport);
	iImageWidth = RectWidth(&m_rectImage);
	iImageHeight = RectHeight(&m_rectImage);
	//iPageWidth = iImageWidth - iViewportWidth;
	//iPageHeight = iImageHeight - iViewportHeight;
	//iPosX = iImageWidth - iPageWidth;
	//iPosY = iImageHeight - iPageHeight;

	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = iImageWidth - 1;
	si.nPage = iViewportWidth;
	si.nPos = -m_rectImage.left;
	si.nTrackPos = 0;
	SetScrollInfo(m_hWnd, SB_HORZ, &si, TRUE);

	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = iImageHeight - 1;
	si.nPage = iViewportHeight;
	si.nPos = -m_rectImage.top;
	si.nTrackPos = 0;
	SetScrollInfo(m_hWnd, SB_VERT, &si, TRUE);
}


void CTexWnd::WM_HScroll(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int iPos;
	
	switch(LOWORD(wParam))
	{
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		iPos = HIWORD(wParam);
		break;
	case SB_LINEUP:
		iPos = GetScrollPos(hWnd, SB_HORZ) - 10;
		break;
	case SB_LINEDOWN:
		iPos = GetScrollPos(hWnd, SB_HORZ) + 10;
		break;
	case SB_PAGEUP:
		iPos = GetScrollPos(hWnd, SB_HORZ) - RectWidth(&m_rectViewport);
		break;
	case SB_PAGEDOWN:
		iPos = GetScrollPos(hWnd, SB_HORZ) + RectWidth(&m_rectViewport);
		break;
	default:
		return;
	}

	RECT rect;
	GetImageRect(&rect);
	int iWidth = RectWidth(&rect);
	rect.left = -iPos;
	rect.right = rect.left + iWidth;
	SetImageRect(&rect);
}



void CTexWnd::WM_VScroll(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int iPos;
	
	switch(LOWORD(wParam))
	{
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		iPos = HIWORD(wParam);
		break;
	case SB_LINEUP:
		iPos = GetScrollPos(hWnd, SB_VERT) - 10;
		break;
	case SB_LINEDOWN:
		iPos = GetScrollPos(hWnd, SB_VERT) + 10;
		break;
	case SB_PAGEUP:
		iPos = GetScrollPos(hWnd, SB_VERT) - RectHeight(&m_rectViewport);
		break;
	case SB_PAGEDOWN:
		iPos = GetScrollPos(hWnd, SB_VERT) + RectHeight(&m_rectViewport);
		break;
	default:
		return;
	}

	RECT rect;
	GetImageRect(&rect);
	int iHeight = RectHeight(&rect);
	rect.top = -iPos;
	rect.bottom = rect.top + iHeight;
	SetImageRect(&rect);
}


void CTexWnd::SetTextureDC(HDC hDC)
{
	m_hTextureDC = hDC;
}


void CTexWnd::SetTextureSize(int iWidth, int iHeight)
{
	m_iTextureWidth = iWidth;
	m_iTextureHeight = iHeight;
	/*
	POINT ptSize;
	POINT ptOrigin;
	GetImagePos(&ptSize, &ptOrigin);
	ptSize.x = iWidth;
	ptSize.y = iHeight;
	SetImagePos(&ptSize, &ptOrigin);
	*/
	UpdateZoom();

	//InvalidateViewport();
}


LRESULT CALLBACK TexWndWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CTexWnd* pTexWnd = (CTexWnd*)GetWindowLong(hWnd, 0);

	switch(uMsg)
	{
	case WM_PAINT:
		pTexWnd->WM_Paint(hWnd, wParam, lParam);
		break;
	case WM_CREATE:
		pTexWnd = new CTexWnd();
		SetWindowLong(hWnd, 0, (LONG)pTexWnd);
		pTexWnd->WM_Create(hWnd, wParam, lParam);
		break;
	case WM_DESTROY:
		pTexWnd->WM_Destroy(hWnd, wParam, lParam);
		delete pTexWnd;
		break;
	case WM_SIZE:
		pTexWnd->WM_Size(hWnd, wParam, lParam);
		break;
    case WM_MOUSEMOVE:
        pTexWnd->WM_MouseMove(hWnd, wParam, lParam);
        break;
    case WM_LBUTTONDOWN:
        pTexWnd->WM_LButtonDown(hWnd, wParam, lParam);
        break;
    case WM_LBUTTONUP:
        pTexWnd->WM_LButtonUp(hWnd, wParam, lParam);
        break;
    case WM_MOUSEWHEEL:
        pTexWnd->WM_MouseWheel(hWnd, wParam, lParam);
        break;
	case WM_KEYDOWN:
		pTexWnd->WM_KeyDown(hWnd, wParam, lParam);
		break;
	case WM_KEYUP:
		pTexWnd->WM_KeyUp(hWnd, wParam, lParam);
		break;
	case WM_HSCROLL:
		pTexWnd->WM_HScroll(hWnd, wParam, lParam);
		break;
	case WM_VSCROLL:
		pTexWnd->WM_VScroll(hWnd, wParam, lParam);
		break;
	case TW_SETDC:
		pTexWnd->SetTextureDC((HDC)lParam);
		break;
	case TW_SETSIZE:
		pTexWnd->SetTextureSize(LOWORD(lParam), HIWORD(lParam));
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}


bool InitTexWnd(HINSTANCE hInst)
{
	WNDCLASSEX wc;

	memset(&wc, 0, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_GLOBALCLASS;
	wc.lpfnWndProc = TexWndWndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = 4;
	wc.hInstance = hInst;
	wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TEXWNDCLASS;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIconSm = 0;

	return RegisterClassEx(&wc) != 0;
}