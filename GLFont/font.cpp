#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "glfont.h"
#include "resource.h"


// TODO: don't end the dialog, make it ShowModal

bool g_bFontDialogStatus;


class CFontDlg
{
public:
	static BOOL CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	void InitDialog(HWND hWnd, HWND hWndFocus, LPARAM param);
	//void EndDialog(int eResult) { ::EndDialog(m_hWnd, eResult); };
	//void EndDialog(int eResult) { ShowWindow(m_hWnd, SW_HIDE); g_bFontDialogStatus = eResult; };
	void EndDialog(int eResult) { EndModalDialog(m_hWnd, eResult); };
	void Command(int iCtrl, int eNotifyCode);
	void Notify(int iCtrl, NMHDR* pnmhdr);
	void AddFont(void);
	void UpdateList(void);
	bool DumpSelected(void);
	HWND m_hWnd;
	HWND m_hList;
	HFONT m_hSampleFont;
	//LOGFONT m_lf;
	//CHOOSEFONT m_cf;
	//char m_szFileName[MAX_PATH];
	OPENFILENAME m_ofn;
};


void CFontDlg::AddFont(void)
{
	if (GetOpenFileName(&m_ofn))
	{
		int flags = FR_PRIVATE;
		if (SendDlgItemMessage(m_hWnd, IDC_FONT_ALLOW_INSTALL, BM_GETCHECK, NULL, NULL) != 0)
		{
			flags = 0;
		}

		if (m_ofn.nFileOffset != 0 && m_ofn.lpstrFile[m_ofn.nFileOffset-1] == '\0')
		{
			char szPath[MAX_PATH];
			char* pszFileName = &m_ofn.lpstrFile[m_ofn.nFileOffset];
			while (true)
			{
				sprintf(szPath, "%s\\%s", m_ofn.lpstrFile, pszFileName);
				AddFontResourceEx(szPath, flags, 0);
				
				while (*pszFileName !='\0')
				{
					pszFileName++;
				}
				pszFileName++;
				if (*pszFileName == '\0')
				{
					break;
				}
			}
		}
		else
		{
			AddFontResourceEx(m_ofn.lpstrFile, flags, 0);
		}

		UpdateList();
	}
}


#define MAX_FONTS 512
int g_nFonts;
char* g_apszFontList[MAX_FONTS];


bool AddToFontList(char* pszFontName)
{
	for (int i = 0; i < g_nFonts; i++)
	{
		if (strcmp(g_apszFontList[i], pszFontName) == 0)
		{
			return false;
		}
	}

	g_apszFontList[g_nFonts++] = AllocString(pszFontName);

	return true;
}


int CALLBACK FontEnumProc( const LOGFONT *lpelfe, const TEXTMETRIC *lpntme, DWORD FontType, LPARAM lParam )
{
	HWND hList = (HWND)lParam;

	if (FontType == TRUETYPE_FONTTYPE)
	{
		if (AddToFontList((char*)lpelfe->lfFaceName))
		{
			LV_ITEM lvi;
			ZeroMemory(&lvi, sizeof(LV_ITEM));
			lvi.mask = LVIF_TEXT;
			lvi.iItem = 0;
			lvi.pszText = (char*)lpelfe->lfFaceName;
			SendMessage(hList, LVM_INSERTITEM, 0, (LPARAM)&lvi);
		}
	}

	return 1;
}


void CFontDlg::UpdateList(void)
{
	// free the old one
	SendMessage(m_hList, LVM_DELETEALLITEMS, NULL, NULL);

	// reset the local font list
	for (int i = 0; i < g_nFonts; i++)
	{
		free(g_apszFontList[i]);
	}
	g_nFonts = 0;

	// the font enum proc will polulate the list
	LOGFONT lf = {0};
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfFaceName[0] = '\0';
	lf.lfPitchAndFamily = 0;
	HDC hDC = GetDC(NULL);
	EnumFontFamiliesEx(hDC, &lf, FontEnumProc, (LPARAM)m_hList, 0);
	ReleaseDC(NULL, hDC);
}


