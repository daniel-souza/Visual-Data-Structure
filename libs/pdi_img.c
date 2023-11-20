/*
 * pdi_img.c
 *
 *  Created on: 03/09/2012
 *      Author: Daniel Souza
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pdi_img.h"

// -----------------------------------------------------------------------------
// UTILITARIOS
// -----------------------------------------------------------------------------
// COMUNS
t_image cloneImg(t_image img) {
	t_image clone;
	clone.depth = img.depth;
	clone.width = img.width;
	clone.height = img.height;
	clone.magicNumber[0] = img.magicNumber[0];
	clone.magicNumber[1] = img.magicNumber[1];
	clone.nChannels = img.nChannels;
	// se nChannels = 1 criara grayscale : se nChannels for igual a 3 criara (r,g,b)
	clone.data.gray = (unsigned char*)malloc((clone.width * clone.height) * (sizeof(unsigned char) * clone.nChannels));
	long unsigned int i;
	for(i = 0; i < clone.width * clone.height * (sizeof(unsigned char) * clone.nChannels); i++)
		clone.data.gray[i] = 0;

	return clone;
}

t_image createImg(c_color component, int width, int height) {
         
    t_image img;
         
    img.width = width;
    img.height = height;
    img.magicNumber[0] = 'P';
    switch(component) {
        case GRAYSCALE_8BPP: {
            img.depth = 255;
            img.nChannels = 1;
            img.magicNumber[1] = '5';
        } break;
        case RGB_8BPP: {
       img.depth = 255;
            img.nChannels = 3;
            img.magicNumber[1] = '6';
        } break;
        default: exit(1);
    }
    img.data.gray = (unsigned char*)malloc(img.width * img.height * (sizeof(unsigned char) * img.nChannels));
    long unsigned int i;
    for(i = 0; i < img.width * img.height * (sizeof(unsigned char) * img.nChannels); i++)
        img.data.gray[i] = 0;
    
    return img;
}

int pointInBounds(int px, int py, int width, int height) {
	return (px >= 0 && px < width) && (py >= 0 && py < height);
}

// ARQUIVO
unsigned char *openP6(FILE *fp, int width, int height) {
	unsigned char *data = (unsigned char*)malloc(width * height * (sizeof(unsigned char)*3)); // sizeof uchar * 3 canais (r,g,b)
	fread(data, 3 * width, height, fp);
	return data;
}

unsigned char *openP5(FILE *fp, int width, int height) {
	unsigned char *data = (unsigned char*)malloc(width * height * sizeof(unsigned char));
	fread(data, width, height, fp);
	return data;
}

t_image pdi_openImg(char *filename) {
	FILE *infp = fopen(filename, "rb");

	t_image image;

	int ch;
	//int step = 0;
	char buff[4];
	//ler magic number
    if (!fgets(buff, sizeof(buff), infp)) {
		perror("file.ppm");
        exit(1);
    } else {
		image.magicNumber[0] = buff[0];
		image.magicNumber[1] = buff[1];
	}

    ch = getc(infp);
    // ignorar comentarios. Em primeiro caso espacos em branco.
    while(ch == '#') { //(9): TAB ou 0x9hex
		while ((ch = getc(infp)) != '\n') ;
		ch = getc(infp);
	}

	ungetc(ch, infp); // coloca de volta o caractere que nao deveria ter lido

	// recuperar largura e comprimento
	fscanf(infp, "%d %d", &image.width, &image.height);

	// recuperar profundidade de cores RGB
	fscanf(infp, "%d", &image.depth);

    while (fgetc(infp) != '\n') { ; }
	// ler pixels
	// caso P6
	image.nChannels = 0;
	switch(image.magicNumber[1]) {

		// -----------------------------------------------------------------
		// Texto
		// -----------------------------------------------------------------
		// PBM
		//case '1': break;
		// PGM
		//case '2': break;
		// PPM
		//case '3': break;
		// -----------------------------------------------------------------
		//Binario
		// -----------------------------------------------------------------
		// PBM
		//case '4': break;
		// PGM
		case '5': {
			image.nChannels = 1;
			image.data.gray = openP5(infp, image.width, image.height);
			break;
		}
		// PPM
		case '6': {
			image.nChannels = 3;
			image.data.gray = openP6(infp, image.width, image.height);
			break;
		}
		default: exit(1);
	}
	fclose(infp);
	//image.data = (unsigned char*)malloc(image.width * image.height * (sizeof(unsigned char) * image.nChannels));
	//fread(image.data, image.nChannels * image.width, image.height, infp);
	return image;
}

void saveImg(t_image img, const char * name) {
     
     FILE *fp;
     //open file for output
     fp = fopen(name, "wb");
	 if (!fp) {
        fprintf(stderr, "Unable to open file");
	    exit(1);
     }
     //write the header file
     fprintf(fp, "P%c\n", img.magicNumber[1]);
     fprintf(fp, "# Created by daniel souza\n");
     fprintf(fp, "%d %d\n", img.width,img.height);
     fprintf(fp, "%d\n", img.depth);
     
    // pixel data
    fwrite(img.data.gray, img.nChannels * img.width, img.height, fp);
    fclose(fp);
}

// -----------------------------------------------------------------------------
// PROCESSAMENTO
// -----------------------------------------------------------------------------
// PROCESSAMENTO 2D
t_image rotateAndCrop(t_image img, double rad) {

	double origem_x, origem_y;
	int px, py, x, y;
	    
	t_image clone = cloneImg(img);
	
	//origem_x = (img.width-1)/2;
	//origem_y = (img.height-1)/2;                        centro
	//                                             0 1  2  3 4 (length: 5) 4: (5-1) ::> centro = (width-1)/2
    // w ou h = 5 (impar), centro = 3 e nao 2,5 :: x x (x) x x
    //                                            centro
    //w ou h = 4 (par), centro = 2,5 e nao 2 :: x x | x x
    // correcao de erro: (tamanho - 1)
	origem_x = (clone.width-1)/2;
	origem_y = (clone.height-1)/2;
    
    int x1, y1;
	for(x = 0; x < img.width; x++) {
		for(y = 0; y < img.height; y++) {
			
			x1 = x - origem_x;
			y1 = y - origem_y;
			
			px = (int)round((x1*cos(rad)) + (y1*sin(rad)));
			py = (int)round(-(x1*sin(rad)) + (y1*cos(rad)));
			
			px = px + (img.width-1)/2;
			py = py + (img.height-1)/2;
			//printf("p(%d,%d) o(%d,%d)\n", px, py, x, y);
			//getch();
			if(pointInBounds(px, py, img.width, img.height) == 1) {
                switch(clone.nChannels) {
					case 1: {
						clone.data.gray[x + (y * clone.width)] = img.data.gray[px + (py * img.width)];
					} break;
					case 3: {
						clone.data.rgb[x + (y * clone.width)].r = img.data.rgb[px + (py * clone.width)].r;
						clone.data.rgb[x + (y * clone.width)].g = img.data.rgb[px + (py * clone.width)].g;
						clone.data.rgb[x + (y * clone.width)].b = img.data.rgb[px + (py * clone.width)].b;
					} break;
				}                
            }
		}
	}
	return clone;
}

t_image rotate(t_image img, double rad) {

	double origem_x, origem_y;
	int px, py, x, y;

	//origem_x = (img.width-1)/2;
	//origem_y = (img.height-1)/2;                        centro
	//                                             0 1  2  3 4 (length: 5) 4: (5-1) ::> centro = (width-1)/2
    // w ou h = 5 (impar), centro = 3 e nao 2,5 :: x x (x) x x
    //                                            centro
    //w ou h = 4 (par), centro = 2,5 e nao 2 :: x x | x x
    // correcao de erro: (tamanho - 1)
	origem_x = (img.width-1)/2;
	origem_y = (img.height-1)/2;
	
	t_image new_image;
	
	int x1, y1;
    { // get length (x,y, rad)
        int xmin, ymin, xmax, ymax;
        int vetx[4], vety[4];

        x1 = 0 - origem_x; y1 = 0 - origem_y;
        vetx[0] = (int)round((x1*cos(rad)) + (y1*sin(rad)));
        vety[0] = (int)round(-(x1*sin(rad)) + (y1*cos(rad)));
        vetx[0] += origem_x; vety[0] += origem_y;
        x1 = 0 - origem_x; y1 = (img.height - 1) - origem_y;
        vetx[1] = (int)round((x1*cos(rad)) + (y1*sin(rad)));
        vety[1] = (int)round(-(x1*sin(rad)) + (y1*cos(rad)));
        vetx[1] += origem_x; vety[1] += origem_y; 
        x1 = (img.width - 1) - origem_x; y1 = 0 - origem_y;
        vetx[2] = (int)round((x1*cos(rad)) + (y1*sin(rad)));
        vety[2] = (int)round(-(x1*sin(rad)) + (y1*cos(rad)));
        vetx[2] += origem_x; vety[2] += origem_y; 
        x1 = (img.width - 1) - origem_x; y1 = (img.height - 1) - origem_y;
        vetx[3] = (int)round((x1*cos(rad)) + (y1*sin(rad)));
        vety[3] = (int)round(-(x1*sin(rad)) + (y1*cos(rad)));
        vetx[3] += origem_x; vety[3] += origem_y;
        xmin = MIN(vetx[0], MIN(vetx[1], MIN(vetx[2], vetx[3])));
        xmax = MAX(vetx[0], MAX(vetx[1], MAX(vetx[2], vetx[3])));
        
        ymin = MIN(vety[0], MIN(vety[1], MIN(vety[2], vety[3])));
        ymax = MAX(vety[0], MAX(vety[1], MAX(vety[2], vety[3])));
        
        
        int xlength, ylength;     //  1  2  3  4  5  6  7  8         max - min + (indice 0)
        xlength = xmax - xmin + 1;// -3 -2 -1  0  1  2  3  4  length: 4  -(-3) + 1 :: 8
        ylength = ymax - ymin + 1;//  |  |  |  |  |  |  |  |
        
        switch(img.nChannels) {
			case 1: {
				new_image = createImg(GRAYSCALE_8BPP, xlength, ylength);
			} break;
			
			//getch();
			case 3: {
			     new_image = createImg(RGB_8BPP, xlength, ylength);
			} break;
		}        
    }
    
	for(x = 0; x < new_image.width; x++) {
		for(y = 0; y < new_image.height; y++) {
			
			x1 = x - ((new_image.width-1)/2);
			y1 = y - ((new_image.width-1)/2);
			
			px = (int)round((x1*cos(rad)) + (y1*sin(rad)));
			py = (int)round(-(x1*sin(rad)) + (y1*cos(rad)));
			
			px = px + origem_x;
			py = py + origem_y;
			
			if(pointInBounds(px, py, img.width, img.height)) {
                switch(new_image.nChannels) {
					case 1: {
						//        [ponto px escala de cinza ,     ponto py    ]           [ ponto x escala de cinza , ponto y   ]
					//	new_image.data[(new_image.nChannels * px) + (py * new_image.width)] = img.data[(img.nChannels * x) + (y * img.width)];
					new_image.data.gray[x + (y * new_image.width)] = img.data.gray[px + (py * img.width)];
					} break;
					case 3: {
						//                  px            +                  py               + (r,g,b)                  x                              y             + (r,g,b)
					//	new_image.data[(px * new_image.nChannels) + (py * new_image.width * new_image.nChannels) + 0] = img.data[(x * img.nChannels) + (y * img.width * new_image.nChannels) + 0];
					//	new_image.data[(px * new_image.nChannels) + (py * new_image.width * new_image.nChannels) + 1] = img.data[(x * img.nChannels) + (y * img.width * new_image.nChannels) + 1];
					//	new_image.data[(px * new_image.nChannels) + (py * new_image.width * new_image.nChannels) + 2] = img.data[(x * img.nChannels) + (y * img.width * new_image.nChannels) + 2];
					new_image.data.rgb[x + (y * new_image.width)].r = img.data.rgb[px + (py * img.width)].r;
					new_image.data.rgb[x + (y * new_image.width)].g = img.data.rgb[px + (py * img.width)].g;
					new_image.data.rgb[x + (y * new_image.width)].b = img.data.rgb[px + (py * img.width)].b;
					} break;
				}
            }
			
		}
	}
	
	return new_image;
}

t_image scale(t_image img, double sx, double sy) {
        
    if(sx < 0) sx *= -1;
    if(sy < 0) sy *= -1;
    
    t_image img2;

    switch(img.nChannels) {
		case 1: {
			img2 = createImg(GRAYSCALE_8BPP, (int) round(img.width * sx), (int) round(img.height * sy));
		} break;
		
		//getch();
		case 3: {
		     img2 = createImg(RGB_8BPP, (int) round(img.width * sx), (int) round(img.height * sy));
		} break;
	}
				
   int x, y, px, py;
   for(x = 0; x < img2.width; x++) {
       for(y = 0; y < img2.height; y++) {
           
           if(sx == 0) px = 0;
           else px = (int)round(x * 1/sx);
           if(sy == 0) py = 0;
           else py = (int)round(y * 1/sy);
           
           if(pointInBounds(px, py, img.width, img.height)) {
               switch(img2.nChannels) {
    	        case 1: {
    					//        [ponto px escala de cinza ,     ponto py    ]           [ ponto x escala de cinza , ponto y   ]
    					img2.data.gray[x + (y * img2.width)] = img.data.gray[px + (py * img.width)];
    				} break;
    			case 3: {
    					//                  px            +                  py               + (r,g,b)                  x                              y             + (r,g,b)
    				//	img2.data[(px * img2.nChannels) + (py * img2.width * img2.nChannels) + 0] = img.data[(x * img.nChannels) + (y * img.width * img2.nChannels) + 0];
    				//	img2.data[(px * img2.nChannels) + (py * img2.width * img2.nChannels) + 1] = img.data[(x * img.nChannels) + (y * img.width * img2.nChannels) + 1];
    				//	img2.data[(px * img2.nChannels) + (py * img2.width * img2.nChannels) + 2] = img.data[(x * img.nChannels) + (y * img.width * img2.nChannels) + 2];
    				img2.data.rgb[x + (y * img2.width)].r = img.data.rgb[px + (py * img.width)].r;
    				img2.data.rgb[x + (y * img2.width)].g = img.data.rgb[px + (py * img.width)].g;
    				img2.data.rgb[x + (y * img2.width)].b = img.data.rgb[px + (py * img.width)].b;
    				} break;
    			}
           }
       }
   }
   return img2;
}

t_image interpolate(t_image img) {
    
    int x, y, count;       //                              1   2   1
    float media[3]; // media[3] convolucao: 1/16   pesos:  2   4   2
    for(x = 0; x < img.width; x++) { //                    1   2   1
        for(y = 0; y < img.height; y++) {
            media[0] = 0; media[1] = 0; media[2] = 0; count = 0;
            switch(img.nChannels) {
                case 1: { 
                    if(img.data.gray[x + y*img.width] == 0) {
                        // diagonais peso 1
                        if(pointInBounds(x-1,y-1, img.width, img.height)) {
                            if(img.data.gray[x-1 + ((y-1) * img.width)] > 0) {count++;
                            media[0] += img.data.gray[x-1 + ((y-1) * img.width)] * 1;}
                        }
                        if(pointInBounds(x+1,y-1, img.width, img.height)) {
                            if(img.data.gray[x+1 + ((y-1) * img.width)] > 0) {count++;
                            media[0] += img.data.gray[x+1 + ((y-1) * img.width)] * 1;}
                        }
                        if(pointInBounds(x-1,y+1, img.width, img.height)) {
                            if(img.data.gray[x-1 + ((y+1) * img.width)] > 0) {count++;
                            media[0] += img.data.gray[x-1 + ((y+1) * img.width)] * 1;}
                        }
                        if(pointInBounds(x+1,y+1, img.width, img.height)) {
                            if(img.data.gray[x+1 + ((y+1) * img.width)] > 0) {count++;
                            media[0] += img.data.gray[x+1 + ((y+1) * img.width)] * 1;}
                        }
                            
                        // horizontal vertical peso 2
                        if(pointInBounds(x,y-1, img.width, img.height)) {
                            if(img.data.gray[x + ((y-1) * img.width)] > 0) {count++;
                            media[0] += img.data.gray[x + ((y-1) * img.width)] * 1;}
                            //count++;
                        }
                        if(pointInBounds(x,y+1, img.width, img.height)) {
                            if(img.data.gray[x + ((y+1) * img.width)] > 0) {count++;
                            media[0] += img.data.gray[x + ((y+1) * img.width)] * 1;}
                        }
                        if(pointInBounds(x-1,y, img.width, img.height)) {
                           //if(img.data[x-1 + (y * img.width)] > 0) count++;
                            media[0] += img.data.gray[x-1 + (y * img.width)] * 1;
                            count++;
                        }
                        if(pointInBounds(x+1,y, img.width, img.height)) {
                            if(img.data.gray[x+1 + (y * img.width)] > 0) {count++;
                            media[0] += img.data.gray[x+1 + (y * img.width)] * 1;}
                            //count++;
                        }
                        
                        // centro peso 4
                        //if(pointInBounds(x+1,y, img.width, img.height)) {
                            //if(img.data[x+1 + (y * img.width)] > 0) count++;
                            //media[3] += img.data[x + (y * img.width)] * 0;
                            //count++;
                        //}
                        if (count > 1) img.data.gray[x + (y * img.width)] = (unsigned char) (media[0] / count);
                        
                    }
                } break;
                case 3: {
                    if(
                        (img.data.rgb[x + (y*img.width)].r == 0) &&
                        (img.data.rgb[x + (y*img.width)].g == 0) &&
                        (img.data.rgb[x + (y*img.width)].b == 0)
                    ) {
                        // diagonais peso 1
                        if(pointInBounds(x-1,y-1, img.width, img.height)) {
                            if(
                                (img.data.rgb[(x-1) + ((y-1) * img.width)].r > 0) ||
                                (img.data.rgb[(x-1) + ((y-1) * img.width)].g > 0) ||
                                (img.data.rgb[(x-1) + ((y-1) * img.width)].b > 0)
                            ) {
                                count++;
                                media[0] += img.data.rgb[(x-1) + ((y-1) * img.width)].r;
                                media[1] += img.data.rgb[(x-1) + ((y-1) * img.width)].g;
                                media[2] += img.data.rgb[(x-1) + ((y-1) * img.width)].b;
                                //media[3] += img.data[(x-1) + ((y-1) * img.width)] * 1;
                            }
                        }   
                        if(pointInBounds(x+1,y-1, img.width, img.height)) {
                            if(
                                (img.data.rgb[(x+1) + ((y-1) * img.width)].r > 0) ||
                                (img.data.rgb[(x+1) + ((y-1) * img.width)].g > 0) ||
                                (img.data.rgb[(x+1) + ((y-1) * img.width)].b > 0)
                            ) {
                                count++;
                                media[0] += img.data.rgb[(x+1) + ((y-1) * img.width)].r;
                                media[1] += img.data.rgb[(x+1) + ((y-1) * img.width)].g;
                                media[2] += img.data.rgb[(x+1) + ((y-1) * img.width)].b;
                            }
                        }
                        if(pointInBounds(x-1,y+1, img.width, img.height)) {
                            if(
                                (img.data.rgb[(x-1) + ((y+1) * img.width)].r > 0) ||
                                (img.data.rgb[(x-1) + ((y+1) * img.width)].g > 0) ||
                                (img.data.rgb[(x-1) + ((y+1) * img.width)].b > 0)
                            ) {
                                count++;
                                media[0] += img.data.rgb[(x-1) + ((y+1) * img.width)].r;
                                media[1] += img.data.rgb[(x-1) + ((y+1) * img.width)].g;
                                media[2] += img.data.rgb[(x-1) + ((y+1) * img.width)].b;
                            }
                        }
                        if(pointInBounds(x+1,y+1, img.width, img.height)) {
                            if(
                                (img.data.rgb[(x+1) + ((y+1) * img.width)].r > 0) ||
                                (img.data.rgb[(x+1) + ((y+1) * img.width)].g > 0) ||
                                (img.data.rgb[(x+1) + ((y+1) * img.width)].b > 0)
                            ) {
                                count++;
                                media[0] += img.data.rgb[(x+1) + ((y+1) * img.width)].r;
                                media[1] += img.data.rgb[(x+1) + ((y+1) * img.width)].g;
                                media[2] += img.data.rgb[(x+1) + ((y+1) * img.width)].b;
                            }
                        }
                            
                        // horizontal vertical peso 2
                        if(pointInBounds(x,y-1, img.width, img.height)) {
                            if(
                                (img.data.rgb[x + ((y-1) * img.width)].r > 0) ||
                                (img.data.rgb[x + ((y-1) * img.width)].g > 0) ||
                                (img.data.rgb[x + ((y-1) * img.width)].b > 0)
                            ) {
                                count++;
                                media[0] += img.data.rgb[x + ((y-1) * img.width)].r;
                                media[1] += img.data.rgb[x + ((y-1) * img.width)].g;
                                media[2] += img.data.rgb[x + ((y-1) * img.width)].b;
                            }
                        }
                        if(pointInBounds(x,y+1, img.width, img.height)) {
                            if(
                                (img.data.rgb[x + ((y+1) * img.width)].r > 0) ||
                                (img.data.rgb[x + ((y+1) * img.width)].g > 0) ||
                                (img.data.rgb[x + ((y+1) * img.width)].b > 0)
                            ) {
                                count++;
                                media[0] += img.data.rgb[x + ((y+1) * img.width)].r;
                                media[1] += img.data.rgb[x + ((y+1) * img.width)].g;
                                media[2] += img.data.rgb[x + ((y+1) * img.width)].b;
                            }
                        }
                        if(pointInBounds(x-1,y, img.width, img.height)) {
                            //if(
                            //    (img.data[((x-1) * img.nChannels) + (y * img.width * img.nChannels) + 0] > 0) ||
                            //    (img.data[((x-1) * img.nChannels) + (y * img.width * img.nChannels) + 1] > 0) ||
                            //    (img.data[((x-1) * img.nChannels) + (y * img.width * img.nChannels) + 2] > 0)
                            //) {
                                count++;
                                media[0] += img.data.rgb[(x-1) + (y * img.width)].r;
                                media[1] += img.data.rgb[(x-1) + (y * img.width)].g;
                                media[2] += img.data.rgb[(x-1) + (y * img.width)].b;
                            //}
                           //if(img.data[x-1 + (y * img.width)] > 0) count++;
                           // media[3] += img.data[x-1 + (y * img.width)] * 1;
                        }
                        if(pointInBounds(x+1,y, img.width, img.height)) {
                            if(
                                (img.data.rgb[(x+1) + (y * img.width)].r > 0) ||
                                (img.data.rgb[(x+1) + (y * img.width)].g > 0) ||
                                (img.data.rgb[(x+1) + (y * img.width)].b > 0)
                            ) {
                                count++;
                                media[0] += img.data.rgb[(x+1) + (y * img.width)].r;
                                media[1] += img.data.rgb[(x+1) + (y * img.width)].g;
                                media[2] += img.data.rgb[(x+1) + (y * img.width)].b;
                            }
                        }
                        
                        // centro peso 4
                        //if(pointInBounds(x+1,y, img.width, img.height)) {
                            //if(img.data[x+1 + (y * img.width)] > 0) count++;
                            //media[3] += img.data[x + (y * img.width)] * 0;
                            //count++;
                        //}
                        if (count > 1) {
                            img.data.rgb[x + (y * img.width)].r = (unsigned char) (media[0] / count);
                            img.data.rgb[x + (y * img.width)].g = (unsigned char) (media[1] / count);
                            img.data.rgb[x + (y * img.width)].b = (unsigned char) (media[2] / count);
                            //img.data[x + (y * img.width)] = (unsigned char) (media[3] / count);
                        }   
                    }
                } break;
            } // fim switch
        } //fim for y
    } // fim for x
    
    return img;
}

HSVPixel RGBtoHSV(RGBPixel colorRGB)
{ 
    float r, g, b, h, s, v;
    r = colorRGB.r / 256.0;
    g = colorRGB.g / 256.0;
    b = colorRGB.b / 256.0;
    float maxColor = MAX(r, MAX(g, b));
    float minColor = MIN(r, MIN(g, b));
    v = maxColor;
    //printf("v = max: %f\n", v);

    if(maxColor == 0)// prevenir divis�o por zero
    {  
        s = 0;
    }
    else  
    {      
        s = (maxColor - minColor) / maxColor;       
    }
    if(s == 0)
    {
        h = 0; // n�o importa seu valor
    }   
    else
    { 
        h = (60*(g - b)) / (maxColor-minColor);
        if(r == maxColor && g >= b) h += 0;
        if(r == maxColor && g < b) h += 360;
        if(g == maxColor) h = ((60*(b - r)) / (maxColor-minColor)) + 120;
        if(b == maxColor) h = ((60*(r - g)) / (maxColor-minColor)) + 240;
        
        /*
        if(r == maxColor) h = (g - b) / (maxColor-minColor);
        else if(g == maxColor) h = 2.0 + (b - r) / (maxColor - minColor);
        else h = 4.0 + (r - g) / (maxColor - minColor);      
        h /= 6.0; //to bring it to a number between 0 and 1
        if (h < 0) h++;
        */
    }
    HSVPixel colorHSV;
    colorHSV.h = (int)h;
    colorHSV.s = s;//(s * 255.0);
    colorHSV.v = v;//(v * 255.0);

    return colorHSV;
}


