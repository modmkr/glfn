
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "glfont.h"
#include "rpanel.h"
#include "rect.h"

#include "resource.h"


char g_szTypeface[MAX_PATH] = "Microsoft Sans Serif";
//bool g_bSystemFont = true;
int g_eCharSet = DEFAULT_CHARSET;
bool g_bUnicode;
bool g_bSetMaxRange = true;
int g_iMaxRange = 0x10000;
// TODO: range table
bool g_bSkipNonPrintable;
bool g_bNoDefaultChar;
int g_iFontSize = 8;
int g_eFontSizeUnits = SU_POINTS;
bool g_bAdjustWidth;
int g_iFontWidth;
bool g_bBold;
bool g_bItalic;
bool g_bUnderline;
bool g_bStrikeOut;
int g_iTextureWidth = 256;
int g_iTextureHeight = 256;
int g_iGridWidth = 16;
int g_iGridHeight = 16;
bool g_bCompactWidth;
bool g_bMarkOverlaps = true;
bool g_bDrawGrid = true;
bool g_bStripBearings;
bool g_bBBoxAlign;
bool g_bAdjustBaseline;
int g_iBaseline;


typedef struct enum_s
{
	char* psz;
	int e;
} enum_t;

enum_t g_aeCharSets[] =
{
	{ "default", DEFAULT_CHARSET },
	{ "ansi", ANSI_CHARSET },
	{ "gb2312", GB2312_CHARSET },
	{ "chinesebig5", CHINESEBIG5_CHARSET },
	{ "hangul", HANGUL_CHARSET },
	{ "johab", JOHAB_CHARSET },
	{ "hebrew", HEBREW_CHARSET },
	{ "arabic", ARABIC_CHARSET },
	{ "greek", GREEK_CHARSET },
	{ "turkish", TURKISH_CHARSET },
	{ "thai", THAI_CHARSET },
	{ "easteurope", EASTEUROPE_CHARSET },
	{ "russian", RUSSIAN_CHARSET },
	{ "mac", MAC_CHARSET },
	{ "baltic", BALTIC_CHARSET },
	{ "shiftjis", SHIFTJIS_CHARSET },
	{ "symbol", SYMBOL_CHARSET },
	{ "vietnamese", VIETNAMESE_CHARSET },
	{ "oem", OEM_CHARSET },
};
int g_nCharSets = sizeof(g_aeCharSets) / sizeof(enum_t);
enum_t g_aeSizeUnits[] =
{
	{ "points", SU_POINTS },
	{ "pixels", SU_PIXELS },
};
int g_nSizeUnits = sizeof(g_aeSizeUnits) / sizeof(enum_t);
char* g_apszTextureSizes[] = { "32", "64", "128", "256", "512", "1024", "2048", "4096" };
int g_nTextureSizes = sizeof(g_apszTextureSizes) / sizeof(char*);
char* g_apszGridSizes[] = { "8", "16", "32", "64", "128", "256" };
int g_nGridSizes = sizeof(g_apszGridSizes) / sizeof(char*);


int FindCharSet(int e)
{
	for (int i = 0; i < g_nCharSets; i++)
	{
		if (g_aeCharSets[i].e == e)
		{
			return i;
		}
	}

	return -1;
}


int GetValidCharSet(int i)
{
	if (i != -1)
	{
		return g_aeCharSets[i].e;
	}

	return DEFAULT_CHARSET;
}


int GetValidRange(char* buffer)
{
	int i = (*buffer == '#')? strtoul(&buffer[1], NULL, 16): atoi(buffer);
	
	return max(0, min(0x00010000, i));
}


int GetValidFontSize(char* buffer)
{
	int i = atoi(buffer);
	
	return max(1, min(1000, i));
}


int GetValidFontWidth(char* buffer)
{
	int i = atoi(buffer);
	
	return max(0, min(1000, i));
}


int GetValidTextureSize(char* buffer)
{
	int i = atoi(buffer);
	
	return max(1, min(8192, i));
}

