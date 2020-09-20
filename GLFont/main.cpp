

#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
#include <stdio.h>

#include "rect.h"
#include "../glfn.h"
#include "glfont.h"
#include "texwnd.h"
#include "rpanel.h"

#include "resource.h"


bool g_bWindowMaximized;
int g_iWindowLeft;
int g_iWindowTop;
int g_iWindowWidth;
int g_iWindowHeight;
char* g_pszSampleText;


HINSTANCE g_hInst;
HWND g_hWnd;
HWND g_hTextureWindow;
HWND g_hSampleWindow;
HWND g_hRightPanel;
HWND g_hFontDialog;
HWND g_hRangeDialog;
HBRUSH g_hBackgroundBrush;
HBRUSH g_hMarkBrush;
HBRUSH g_hClipBrush;
HPEN g_hGridPen;
HFONT g_hDefaultFont;

HDC g_hDCOutput;
HBITMAP g_hEmptyBitmap;
HBITMAP g_hBitmap;
BYTE* g_data;
int g_iBitmapWidth;
int g_iBitmapHeight;

HFONT g_hFont;
typedef struct font_settings_s
{
	int iFontSize;
	int eFontSizeUnits;
	bool bAdjustWidth;
	int iFontWidth;
	bool bBold;
	bool bItalic;
	bool bUnderline;
	bool bStrikeOut;
	int eCharSet;
	char szTypeface[MAX_PATH];
} font_settings_t;
font_settings_t g_fsCurrent;

glf_font_metrics_t g_fm;

char g_szOutFaceName[256];
char g_szOutFullName[256];

#define MAX_GLYPHS 0x10000
int g_iDefaultGlyph;
bool g_bIsDefaultGlyphPrintable;
int g_iLastGlyph;
int g_nTotalGlyphs;
int g_nTotalKerningPairs;
glf_glyph_t* g_agl[MAX_GLYPHS];
glf_kerning_pair_t* g_akp[MAX_GLYPHS];

OPENFILENAME g_ofn;


char* AllocString(const char* pszSrc)
{
	size_t iSize;
	char* psz;

	iSize = strlen(pszSrc) + 1;
	psz = (char*)malloc(iSize);

	if (psz != NULL)
	{
		memcpy(psz, pszSrc, iSize);
	}

	return psz;
}


char* ReallocString(char* pszSrc, char* pszNew)
{
	if (pszSrc != NULL)
	{
		free(pszSrc);
	}

	return (pszNew != NULL)? AllocString(pszNew): NULL;
}


char* GetExtension(const char* psz)
{
	size_t i;

	i = strlen(psz);

	for (; i > 0; --i)
	{
		if (psz[i] == '.')
		{
			return (char*)&psz[i+1];
		}
		else if (psz[i] == '\\' || psz[i] == '/')
		{
			break;
		}
	}

	return NULL;
}


char* GetFileName(const char* psz)
{
	const char* pszFileName;
	int i;

	pszFileName = psz;

	for (i = 0; psz[i] != '\0'; i++)
	{
		if (psz[i] == '\\' || psz[i] == '/')
		{
			pszFileName = &psz[i+1];
		}
	}

	return (char*)pszFileName;
}


int g_iDialogResult;

int ShowModalDialog(HWND hDlg, HWND hWndParent)
{
	MSG msg;

	EnableWindow(hWndParent, FALSE);
	
	ShowWindow(hDlg, SW_SHOWNORMAL);
	while (IsWindowVisible(hDlg))
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

	return g_iDialogResult;
}


void EndModalDialog(HWND hDlg, int eResult)
{
	g_iDialogResult = eResult;
	ShowWindow(hDlg, SW_HIDE);
}


void ShowMessage(char* pszMessage)
{
	MessageBox(g_hWnd, pszMessage, "Error", MB_OK | MB_ICONWARNING);
}


void UpdateBitmap(void)
{
	if (g_hBitmap != NULL)
	{
		if (g_iBitmapWidth != g_iTextureWidth ||
			g_iBitmapHeight != g_iTextureHeight)
		{
			SelectObject(g_hDCOutput, g_hEmptyBitmap);
			DeleteObject(g_hBitmap);
			g_hBitmap = NULL;
		}
	}

	if (g_hBitmap == NULL)
	{
		g_iBitmapWidth = g_iTextureWidth;
		g_iBitmapHeight = g_iTextureHeight;
		
		BITMAPINFO bmi;
		memset(&bmi, 0, sizeof(BITMAPINFO));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = g_iBitmapWidth;
		bmi.bmiHeader.biHeight = -g_iBitmapHeight;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biSizeImage = 4 * g_iBitmapWidth * g_iBitmapHeight;
		g_hBitmap = CreateDIBSection(g_hDCOutput, &bmi, DIB_RGB_COLORS, (void**)&g_data, NULL, NULL);
		if (g_hBitmap == NULL)
		{
			ShowMessage("CreateDIBSection() failed. No texture was created.");
			return;
		}
		SelectObject(g_hDCOutput, g_hBitmap);
	}
}


