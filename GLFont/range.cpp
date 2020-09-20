#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "glfont.h"
#include "resource.h"


int g_nRanges;
char_range_t* g_acr;

#define ITEM_TEXT_MAX 32


class CRangeDlg
{
public:
	static BOOL CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void WriteToFile(FILE* stream);
	void ReadFromFile(FILE* stream);
	void Apply(void);
private:
	void InitDialog(HWND hWnd, HWND hWndFocus, LPARAM param);
	//void EndDialog(int eResult) { ::EndDialog(m_hWnd, eResult); };
	void Hide(bool bApply);
	void Command(int iCtrl, int eNotifyCode);
	void Notify(int iCtrl, NMHDR* pnmhdr);
	bool IsValidIndex(int i) { return i != -1; };
	unsigned int ParseRange(char* psz);
	int GetNumItems(void);
	int GetSelected(void);
	void SetItem(int i);
	void Set(void);
	void GetItem(int i);
	void Get(void);
	void MoveItem(int iFrom, int iTo);
	void MoveUp(void);
	void MoveDown(void);
	void Add(void);
	void Remove(void);
	void Load(void);
	void Save(void);
	HWND m_hWnd;
	HWND m_hList;
	OPENFILENAME m_ofn;
};


unsigned int CRangeDlg::ParseRange(char* psz)
{
	unsigned int iRange;
	
	if (*psz == '#')
	{
		iRange = strtoul(&psz[1], NULL, 16);
	}
	else
	{
		iRange = strtoul(psz, NULL, 10);
	}
	
	return max(0, min(0xFFFF, iRange));
}


void CRangeDlg::Apply(void)
{
	g_nRanges = 0;
	if (g_acr != NULL)
	{
		free(g_acr);
		g_acr = NULL;
	}
	
	int nItems = GetNumItems();
	if (nItems != 0)
	{
		g_nRanges = nItems;
		g_acr = (char_range_t*)malloc(sizeof(char_range_t)*nItems);
	}

	char buffer[32];
	int i;

	for (i = 0; i < nItems; i++)
	{
		ListView_GetItemText(m_hList, i, 0, buffer, sizeof(buffer));
		g_acr[i].cFirst = ParseRange(buffer);
		ListView_GetItemText(m_hList, i, 1, buffer, sizeof(buffer));
		unsigned int cLast = ParseRange(buffer);
		if (cLast < g_acr[i].cFirst)
		{
			g_acr[i].iCount = 0;
		}
		else
		{
			g_acr[i].iCount = cLast - g_acr[i].cFirst + 1;
		}
	}
}


void CRangeDlg::Hide(bool bApply)
{
	if (bApply)
	{
		Apply();
	}

	//ShowWindow(m_hWnd, SW_HIDE);
	EndModalDialog(m_hWnd, 0);
}


int CRangeDlg::GetNumItems(void)
{
	return ListView_GetItemCount(m_hList);
}


int CRangeDlg::GetSelected(void)
{
	int flags = LVNI_ALL | LVNI_SELECTED;
	return (int)SendMessage(m_hList, LVM_GETNEXTITEM, -1, (LPARAM)flags);
}


void CRangeDlg::SetItem(int i)
{
	char buffer[32];
	SendDlgItemMessage(m_hWnd, IDC_RANGE_MIN, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);
	ListView_SetItemText(m_hList, i, 0, buffer);
	SendDlgItemMessage(m_hWnd, IDC_RANGE_MAX, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);
	ListView_SetItemText(m_hList, i, 1, buffer);
}


void CRangeDlg::Set(void)
{
	int iItem = GetSelected();
	if (iItem != -1)
	{
		SetItem(iItem);
	}
}


void CRangeDlg::GetItem(int i)
{
	char buffer[32];
	ListView_GetItemText(m_hList, i, 0, buffer, sizeof(buffer));
	SendDlgItemMessage(m_hWnd, IDC_RANGE_MIN, WM_SETTEXT, NULL, (LPARAM)buffer);
	ListView_GetItemText(m_hList, i, 1, buffer, sizeof(buffer));
	SendDlgItemMessage(m_hWnd, IDC_RANGE_MAX, WM_SETTEXT, NULL, (LPARAM)buffer);
}


void CRangeDlg::Get(void)
{
	int iItem = GetSelected();
	if (iItem != -1)
	{
		GetItem(iItem);
	}
}


void CRangeDlg::MoveItem(int iFrom, int iTo)
{
	char szText1[32];
	char szText2[32];
	int i;
	for (i = 0; i < 2; i++) // for all columns
	{
		ListView_GetItemText(m_hList, iFrom, i, szText1, ITEM_TEXT_MAX);
		ListView_GetItemText(m_hList, iTo, i, szText2, ITEM_TEXT_MAX);
		ListView_SetItemText(m_hList, iFrom, i, szText2);
		ListView_SetItemText(m_hList, iTo, i, szText1);
	}
	ListView_SetItemState(m_hList, iTo, LVIS_SELECTED, LVIS_SELECTED);
}


