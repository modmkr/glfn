
extern HINSTANCE g_hInst;
extern HWND g_hWnd;
extern HWND g_hFontDialog;
extern HWND g_hRangeDialog;

enum SizeUnits
{
	SU_POINTS,
	SU_PIXELS
};

extern char g_szTypeface[MAX_PATH];
extern int g_eCharSet;
extern bool g_bUnicode;
extern bool g_bSetMaxRange;
extern int g_iMaxRange;
extern bool g_bSkipNonPrintable;
extern bool g_bNoDefaultChar;
extern int g_iFontSize;
extern int g_eFontSizeUnits;
extern bool g_bAdjustWidth;
extern int g_iFontWidth;
extern bool g_bBold;
extern bool g_bItalic;
extern bool g_bUnderline;
extern bool g_bStrikeOut;
extern int g_iTextureWidth;
extern int g_iTextureHeight;
extern int g_iGridWidth;
extern int g_iGridHeight;
extern bool g_bCompactWidth;
extern bool g_bMarkOverlaps;
extern bool g_bDrawGrid;
extern bool g_bStripBearings;
extern bool g_bBBoxAlign;
extern bool g_bAdjustBaseline;
extern int g_iBaseline;
extern char* g_pszSampleText;

typedef struct char_range_s
{
	unsigned short cFirst;
	unsigned short iCount;
} char_range_t;
// we do not save these
extern int g_nRanges;
extern char_range_t* g_acr;

void SaveTexture(void);
void UpdateTexture(void);
void ResetSample(void);
//int FontDialog(HWND hWndParent);
//int RangeDialog(HWND hWndParent);
HWND CreateFontDialog(HWND hWndParent);
HWND CreateRangeDialog(HWND hWndParent);
void InitRangeFromFile(FILE* stream);
void WriteRangeToFile(FILE* stream);

#define DWORD_ALIGNED(v) ((((unsigned int)(v)) + 3) & (-4))
char* AllocString(const char* pszSrc);
char* ReallocString(char* pszSrc, char* pszNew);
char* GetExtension(const char* psz);

int ShowModalDialog(HWND hDlg, HWND hWndParent);
void EndModalDialog(HWND hDlg, int eResult);
