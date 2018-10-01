#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define EDGE 255
#define NON_EDGE 0


int getE2NETransitions(int, int, int, unsigned char *);
int getENeighbours(int, int, int, unsigned char *);
int CheckNEWSpixels(int , int , int, unsigned char *);
int CheckErase(int, int, int);
void threshold(int, unsigned char *, unsigned char *, int, int);

int getE2NETransitions(int r, int c, int COLS1, unsigned char *binary){

	int transitions = 0;

	transitions += (binary[(r-1)*COLS1 + c] == EDGE && binary[(r-1)*COLS1+(c+1)] == NON_EDGE);
	transitions += (binary[(r-1)*COLS1 + (c+1)] == EDGE && binary[(r)*COLS1 + (c+1)] == NON_EDGE);
	transitions += (binary[(r)*COLS1 + (c+1)] == EDGE && binary[(r+1)*COLS1 + (c+1)] == NON_EDGE);
	transitions += (binary[(r+1)*COLS1 + (c+1)] == EDGE && binary[(r+1)*COLS1 + c] == NON_EDGE);
	transitions += (binary[(r+1)*COLS1 + c] == EDGE && binary[(r+1)*COLS1 + (c-1)] == NON_EDGE);
	transitions += (binary[(r+1)*COLS1 + (c-1)] == EDGE && binary[(r)*COLS1 + (c-1)] == NON_EDGE);
	transitions += (binary[(r)*COLS1 + (c-1)] == EDGE && binary[(r-1)*COLS1 + (c-1)] == NON_EDGE);
	transitions += (binary[(r-1)*COLS1 + (c-1)] == EDGE && binary[(r-1)*COLS1 + (c)] == NON_EDGE);

	return transitions;
}

int getENeighbours(int r, int c, int COLS1, unsigned char *binary){

	int neighbours = 0;
 
	neighbours += (binary[(r-1)*COLS1 + c] == EDGE);
	neighbours += (binary[(r-1)*COLS1 + (c+1)] == EDGE);
	neighbours += (binary[(r)*COLS1 + (c+1)] == EDGE);
	neighbours += (binary[(r+1)*COLS1 + (c+1)] == EDGE);
	neighbours += (binary[(r+1)*COLS1 + c] == EDGE);
	neighbours += (binary[(r+1)*COLS1 + (c-1)] == EDGE);
	neighbours += (binary[(r)*COLS1 + (c-1)] == EDGE);
	neighbours += (binary[(r-1)*COLS1 + (c-1)] == EDGE);


	return neighbours;
}

int CheckNEWSpixels(int r, int c, int COLS1, unsigned char *binary){
  
  int A, B, C, D;
  A = binary[(r-1)*COLS1 + (c)];
  B = binary[(r)*COLS1 + (c+1)];
  C = binary[(r)*COLS1 + (c-1)];
  D = binary[(r+1)*COLS1 + (c)];
   
	return (A == NON_EDGE || B == NON_EDGE || (C == NON_EDGE && D == NON_EDGE));
}

int CheckErase(transitions, neighbours, edge_test){
  return (transitions == 1 && neighbours >= 3 && neighbours <= 7 && edge_test == 1);
}

void threshold(int tval, unsigned char* msf, unsigned char *binary_msf, int ROWS1, int COLS1){

  int i;
  
  for(i = 0; i < (ROWS1 * COLS1); i++){
    if(msf[i] > tval){
      binary_msf[i] = 255;
    }
    else{
      binary_msf[i] = 0;
    }
  }
}

