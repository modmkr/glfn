

#include <pshpack1.h>

typedef struct glf_glyph_s
{
	int iX;
	int iY;
	int iWidth;
	int iHeight;
	int iLeftBearing;
	int iTopBearing; // the offset from the baseline to the glyph's top
	int iAdvance;
	int nKerningPairs;
} glf_glyph_t;
// an array of kering pair structures follows immediately after this structure, size of nKerningPairs

typedef struct glf_kerning_pair_s
{
	unsigned int cSecond; // the second symbol in the pair
	int iKernAmount;
} glf_kerning_pair_t;

typedef struct glf_glyph_table_header_s
{
	int iTableLen; // we support up to 65536 chars, but you can specify the exact size, the length is usually iLastChar+1
	int iDefaultChar; // an offset to the glyph storage (just like the ones the glyph table contains)
} glf_glyph_table_header_t;
// the glyph table follows immediately after the structure, it consists of integer offsets to the glyph storage
// the glyph storage follows immediately after the glyph table
// if the offset within the table is 0, the char is considered to be unprintable, the default char should be used instead
// note that all the offsets are file offsets (i.e. from the beginning of the file)
/* -- cut?
// these do match the GL_ formats to glTexImage2D
// so the data must be immediately loadable by this function (with no preprocessing)
// there is no unpack alignment used, the alignment is always 1 byte
enum GLFTextureFormat
{
	GLF_ALPHA = 1,
	GLF_LUMINANCE,
	GLF_LUMINANCE_ALPHA,
	GLF_RGB,
	GLF_BGR,
	GLF_RGBA,
	GLF_BGRA
};
*/
typedef struct glf_texture_header_s
{
	int iWidth;
	int iHeight;
	//int format;
	//int iDataSize;
} glf_texture_header_t;

typedef struct glf_font_metrics_s
{
	int iAscent;
	int iDescent;
	int iLineGap;
	int iHeight; // x-height
	int iCapHeight;
} glf_font_metrics_t;

typedef struct glf_file_header_s
{
	unsigned int magic; // GLFN
	unsigned short version;
	unsigned short flags;
	unsigned int iGlyphTableOffset;
	unsigned int iTextureOffset;
	glf_font_metrics_t fm;
} glf_file_header_t;

#define GLF_VERSION 1
#define GLF_MAGIC 'NFLG'

#include <poppack.h>