RGBPixel HSVtoRGB(HSVPixel colorHSV) {

	 float r, g, b, h, s, v; //this function works with floats between 0 and 1
	 h = (colorHSV.h / 360.0f);
	 s = colorHSV.s / 1.0f;
	 v = colorHSV.v / 1.0f;
	 //printf("h = (colorHSV.h / 360.0f): %f = (%d / 360.0f)", h,colorHSV.h);
	 //printf("\\#hsv: h: %f s: %f v: %f\n", h,s,v);

	 //If saturation is 0, the color is a shade of gray
	 if(s == 0) r = g = b = v;
	 //If saturation > 0, more complex calculations are needed
	 else
	 {
		 float f, p, q, t;
	 	 int i;
	 	 h *= 6; //to bring hue to a number between 0 and 6, better for the calculations
	 	 i = (int)floor(h);  //e.g. 2.7 becomes 2 and 3.01 becomes 3 or 4.9999 becomes 4
	 	 f = h - i;  //the fractional part of h
	 	 p = v * (1 - s);
	 	 q = v * (1 - (s * f));
	 	 t = v * (1 - (s * (1 - f)));
	 	 switch(i)
	 	 {
	 	     case 0: r = v; g = t; b = p; break;
	 	     case 1: r = q; g = v; b = p; break;
	 	     case 2: r = p; g = v; b = t; break;
	 	     case 3: r = p; g = q; b = v; break;
	 	     case 4: r = t; g = p; b = v; break;
	 	     case 5: r = v; g = p; b = q; break;
	 	 }
	 }

	 RGBPixel colorRGB;
	 // ceil arredonda acima.. por poder ocorrer perda de dados em fracoes
	 // ex: entrada h: 141      s: 0.824561  v: 0.222656
	 //     saida   r: 9.960937 g: 56.777344 b: 26.346686 -> ceil -> r: 10 g: 57 b: 27
	 colorRGB.r = (int)ceil(r * 255.0);
	 colorRGB.g = (int)ceil(g * 255.0);
	 colorRGB.b = (int)ceil(b * 255.0);

	 return colorRGB;
}