/*
int FindTextureSize(int i)
{
	char buffer[16];
	sprintf(buffer, "%d", i);
	for (int i = 0; i < g_nTextureSizes; i++)
	{
		if (strcmp(g_apszTextureSizes[i], buffer) == 0)
		{
			return i;
		}
	}

	return -1;
}


int FindGridSize(int i)
{
	char buffer[16];
	sprintf(buffer, "%d", i);
	for (int i = 0; i < g_nGridSizes; i++)
	{
		if (strcmp(g_apszGridSizes[i], buffer) == 0)
		{
			return i;
		}
	}

	return -1;
}
*/

class CRPanelDlg
{
public:
	static BOOL CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	void InitDialog(HWND hWnd, HWND hWndFocus, LPARAM param);
	void EndDialog(int eResult) {};
	void Command(int iCtrl, int eNotifyCode);
	void Notify(int iCtrl, NMHDR* pnmhdr);
	void CheckUnicode(void);
	void CheckSetMaxRange(void);
	void CheckAdjustWidth(void);
	void CheckAdjustBaseline(void);
	void Load(void);
	void Save(void);
	void ChooseSystemFont(void);
	void LoadFromFile(void);
	void SpecifyRange(void);
	void Update(void);
	void SetInfo(char* psz);
	HWND m_hWnd;
	//LOGFONT m_lf;
	CHOOSEFONT m_cf;
	//char m_szFileName[MAX_PATH];
	OPENFILENAME m_ofn;
};


void CRPanelDlg::CheckUnicode(void)
{
	//bool bUnicode = (SendDlgItemMessage(m_hWnd, IDC_PANEL_UNICODE, BM_GETCHECK, 0, NULL) != 0);
	//EnableWindow(GetDlgItem(m_hWnd, IDC_PANEL_CHARSET), bUnicode? FALSE: TRUE);
}


void CRPanelDlg::CheckSetMaxRange(void)
{
	bool bSetMax = (SendDlgItemMessage(m_hWnd, IDC_PANEL_SET_RANGE_MAX, BM_GETCHECK, 0, NULL) != 0);
	EnableWindow(GetDlgItem(m_hWnd, IDC_PANEL_RANGE_MAX), bSetMax? TRUE: FALSE);
	EnableWindow(GetDlgItem(m_hWnd, IDC_PANEL_SPECIFY_RANGE), bSetMax? FALSE: TRUE);
}


void CRPanelDlg::CheckAdjustWidth(void)
{
	bool b = (SendDlgItemMessage(m_hWnd, IDC_PANEL_ADJUST_WIDTH, BM_GETCHECK, 0, NULL) != 0);
	EnableWindow(GetDlgItem(m_hWnd, IDC_PANEL_WIDTH), !b? FALSE: TRUE);
	EnableWindow(GetDlgItem(m_hWnd, IDC_PANEL_WIDTH_SPIN), !b? FALSE: TRUE);
}


void CRPanelDlg::CheckAdjustBaseline(void)
{
	bool b = (SendDlgItemMessage(m_hWnd, IDC_PANEL_ADJUST_BASELINE, BM_GETCHECK, 0, NULL) != 0);
	EnableWindow(GetDlgItem(m_hWnd, IDC_PANEL_BASELINE), !b? FALSE: TRUE);
	EnableWindow(GetDlgItem(m_hWnd, IDC_PANEL_BASELINE_SPIN), !b? FALSE: TRUE);
}


void CRPanelDlg::ChooseSystemFont(void)
{
	int eResult = ShowModalDialog(g_hFontDialog, g_hWnd);
	
	if (eResult)
	{
		SendDlgItemMessage(m_hWnd, IDC_PANEL_TYPEFACE, WM_SETTEXT, NULL, (LPARAM)g_szTypeface);
		//g_bSystemFont = true;
	}
}


void CRPanelDlg::LoadFromFile(void)
{
	if (GetOpenFileName(&m_ofn))
	{
		SendDlgItemMessage(m_hWnd, IDC_PANEL_TYPEFACE, WM_SETTEXT, NULL, (LPARAM)m_ofn.lpstrFile);
		//g_bSystemFont = false;
	}
}