void CRangeDlg::MoveUp(void)
{
	int iItem = GetSelected();
	if (iItem != -1)
	{
		if (iItem > 0)
		{
			MoveItem(iItem, iItem-1);
		}
	}
}


void CRangeDlg::MoveDown(void)
{
	int iItem = GetSelected();
	if (iItem != -1)
	{
		if (iItem + 1 < GetNumItems())
		{
			MoveItem(iItem, iItem+1);
		}
	}
}


void CRangeDlg::Add(void)
{
	int iItem = GetSelected();
	if (iItem == -1)
	{
		iItem = GetNumItems();
	}
	else
	{
		iItem++;
	}
	LV_ITEM lvi;
	ZeroMemory(&lvi, sizeof(LV_ITEM));
	lvi.mask = 0;
	lvi.iItem = iItem;
	SendMessage(m_hList, LVM_INSERTITEM, 0, (LPARAM)&lvi);
	SetItem(iItem);
	ListView_SetItemState(m_hList, iItem, LVIS_SELECTED, LVIS_SELECTED);
}


void CRangeDlg::Remove(void)
{
	int iItem = GetSelected();
	if (iItem != -1)
	{
		SendMessage(m_hList, LVM_DELETEITEM, (WPARAM)iItem, NULL);
		ListView_SetItemState(m_hList, max(0,iItem-1), LVIS_SELECTED, LVIS_SELECTED);
	}
}


void CRangeDlg::WriteToFile(FILE* stream)
{
	int nItems = GetNumItems();
	fwrite(&nItems, sizeof(int), 1, stream);

	char buffer[32];
	int iLen;
	int i;

	for (i = 0; i < nItems; i++)
	{
		ListView_GetItemText(m_hList, i, 0, buffer, sizeof(buffer));
		iLen = strlen(buffer)+1;
		fwrite(&iLen, sizeof(int), 1, stream);
		fwrite(buffer, iLen, 1, stream);
		ListView_GetItemText(m_hList, i, 1, buffer, sizeof(buffer));
		iLen = strlen(buffer)+1;
		fwrite(&iLen, sizeof(int), 1, stream);
		fwrite(buffer, iLen, 1, stream);
	}
}


void CRangeDlg::ReadFromFile(FILE* stream)
{
	SendMessage(m_hList, LVM_DELETEALLITEMS, 0, NULL);

	int nItems;
	fread(&nItems, sizeof(int), 1, stream);
	
	LV_ITEM lvi;
	ZeroMemory(&lvi, sizeof(LV_ITEM));
	lvi.mask = 0;

	char buffer[32];
	int iLen;
	int i;
	
	for (i = 0; i < nItems; i++)
	{
		lvi.iItem = i;
		SendMessage(m_hList, LVM_INSERTITEM, 0, (LPARAM)&lvi);
		fread(&iLen, sizeof(int), 1, stream);
		fread(buffer, iLen, 1, stream);
		ListView_SetItemText(m_hList, i, 0, buffer);
		fread(&iLen, sizeof(int), 1, stream);
		fread(buffer, iLen, 1, stream);
		ListView_SetItemText(m_hList, i, 1, buffer);
	}
}


void CRangeDlg::Load(void)
{
	m_ofn.Flags |= OFN_FILEMUSTEXIST;
	if (GetOpenFileName(&m_ofn))
	{
		FILE* stream = fopen(m_ofn.lpstrFile, "rb");
		if (stream != NULL)
		{
			ReadFromFile(stream);

			fclose(stream);
		}
	}
}


void CRangeDlg::Save(void)
{
	m_ofn.Flags &= ~OFN_FILEMUSTEXIST;
	//m_ofn.Flags |= OFN_OVERWRITEPROMPT;
	if (GetSaveFileName(&m_ofn))
	{
		if (GetExtension(m_ofn.lpstrFile) == NULL)
		{
			strcat(m_ofn.lpstrFile, ".cr");
		}
		
		FILE* stream = fopen(m_ofn.lpstrFile, "wb");
		if (stream != NULL)
		{
			WriteToFile(stream);

			fclose(stream);
		}
	}
}