HSVPixel *getHSVArray(t_image img) {
    if(img.nChannels == 1) exit(0);
    HSVPixel *hsv = (HSVPixel *)malloc((img.width * img.height) * sizeof(HSVPixel));
    
    int i;
    for(i = 0; i < img.width * img.height; i++) hsv[i] = RGBtoHSV(img.data.rgb[i]);
    
    for(i = 0; i < 10; i++ ) printf("rgb[%d].r: %d rgb[%d].g: %d rgb[%d].b: %d\n", i, img.data.rgb[i].r, i, img.data.rgb[i].g, i, img.data.rgb[i].b);
    printf("\n");
    
    for(i = 0; i < 10; i++ ) printf("HSV[%d].v: %f\n", i, hsv[i].v);
    printf("\n");
    
    return hsv;
}

u_histogram getHistogram(t_image img) {
    u_histogram hist;
    hist.gray = (int *) malloc(img.depth * (sizeof(int) * img.nChannels));
    
    int x, y;
    for(x = 0; x < img.depth * img.nChannels; x++)
        hist.gray[x] = 0;
    
    for(x = 0; x < img.width; x++) {
		for(y = 0; y < img.height; y++) {
              
            switch(img.nChannels) {
				case 1: {
					hist.gray[img.data.gray[x + (y * img.width)]]++;
				} break;
				case 3: {
                    hist.rgb[img.data.rgb[x + (y * img.width)].r].r++;
                    hist.rgb[img.data.rgb[x + (y * img.width)].g].g++;
                    hist.rgb[img.data.rgb[x + (y * img.width)].b].b++;
				} break;
			}
		}
	}
    
    return hist;
}