void CRPanelDlg::SpecifyRange(void)
{
	//RangeDialog(g_hWnd);
	ShowModalDialog(g_hRangeDialog, g_hWnd);
}


void CRPanelDlg::Update(void)
{
	Save();
	// notify main to update self
	UpdateTexture();
}


void CRPanelDlg::SetInfo(char* psz)
{
	SendDlgItemMessage(m_hWnd, IDC_PANEL_INFO_TEXT, WM_SETTEXT, NULL, (LPARAM)psz);
}


void CRPanelDlg::Load(void)
{
	char buffer[64];
	int i;
	
	SendDlgItemMessage(m_hWnd, IDC_PANEL_TYPEFACE, WM_SETTEXT, NULL, (LPARAM)g_szTypeface);
	for (i = 0; i < g_nCharSets; i++)
	{
		SendDlgItemMessage(m_hWnd, IDC_PANEL_CHARSET, CB_ADDSTRING, NULL, (LPARAM)g_aeCharSets[i].psz);
	}
	SendDlgItemMessage(m_hWnd, IDC_PANEL_CHARSET, CB_SETCURSEL, (WPARAM)FindCharSet(g_eCharSet), NULL);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_UNICODE, BM_SETCHECK, (WPARAM)g_bUnicode, NULL);
	CheckUnicode();
	SendDlgItemMessage(m_hWnd, IDC_PANEL_SET_RANGE_MAX, BM_SETCHECK, (WPARAM)g_bSetMaxRange, NULL);
	CheckSetMaxRange();
	sprintf(buffer, "#%.4X", g_iMaxRange);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_RANGE_MAX, WM_SETTEXT, NULL, (LPARAM)buffer);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_SKIP_NONPRINTABLE, BM_SETCHECK, (WPARAM)g_bSkipNonPrintable, NULL);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_NO_DEFAULT, BM_SETCHECK, (WPARAM)g_bNoDefaultChar, NULL);
	sprintf(buffer, "%d", g_iFontSize);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_SIZE, WM_SETTEXT, NULL, (LPARAM)buffer);
	for (i = 0; i < g_nSizeUnits; i++)
	{
		SendDlgItemMessage(m_hWnd, IDC_PANEL_SIZE_UNITS, CB_ADDSTRING, NULL, (LPARAM)g_aeSizeUnits[i].psz);
	}
	SendDlgItemMessage(m_hWnd, IDC_PANEL_SIZE_UNITS, CB_SETCURSEL, (WPARAM)g_eFontSizeUnits, NULL);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_ADJUST_WIDTH, BM_SETCHECK, (WPARAM)g_bAdjustWidth, NULL);
	CheckAdjustWidth();
	sprintf(buffer, "%d", g_iFontWidth);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_WIDTH, WM_SETTEXT, NULL, (LPARAM)buffer);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_BOLD, BM_SETCHECK, (WPARAM)g_bBold, NULL);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_ITALIC, BM_SETCHECK, (WPARAM)g_bItalic, NULL);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_UNDERLINE, BM_SETCHECK, (WPARAM)g_bUnderline, NULL);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_STRIKE_OUT, BM_SETCHECK, (WPARAM)g_bStrikeOut, NULL);
	for (i = 0; i < g_nTextureSizes; i++)
	{
		SendDlgItemMessage(m_hWnd, IDC_PANEL_TEXTURE_WIDTH, CB_ADDSTRING, NULL, (LPARAM)g_apszTextureSizes[i]);
	}
	sprintf(buffer, "%d", g_iTextureWidth);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_TEXTURE_WIDTH, WM_SETTEXT, NULL, (LPARAM)buffer);
	for (i = 0; i < g_nTextureSizes; i++)
	{
		SendDlgItemMessage(m_hWnd, IDC_PANEL_TEXTURE_HEIGHT, CB_ADDSTRING, NULL, (LPARAM)g_apszTextureSizes[i]);
	}
	sprintf(buffer, "%d", g_iTextureHeight);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_TEXTURE_HEIGHT, WM_SETTEXT, NULL, (LPARAM)buffer);
	for (i = 0; i < g_nGridSizes; i++)
	{
		SendDlgItemMessage(m_hWnd, IDC_PANEL_GRID_WIDTH, CB_ADDSTRING, NULL, (LPARAM)g_apszGridSizes[i]);
	}
	sprintf(buffer, "%d", g_iGridWidth);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_GRID_WIDTH, WM_SETTEXT, NULL, (LPARAM)buffer);
	for (i = 0; i < g_nGridSizes; i++)
	{
		SendDlgItemMessage(m_hWnd, IDC_PANEL_GRID_HEIGHT, CB_ADDSTRING, NULL, (LPARAM)g_apszGridSizes[i]);
	}
	sprintf(buffer, "%d", g_iGridHeight);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_GRID_HEIGHT, WM_SETTEXT, NULL, (LPARAM)buffer);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_COMPACT_WIDTH, BM_SETCHECK, (WPARAM)g_bCompactWidth, NULL);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_MARK_OVERLAPS, BM_SETCHECK, (WPARAM)g_bMarkOverlaps, NULL);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_DRAW_GRID, BM_SETCHECK, (WPARAM)g_bDrawGrid, NULL);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_STRIP_BEARINGS, BM_SETCHECK, (WPARAM)g_bStripBearings, NULL);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_BBOX_ALIGN, BM_SETCHECK, (WPARAM)g_bBBoxAlign, NULL);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_ADJUST_BASELINE, BM_SETCHECK, (WPARAM)g_bAdjustBaseline, NULL);
	CheckAdjustBaseline();
	sprintf(buffer, "%d", g_iBaseline);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_BASELINE, WM_SETTEXT, NULL, (LPARAM)buffer);
}


