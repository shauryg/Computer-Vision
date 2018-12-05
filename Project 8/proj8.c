#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define ROWS 128
#define COLS 128
#define DISTANCE 3 //Pixels

#define SQR(x) ((x)*(x))

void get_3D_points(unsigned char *, double **);

void calc_surface_norm(double **, double **, int, int, int);



int main(void){

  FILE *fpt;
  int	 ROWS1,COLS1,BYTES1;
  char header[320];
  unsigned char *image, *binary, *threshold;
  double **P, **S, *D;


	int i;
	/*--------------------OPEN AND READ INPUT IMAGE---------------------*/
	if ((fpt=fopen("chair-range.ppm","rb")) == NULL){
		  printf("Unable to open chair-range.ppm for reading\n");
		  exit(0);
	}

    fscanf(fpt,"%s %d %d %d\n",header,&COLS1,&ROWS1,&BYTES1);
	if (strcmp(header,"P5") != 0  ||  BYTES1 != 255){
		  printf("Not a greyscale 8-bit PPM image\n");
		  exit(0);
	}

  image = (unsigned char *)calloc(ROWS1*COLS1,sizeof(unsigned char));
  header[0]=fgetc(fpt);	/* read white-space character that separates header */
  fread(image,1,COLS1*ROWS1,fpt);
  fclose(fpt);

	/*--------------------THRESHOLD BASED ON PIXEL---------------------*/

	binary = (unsigned char *)calloc(ROWS1*COLS1,sizeof(unsigned char));

	for(i = 0; i < (ROWS1*COLS1); i++){
		if(image[i] < 140)
        	binary[i] = 255;
    }

	/*--------------GET THE 3D COORDINATES OF THE RANGE IMAGE--------------*/
 
  //P[] = X, Y, Z
  P = (double **)calloc(3, sizeof(double *));
  for(i = 0; i < 3; i++)
    *(P + i) = (double *)calloc(ROWS1 * COLS1, sizeof(double));
    
	get_3D_points(image, P);

	for(i = 0; i < 1; i++){
		printf("\n3d coordinates pixel %d : %lf %lf %lf\n", i, P[0][i], P[1][i], P[2][i]);
	}

	/*----------------------CALCULATE SURFACE NORMALS---------------------*/
 
  S = (double **)calloc(3, sizeof(double *));
  for(i = 0; i < 3; i++)
    *(S + i) = (double *)calloc(ROWS1 * COLS1, sizeof(double));

  calc_surface_norm(S, P, ROWS1, COLS1, DISTANCE);

  for(i = 0; i < 1; i++){
		printf("\nSurface Normals pixel %d : %lf %lf %lf\n", i, S[0][i], S[1][i], S[2][i]);
	}
 
  /*-----------------------CALCULATE DISTANCE -------------------------*/
  
  D = (double *)calloc(ROWS1*COLS1, sizeof(double));
  
  for(i = 0; i < ROWS1 * COLS1; i++){
    D[i] = sqrt(SQR(S[0][i]) + SQR(S[1][i]) + SQR(S[2][i]));   
  }
  
  for(i = 0; i < 1; i++){
		printf("\nDistances %d : %lf\n", i, D[i]);
	}
 
  /*-------------------------------------------------------------------*/
  
  
  
  

 
	return 0;
}

void calc_surface_norm(double **S, double **P, int row, int col, int distance){

  int r, c, i;
  double A[3], B[3];
  int x = 0, y = 1, z = 2;
  
  for(r = 0; r < row - distance; r++){
    for(c = 0; c < col - distance; c++){
      for(i = 0; i < 3; i++){
        A[i] = P[i][(r + distance)*col + (c)] - P[i][(r)*col + (c)];
        B[i] = P[i][(r)*col + (c + distance)] - P[i][(r)*col + (c)];
      }
      S[0][r*col + c] = A[y]*B[z] - A[z]*B[y];
      S[1][r*col + c] = A[z]*B[x] - A[x]*B[z];
      S[2][r*col + c] = A[x]*B[y] - A[y]*B[x];
    }
  } 
} 

void get_3D_points(unsigned char *image, double **P){

  int	r,c;
  double cp[7];
  double xangle,yangle,dist;
  double ScanDirectionFlag,SlantCorrection;
  unsigned char RangeImage[ROWS*COLS];
  int ImageTypeFlag;

  ImageTypeFlag = 0;
  
  cp[0]=1220.7;		/* horizontal mirror angular velocity in rpm */
  cp[1]=32.0;		/* scan time per single pixel in microseconds */
  cp[2]=(COLS/2)-0.5;		/* middle value of columns */
  cp[3]=1220.7/192.0;	/* vertical mirror angular velocity in rpm */
  cp[4]=6.14;		/* scan time (with retrace) per line in milliseconds */
  cp[5]=(ROWS/2)-0.5;		/* middle value of rows */
  cp[6]=10.0;		/* standoff distance in range units (3.66cm per r.u.) */
  
  cp[0]=cp[0]*3.1415927/30.0;	/* convert rpm to rad/sec */
  cp[3]=cp[3]*3.1415927/30.0;	/* convert rpm to rad/sec */
  cp[0]=2.0*cp[0];		/* beam ang. vel. is twice mirror ang. vel. */
  cp[3]=2.0*cp[3];		/* beam ang. vel. is twice mirror ang. vel. */
  cp[1]/=1000000.0;		/* units are microseconds : 10^-6 */
  cp[4]/=1000.0;			/* units are milliseconds : 10^-3 */
  
  switch(ImageTypeFlag){
    case 1:		/* Odetics image -- scan direction upward */
      ScanDirectionFlag=-1;
      break;
    case 0:		/* Odetics image -- scan direction downward */
      ScanDirectionFlag=1;
      break;
    default:		/* in case we want to do this on synthetic model */
      ScanDirectionFlag=0;
      break;
  }
  
  if (ImageTypeFlag != 3){
  	for (r=0; r<ROWS; r++){
    	for (c=0; c<COLS; c++){
  			SlantCorrection = cp[3]*cp[1]*((double)c-cp[2]);
  			xangle = cp[0]*cp[1]*((double)c-cp[2]);
  			yangle = (cp[3]*cp[4]*(cp[5]-(double)r)) + SlantCorrection*ScanDirectionFlag;	
  			dist = (double)image[r*COLS+c]+cp[6];
         
        P[2][r*COLS + c] = sqrt((dist*dist)/(1.0+(tan(xangle)*tan(xangle)) + (tan(yangle)*tan(yangle))));
  			P[0][r*COLS + c] = tan(xangle)*P[2][r*COLS + c];
  			P[1][r*COLS + c] = tan(yangle)*P[2][r*COLS + c];
      }
  	}
  }
}



