/*
 * pdi_img.h
 *
 *  Created on: 03/09/2012
 *      Author: Daniel Souza
 */

#ifndef PDI_IMG_H_
#define PDI_IMG_H_

#include <stdio.h>

#define PI 3.1415926535897932384626433
//#define pi 3.14159265358979323846
//#define PI 3.14159265

#define degToRad(deg) deg * (PI/180)
#define radToDeg(rad) (rad * 180) / pi

#define MAX(a,b) ((a > b) ? a : b)
#define MIN(a,b) ((a < b) ? a : b)

// -----------------------------------------------------------------------------
// ESTRUTURAS PRINCIPAIS
// -----------------------------------------------------------------------------

typedef enum COMPONENT_COLOR {
      GRAYSCALE_8BPP,
      RGB_8BPP
} c_color;

/* RGB & Graysacle pixel */
typedef struct {
	unsigned char r,g,b;
} RGBPixel;

typedef union {
    unsigned char *gray;
    RGBPixel *rgb;
} u_pixel;
/* RGB & Graysacle pixel */

typedef struct {
	char magicNumber[2];
	int width, height;
	int depth;
	int nChannels;
	u_pixel data;	
} t_image;

// -----------------------------------------------------------------------------
// UTILITARIOS
// -----------------------------------------------------------------------------
// COMUNS
t_image cloneImg(t_image img);
t_image createImg(c_color component, int width, int height);
int pointInBounds(int px, int py, int width, int height);
// ARQUIVO
unsigned char *openP62(FILE *fp, int width, int height);
unsigned char *openP52(FILE *fp, int width, int height);
t_image pdi_openImg(char *filename);
void saveImg(t_image img, const char * path);

// -----------------------------------------------------------------------------
// PROCESSAMENTO
// -----------------------------------------------------------------------------
// PROCESSAMENTO 2D
t_image rotateAndCrop(t_image img, double rad);
t_image rotate(t_image img, double rad);
t_image rotateBilinear(t_image img,  double rad);
t_image scale(t_image img, double sx, double sy);
t_image scaleBilinear(t_image img, double sx, double sy);
t_image interpolate(t_image img);
t_image interpolateBilinear(t_image img);


/* processamento HSV */
typedef struct {
	int h;
    float s, v;
} HSVPixel;

HSVPixel RGBtoHSV(RGBPixel);
RGBPixel HSVtoRGB(HSVPixel);
HSVPixel *getHSVArray(t_image);

/* processamento HSV */

/* processamento histograma */
typedef struct {
   int r, g, b;
} RGBHist;

typedef union {
    int *gray;
    RGBHist *rgb;
} u_histogram;

u_histogram getHistogram(t_image);
unsigned int *getHistogram2(t_image);
/* processamento histograma */

unsigned char *resizeBilinearGray(unsigned char  *pixels, int w, int h, int w2, int h2);

// -----------------------------------------------------------------------------
// PARTE 2 TRABALHO 2
// -----------------------------------------------------------------------------

typedef struct {
    int dim;
    double *k;
} t_kernel;

t_image transfLinear(t_image img, int f1, int f2, int g1, int g2);

t_image transfCDF(t_image img);

t_image casamentoHistograma(t_image img, t_image des);

t_image filtLinear(t_image img, t_kernel k);

#endif /* PDI_IMG_H_ */