void CRPanelDlg::Save(void)
{
	char buffer[64];

	SendDlgItemMessage(m_hWnd, IDC_PANEL_TYPEFACE, WM_GETTEXT, (WPARAM)sizeof(g_szTypeface), (LPARAM)g_szTypeface);
	g_eCharSet = GetValidCharSet((int)SendDlgItemMessage(m_hWnd, IDC_PANEL_CHARSET, CB_GETCURSEL, NULL, NULL));
	g_bUnicode = (SendDlgItemMessage(m_hWnd, IDC_PANEL_UNICODE, BM_GETCHECK, NULL, NULL) != 0);
	g_bSetMaxRange = (SendDlgItemMessage(m_hWnd, IDC_PANEL_SET_RANGE_MAX, BM_GETCHECK, NULL, NULL) != 0);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_RANGE_MAX, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);
	g_iMaxRange = GetValidRange(buffer);
	g_bSkipNonPrintable = (SendDlgItemMessage(m_hWnd, IDC_PANEL_SKIP_NONPRINTABLE, BM_GETCHECK, NULL, NULL) != 0);
	g_bNoDefaultChar = (SendDlgItemMessage(m_hWnd, IDC_PANEL_NO_DEFAULT, BM_GETCHECK, NULL, NULL) != 0);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_SIZE, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);
	g_iFontSize = GetValidFontSize(buffer);
	g_eFontSizeUnits = (int)SendDlgItemMessage(m_hWnd, IDC_PANEL_SIZE_UNITS, CB_GETCURSEL, NULL, NULL);
	g_bAdjustWidth = (SendDlgItemMessage(m_hWnd, IDC_PANEL_ADJUST_WIDTH, BM_GETCHECK, NULL, NULL) != 0);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_WIDTH, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);
	g_iFontWidth = GetValidFontWidth(buffer);
	g_bBold = (SendDlgItemMessage(m_hWnd, IDC_PANEL_BOLD, BM_GETCHECK, NULL, NULL) != 0);
	g_bItalic = (SendDlgItemMessage(m_hWnd, IDC_PANEL_ITALIC, BM_GETCHECK, NULL, NULL) != 0);
	g_bUnderline = (SendDlgItemMessage(m_hWnd, IDC_PANEL_UNDERLINE, BM_GETCHECK, NULL, NULL) != 0);
	g_bStrikeOut = (SendDlgItemMessage(m_hWnd, IDC_PANEL_STRIKE_OUT, BM_GETCHECK, NULL, NULL) != 0);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_TEXTURE_WIDTH, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);
	g_iTextureWidth = GetValidTextureSize(buffer);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_TEXTURE_HEIGHT, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);
	g_iTextureHeight = GetValidTextureSize(buffer);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_GRID_WIDTH, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);
	g_iGridWidth = GetValidTextureSize(buffer);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_GRID_HEIGHT, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);
	g_iGridHeight = GetValidTextureSize(buffer);
	g_bCompactWidth = (SendDlgItemMessage(m_hWnd, IDC_PANEL_COMPACT_WIDTH, BM_GETCHECK, NULL, NULL) != 0);
	g_bMarkOverlaps = (SendDlgItemMessage(m_hWnd, IDC_PANEL_MARK_OVERLAPS, BM_GETCHECK, NULL, NULL) != 0);
	g_bDrawGrid = (SendDlgItemMessage(m_hWnd, IDC_PANEL_DRAW_GRID, BM_GETCHECK, NULL, NULL) != 0);
	g_bStripBearings = (SendDlgItemMessage(m_hWnd, IDC_PANEL_STRIP_BEARINGS, BM_GETCHECK, NULL, NULL) != 0);
	g_bBBoxAlign = (SendDlgItemMessage(m_hWnd, IDC_PANEL_BBOX_ALIGN, BM_GETCHECK, NULL, NULL) != 0);
	g_bAdjustBaseline = (SendDlgItemMessage(m_hWnd, IDC_PANEL_ADJUST_BASELINE, BM_GETCHECK, NULL, NULL) != 0);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_BASELINE, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);
	g_iBaseline = atoi(buffer);
}