unsigned int *getHistogram2(t_image img) {
    unsigned int *hist;
    int x, y;
    switch(img.nChannels) {
        case 1: {
            hist = (unsigned int *) malloc((img.depth + 1) * (sizeof(int) * img.nChannels));
            for(x = 0; x < img.depth + 1; x++) hist[x] = 0;
        } break;
        case 3: {
            hist = (unsigned int *) malloc(sizeof(int) * (64 + 1));
            for(x = 0; x < 64 + 1; x++) hist[x] = 0;
        } break;
    }
    
    RGBPixel rgbPix;
    for(x = 0; x < img.width; x++) {
		for(y = 0; y < img.height; y++) {
            switch(img.nChannels) {
				case 1: {
					hist[img.data.gray[x + (y * img.width)]]++;
				} break;
				case 3: {
                    
                    
                    rgbPix.r = img.data.rgb[x + (y * img.width)].r;
                    rgbPix.g = img.data.rgb[x + (y * img.width)].g;
                    rgbPix.b = img.data.rgb[x + (y * img.width)].b;
                    
                    hist [
                        (int)( // cast para int :: mesmo que floor
                           (rgbPix.r/64) +
                           (4 * (rgbPix.g/64)) +
                           (16 * (rgbPix.b/64))
                        )
                    ]++;
    				} break;
			}
		}
	}
    
    return hist;
}

