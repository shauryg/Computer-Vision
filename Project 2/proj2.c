#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>


#define MAX 255
#define MIN 0

int main(int argc, char **argv)
{
  FILE	*fpt, *fpt2, *fpt3;
  int		ROWS1,COLS1,BYTES1, ROWS2,COLS2,BYTES2;
  char	header[320];
  unsigned char	*image, *template, *convout, *binary;
  int *nor_template, *output ;
  int i, max, min, avg = 0;
  int r,c,r2,c2, sum;
  char letter[2];
  int col, row;
  float threshhold, tp, fp, tn, fn, tp_flag, fp_flag, tn_flag, fn_flag;

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

/*--------------------OPEN AND READ TEMPLATE IMAGE---------------------*/

    if ((fpt=fopen("parenthood_e_template.ppm","rb")) == NULL){
		  printf("Unable to open parenthood_e_template.ppm for reading\n");
		  exit(0);
	}

    fscanf(fpt,"%s %d %d %d\n",header,&COLS2,&ROWS2,&BYTES2);

	if (strcmp(header,"P5") != 0  ||  BYTES2 != 255){
		  printf("Not a greyscale 8-bit PPM image\n");
		  exit(0);
	}

    template = (unsigned char *)calloc(ROWS2*COLS2,sizeof(unsigned char));
    header[0]=fgetc(fpt);	/* read white-space character that separates header */
	fread(template,1,COLS2*ROWS2,fpt);
	fclose(fpt);
 
/*--------------------ZERO MEAN CENTER THE FILTER---------------------*/
    nor_template = (int *)calloc(ROWS1*COLS1,sizeof(int));

	for(i = 0; i < (ROWS2*COLS2); i++){
        avg += template[i];
    }
  
    avg /= (ROWS2*COLS2);
 
    for(i = 0; i < ROWS2*COLS2; i++){
        nor_template[i] = template[i] - avg;
    }

/*-------------------- CONVOLVE THE FILTER ACROSS THE INPUT IMAGE ---------------------*/

    output = (int *)calloc(ROWS1*COLS1,sizeof(int));
	
	for(r=7;r<ROWS1 - 7;r++){
    for(c=4;c<COLS1 - 4;c++){
      sum = 0;
      for (r2=-7; r2 < (ROWS2 - 7); r2++){
        for (c2=-4; c2 < (COLS2 - 4); c2++){
          sum +=  nor_template[((r2  + ROWS2/2)*COLS2) + (c2 + COLS2/2)]*image[((r+r2)*COLS1) + (c+c2)];        
        }
      }
			output[(r)*COLS1 + (c)] = sum;
    }
  }

/*--------------------NORMALIZE THE IMAGE---------------------*/
	max = output[(ROWS2/2)*COLS1 + (COLS2/2)];
	min = output[(ROWS2/2)*COLS1 + (COLS2/2)];

	convout = (unsigned char *)calloc(ROWS1*COLS1,sizeof(unsigned char));

	for(r=7;r<ROWS1 - 7;r++){
    for(c=4;c<COLS1 - 4;c++){
      if(output[(r)*COLS1 + (c)] > max)
        max = output[(r)*COLS1 + (c)];
      if(output[(r)*COLS1 + (c)] < min)
        min = output[(r)*COLS1 + (c)];
    }
	}

	printf(" MAX : %d\tMIN: %d \n\n", max, min);
	//printf("Pre Normalize:  %d ", output[9600]);

    for(i = 0; i < (ROWS1*COLS1); i++){
        convout[i] = ((output[i] - min)*(MAX - MIN)/(max-min)) + MIN;	        
    }

	//printf("Post Normalize: %d ", convout[9600]);
/*--------------------THRESHOLD THE IMAGE---------------------*/

	binary = (unsigned char *)calloc(ROWS1*COLS1,sizeof(unsigned char));
  threshhold = 0;
  
  if ((fpt=fopen("parenthood_gt.txt","rb")) == NULL){
		  printf("Unable to open parenthood_gt.txt for reading\n");
		  exit(0);
	}
    
  if ((fpt2=fopen("ROC.txt","w")) == NULL){
		  printf("Unable to open ROC.txt for writing\n");
		  exit(0);
	}
 
  while(threshhold < 255){
 	  for(i = 0; i < (ROWS1*COLS1); i++){
		  if(convout[i] > threshhold)
   	    binary[i] = 255;
      else binary[i] = 0;
    }
    if(threshhold == 205){
      fpt3=fopen("binary.ppm","w");
  	  fprintf(fpt3,"P5 %d %d 255\n",COLS1,ROWS1);
  	  fwrite(binary,COLS1*ROWS1,1,fpt3);
	    fclose(fpt3);
    }
    
    
    tp = 0;
    fp = 0;
    tn = 0;
    fn = 0;
    while(fscanf(fpt, "%s %d %d", letter, &col, &row) != EOF){
      tp_flag = 0;
      fp_flag = 0;
      fn_flag = 0;
      tn_flag = 0;
      for(r = row - 7; r <= row + 7; r++){
        for(c = col - 4; c <= col + 4; c++){
          if(binary[r*COLS1 + c] >= threshhold && strcmp(letter, "e") == 0){
            tp_flag = 1;
            break;     
          }
          else if(binary[r*COLS1 + c] >= threshhold && strcmp(letter, "e") != 0){
            fp_flag = 1;
            break;  
          }
          else if(binary[r*COLS1 + c] < threshhold && strcmp(letter, "e") == 0){
            fn_flag = 1;
          }
          else if(binary[r*COLS1 + c] < threshhold && strcmp(letter, "e") != 0){
            tn_flag = 1;
          }
        }
        if(tp_flag == 1 || fp_flag == 1)
          break;      
      }
      if(tp_flag == 1){
        tp += 1;
      }
      else if(fp_flag == 1){
        fp += 1;
      }
      else if(tn_flag == 1){
        tn += 1;
      }
      else if(fn_flag == 1){
        fn += 1;
      }
    }  
    printf("\nFor %1.0f : TP = %1.0f FP = %1.0f, FN = %1.0f, TN = %1.0f, TPR = %f, FPR = %f, PPV = %f", threshhold, tp, fp, fn, tn, (tp/(tp+fn)),(fp/(fp+tn)),(fp/(fp+tp)));
    threshhold += 1;
    fprintf(fpt2,"%1.0f %f %f\n", threshhold,(tp/(tp+fn)),(fp/(fp+tn)));
    rewind(fpt);
  }
   
  
/*--------------------WRITE OUT THE IMAGE---------------------*/

	fpt2=fopen("convolution.ppm","w");
	fprintf(fpt2,"P5 %d %d 255\n",COLS1,ROWS1);
	fwrite(convout,COLS1*ROWS1,1,fpt2);
	fclose(fpt2);
 
  return 0;

}