void CRPanelDlg::InitDialog(HWND hWnd, HWND hWndFocus, LPARAM param)
{
	m_hWnd = hWnd;

	memset(&m_cf, 0, sizeof(CHOOSEFONT));
	m_cf.lStructSize = sizeof(CHOOSEFONT);
	m_cf.hwndOwner = m_hWnd;
	m_cf.lpLogFont = (LOGFONT*)malloc(sizeof(LOGFONT));
	memset(m_cf.lpLogFont, 0, sizeof(LOGFONT));
	//if (g_bSystemFont)
	{
		strncpy(m_cf.lpLogFont->lfFaceName, g_szTypeface, 32);
	}
	m_cf.iPointSize = g_iFontSize;
	m_cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_TTONLY;
	m_cf.hInstance = GetModuleHandle(NULL);

	memset(&m_ofn, 0, sizeof(OPENFILENAME));
	m_ofn.lStructSize = sizeof(OPENFILENAME);
	m_ofn.hwndOwner = m_hWnd;
	m_ofn.hInstance = GetModuleHandle(NULL);
	m_ofn.lpstrFilter = "All Files (*.*)\0*.*;\0";
	m_ofn.lpstrFile = (char*)malloc(MAX_PATH);
	memset(m_ofn.lpstrFile, 0, MAX_PATH);
	//if (!g_bSystemFont)
	{
		strcpy(m_ofn.lpstrFile, g_szTypeface);
	}
	m_ofn.nMaxFile = MAX_PATH;
	m_ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	//
	SendDlgItemMessage(m_hWnd, IDC_PANEL_SIZE_SPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(m_hWnd, IDC_PANEL_SIZE), NULL);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_SIZE_SPIN, UDM_SETRANGE, 0, MAKELONG(256,1));
	SendDlgItemMessage(m_hWnd, IDC_PANEL_WIDTH_SPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(m_hWnd, IDC_PANEL_WIDTH), NULL);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_WIDTH_SPIN, UDM_SETRANGE, 0, MAKELONG(256,0));
	SendDlgItemMessage(m_hWnd, IDC_PANEL_BASELINE_SPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(m_hWnd, IDC_PANEL_BASELINE), NULL);
	SendDlgItemMessage(m_hWnd, IDC_PANEL_BASELINE_SPIN, UDM_SETRANGE, 0, MAKELONG(8192,0));
	Load();
}