// referencia http://www.ajdesigner.com/phpinterpolation/bilinear_interpolation_equation.php
// referencia linear: http://www.ajdesigner.com/phpinterpolation/linear_interpolation_equation.php

t_image scaleBilinear(t_image img, double sx, double sy) {
    
    if(sx < 0) sx *= -1;
    if(sy < 0) sy *= -1;
    
    t_image scale;

    switch(img.nChannels) {
		case 1: {
			scale = createImg(GRAYSCALE_8BPP, (int) round(img.width * sx), (int) round(img.height * sy));
		} break;
		
		//getch();
		case 3: {
		     scale = createImg(RGB_8BPP, (int) round(img.width * sx), (int) round(img.height * sy));
		} break;
	}
    
    int A, B, C, D, px, py, index;
    float x_ratio = ((sx == 0) ? 0 : ((float)(img.width-1)/scale.width));
    float y_ratio = ((sy == 0) ? 0 : ((float)(img.height-1)/scale.height));
    float x_diff, y_diff;
    
    int x,y;
    int gray; RGBPixel rgbPix;
    for (x=0; x < scale.width; x++) {
        for (y=0; y < scale.height; y++) {
            
            // floor
            px = (int)(x_ratio * x);
            py = (int)(y_ratio * y);
            x_diff = (x_ratio * x) - px;
            y_diff = (y_ratio * y) - py;
            index = px + (py * img.width);
            
            switch(img.nChannels) {
                case 1: {
                    A = img.data.gray[index] & 0xff;
                    B = img.data.gray[index+1] & 0xff;
                    C = img.data.gray[index+img.width] & 0xff;
                    D = img.data.gray[index+img.width+1] & 0xff;
                    
                    gray = (int)(
                    A*(1-x_diff)*(1-y_diff) +  B*(x_diff)*(1-y_diff) +
                    C*(y_diff)*(1-x_diff)   +  D*(x_diff*y_diff)
                    );
                    
                    scale.data.gray[x + (y * scale.width)] = gray;
                    
                } break;
                case 3: {             
                    A = img.data.rgb[index].r & 0xff;
                    B = img.data.rgb[index+1].r & 0xff;
                    C = img.data.rgb[index+img.width].r & 0xff;
                    D = img.data.rgb[index+img.width + 1].r & 0xff;
                    
                    rgbPix.r = (int)(
                    A*(1-x_diff)*(1-y_diff) +  B*(x_diff)*(1-y_diff) +
                    C*(y_diff)*(1-x_diff)   +  D*(x_diff*y_diff)
                    );
                    
                    A = img.data.rgb[index].g & 0xff;
                    B = img.data.rgb[index+1].g & 0xff;
                    C = img.data.rgb[index+img.width].g & 0xff;
                    D = img.data.rgb[index+img.width + 1].g & 0xff;
                    
                    rgbPix.g = (int)(
                    A*(1-x_diff)*(1-y_diff) +  B*(x_diff)*(1-y_diff) +
                    C*(y_diff)*(1-x_diff)   +  D*(x_diff*y_diff)
                    );
                    
                    A = img.data.rgb[index].b & 0xff;
                    B = img.data.rgb[index+1].b & 0xff;
                    C = img.data.rgb[index+img.width].b & 0xff;
                    D = img.data.rgb[index+img.width + 1].b & 0xff;
                    
                    rgbPix.b = (int)(
                    A*(1-x_diff)*(1-y_diff) +  B*(x_diff)*(1-y_diff) +
                    C*(y_diff)*(1-x_diff)   +  D*(x_diff*y_diff)
                    );
                    
                    scale.data.rgb[x + (y * scale.width)].r = rgbPix.r;
                    scale.data.rgb[x + (y * scale.width)].g = rgbPix.g;
                    scale.data.rgb[x + (y * scale.width)].b = rgbPix.b;
                } break;
            }
                          
        }
    }
    return scale;
}