int main(int argc, char **argv)
{
	FILE	*fpt, *fpt2;
	int		ROWS1,COLS1,BYTES1;
  char	header[320], letter[2];
  unsigned char	*image, *msf, *binary, *binary_msf;
  int *endpoints, *branchpoints;
	int i, r, c, r2, c2, transition;
	int *mark_for_erasure, flag;
  int endpoint, branchpoint, detected = 0, transitions = 0, tendpoints, tbranchpoints;
  int row, col, tval;
  float tp, fp, tn, fn;
  int checkPoints;
/*--------------------OPEN AND READ INPUT IMAGE---------------------*/
	if ((fpt=fopen("parenthood.ppm","rb")) == NULL){
		  printf("Unable to open parenthood.ppm for reading\n");
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

/*--------------------OPEN AND READ MSF IMAGE---------------------*/

  if ((fpt=fopen("convolution.ppm","rb")) == NULL){
		  printf("Unable to open convolution.ppm for reading\n");
		  exit(0);
	}

    fscanf(fpt,"%s %d %d %d\n",header,&COLS1,&ROWS1,&BYTES1);

	if (strcmp(header,"P5") != 0  ||  BYTES1 != 255){
		  printf("Not a greyscale 8-bit PPM image\n");
		  exit(0);
	}

    msf = (unsigned char *)calloc(ROWS1*COLS1,sizeof(unsigned char));
    header[0]=fgetc(fpt);	 
	  fread(msf,1,COLS1*ROWS1,fpt);
	  fclose(fpt);
/*-------------------- THRESHOLD AT 128 ---------------------*/

  binary = (unsigned char *)calloc(ROWS1*COLS1,sizeof(unsigned char));
	mark_for_erasure = (int *)calloc(ROWS1*COLS1,sizeof(int));
  
	flag = 1;
	int iteration = 0;
	int count = 0;
	for(i = 0; i < (ROWS1*COLS1); i++){
		if(image[i] <=  128)
      binary[i] = 255;
    else{
      binary[i] = 0;
    }
  }

	while(iteration <= 10 && flag == 1){
		flag = 0;
		for(r=1;r<=(ROWS1 - 1);r++){
      for(c=1;c<=(COLS1 - 1);c++){
        if(binary[r*COLS1 + c] == EDGE){
  				mark_for_erasure[(r*COLS1) + c] = CheckErase(getE2NETransitions(r,c, COLS1, binary), getENeighbours(r,c, COLS1, binary), CheckNEWSpixels(r,c, COLS1, binary));
  				flag = (mark_for_erasure[r*COLS1 + c] > 0) ? 1 : flag;                     
        }		
      } 
		}		  
   	count = 0;
		for(i=0; i < (ROWS1*COLS1); i++){
      if(mark_for_erasure[i] == 1){
			  binary[i] = NON_EDGE;
        mark_for_erasure[i] = 0;
        count++;
      }
		}
		printf("\nFor Iteration %d, Number of Erasures : %d\n", iteration, count);
	  iteration++;
	}
  
/*------------------------------------------------------*/
 
 
 if ((fpt2=fopen("parenthood_gt.txt","rb")) == NULL){
		  printf("Unable to open parenthood_gt.txt for reading\n");
		  exit(0);
	}
 
 if ((fpt=fopen("ROC_BPEP.txt","w")) == NULL){
		  printf("Unable to open ROC.txt for writing\n");
		  exit(0);
	}
 
 branchpoints = (int *)calloc(ROWS1*COLS1,sizeof(int));
 endpoints = (int *)calloc(ROWS1*COLS1,sizeof(int));
 
  for(r=1;r<=(ROWS1 - 1);r++){
    for(c=1;c<=(COLS1 - 1);c++){
      if(binary[r*COLS1 + c] == 255){
        transition = getE2NETransitions(r, c, COLS1, binary);
        if(transition == 1){
          endpoints[r*COLS1 + c] = 1;
          branchpoints[r*COLS1 + c] = 0;
        }
        else if(transition > 2){
          endpoints[r*COLS1 + c] = 0;
          branchpoints[r*COLS1 + c] = 1;
        }
        else{
          endpoints[r*COLS1 + c] = 0;
          branchpoints[r*COLS1 + c] = 0;
        }
      }
    }
  }

  binary_msf = (unsigned char *)calloc(ROWS1*COLS1,sizeof(unsigned char));
  tval = 0;
  checkPoints = 0;
  endpoint = branchpoint = 0;
  while(tval <= 255){
    threshold(tval, msf, binary_msf, ROWS1, COLS1);
    tp = fp = tn = fn = 0;
    while(fscanf(fpt2, "%s %d %d", letter, &col, &row) != EOF){
      flag = 0;
      for(r = row - 7; r <= row + 7; r++){
        for(c = col - 5; c <= col + 3; c++){
          if(endpoints[r*COLS1 + c] == 1)
            endpoint++;
          else if(branchpoints[r*COLS1 + c] == 1)
            branchpoint++;
          if(binary_msf[r*COLS1 + c] == 255){
            checkPoints = 1;  
          }
        }      
      }
      if(checkPoints == 1 && endpoint == 1 && branchpoint == 1 && strcmp(letter, "e") == 0){
        tp++;
      }
      else if(checkPoints == 1 && endpoint != 1 && branchpoint  != 1 && strcmp(letter, "e") == 0){
        fp++;
      }
      else if(checkPoints == 0 && strcmp(letter, "e") == 0){
        fn++;
      }
      else if(strcmp(letter, "e") != 0 && checkPoints == 0){
        tn++;
      }  
      checkPoints = 0;
      endpoint = 0;
      branchpoint = 0;  
    }
    printf("For threshold %d  TP = %f FP = %f, TN = %f, FN = %f\n", tval, tp, fp, tn, fn);
    fprintf(fpt,"%d %f %f\n", tval,(tp/(tp+fn)),(fp/(fp+tn)));
    tval+=1; 
    rewind(fpt2);
  }

/*------------------------------------------------------*/	
	fpt=fopen("binary.ppm","w");
	fprintf(fpt,"P5 %d %d 255\n",COLS1,ROWS1);
	fwrite(binary,COLS1*ROWS1,1,fpt);
	fclose(fpt);       

	return 0;
}