void CRPanelDlg::Command(int eCmd, int eNotifyCode)
{
	if (eNotifyCode== BN_CLICKED)
	{
		switch (eCmd)
		{
		case IDC_PANEL_CHOOSE:
			ChooseSystemFont();
			break;
		case IDC_PANEL_FROM_FILE:
			LoadFromFile();
			break;
		case IDC_PANEL_UNICODE:
			CheckUnicode();
			break;
		case IDC_PANEL_SET_RANGE_MAX:
			CheckSetMaxRange();
			break;
		case IDC_PANEL_ADJUST_WIDTH:
			CheckAdjustWidth();
			break;
		case IDC_PANEL_ADJUST_BASELINE:
			CheckAdjustBaseline();
			break;
		case IDC_PANEL_SPECIFY_RANGE:
			SpecifyRange();
			break;
		case IDC_PANEL_UPDATE:
			Update();
			SetFocus(g_hWnd); // prevent focusing on the update button
			break;
		case IDC_PANEL_SAVE:
			SaveTexture();
			break;
		case IDC_PANEL_PANGAM:
			ResetSample();
			SetFocus(g_hWnd);
			break;
		}
	}
}


void CRPanelDlg::Notify(int iCtrl, NMHDR* pnmhdr)
{
	//
}


BOOL CALLBACK CRPanelDlg::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CRPanelDlg* pDlg;
	
	if (uMsg == WM_INITDIALOG)
	{
		pDlg = new CRPanelDlg();
		SetWindowLong(hWnd, DWL_USER, (LONG)pDlg);

		pDlg->m_hWnd = hWnd;

		pDlg->InitDialog(hWnd, (HWND)wParam, lParam);
	}
	else
	{
		pDlg = (CRPanelDlg*)GetWindowLong(hWnd, DWL_USER);
		
		switch (uMsg)
		{
		case WM_COMMAND:
			pDlg->Command(LOWORD(wParam), HIWORD(wParam));
			break;
		case WM_NOTIFY:
			pDlg->Notify((int)wParam, (NMHDR*)lParam);
			break;
		case WM_CLOSE:
			pDlg->EndDialog(0);
			break;
		case RP_SET_INFO:
			pDlg->SetInfo((char*)lParam);
			break;
		default:
			return FALSE;
		}
	}

	return TRUE;
}



class CRightPanel
{
public:
	void Init(void);
	//void UpdateDlg(void);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	HWND m_hWnd;
	HWND m_hDlg;
	CRPanelDlg m_pDlg;

	int Create(HWND hWnd, CREATESTRUCT *pcs);
	void Destroy(void);
	void Size(int x, int y, int flags);
	void VScroll(int nScrollCode, int nPos, HWND hWndScrollBar);
	void SetInfo(char* psz);
	void GetDlgRect(RECT* prect);
	//void Command(int iCtrl, int eNotifyCode);
	//void Notify(int iCtrl, NMHDR* pnmhdr);
};


int CRightPanel::Create(HWND hWnd, CREATESTRUCT* pcs)
{
	m_hWnd = hWnd;

	//SendMessage(hWnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
	m_hDlg = CreateDialogParam(pcs->hInstance, MAKEINTRESOURCE(IDD_DLG_RPANEL), m_hWnd, CRPanelDlg::DlgProc, NULL);

	return 0;
}


void CRightPanel::Destroy(void)
{
	//
}


void CRightPanel::GetDlgRect(RECT* prect)
{
	GetWindowRect(m_hDlg, prect);
	POINT pt = { prect->left, prect->top };
	ScreenToClient(m_hWnd, &pt);
	OffsetRect(prect, pt.x-prect->left,pt.y-prect->top);
}