t_image interpolateBilinear(t_image img) {
    
    int A, B, C, D, px, py, index;
    float x_ratio = (float)(img.width-1)/img.width;
    float y_ratio = (float)(img.height-1)/img.height;
    float x_diff, y_diff;
    
    int x,y;
    int gray; RGBPixel rgbPix;
    
    t_image clone = cloneImg(img);
    
    for (x=0;x<img.width;x++) {
        for (y=0;y<img.height;y++) {
            
            px = (int)(x_ratio * x);
            py = (int)(y_ratio * y);
            x_diff = (x_ratio * x) - px;
            y_diff = (y_ratio * y) - py;
            index = px + (py * img.width);
            
            switch(img.nChannels) {
                case 1: {
                    A = img.data.gray[index] & 0xff;
                    B = img.data.gray[index+1] & 0xff;
                    C = img.data.gray[index+img.width] & 0xff;
                    D = img.data.gray[index+img.width+1] & 0xff;
                    
                    gray = (int)(
                    A*(1-x_diff)*(1-y_diff) +  B*(x_diff)*(1-y_diff) +
                    C*(y_diff)*(1-x_diff)   +  D*(x_diff*y_diff)
                    );
                    
                    clone.data.gray[x + (y * img.width)] = gray;
                    //img.data.gray[x + (y * img.width)] = gray;
                    
                } break;
                case 3: {             
                    A = img.data.rgb[index].r & 0xff;
                    B = img.data.rgb[index+1].r & 0xff;
                    C = img.data.rgb[index+img.width].r & 0xff;
                    D = img.data.rgb[index+img.width + 1].r & 0xff;
                    
                    rgbPix.r = (int)(
                    A*(1-x_diff)*(1-y_diff) +  B*(x_diff)*(1-y_diff) +
                    C*(y_diff)*(1-x_diff)   +  D*(x_diff*y_diff)
                    );
                    
                    A = img.data.rgb[index].g & 0xff;
                    B = img.data.rgb[index+1].g & 0xff;
                    C = img.data.rgb[index+img.width].g & 0xff;
                    D = img.data.rgb[index+img.width + 1].g & 0xff;
                    
                    rgbPix.g = (int)(
                    A*(1-x_diff)*(1-y_diff) +  B*(x_diff)*(1-y_diff) +
                    C*(y_diff)*(1-x_diff)   +  D*(x_diff*y_diff)
                    );
                    
                    A = img.data.rgb[index].b & 0xff;
                    B = img.data.rgb[index+1].b & 0xff;
                    C = img.data.rgb[index+img.width].b & 0xff;
                    D = img.data.rgb[index+img.width + 1].b & 0xff;
                    
                    rgbPix.b = (int)(
                    A*(1-x_diff)*(1-y_diff) +  B*(x_diff)*(1-y_diff) +
                    C*(y_diff)*(1-x_diff)   +  D*(x_diff*y_diff)
                    );
                    
                    clone.data.rgb[x + (y * img.width)].r = rgbPix.r;
                    clone.data.rgb[x + (y * img.width)].g = rgbPix.g;
                    clone.data.rgb[x + (y * img.width)].b = rgbPix.b;
                    
                    //img.data.rgb[x + (y * img.width)].r = rgbPix.r;
                    //img.data.rgb[x + (y * img.width)].g = rgbPix.g;
                    //img.data.rgb[x + (y * img.width)].b = rgbPix.b;
                } break;
            }
                          
        }
    }
    return clone;
    //return img;
}


/*
Aspect Ratios for Image Rotator

To ensure the best results when adding your own images to the /custom/rotator folder, make sure they fit into one of the following aspect ratios:

    1:1 from 0.9950 to 1.0050 ratio_x (width -1) / width = 0.99511719
    1:2 from 0.4980 to 0.5020
    2:1 from 1.9810 to 2.0190
    2:3 from 0.6587 to 0.6746
    3:2 from 1.4737 to 1.5273
    3:4 from 0.7412 to 0.7590
    4:3 from 1.3125 to 1.3548
    4:5 from 0.7925 to 0.8077
    5:4 from 1.2353 to 1.2651
    9:16 from 0.5558 to 0.5693
    16:9 from 1.7406 to 1.8166
*/
t_image rotateBilinear(t_image img,  double rad) {
    double origem_x, origem_y;
	int x, y;

	//origem_x = (img.width-1)/2;
	//origem_y = (img.height-1)/2;                        centro
	//                                             0 1  2  3 4 (length: 5) 4: (5-1) ::> centro = (width-1)/2
    // w ou h = 5 (impar), centro = 3 e nao 2,5 :: x x (x) x x
    //                                            centro
    //w ou h = 4 (par), centro = 2,5 e nao 2 :: x x | x x
    // correcao de erro: (tamanho - 1)
	origem_x = (img.width-1)/2;
	origem_y = (img.height-1)/2;
	
	t_image new_image;
	
	int x1, y1;
    { // get length (x,y, rad)
        int xmin, ymin, xmax, ymax;
        int vetx[4], vety[4];

        x1 = 0 - origem_x; y1 = 0 - origem_y;
        vetx[0] = (int)round((x1*cos(rad)) + (y1*sin(rad)));
        vety[0] = (int)round(-(x1*sin(rad)) + (y1*cos(rad)));
        vetx[0] += origem_x; vety[0] += origem_y;
        x1 = 0 - origem_x; y1 = (img.height - 1) - origem_y;
        vetx[1] = (int)round((x1*cos(rad)) + (y1*sin(rad)));
        vety[1] = (int)round(-(x1*sin(rad)) + (y1*cos(rad)));
        vetx[1] += origem_x; vety[1] += origem_y; 
        x1 = (img.width - 1) - origem_x; y1 = 0 - origem_y;
        vetx[2] = (int)round((x1*cos(rad)) + (y1*sin(rad)));
        vety[2] = (int)round(-(x1*sin(rad)) + (y1*cos(rad)));
        vetx[2] += origem_x; vety[2] += origem_y; 
        x1 = (img.width - 1) - origem_x; y1 = (img.height - 1) - origem_y;
        vetx[3] = (int)round((x1*cos(rad)) + (y1*sin(rad)));
        vety[3] = (int)round(-(x1*sin(rad)) + (y1*cos(rad)));
        vetx[3] += origem_x; vety[3] += origem_y;
        xmin = MIN(vetx[0], MIN(vetx[1], MIN(vetx[2], vetx[3])));
        xmax = MAX(vetx[0], MAX(vetx[1], MAX(vetx[2], vetx[3])));
        
        ymin = MIN(vety[0], MIN(vety[1], MIN(vety[2], vety[3])));
        ymax = MAX(vety[0], MAX(vety[1], MAX(vety[2], vety[3])));
        
        
        int xlength, ylength;     //  1  2  3  4  5  6  7  8         max - min + (indice 0)
        xlength = xmax - xmin + 1;// -3 -2 -1  0  1  2  3  4  length: 4  -(-3) + 1 :: 8
        ylength = ymax - ymin + 1;//  |  |  |  |  |  |  |  |

        switch(img.nChannels) {
			case 1: {
				new_image = createImg(GRAYSCALE_8BPP, xlength, ylength);
			} break;
			
			//getch();
			case 3: {
			     new_image = createImg(RGB_8BPP, xlength, ylength);
			} break;
		}        
    }
    
    int A, B, C, D, index;
    //float x_ratio = (float)(img.width-1)/img.width;
    //float y_ratio = (float)(img.height-1)/img.height;
    float x_diff, y_diff, dx, dy;
    
    int gray; RGBPixel rgbPix; float px, py;
	for(x = 0; x < new_image.width; x++) {
		for(y = 0; y < new_image.height; y++) {
			
			
			x1 = x - ((new_image.width-1)/2);
			y1 = y - ((new_image.width-1)/2);
			
			px = ((x1*cos(rad)) + (y1*sin(rad)));
			py = (-(x1*sin(rad)) + (y1*cos(rad)));
			
			px = px + origem_x;
			py = py + origem_y;	
			
            dx = (int)(px);
			dy = (int)(py);
            x_diff = px - dx;
            y_diff = py - dy;
            index = dx + (dy * img.width);
			
			if(pointInBounds(px, py, img.width, img.height)) {
                switch(new_image.nChannels) {
					case 1: {
                        A = img.data.gray[index] & 0xff;
                        B = img.data.gray[index+1] & 0xff;
                        C = img.data.gray[index+img.width] & 0xff;
                        D = img.data.gray[index+img.width+1] & 0xff;
                        //A = img.data.gray[(int)(floor(px) + (floor(py) * img.width))] & 0xff;
                        //B = img.data.gray[(int)(ceil(px)  + (floor(py) * img.width))] & 0xff;
                        //C = img.data.gray[(int)(floor(px) + (ceil(py) * img.width)) ] & 0xff;
                        //D = img.data.gray[(int)(ceil(px)  + (ceil(py) * img.width)) ] & 0xff;
                        
                        gray = (int)(
                            A*(1-x_diff)*(1-y_diff) +  B*(x_diff)*(1-y_diff) +
                            C*(y_diff)*(1-x_diff)   +  D*(x_diff*y_diff)
                        );
                        
                        //img.data.gray[x + (y * img.width)] = gray;
						//        [ponto px escala de cinza ,     ponto py    ]           [ ponto x escala de cinza , ponto y   ]
					    //	new_image.data[(new_image.nChannels * px) + (py * new_image.width)] = img.data[(img.nChannels * x) + (y * img.width)];
					    new_image.data.gray[x + (y * new_image.width)] = gray;
					} break;
					case 3: {
    					A = img.data.rgb[index].r & 0xff;
                        B = img.data.rgb[index+1].r & 0xff;
                        C = img.data.rgb[index+img.width].r & 0xff;
                        D = img.data.rgb[index+img.width+1].r & 0xff;
                        
                        rgbPix.r = (int)(
                        A*(1-x_diff)*(1-y_diff) +  B*(x_diff)*(1-y_diff) +
                        C*(y_diff)*(1-x_diff)   +  D*(x_diff*y_diff)
                        );
                        
                        A = img.data.rgb[index].g & 0xff;
                        B = img.data.rgb[index+1].g & 0xff;
                        C = img.data.rgb[index+img.width].g & 0xff;
                        D = img.data.rgb[index+img.width+1].g & 0xff;
                        
                        rgbPix.g = (int)(
                        A*(1-x_diff)*(1-y_diff) +  B*(x_diff)*(1-y_diff) +
                        C*(y_diff)*(1-x_diff)   +  D*(x_diff*y_diff)
                        );
                        
                        A = img.data.rgb[index].b & 0xff;
                        B = img.data.rgb[index+1].b & 0xff;
                        C = img.data.rgb[index+img.width].b & 0xff;
                        D = img.data.rgb[index+img.width+1].b & 0xff;
                        
                        rgbPix.b = (int)(
                        A*(1-x_diff)*(1-y_diff) +  B*(x_diff)*(1-y_diff) +
                        C*(y_diff)*(1-x_diff)   +  D*(x_diff*y_diff)
                        );
                        
                        new_image.data.rgb[x + (y * new_image.width)].r = rgbPix.r;
                        new_image.data.rgb[x + (y * new_image.width)].g = rgbPix.g;
                        new_image.data.rgb[x + (y * new_image.width)].b = rgbPix.b;
					} break;
				}
            }
			
		}
	}
	
	return new_image;
}