#define MAX_FILE_NAME_BUFFER 8192
void CFontDlg::InitDialog(HWND hWnd, HWND hWndFocus, LPARAM param)
{
	m_hWnd = hWnd;
	m_hSampleFont = NULL;

	memset(&m_ofn, 0, sizeof(OPENFILENAME));
	m_ofn.lStructSize = sizeof(OPENFILENAME);
	m_ofn.hwndOwner = m_hWnd;
	m_ofn.hInstance = GetModuleHandle(NULL);
	m_ofn.lpstrFilter = "Supported Font Files (*.ttf;*.otf;)\0*.ttf;*.otf;\0All Files (*.*)\0*.*;\0";
	m_ofn.lpstrFile = (char*)malloc(MAX_FILE_NAME_BUFFER);
	m_ofn.lpstrFile[0] = '\0';
	m_ofn.lpstrFile[1] = '\0';
	m_ofn.nMaxFile = MAX_FILE_NAME_BUFFER;
	m_ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT;

	m_hList = GetDlgItem(m_hWnd, IDC_FONT_LIST);
	int iStyleEx = LVS_EX_FULLROWSELECT;
	SendMessage(m_hList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, (LPARAM)iStyleEx);
	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(LV_COLUMN));
	lvc.mask = LVCF_WIDTH | LVCF_TEXT;
	lvc.cx = 125;
	lvc.pszText = "Face Name";
	SendMessage(m_hList, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);
	lvc.mask |= LVCF_SUBITEM;
	lvc.cx = 75;
	lvc.pszText = "Property";
	lvc.iSubItem = 1;
	SendMessage(m_hList, LVM_INSERTCOLUMN, 1, (LPARAM)&lvc);
	UpdateList();

	SendDlgItemMessage(m_hWnd, IDC_FONT_SAMPLE, WM_SETTEXT, NULL, (LPARAM)g_pszSampleText);
}


bool CFontDlg::DumpSelected(void)
{
	int flags = LVNI_ALL | LVNI_SELECTED;
	int i = (int)SendMessage(m_hList, LVM_GETNEXTITEM, -1, (LPARAM)flags);
	if (i != -1)
	{
		ListView_GetItemText(m_hList, i, 0, g_szTypeface, MAX_PATH);
		return true;
	}
	return false;
}


void CFontDlg::Command(int eCmd, int eNotifyCode)
{
	if (eNotifyCode== BN_CLICKED)
	{
		switch (eCmd)
		{
		case IDC_FONT_ADD:
			AddFont();
			break;
		case IDOK:
			EndDialog(DumpSelected());
			break;
		case IDCANCEL:
			EndDialog(0);
			break;
		}
	}
}


void CFontDlg::Notify(int iCtrl, NMHDR* pnmhdr)
{
	if (iCtrl = IDC_FONT_LIST)
	{
		NM_LISTVIEW* pnml = (NM_LISTVIEW*)pnmhdr;
		
		if (pnmhdr->code == LVN_ITEMCHANGED)
		{
			if ((pnml->uChanged == LVIF_STATE) && (pnml->uNewState & LVIS_SELECTED) && (pnml->iItem != -1))
			{
				char buffer[MAX_PATH];
				ListView_GetItemText(m_hList, pnml->iItem, 0, buffer, MAX_PATH);
				//UpdateSampleFont(buffer);
				if (m_hSampleFont != NULL)
				{
					DeleteObject(m_hSampleFont);
				}
				m_hSampleFont = CreateFont(
					-13,0,0,0,
					FW_DONTCARE,
					FALSE,
					FALSE,
					FALSE,
					DEFAULT_CHARSET,
					OUT_TT_PRECIS,CLIP_DEFAULT_PRECIS,
					DEFAULT_QUALITY,DEFAULT_PITCH,
					buffer
					);
				SendDlgItemMessage(m_hWnd, IDC_FONT_SAMPLE, WM_SETFONT, (WPARAM)m_hSampleFont, MAKELPARAM(TRUE, 0));
			}
		}
	}
}


BOOL CALLBACK CFontDlg::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CFontDlg* pDlg;
	
	if (uMsg == WM_INITDIALOG)
	{
		pDlg = new CFontDlg();
		SetWindowLong(hWnd, DWL_USER, (LONG)pDlg);

		pDlg->m_hWnd = hWnd;

		pDlg->InitDialog(hWnd, (HWND)wParam, lParam);
	}
	else
	{
		pDlg = (CFontDlg*)GetWindowLong(hWnd, DWL_USER);
		
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
		default:
			return FALSE;
		}
	}

	return TRUE;
}

/*
int FontDialog(HWND hWndParent)
{
	static HWND s_hDlg;
	if (s_hDlg == NULL)
	{
		s_hDlg = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_DLG_CHOOSEFONT), hWndParent, CFontDlg::DlgProc);
	}

	EnableWindow(hWndParent, FALSE);
	ShowWindow(s_hDlg, SW_SHOWNORMAL);

	MSG msg;

	while (IsWindowVisible(s_hDlg))
	{
		if (!GetMessage(&msg, NULL, 0, 0))
		{
			PostQuitMessage(0);
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	EnableWindow(hWndParent, TRUE);
	SetForegroundWindow(hWndParent);

	return g_bFontDialogStatus;

	//return DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_DLG_CHOOSEFONT), hWndParent, CFontDlg::DlgProc);
}
*/

HWND CreateFontDialog(HWND hWndParent)
{
	return CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_DLG_CHOOSEFONT), hWndParent, CFontDlg::DlgProc);
}