void CRightPanel::Size(int xS, int yS, int flags)
{
	RECT rect;
	GetClientRect(m_hWnd, &rect);
	int iClientHeight = RectHeight(&rect);
	RECT rectDlg;
	GetDlgRect(&rectDlg);
	int iDlgHeight = RectHeight(&rectDlg);

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = iDlgHeight - 1;
	si.nPage = iClientHeight;
	si.nPos = -rectDlg.top;
	si.nTrackPos = 0;
	SetScrollInfo(m_hWnd, SB_VERT, &si, TRUE);

	SendMessage(m_hWnd, WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION,GetScrollPos(m_hWnd,SB_VERT)), NULL);
}


void CRightPanel::VScroll(int nScrollCode, int nPos, HWND hWndScrollBar)
{
	int iPos;
	RECT rect;
	GetClientRect(m_hWnd, &rect);

	switch (nScrollCode)
	{
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		iPos = nPos;
		break;
	case SB_LINEUP:
		iPos = GetScrollPos(m_hWnd, SB_VERT) - 20;
		break;
	case SB_LINEDOWN:
		iPos = GetScrollPos(m_hWnd, SB_VERT) + 20;
		break;
	case SB_PAGEUP:
		iPos = GetScrollPos(m_hWnd, SB_VERT) - RectHeight(&rect);
		break;
	case SB_PAGEDOWN:
		iPos = GetScrollPos(m_hWnd, SB_VERT) + RectHeight(&rect);
		break;
	default:
		return;
	}
	
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_POS;
	si.nPos = iPos;
	SetScrollInfo(m_hWnd, SB_VERT, &si, TRUE);
	
	iPos = GetScrollPos(m_hWnd, SB_VERT);

	RECT rectDlg;
	GetDlgRect(&rectDlg);
	int iHeight = RectHeight(&rectDlg);
	rectDlg.top = -iPos;
	rectDlg.bottom = rectDlg.top + iHeight;
	MoveWindow(m_hDlg, 0, rectDlg.top, RectWidth(&rectDlg), RectHeight(&rectDlg), TRUE);
}


void CRightPanel::SetInfo(char* psz)
{
	SendMessage(m_hDlg, RP_SET_INFO, 0, (LPARAM)psz);
}


LRESULT CALLBACK CRightPanel::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CRightPanel* pWnd = (CRightPanel*)GetWindowLong(hWnd, 0);
	LRESULT lResult = NULL;

	switch (uMsg)
	{

	case WM_SIZE:
		pWnd->Size((short)LOWORD(lParam), (short)HIWORD(lParam), (int)wParam);
		break;

	case WM_VSCROLL:
		pWnd->VScroll((int)LOWORD(wParam), (int)HIWORD(wParam), (HWND)lParam);
		break;

	case RP_SET_INFO:
		pWnd->SetInfo((char*)lParam);
		break;

	//case WM_COMMAND:
	//	pWnd->Command(LOWORD(wParam), HIWORD(wParam));
	//	break;

	//case WM_NOTIFY:
	//	pWnd->Notify(wParam, (NMHDR*)lParam);
	//	break;

	case WM_CREATE:
		pWnd = new CRightPanel();
		SetWindowLong(hWnd, 0, (LONG)pWnd);
		lResult = pWnd->Create(hWnd, (CREATESTRUCT*)lParam);
		break;

	case WM_DESTROY:
		pWnd->Destroy();
		delete pWnd;
		break;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return lResult;
}




bool InitRightPanel(HINSTANCE hInst)
{
	WNDCLASSEX wc;

	memset(&wc, 0, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_GLOBALCLASS;
	wc.lpfnWndProc = CRightPanel::WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = 4;
	wc.hInstance = hInst;
	wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = RIGHTPANELCLASS;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIconSm = 0;

	return RegisterClassEx(&wc) != 0;
}