// -----------------------------------------------------------------------------
// PARTE 1 TRABALHO 2
// -----------------------------------------------------------------------------

t_image transfLinear(t_image img, int f1, int f2, int g1, int g2) {
    
    int x,y; HSVPixel hsvPix;
    for(x = 0; x < img.width; x++)
        for(y = 0; y < img.height; y++) {
            switch(img.nChannels) {
                case 1: {
                    if(img.data.gray[x + (y * img.width)] < f1) img.data.gray[x + (y * img.width)] = g1;
                    else if(f1 <= img.data.gray[x + (y * img.width)] && img.data.gray[x + (y * img.width)] < f2) {
                        img.data.gray[x + (y * img.width)] = (int)(((g2 - g1)/(float)(f2 - f1)) * (img.data.gray[x + (y * img.width)] - f1)) + g1;
                    }
                    else if(img.data.gray[x + (y * img.width)] >= f2) img.data.gray[x + (y * img.width)] = g2;
                } break;
                case 3: {
                    //printf("r: %d g: %d b: %d\n", img.data.rgb[x + (y * img.width)].r, img.data.rgb[x + (y * img.width)].g, img.data.rgb[x + (y * img.width)].b);
                    hsvPix = RGBtoHSV(img.data.rgb[x + (y * img.width)]);
                    hsvPix.v = round(hsvPix.v * 255.0f);
                    //printf("h: %d s: %f v: %f\n", hsvPix.h,hsvPix.s,hsvPix.v);
                    //printf("r: %d g: %d b: %d\n", rgbp.r, rgbp.g, rgbp.b);
                    
                    // v de 0 a 1
                    if(hsvPix.v < f1) hsvPix.v = g1;
                    else if(f1 <= hsvPix.v && hsvPix.v < f2) {
                        hsvPix.v = (((g2 - g1)/(float)(f2 - f1)) * (hsvPix.v - f1)) + g1;
                    }
                    else if(hsvPix.v >= f2) hsvPix.v = g2;
                    
                    hsvPix.v = hsvPix.v / 255.0f;
                    
                    img.data.rgb[x + (y * img.width)] = HSVtoRGB(hsvPix);
                    //getch();
                    
                } break;
            }
        }
    
    return img;
}
/*
 Cumulative Distribution Function (CDF)
*/
unsigned int *getCDF(unsigned int in[], unsigned int depth) {
    
    unsigned int *out = malloc(depth * sizeof(unsigned int));
    unsigned int i;    
    int sum = out[0] = in[0];
    for(i = 1; i <= depth; i++) {
        sum += in[i];
        out[i] = sum;
    }
    return out;
}

unsigned int *getInverseCDF(unsigned int in[], unsigned int depth, int nPixels) {
    
    int diff, min, i, j;
    for(i = 0; i < 256; i++) {
        diff = abs(i - (int)(depth*in[0]/(float)nPixels));
	    min = in[0];
	    for(j = 0; j < 256; j++) {
            if(abs(i - ((int)((in[j] - in[0])/(float)(nPixels - in[0])*depth))) < diff) {
                diff = abs(i - ((int)((in[j] - in[0])/(float)((nPixels) - in[0])*depth)));
                min = j;
            }
        }
        in[i] = min;
    }
    return in;
}