void CRangeDlg::InitDialog(HWND hWnd, HWND hWndFocus, LPARAM param)
{
	m_hWnd = hWnd;
	m_hList = GetDlgItem(m_hWnd, IDC_RANGE_LIST);

	memset(&m_ofn, 0, sizeof(OPENFILENAME));
	m_ofn.lStructSize = sizeof(OPENFILENAME);
	m_ofn.hwndOwner = m_hWnd;
	m_ofn.hInstance = GetModuleHandle(NULL);
	m_ofn.lpstrFilter = "CR files (*.cr)\0*.cr;\0All Files (*.*)\0*.*;\0";
	m_ofn.lpstrFile = (char*)malloc(MAX_PATH);
	m_ofn.lpstrFile[0] = '\0';
	m_ofn.nMaxFile = MAX_PATH;
	m_ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	int iStyleEx = LVS_EX_FULLROWSELECT;
	SendMessage(m_hList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, (LPARAM)iStyleEx);

	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(LV_COLUMN));
	lvc.mask = LVCF_WIDTH | LVCF_TEXT;
	lvc.cx = 75;
	lvc.pszText = "Min";
	SendMessage(m_hList, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);
	lvc.mask |= LVCF_SUBITEM;
	lvc.cx = 75;
	lvc.pszText = "Max";
	lvc.iSubItem = 1;
	SendMessage(m_hList, LVM_INSERTCOLUMN, 1, (LPARAM)&lvc);

	SendDlgItemMessage(m_hWnd, IDC_RANGE_MIN, WM_SETTEXT, NULL, (LPARAM)"#00");
	SendDlgItemMessage(m_hWnd, IDC_RANGE_MAX, WM_SETTEXT, NULL, (LPARAM)"#FF");
}


void CRangeDlg::Command(int eCmd, int eNotifyCode)
{
	if (eNotifyCode== BN_CLICKED)
	{
		switch (eCmd)
		{
		case IDC_RANGE_SET:
			Set();
			break;
		case IDC_RANGE_MOVE_UP:
			MoveUp();
			break;
		case IDC_RANGE_MOVE_DOWN:
			MoveDown();
			break;
		case IDC_RANGE_ADD:
			Add();
			break;
		case IDC_RANGE_REMOVE:
			Remove();
			break;
		case IDC_RANGE_LOAD:
			Load();
			break;
		case IDC_RANGE_SAVE:
			Save();
			break;
		case IDOK:
			Hide(true);
			break;
		case IDCANCEL:
			Hide(false);
			//EndDialog(0);
			break;
		}
	}
}


void CRangeDlg::Notify(int iCtrl, NMHDR* pnmhdr)
{
	if (iCtrl = IDC_RANGE_LIST)
	{
		NM_LISTVIEW* pnml = (NM_LISTVIEW*)pnmhdr;
		
		if (pnmhdr->code == LVN_ITEMCHANGED)
		{
			if ((pnml->uChanged == LVIF_STATE) && (pnml->uNewState & LVIS_SELECTED) && (pnml->iItem != -1))
			{
				GetItem(pnml->iItem);
			}
		}
	}
}


BOOL CALLBACK CRangeDlg::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CRangeDlg* pDlg;
	
	if (uMsg == WM_INITDIALOG)
	{
		pDlg = new CRangeDlg();
		SetWindowLong(hWnd, DWL_USER, (LONG)pDlg);

		pDlg->m_hWnd = hWnd;

		pDlg->InitDialog(hWnd, (HWND)wParam, lParam);
	}
	else
	{
		pDlg = (CRangeDlg*)GetWindowLong(hWnd, DWL_USER);
		
		switch (uMsg)
		{
		case WM_COMMAND:
			pDlg->Command(LOWORD(wParam), HIWORD(wParam));
			break;
		case WM_NOTIFY:
			pDlg->Notify((int)wParam, (NMHDR*)lParam);
			break;
		case WM_CLOSE:
			//pDlg->EndDialog(0);
			pDlg->Hide(false);
			break;
		default:
			return FALSE;
		}
	}

	return TRUE;
}

/*
int RangeDialog(HWND hWndParent)
{
	static HWND s_hDlg;

	if (s_hDlg == NULL)
	{
		s_hDlg = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_DLG_RANGE), hWndParent, CRangeDlg::DlgProc);
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

	return 1;
	
	//return DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_DLG_RANGE), hWndParent, CRangeDlg::DlgProc);
}
*/


void InitRangeFromFile(FILE* stream)
{
	CRangeDlg* pDlg = (CRangeDlg*)GetWindowLong(g_hRangeDialog, DWL_USER);

	pDlg->ReadFromFile(stream);
	pDlg->Apply();
}


void WriteRangeToFile(FILE* stream)
{
	CRangeDlg* pDlg = (CRangeDlg*)GetWindowLong(g_hRangeDialog, DWL_USER);

	pDlg->WriteToFile(stream);
}


HWND CreateRangeDialog(HWND hWndParent)
{
	return CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_DLG_RANGE), hWndParent, CRangeDlg::DlgProc);
}