void SetSampleWindowFont(HFONT hFont)
{
	SendMessage(g_hSampleWindow, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
}


bool IsFontChanged(font_settings_t& fs)
{
	return !( strcmp(fs.szTypeface, g_szTypeface) == 0 &&
		fs.iFontSize == g_iFontSize &&
		fs.eFontSizeUnits == g_eFontSizeUnits &&
		fs.bAdjustWidth == g_bAdjustWidth &&
		(fs.iFontWidth == g_iFontWidth || !g_bAdjustWidth) &&
		fs.bBold == g_bBold &&
		fs.bItalic == g_bItalic &&
		fs.bUnderline == g_bUnderline &&
		fs.bStrikeOut == g_bStrikeOut &&
		fs.eCharSet == g_eCharSet );
}


void UpdateFontSettings(font_settings_t& fs)
{
	strcpy(fs.szTypeface, g_szTypeface);
	fs.iFontSize = g_iFontSize;
	fs.eFontSizeUnits = g_eFontSizeUnits;
	fs.bAdjustWidth = g_bAdjustWidth;
	fs.iFontWidth = g_iFontWidth;
	fs.bBold = g_bBold;
	fs.bItalic = g_bItalic;
	fs.bUnderline = g_bUnderline;
	fs.bStrikeOut = g_bStrikeOut;
	fs.eCharSet = g_eCharSet;
}


void UpdateFont(void)
{
	if (g_hFont != NULL)
	{
		// check if the font has changed
		if (IsFontChanged(g_fsCurrent))
		{
			SetSampleWindowFont(g_hDefaultFont);
			SelectObject(g_hDCOutput, g_hDefaultFont);
			DeleteObject(g_hFont);
			g_hFont = NULL;
			/*
			if (!g_bSystemFont_Current)
			{
				RemoveFontResourceEx(g_szTypeface_Current, FR_PRIVATE, 0);
			}
			*/
		}
	}

	if (g_hFont == NULL)
	{
		// fill the struct
		UpdateFontSettings(g_fsCurrent);

		int iHeight;
		switch (g_eFontSizeUnits)
		{
		case SU_POINTS:
			iHeight = -MulDiv(g_iFontSize, GetDeviceCaps(g_hDCOutput, LOGPIXELSY), 72);
			break;
		case SU_PIXELS:
		default:
			iHeight = g_iFontSize;
			break;
		}
		
		g_hFont = CreateFont(
			iHeight,g_bAdjustWidth?g_iFontWidth:0,
			0,0,
			g_bBold? FW_BOLD: FW_DONTCARE,
			g_bItalic? TRUE: FALSE,
			g_bUnderline? TRUE: FALSE,
			g_bStrikeOut? TRUE: FALSE,
			g_eCharSet,
			OUT_TT_PRECIS,CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,DEFAULT_PITCH,
			g_szTypeface
			);
		if (g_hFont == NULL)
		{
			ShowMessage("CreateFont() failed. A default font will be used.");
			return;
		}

		SetSampleWindowFont(g_hFont);
		SelectObject(g_hDCOutput, g_hFont);
	}
}


void ProcessFont(void)
{
	// for the glyph metrics
	GLYPHMETRICS gm;
	MAT2 mat = {{0,1},{0,0},{0,0},{0,1}};
	int iSize;
	int x, y;
	int i, j;

	if (g_hBitmap == NULL)
	{
		return;
	}

	// clear bg
	SelectObject(g_hDCOutput, g_hBackgroundBrush);
	PatBlt(g_hDCOutput, 0, 0, g_iBitmapWidth, g_iBitmapHeight, PATCOPY);

	if (g_bDrawGrid)
	{
		SelectObject(g_hDCOutput, g_hGridPen);
		for (y = 0; y < g_iTextureHeight; y+=g_iGridHeight)
		{
			MoveToEx(g_hDCOutput, 0, y+g_iGridHeight-1, NULL);
			LineTo(g_hDCOutput, g_iTextureWidth, y+g_iGridHeight-1);
		}
		if (!g_bCompactWidth)
		{
			for (x = 0; x < g_iTextureWidth; x+=g_iGridWidth)
			{
				MoveToEx(g_hDCOutput, x+g_iGridWidth-1, 0, NULL);
				LineTo(g_hDCOutput, x+g_iGridWidth-1, g_iTextureHeight);
			}
		}
	}
	
	if (g_hFont == NULL)
	{
		return;
	}

	// init ranges
	iSize = GetFontUnicodeRanges(g_hDCOutput, NULL);
	GLYPHSET* pgs = (GLYPHSET*)malloc(iSize);
	GetFontUnicodeRanges(g_hDCOutput, pgs);
	int iLastPrintable = 0;
	static bool abPrintable[0x10000]; // for the entire unicode set
	memset(abPrintable, 0, sizeof(abPrintable));
	for (j = 0; j < pgs->cRanges; j++)
	{
		int iLast = pgs->ranges[j].wcLow + pgs->ranges[j].cGlyphs;
		for (i = pgs->ranges[j].wcLow; i < iLast; i++)
		{
			abPrintable[i] = true;
		}
		if (iLast > iLastPrintable)
		{
			iLastPrintable = iLast;
		}
	}

	// font metrics
	memset(&g_fm, 0, sizeof(g_fm));
	int iResult;
	if (g_bUnicode)
	{
		OUTLINETEXTMETRICW* potm;
		iSize = GetOutlineTextMetricsW(g_hDCOutput, 0, NULL);
		potm = (OUTLINETEXTMETRICW*)malloc(iSize);
		iResult = GetOutlineTextMetricsW(g_hDCOutput, iSize, potm);
		g_fm.iAscent = potm->otmAscent;
		g_fm.iDescent = potm->otmDescent;
		g_fm.iLineGap = potm->otmLineGap;
		g_iDefaultGlyph = potm->otmTextMetrics.tmDefaultChar;
		g_bIsDefaultGlyphPrintable = true;
		wchar_t buffer[256];
		wcscpy(buffer, (const wchar_t*)((BYTE*)potm + (int)potm->otmpFamilyName));
		WideCharToMultiByte(CP_ACP, 0, buffer, -1, g_szOutFaceName, 256, NULL, NULL);
		wcscpy(buffer, (const wchar_t*)((BYTE*)potm + (int)potm->otmpFullName));
		WideCharToMultiByte(CP_ACP, 0, buffer, -1, g_szOutFullName, 256, NULL, NULL);
		free(potm);
	}
	else
	{
		OUTLINETEXTMETRICA* potm;
		iSize = GetOutlineTextMetricsA(g_hDCOutput, 0, NULL);
		potm = (OUTLINETEXTMETRICA*)malloc(iSize);
		iResult = GetOutlineTextMetricsA(g_hDCOutput, iSize, potm);
		g_fm.iAscent = potm->otmAscent;
		g_fm.iDescent = potm->otmDescent;
		g_fm.iLineGap = potm->otmLineGap;
		g_iDefaultGlyph = potm->otmTextMetrics.tmDefaultChar;
		g_bIsDefaultGlyphPrintable = true;
		strcpy(g_szOutFaceName, (const char*)((BYTE*)potm + (int)potm->otmpFamilyName));
		strcpy(g_szOutFullName, (const char*)((BYTE*)potm + (int)potm->otmpFullName));
		free(potm);
	}
	if (g_bNoDefaultChar)
	{
		g_iDefaultGlyph = -1;
	}
	if (!iResult)
	{
		ShowMessage("GetOutlineTextMetrics() failed. A true type font is required.");
		return;
	}

	// XXX: A-W? assume <128 are always the same
	if (abPrintable['x'])
	{
		memset(&gm,0,sizeof(gm));
		GetGlyphOutline(g_hDCOutput, 'x', GGO_METRICS, &gm, 0, NULL, &mat);
		g_fm.iHeight = gm.gmptGlyphOrigin.y;
	}
	if (abPrintable['X'])
	{
		memset(&gm,0,sizeof(gm));
		GetGlyphOutline(g_hDCOutput, 'X', GGO_METRICS, &gm, 0, NULL, &mat);
		g_fm.iCapHeight = gm.gmptGlyphOrigin.y;
	}

	// kernings
	int nKerningPairs;
	KERNINGPAIR *pkp;
	if (g_bUnicode)
	{
		nKerningPairs = GetKerningPairsW(g_hDCOutput, 0, NULL);
		pkp = (KERNINGPAIR*)malloc(nKerningPairs*sizeof(KERNINGPAIR));
		GetKerningPairsW(g_hDCOutput, nKerningPairs, pkp);
	}
	else
	{
		nKerningPairs = GetKerningPairsA(g_hDCOutput, 0, NULL);
		pkp = (KERNINGPAIR*)malloc(nKerningPairs*sizeof(KERNINGPAIR));
		GetKerningPairsA(g_hDCOutput, nKerningPairs, pkp);
	}

	int iBaselineOffset;
	if (g_bAdjustBaseline)
	{
		iBaselineOffset = g_iBaseline;
	}
	else
	{
		iBaselineOffset = ((g_iGridHeight - (g_fm.iAscent - g_fm.iDescent)) / 2) + g_fm.iAscent;
	}

	int iMaxChar;
	bool bSpecifyRange;
	static bool abInRange[0x10000];
	if (g_bSetMaxRange || g_nRanges == 0)
	{
		iMaxChar = g_bSetMaxRange? g_iMaxRange: 0x10000;
		bSpecifyRange = false;
	}
	else
	{
		iMaxChar = 0;
		bSpecifyRange = true;
		memset(abInRange, 0, sizeof(abInRange));
		for (j = 0; j < g_nRanges; j++)
		{
			int iLast = g_acr[j].cFirst + g_acr[j].iCount;
			for (i = g_acr[j].cFirst; i < iLast; i++)
			{
				abInRange[i] = true;
			}
			if (iLast > iMaxChar)
			{
				iMaxChar = iLast;
			}
		}
	}
	if (!g_bUnicode)
	{
		iMaxChar = min(0x100, iMaxChar);
	}

	x = 0;
	y = 0;

	for (i = 0; i < iMaxChar; i++)
	{
		if (g_iDefaultGlyph == i)
		{
			// ?
		}
		if (bSpecifyRange)
		{
			if (!abInRange[i])
			{
				if (i != g_iDefaultGlyph)
				{
					continue;
				}
				g_bIsDefaultGlyphPrintable = false;
			}
		}

		if (g_bSkipNonPrintable)
		{
			if (!abPrintable[i])
			{
				if (i != g_iDefaultGlyph)
				{
					continue;
				}
				g_bIsDefaultGlyphPrintable = false;
			}
		}

		memset(&gm,0,sizeof(gm));
		if (g_bUnicode)
		{
			iResult = GetGlyphOutlineW(g_hDCOutput, i, GGO_METRICS, &gm, 0, NULL, &mat);
		}
		else
		{
			iResult = GetGlyphOutlineA(g_hDCOutput, i, GGO_METRICS, &gm, 0, NULL, &mat);
		}
		if (!iResult)
		{
			continue;
		}

		int iCellWidth;// = (g_bCompactWidth)? (g_bStripBearings)? gm.gmCellIncX: gm.gmBlackBoxX: g_iGridWidth;
		if (g_bCompactWidth)
		{
			iCellWidth = (g_bStripBearings)? gm.gmCellIncX: gm.gmBlackBoxX;
		}
		else
		{
			iCellWidth = g_iGridWidth;
		}

		if (iCellWidth > g_iTextureWidth)
		{
			// 
			break;
		}

		// check if we should start a new line here
		if ((x+iCellWidth) > g_iTextureWidth)
		{
			x = 0;
			y += g_iGridHeight;
			if ((y+g_iGridHeight) > g_iTextureHeight)
			{
				break;
			}
		}

		RECT rc; // glyph bbox rect (relative to x,y)
		int iLeftBearing;
		
		if (g_bStripBearings)
		{
			rc.left = gm.gmptGlyphOrigin.x < 0? gm.gmptGlyphOrigin.x: 0;
			rc.right = 0 + gm.gmCellIncX;
			iLeftBearing = 0;
		}
		else
		{
			rc.left = 0;
			rc.right = rc.left + gm.gmBlackBoxX;
			iLeftBearing = gm.gmptGlyphOrigin.x;
		}

		if (g_bBBoxAlign)
		{
			rc.top = 0;
			rc.bottom = rc.top + gm.gmBlackBoxY;
		}
		else
		{
			rc.top = iBaselineOffset - gm.gmptGlyphOrigin.y;
			rc.bottom = rc.top + gm.gmBlackBoxY;
		}

		RECT rcGlyph; // glyph boundaries (in texture coordinates)
		rcGlyph.left = x + rc.left;
		rcGlyph.top = y + rc.top;
		rcGlyph.right = x + rc.right;
		rcGlyph.bottom = y + rc.bottom;

		if (g_bMarkOverlaps)
		{
			bool bShouldMark = false;
			bool bClip = false;

			if (/*rc.left < 0 ||*/ rc.right > iCellWidth ||
				rc.top < 0 || rc.bottom > g_iGridHeight)
			{
				bShouldMark = true;
			}
			if (rc.left < 0)
			{
				bClip = true;
			}

			if (bShouldMark || bClip)
			{
				SelectObject(g_hDCOutput, bClip? g_hClipBrush: g_hMarkBrush);
				PatBlt(g_hDCOutput, rcGlyph.left, rcGlyph.top, RectWidth(&rcGlyph), RectHeight(&rcGlyph), PATCOPY);
			}
		}

		// setup the clip rect
		RECT rcClip = { x, y, x + iCellWidth, y + g_iGridHeight };

		int xOrigin = x - iLeftBearing;
		int yOrigin = rcGlyph.top + gm.gmptGlyphOrigin.y;
		//int flags = (g_bMarkOverlaps)? 0: ETO_CLIPPED;
		int flags = ETO_CLIPPED;
		if (g_bUnicode)
		{
			ExtTextOutW(g_hDCOutput, xOrigin, yOrigin, flags, &rcClip, (wchar_t*)&i, 1, NULL);
		}
		else
		{
			ExtTextOutA(g_hDCOutput, xOrigin, yOrigin, flags, &rcClip, (char*)&i, 1, NULL);
		}

		// intersect the rects
		RECT rcIntersect;
		IntersectRect(&rcIntersect, &rcGlyph, &rcClip);

		g_agl[i] = (glf_glyph_t*) malloc(sizeof(glf_glyph_t));
		memset(g_agl[i], 0, sizeof(glf_glyph_t));
		g_agl[i]->iX = rcIntersect.left;
		g_agl[i]->iY = rcIntersect.top;
		g_agl[i]->iWidth = RectWidth(&rcIntersect);
		g_agl[i]->iHeight = RectHeight(&rcIntersect);
		g_agl[i]->iLeftBearing = iLeftBearing;
		g_agl[i]->iTopBearing = gm.gmptGlyphOrigin.y;
		g_agl[i]->iAdvance = gm.gmCellIncX;
		g_agl[i]->nKerningPairs = 0;

		// kernings, first pass - count
		for (j = 0; j < nKerningPairs; j++)
		{
			if (pkp[j].wFirst == (unsigned)i && pkp[j].iKernAmount != 0)
			{
				g_agl[i]->nKerningPairs++;
				g_nTotalKerningPairs++;
			}
		}
		// second pass kerning - store
		if (g_agl[i]->nKerningPairs > 0)
		{
			g_akp[i] = (glf_kerning_pair_t*)malloc(g_agl[i]->nKerningPairs*sizeof(glf_kerning_pair_t));
			int n = 0;
			for (j = 0; j < nKerningPairs; j++)
			{
				if (pkp[j].wFirst == (unsigned)i && pkp[j].iKernAmount != 0)
				{
					g_akp[i][n].cSecond = pkp[j].wSecond;
					g_akp[i][n].iKernAmount = pkp[j].iKernAmount;
					n++;
				}
			}
		}

		x += iCellWidth;

		g_iLastGlyph = i;
		g_nTotalGlyphs++;
	}

	if (g_iDefaultGlyph != -1)
	{
		if (g_agl[g_iDefaultGlyph] == NULL)
		{
			g_iDefaultGlyph = -1;
		}
	}

	free(pkp);
	free(pgs);
}


void UpdateInfo(void)
{
	char buffer[1024];
	sprintf(buffer,
		"Family name: %s\n"
		"Full name: %s\n"
		"Total chars: %d\n"
		"Last char: #%.4X\n"
		"Default char: #%.4X\n"
		"Total kernings: %d\n"
		,
		g_szOutFaceName,
		g_szOutFullName,
		g_nTotalGlyphs,
		g_iLastGlyph,
		g_iDefaultGlyph,
		g_nTotalKerningPairs
		);

	SendMessage(g_hRightPanel, RP_SET_INFO, 0, (LPARAM)buffer);
}


void UpdateTexture(void)
{
	int i;

	for (i = 0; i <= g_iLastGlyph; i++)
	{
		if (g_agl[i] != NULL)
		{
			free(g_agl[i]);
			g_agl[i] = NULL;
		}
		if (g_akp[i] != NULL)
		{
			free(g_akp[i]);
			g_akp[i] = NULL;
		}
	}
	g_iDefaultGlyph = -1;
	g_bIsDefaultGlyphPrintable = false;
	g_iLastGlyph = -1;
	g_nTotalGlyphs = 0;
	g_nTotalKerningPairs = 0;
	g_szOutFaceName[0] = 0;
	g_szOutFullName[0] = 0;

	UpdateBitmap();
	if (g_hBitmap == NULL)
	{
		return;
	}

	UpdateFont();
	if (g_hFont == NULL)
	{
		return;
	}

	ProcessFont();

	UpdateInfo();

	SendMessage(g_hTextureWindow, TW_SETSIZE, 0, MAKELPARAM(g_iBitmapWidth, g_iBitmapHeight));
	InvalidateRect(g_hTextureWindow, NULL, TRUE);
}


char* GetDefaultPangam(void)
{
	return "The quick brown fox jumps over the lazy dog.";
}


void SetPangam(char* psz)
{
	g_pszSampleText = ReallocString(g_pszSampleText, psz);
}


void UpdateSampleWindow(void)
{
	SendMessage(g_hSampleWindow, WM_SETTEXT, 0, (LPARAM)g_pszSampleText);
}


void ResetSample(void)
{
	SetPangam(GetDefaultPangam());
	UpdateSampleWindow();
}


void SetDefaultExt(char* pszFileName, char* pszDefaultExt)
{
	char* pszExt = GetExtension(pszFileName);
	if (pszExt == NULL)
	{
		strcat(pszFileName, pszDefaultExt);
	}
}


void WriteGlyph(FILE* stream, int i)
{
	fprintf(stream, 
		"\tX=%d\n"
		"\tY=%d\n"
		"\tWidth=%d\n"
		"\tHeight=%d\n"
		"\tLeftBearing=%d\n"
		"\tTopBearing=%d\n"
		"\tAdvance=%d\n",
		g_agl[i]->iX,
		g_agl[i]->iY,
		g_agl[i]->iWidth,
		g_agl[i]->iHeight,
		g_agl[i]->iLeftBearing,
		g_agl[i]->iTopBearing,
		g_agl[i]->iAdvance
		);

	for (int j = 0; j < g_agl[i]->nKerningPairs; j++)
	{
		fprintf(stream, "\tKerning=#%.4X,%d\n", g_akp[i][j].cSecond, g_akp[i][j].iKernAmount);
	}
	
}


//
// the exporters
//


// plain text (metrics only)

void SaveCharacterMap(char* pszFileName)
{
	FILE* stream = fopen(pszFileName, "wt");
	if (stream)
	{
		fprintf(stream,
			"GLFontHeader\n"
			"\tLastGlyph=#%.4X\n"
			"\tTextureWidth=%d\n"
			"\tTextureHeight=%d\n"
			"End\n",
			g_iLastGlyph,
			g_iBitmapWidth,
			g_iBitmapHeight
			);

		fprintf(stream, 
			"FontMetrics\n"
			"\tAscent=%d\n"
			"\tDescent=%d\n"
			"\tLineGap=%d\n"
			//"\tWidth=%d\n"
			"\tHeight=%d\n"
			"\tCapHeight=%d\n"
			"End\n",
			g_fm.iAscent,
			g_fm.iDescent,
			g_fm.iLineGap,
			//g_fm.iWidth,
			g_fm.iHeight,
			g_fm.iCapHeight
			);

		if (g_iDefaultGlyph != -1)
		{
			fprintf(stream, "DefaultGlyph\n");
			WriteGlyph(stream, g_iDefaultGlyph);
			fprintf(stream, "End\n");
		}

		int i, j;
		for (i = 0; i <= g_iLastGlyph; i++)
		{
			if (i == g_iDefaultGlyph)
			{
				if (!g_bIsDefaultGlyphPrintable)
				{
					continue;
				}
			}
			
			if (g_agl[i] != NULL)
			{
				fprintf(stream, "Glyph #%.4X\n", i);
				WriteGlyph(stream, i);
				fprintf(stream, "End\n");
			}
		}

		fclose(stream);
	}
}


void WriteGlyphBin(FILE* stream, int i)
{	
	fwrite(g_agl[i], sizeof(glf_glyph_t), 1, stream);
	if (g_agl[i]->nKerningPairs)
	{
		fwrite(g_akp[i], g_agl[i]->nKerningPairs*sizeof(glf_kerning_pair_t), 1, stream);
	}
}


// this is my own binary format i used for a couple of projects...
// see glfn.h for the format specs

void SaveGLFont(char* pszFileName)
{
	FILE* stream;
	stream = fopen(pszFileName, "wb");
	if (stream)
	{
		// file header
		glf_file_header_t fh;
		int iTextureOffset = sizeof(glf_file_header_t);
		int iTextureDataSize = g_iBitmapWidth * g_iBitmapHeight;
		int iGlyphTableOffset = iTextureOffset + sizeof(glf_texture_header_t) + iTextureDataSize;
		fh.magic = GLF_MAGIC;
		fh.version = GLF_VERSION;
		fh.flags = 0;
		fh.iGlyphTableOffset = iGlyphTableOffset;
		fh.iTextureOffset = iTextureOffset;
		memcpy(&fh.fm, &g_fm, sizeof(glf_font_metrics_t));
		fwrite(&fh, sizeof(glf_file_header_t), 1, stream);

		// texture header
		glf_texture_header_t th;
		th.iWidth = g_iBitmapWidth;
		th.iHeight = g_iBitmapHeight;
		//th.format = GLF_RGBA;
		//th.iDataSize = iTextureDataSize;
		fwrite(&th, sizeof(glf_texture_header_t), 1, stream);

		// texture data
		//fwrite(g_data, iTextureDataSize, 1, stream);
		BYTE* buffer = (BYTE*)malloc(g_iBitmapWidth);
		int x, y;
		for (y = 0; y < g_iBitmapHeight; y++)
		{
			BYTE* scan = &g_data[g_iBitmapWidth*4*y];
			for (x = 0; x < g_iBitmapWidth; x++)
			{
				buffer[x] = scan[x*4+1];
			}
			fwrite(buffer, g_iBitmapWidth, 1, stream);
		}
		free(buffer);

		// glyph data size
		int iGlyphDataSize = 
			g_nTotalGlyphs * sizeof(glf_glyph_t) +
			g_nTotalKerningPairs * sizeof(glf_kerning_pair_t);
		fwrite(&iGlyphDataSize, sizeof(int), 1, stream);

		// glyph table
		int iGlyphTableLen = g_iLastGlyph+1;
		int iGlyphTableSize = sizeof(int)*iGlyphTableLen;
		int* aiGlyphTable = (int*)malloc(iGlyphTableSize);
		memset(aiGlyphTable, 0, iGlyphTableSize);
		glf_glyph_table_header_t gth;
		gth.iTableLen = iGlyphTableLen;
		gth.iDefaultChar = 0;

		// store the glyph data
		int i;
		for (i = 0; i <= g_iLastGlyph; i++)
		{
			if (g_agl[i] != NULL)
			{
				int iOffset = ftell(stream);

				fwrite(g_agl[i], sizeof(glf_glyph_t), 1, stream);
				if (g_agl[i]->nKerningPairs)
				{
					fwrite(g_akp[i], g_agl[i]->nKerningPairs*sizeof(glf_kerning_pair_t), 1, stream);
				}

				if (g_iDefaultGlyph == i)
				{
					gth.iDefaultChar = iOffset;
					
					if (!g_bIsDefaultGlyphPrintable)
					{
						continue;
					}
				}
				
				aiGlyphTable[i] = iOffset;
			}
		}

		// finally write down the table
		fwrite(&gth, sizeof(glf_glyph_table_header_t), 1, stream);
		fwrite(aiGlyphTable, iGlyphTableSize, 1, stream);

		free(aiGlyphTable);

		fclose(stream);
	}
}


void SaveBitmap(char* pszFileName)
{
	FILE* stream;
	BITMAPFILEHEADER bmf;
	BITMAPINFOHEADER bmi;
	int iDataSize;
	int iHeaderSize;
	int iFileSize;

	stream = fopen(pszFileName, "wb");

	if (stream != NULL)
	{
		iHeaderSize = DWORD_ALIGNED(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
		iDataSize = 4 * g_iBitmapWidth * g_iBitmapHeight;
		iFileSize = iHeaderSize + iDataSize;

		memset(&bmf, 0, sizeof(BITMAPFILEHEADER));
		bmf.bfType = 'MB';
		bmf.bfSize = iFileSize;
		bmf.bfOffBits = iHeaderSize;
		fwrite(&bmf, sizeof(BITMAPFILEHEADER), 1, stream);

		memset(&bmi, 0, sizeof(BITMAPINFOHEADER));
		bmi.biSize = sizeof(BITMAPINFOHEADER);
		bmi.biWidth = g_iBitmapWidth;
		bmi.biHeight = -g_iBitmapHeight;
		bmi.biPlanes = 1;
		bmi.biBitCount = 32;
		fwrite(&bmi, sizeof(BITMAPINFOHEADER), 1, stream);

		fseek(stream, iHeaderSize, SEEK_SET);
		fwrite(g_data, iDataSize, 1, stream);

		fclose(stream);
	}
}


#include "targa.h"


void SaveTGA(char* pszFileName, int nBPP, bool bAlpha)
{
	FILE* stream = fopen(pszFileName, "wb");
	if (stream != NULL)
	{
		tga_file_header_t fh;
		//int iHeadersSize = sizeof(tga_file_header_t);
		//int iDataSize = nBPP * g_iBitmapWidth * g_iBitmapHeight;
		//int iFileSize = iHeadersSize + iDataSize;
		int nAlphaBits = (nBPP == 4)? 8: 0;

		memset(&fh, 0, sizeof(tga_file_header_t));
		fh.iIDLen = 0;
		fh.eColorMapType = 0;
		fh.eImageType = (nBPP == 1)? TGA_BW: TGA_RGB;
		fh.cm.iStart = 0;
		fh.cm.iLength = 0;
		fh.cm.iEntrySize = 0;
		fh.img.xOrigin = 0;
		fh.img.yOrigin = 0;
		fh.img.iWidth = g_iBitmapWidth;
		fh.img.iHeight = g_iBitmapHeight;
		fh.img.iPixelSize = nBPP * 8;
		fh.img.flags = nAlphaBits | TGA_INVERTTOPBOTTOM;
		fwrite(&fh, sizeof(tga_file_header_t), 1, stream);

		if (nBPP == 1)
		{
			// grayscale
			BYTE* buffer = (BYTE*)malloc(g_iBitmapWidth);
			int x, y;
			for (y = 0; y < g_iBitmapHeight; y++)
			{
				BYTE* scan = &g_data[g_iBitmapWidth*4*y];
				for (x = 0; x < g_iBitmapWidth; x++)
				{
					buffer[x] = scan[x*4+1];
				}
				fwrite(buffer, g_iBitmapWidth, 1, stream);
			}
			free(buffer);
		}
		else if (nBPP == 4)
		{
			// RGB or alpha
			BYTE* buffer = (BYTE*)malloc(g_iBitmapWidth*4);
			int x, y;
			for (y = 0; y < g_iBitmapHeight; y++)
			{
				BYTE* scan = &g_data[g_iBitmapWidth*4*y];
				for (x = 0; x < g_iBitmapWidth; x++)
				{
					BYTE* src_pixel = &scan[x*4];
					BYTE* dst_pixel = &buffer[x*4];
					if (bAlpha)
					{
						dst_pixel[0] = 0;
						dst_pixel[1] = 0;
						dst_pixel[2] = 0;
						dst_pixel[3] = src_pixel[1];
					}
					else
					{
						dst_pixel[0] = src_pixel[1];
						dst_pixel[1] = src_pixel[1];
						dst_pixel[2] = src_pixel[1];
						dst_pixel[3] = 0xFF;
					}
				}
				fwrite(buffer, g_iBitmapWidth*4, 1, stream);
			}
			free(buffer);
		}

		fclose(stream);
	}
}


enum OutputTypes
{
	OUT_CHAR_MAPPING = 0,
	OUT_GLFONT,
	OUT_BITMAP,
	OUT_TGA8,
	OUT_TGA32_COLOR,
	OUT_TGA32_ALPHA,
};


void SaveTexture(void)
{
	if (GetSaveFileName(&g_ofn))
	{
		switch (g_ofn.nFilterIndex-1)
		{
		case OUT_CHAR_MAPPING:
			SetDefaultExt(g_ofn.lpstrFile, ".txt");
			SaveCharacterMap(g_ofn.lpstrFile);
			break;
		case OUT_GLFONT:
			SetDefaultExt(g_ofn.lpstrFile, ".glf");
			SaveGLFont(g_ofn.lpstrFile);
			break;
		case OUT_BITMAP:
			SetDefaultExt(g_ofn.lpstrFile, ".bmp");
			SaveBitmap(g_ofn.lpstrFile);
			break;
		case OUT_TGA8:
			SetDefaultExt(g_ofn.lpstrFile, ".tga");
			SaveTGA(g_ofn.lpstrFile, 1, false);
			break;
		case OUT_TGA32_COLOR:
			SetDefaultExt(g_ofn.lpstrFile, ".tga");
			SaveTGA(g_ofn.lpstrFile, 4, false);
			break;
		case OUT_TGA32_ALPHA:
			SetDefaultExt(g_ofn.lpstrFile, ".tga");
			SaveTGA(g_ofn.lpstrFile, 4, true);
			break;
		}
	}
}


void WM_Create(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	g_ofn.lStructSize = sizeof(OPENFILENAME);
	g_ofn.hwndOwner = hWnd;
	g_ofn.hInstance = g_hInst;
	g_ofn.lpstrFilter = 
		// don't break the order
		"Character Mapping (*.txt)\0*.txt;\0"
		"Binary GLFont (*.glf)\0*.glf;\0"
		"Raw Bitmap 32-bit (*.bmp)\0*.bmp;\0"
		"TGA 8-bit (*.tga)\0*.tga;\0"
		"TGA 32-bit (color) (*.tga)\0*.tga;\0"
		"TGA 32-bit (alpha) (*.tga)\0*.tga;\0";
	g_ofn.lpstrFile = (char*)malloc(MAX_PATH);
	g_ofn.lpstrFile[0] = '\0';
	g_ofn.nMaxFile = MAX_PATH;
	g_ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	HDC hDC = GetDC(NULL); // desktop
	g_hDCOutput = CreateCompatibleDC(hDC);
	g_hEmptyBitmap = CreateCompatibleBitmap(hDC, 1, 1);
	SetBkMode(g_hDCOutput, TRANSPARENT);
	SetTextColor(g_hDCOutput, 0x00FFFFFF);
	SetTextAlign(g_hDCOutput, TA_BASELINE);
	ReleaseDC(NULL, hDC);

	g_hBackgroundBrush = CreateSolidBrush(0);
	g_hMarkBrush = CreateSolidBrush(0x000000FF);
	g_hClipBrush = CreateSolidBrush(0x00FF0000);
	g_hGridPen = CreatePen(PS_SOLID, 1, 0x00FF00FF);

	g_hDefaultFont = CreateFont(
		-11,0,0,0,
		FW_DONTCARE,
		FALSE,
		FALSE,
		FALSE,
		DEFAULT_CHARSET,
        OUT_TT_ONLY_PRECIS,CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,DEFAULT_PITCH,
		"Microsoft Sans Serif"
		);
	
	g_hTextureWindow = CreateWindow(TEXWNDCLASS,NULL,WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL,0,0,1,1,hWnd,NULL,g_hInst,NULL);
	g_hSampleWindow = CreateWindow("EDIT",NULL,WS_CHILD|WS_VISIBLE|WS_BORDER|ES_MULTILINE|ES_AUTOHSCROLL|ES_AUTOVSCROLL,0,0,1,1,hWnd,NULL,g_hInst,NULL);
	g_hRightPanel = CreateWindow(RIGHTPANELCLASS,NULL,WS_CHILD|WS_VISIBLE|WS_VSCROLL,0,0,1,1,hWnd,NULL,g_hInst,NULL);
	g_hFontDialog = CreateFontDialog(hWnd);
	g_hRangeDialog = CreateRangeDialog(hWnd);

	SendMessage(g_hTextureWindow, TW_SETDC, 0, (LPARAM)g_hDCOutput);
	UpdateSampleWindow();

	SetFocus(hWnd);

	UpdateTexture();
}


void WM_Close(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	RECT rect;

	if (!IsIconic(g_hWnd))
	{
		g_bWindowMaximized = (IsZoomed(g_hWnd) == TRUE);

		if (!g_bWindowMaximized)
		{
			if (GetWindowRect(g_hWnd, &rect))
			{
				g_iWindowLeft = rect.left;
				g_iWindowTop = rect.top;
				g_iWindowWidth = rect.right - rect.left;
				g_iWindowHeight = rect.bottom - rect.top;
			}
		}
	}

	// sample text
	if (g_pszSampleText != NULL)
	{
		free(g_pszSampleText);
		g_pszSampleText = NULL;
	}
	int iLen = (int)SendMessage(g_hSampleWindow, WM_GETTEXTLENGTH, 0, NULL) + 1;
	if (iLen != NULL)
	{
		g_pszSampleText = (char*)malloc(iLen);
		SendMessage(g_hSampleWindow, WM_GETTEXT, (WPARAM)iLen, (LPARAM)g_pszSampleText);
	}
}


#define RPANEL_WIDTH 200
#define SAMPLEWND_HEIGHT 150
void WM_Size(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	int xCross = rect.right-RPANEL_WIDTH;
	int yCross = rect.bottom-SAMPLEWND_HEIGHT;
	MoveWindow(g_hTextureWindow, 0, 0, xCross, yCross, TRUE);
	MoveWindow(g_hSampleWindow, 0, yCross, xCross, SAMPLEWND_HEIGHT, TRUE);
	MoveWindow(g_hRightPanel, xCross, 0, RPANEL_WIDTH, rect.bottom, TRUE);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		WM_Create(hWnd, wParam, lParam);
		break;

	case WM_SIZE:
		WM_Size(hWnd, wParam, lParam);
		break;

	case WM_KEYDOWN:
		SendMessage(g_hTextureWindow, WM_KEYDOWN, wParam, lParam);
		break;
	case WM_KEYUP:
		SendMessage(g_hTextureWindow, WM_KEYUP, wParam, lParam);
		break;
	case WM_MOUSEWHEEL:
		SendMessage(g_hTextureWindow, WM_MOUSEWHEEL, wParam, lParam);
		break;
	/*
	case WM_PAINT:
		WM_Paint(hWnd, wParam, lParam);
		break;
	*/
	case WM_CLOSE:
		WM_Close(hWnd, wParam, lParam);
		PostQuitMessage(0);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}


char* GetConfigFileName(void)
{
	static char szFileName[MAX_PATH];
	if (*szFileName == '\0')
	{
		GetModuleFileName(NULL, szFileName, MAX_PATH);
		strcat(szFileName, ".bin");
	}

	return szFileName;
}


#include "cfgbin.h"
CCfgFile* g_pConfig;

void InitSettings(void)
{
	g_pConfig->AddValue("Typeface",			VAR_SZ,		g_szTypeface);
	g_pConfig->AddValue("CharSet",			VAR_INT,	&g_eCharSet);
	g_pConfig->AddValue("Unicode",			VAR_BOOL,	&g_bUnicode);
	g_pConfig->AddValue("SetMaxRange",		VAR_BOOL,	&g_bSetMaxRange);
	g_pConfig->AddValue("MaxRange",			VAR_INT,	&g_iMaxRange);
	g_pConfig->AddValue("SkipNonPrintable", VAR_BOOL,	&g_bSkipNonPrintable);
	g_pConfig->AddValue("NoDefaultChar",	VAR_BOOL,	&g_bNoDefaultChar);
	g_pConfig->AddValue("FontSize",			VAR_INT,	&g_iFontSize);
	g_pConfig->AddValue("FontSizeUnits",	VAR_INT,	&g_eFontSizeUnits);
	g_pConfig->AddValue("AdjustWidth",		VAR_BOOL,	&g_bAdjustWidth);
	g_pConfig->AddValue("FontWidth", VAR_INT, &g_iFontWidth);
	g_pConfig->AddValue("Bold", VAR_BOOL, &g_bBold);
	g_pConfig->AddValue("Italic", VAR_BOOL, &g_bItalic);
	g_pConfig->AddValue("Underline", VAR_BOOL, &g_bUnderline);
	g_pConfig->AddValue("StrikeOut", VAR_BOOL, &g_bStrikeOut);
	g_pConfig->AddValue("TextureWidth", VAR_INT, &g_iTextureWidth);
	g_pConfig->AddValue("TextureHeight", VAR_INT, &g_iTextureHeight);
	g_pConfig->AddValue("GridWidth", VAR_INT, &g_iGridWidth);
	g_pConfig->AddValue("GridHeight", VAR_INT, &g_iGridHeight);
	g_pConfig->AddValue("CompactWidth", VAR_BOOL, &g_bCompactWidth);
	g_pConfig->AddValue("MarkOverlaps", VAR_BOOL, &g_bMarkOverlaps);
	g_pConfig->AddValue("DrawGrid", VAR_BOOL, &g_bDrawGrid);
	g_pConfig->AddValue("StripBearings", VAR_BOOL, &g_bStripBearings);
	g_pConfig->AddValue("BBoxAlign", VAR_BOOL, &g_bBBoxAlign);
	g_pConfig->AddValue("AdjustBaseline", VAR_BOOL, &g_bAdjustBaseline);
	g_pConfig->AddValue("Baseline", VAR_INT, &g_iBaseline);

	g_pConfig->AddValue("WindowMaximized", VAR_BOOL, &g_bWindowMaximized);
	g_pConfig->AddValue("WindowLeft", VAR_INT, &g_iWindowLeft);
	g_pConfig->AddValue("WindowTop", VAR_INT, &g_iWindowTop);
	g_pConfig->AddValue("WindowWidth", VAR_INT, &g_iWindowWidth);
	g_pConfig->AddValue("WindowHeight", VAR_INT, &g_iWindowHeight);
	g_pConfig->AddValue("SampleText", VAR_SZ_PTR, &g_pszSampleText);

	SetPangam(GetDefaultPangam());
}


void LoadSettings(void)
{
	g_pConfig = new CCfgFile();
	InitSettings();

	FILE* stream = fopen(GetConfigFileName(), "rb");
	if (stream)
	{
		g_pConfig->Load(stream);
		//LoadRangesFromFile(stream);
		fclose(stream);
	}
}


void SaveSettings(void)
{
	FILE* stream = fopen(GetConfigFileName(), "wb");
	if (stream)
	{
		g_pConfig->Save(stream);
		//SaveRangesToFile(stream);
		fclose(stream);
	}
	delete g_pConfig;
}


#define MAINWINDOWCLASS "GLFontWndClass"
int APIENTRY WinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR     CmdLine,
					int       nCmdShow)
{
	WNDCLASSEX wc;
	HWND hWnd;
	MSG msg;

	g_iWindowWidth = 640;
	g_iWindowHeight = 480;
	g_iWindowLeft = (GetSystemMetrics(SM_CXSCREEN) - g_iWindowWidth) / 2;
	g_iWindowTop = (GetSystemMetrics(SM_CYSCREEN) - g_iWindowHeight) / 2;

	g_hInst = hInstance;

	InitCommonControls();

	LoadSettings();

	InitTexWnd(hInstance);
	InitRightPanel(hInstance);

	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = NULL; //CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE); //NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = MAINWINDOWCLASS;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIconSm = 0;

	if (RegisterClassEx(&wc))
	{
		hWnd = CreateWindowEx(
			NULL,
			MAINWINDOWCLASS,
			"GLFont",
			WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
			g_iWindowLeft,
			g_iWindowTop,
			g_iWindowWidth, 
			g_iWindowHeight, 
			NULL,
			NULL,
			hInstance,
			NULL
			);

		if (hWnd != NULL)
		{
			g_hWnd = hWnd;
			
			ShowWindow(hWnd, g_bWindowMaximized? SW_SHOWMAXIMIZED: SW_SHOWNORMAL);

			while (GetMessage(&msg, NULL, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	SaveSettings();

	return 0;
}