/*
 Cumulative Distribution Function (CDF) Transformation
 im(v) = round((cdf(img[v]) -cdfmin)/((M x N) - cdfmin) * (depth_color))
 cdfmin = cdf[0]
*/
t_image transfCDF(t_image img) {
    unsigned int *hist = getHistogram2(img);
    unsigned int *imgCdf;
    
    t_image out = cloneImg(img);
    int i;
    
    switch(img.nChannels) {
        case 1: {
            imgCdf = getCDF(hist, 255);
            for(i = 0; i < img.width * img.height; i++)
                out.data.gray[i] = round( (imgCdf[img.data.gray[i]] - imgCdf[0]) /(float)((img.width * img.height) - imgCdf[0]) * img.depth );
        } break;
        case 3: {
            
            unsigned int rgbHist[256];
            HSVPixel *hsv2 = (HSVPixel *)malloc(img.width * img.height * sizeof(HSVPixel));
            for(i = 0; i <= img.depth; i++) rgbHist[i] = 0;
            for(i = 0; i < img.width * img.height; i++) {
                hsv2[i] = RGBtoHSV(img.data.rgb[i]);
                hsv2[i].v *= 255;
                rgbHist[(int)round(hsv2[i].v)]++;
            }
            
            imgCdf = getCDF(rgbHist, 255);
            HSVPixel hsv;
            
            for(i = 0; i < img.width * img.height; i++) {
                hsv = hsv2[i];
                hsv.v = round( (imgCdf[(int)hsv.v] - imgCdf[0]) / (float)((img.width * img.height) - imgCdf[0]) * 255);
                hsv.v /= 255;
                out.data.rgb[i] = HSVtoRGB(hsv);
            }
            free(hsv2);
        } break;
    }
    free(hist);
    hist = getHistogram2(out);    
    //getch();
    return out;
    
}

/*    
    g = Td^-1(r) = Td^-1[Tf(r)]
*/
t_image casamentoHistograma(t_image img, t_image des) {
    
    //if(img.nChannels != des.nChannels) {fprintf(stderr, "Imagens com canais diferentes!\n"); exit(1);}
    
    unsigned int *desHist; // PDF discreto de img e des
    unsigned int *desCdf;   // CDF de des
    // int *casHist;           // funcao de remapeamento de pixel casamento de histograma
    HSVPixel *HSVimg = NULL, *HSVdes = NULL;
    t_image g = cloneImg(img);
    int i;
    
    g = transfCDF(img); // transforma��o (equaliza��o) acumulada da imagem original
    if(g.nChannels == 3) HSVimg = getHSVArray(g);
    if(des.nChannels == 3) HSVdes = getHSVArray(des);
    if(HSVdes != NULL) {
        desHist = (unsigned int *) malloc((img.depth + 1) * sizeof(int));
        for(i = 0; i <= img.depth; i++) desHist[i] = 0;
        for(i = 0; i <= des.width * des.height; i++) {
            HSVdes[i].v *= img.depth;
            desHist[(int)round(HSVdes[i].v)]++;
        }
    } else { // escala de cinza
        desHist = getHistogram2(des);
    }
    // mapeamento
    desCdf = getCDF(desHist, img.depth);
    // remapeamento :: inverso colorido ou escala de cinza
    desCdf = getInverseCDF(desCdf, img.depth, img.width * img.height); // inversa normalizada e equalizada
    for(i = 0; i < img.width * img.height; i++) {
        if(HSVimg != NULL) {
            HSVimg[i].v = desCdf[(int)round(HSVimg[i].v * img.depth)]; // v normalizado de 0 � img.H
            HSVimg[i].v /= img.depth; // normalizado novamente de 0 � 1 para convers�o HSVtoRGB()
            g.data.rgb[i] = HSVtoRGB(HSVimg[i]);
        } else { // escala de cinza
            g.data.gray[i] = desCdf[g.data.gray[i]];
        }
    }
    
    return g;
    
}

t_image filtLinear(t_image img, t_kernel k) {
    if((k.dim % 2) == 0) { printf("invalid Kernel\n"); return img; }
    t_image out = cloneImg(img);
    double value[3];
    
    int i,j,x,y, kcenter = (k.dim-1)/2, px, py;
    for(i = 0; i < img.width; i++) {
        for(j = 0; j < img.height; j++) {
            
            // seta a somat�ria para zero
            value[0] = value[1] = value[2] = 0;
            
            for(x = k.dim -1; x >= 0; x--) {
                
                for(y = k.dim -1; y >= 0; y--) {
                    /*
                      para i, j
                      centro_kernel = distancia do ponto K(0,0) para K(kcenter,kcenter)
                      supondo kernel 3x3
                      xy(0,0) : ij(i-1, j-1) || xy(0,1) : ij(i-1, j) || xy(0,2) : ij(i-1, j+1)
                      xy(1,0) : ij(i, j-1)   ||  xy(1,1) : ij(i, j)  ||  xy(1,2) : ij(i, j+1)
                      xy(2,0) : ij(i+1, j-1) || xy(1,1) : ij(i+1, j) || xy(1,2) : ij(i+1, j+1)
                      
                      apos isso, eh verificado se o ponto esta dentro da imagem
                      em:
                           px = i - kcenter + x
                           py = j - kcenter + y
                    */
                    // inverso
                    px = i + kcenter - x;
                    py = j + kcenter - y;
                    
                    if(pointInBounds(px, py, img.width, img.height)) {
                        py *= img.width;
                        if(out.nChannels == 3) {
                            value[0] += img.data.rgb[px + py].r * k.k[x + (y * k.dim)];
                            value[1] += img.data.rgb[px + py].g * k.k[x + (y * k.dim)];
                            value[2] += img.data.rgb[px + py].b * k.k[x + (y * k.dim)];
                        } else {
                            value[0] += img.data.gray[px + py] * k.k[x + (y * k.dim)];
                        }
                    }
                    
                }
            }
            
            if(out.nChannels == 3) {
                if((int)round(value[0]) < 0) out.data.rgb[i + (j * img.width)].r = 0;
                else if((int)round(value[0]) > 255) out.data.rgb[i + (j * img.width)].r = 255;
                else out.data.rgb[i + (j * img.width)].r = (int)round(value[0]);
                if((int)round(value[0]) < 0) out.data.rgb[i + (j * img.width)].g = 0;
                else if((int)round(value[0]) > 255) out.data.rgb[i + (j * img.width)].g = 255;
                else out.data.rgb[i + (j * img.width)].g = (int)round(value[1]);
                if((int)round(value[0]) < 0) out.data.rgb[i + (j * img.width)].b = 0;
                else if((int)round(value[0]) > 255) out.data.rgb[i + (j * img.width)].b = 255;
                else out.data.rgb[i + (j * img.width)].b = (int)round(value[2]);
            } else {
                if((int)round(value[0]) < 0) out.data.gray[i + (j * img.width)] = 0;
                else if((int)round(value[0]) > 255) out.data.gray[i + (j * img.width)] = 255;
                else out.data.gray[i + (j * img.width)] = (int)round(value[0]);
            }

        }
    }
    return out;
}
