

#define TGA_NONE			0
#define TGA_PAL				1
#define TGA_RGB				2
#define TGA_BW				3
#define TGA_RLE_PAL			9
#define TGA_RLE_RGB			10
#define TGA_RLE_BW			11
//#define TGA_XZ			32
//#define TGA_XZ1			33

//#define TGAORIGIN_LEFT

#define TGA_INVERTLEFTRIGHT 0x10
#define TGA_INVERTTOPBOTTOM 0x20

#include <pshpack1.h>

typedef struct tga_file_header_s
{
	unsigned char iIDLen;
	unsigned char eColorMapType;
	unsigned char eImageType;
	/*
	WORD iColorMapOrigin;
	WORD iColorMapLength;
	BYTE iColorMapEntrySize;
    */
	struct { //color map specification
		unsigned short iStart;
		unsigned short iLength;
		unsigned char iEntrySize;
	} cm;
	/*
	WORD xImageOrigin;
	WORD yImageOrigin;
	WORD iImageWidth;
	WORD iImageHeight;
	BYTE iImagePixelSize;
	BYTE iImageDescriptor;
	*/
	struct { //image specification
		unsigned short xOrigin;
		unsigned short yOrigin;
		unsigned short iWidth;
		unsigned short iHeight;
		unsigned char iPixelSize;
		unsigned char flags;
	} img;
} tga_file_header_t;

#include <poppack.h>
