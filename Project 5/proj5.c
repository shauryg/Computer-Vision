#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define CONTOUR_COLS 7
#define CONTOUR_ROWS 7
#define SQR(x) ((x)*(x))
#define WINDOW 7
#define TEST_VALUE 0

void read_initial_contours(char [], int** , int**, int *);
void find_max_min(float [],  int, int *, int *);
void normalize(float [], int, int, int, int , int);
void sobel_filter(unsigned char * , unsigned char * , int , int );

int main(int argc, char **argv){
	FILE	*fpt;
	int		ROWS1,COLS1,BYTES1;
  char	header[320];
  unsigned char	*image, *initial_contour, *final_contour, *sobel_out;
	int *crow, *ccol, *next_row, *next_col;
	int i, j, r, c, size, max, min;
	float energy[WINDOW*WINDOW], temp[WINDOW*WINDOW];
	float avg_distance = 0, min_c;
	int total;

/*--------------------OPEN AND READ INPUT IMAGE---------------------*/
	if ((fpt=fopen("hawk.ppm","rb")) == NULL){
		  printf("Unable to open hawk.ppm for reading\n");
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
/*--------------------OPEN AND READ CONTOUR DATA---------------------*/
	
	read_initial_contours("initial_countour.txt", &ccol, &crow, &size);
	
/*--------------------DRAW INITAL CONTOURS ON IMAGE---------------------*/
	initial_contour = (unsigned char *)calloc(ROWS1 * COLS1, sizeof(unsigned char));
	for(i = 0; i < ROWS1 * COLS1; i++){
		initial_contour[i] = image[i];
	}

	for(i = 0; i < size; i++){
		for(r = -3; r <= 3; r++){
			for(c = -3; c <= 3; c++){
				if(r == 0 || c == 0){
					initial_contour[(crow[i] + r)*COLS1 + (ccol[i] + c)] = 0;				
				}					
			} 		
		}
	}
	fpt=fopen("inital_contours.ppm","w");
	fprintf(fpt,"P5 %d %d 255\n",COLS1,ROWS1);
	fwrite(initial_contour,COLS1*ROWS1,1,fpt);
	fclose(fpt);
	
/*-------------------CALCULATE AVG DISTANCE------------------------------*/

	next_row = (int *)calloc(size, sizeof(int));
	next_col = (int *)calloc(size, sizeof(int));
	
/*--------------------CALCULATE SOBEL FILTER OUTPUT---------------------------*/
	
	sobel_out = (unsigned char *)calloc(ROWS1*COLS1,sizeof(unsigned char));	
	sobel_filter(image, sobel_out, ROWS1, COLS1);

  
/*--------------------CONTOURING---------------------*/

	int k = 0;
	i = 0;
  
	for(total = 0; total < 30; total++){
		for(i = 0; i < 42; i++){
   
		/*--------------------CALCULATE FIRST INTERNAL ENERGY---------------------*/
			for(r = -3; r <= 3; r++){
				for(c = -3; c <= 3; c++){
					energy[(r + 3)*WINDOW + (c + 3)] = SQR((crow[i] + r) - crow[(i + 1)%size]) + SQR((ccol[i] + c) - ccol[(i + 1)%size]);
				}
			}

		/*--------------------CALCULATE SECOND INTERNAL ENERGY---------------------*/ 
      
      avg_distance = 0.0;
			for(k = 0; k < size; k++){
				avg_distance = avg_distance + sqrt(SQR(crow[k] - crow[(k+1)%size]) + SQR(ccol[k] - ccol[(k+1)%size]));
			}
		  avg_distance /= (size); 
      
			for(r = -3; r <= 3; r++){
				for(c = -3; c <= 3; c++){
				  temp[(r + 3)*WINDOW + (c + 3)] = SQR(avg_distance - sqrt(energy[(r + 3)*WINDOW + (c + 3)]));
        }
			}
      
      find_max_min(energy, WINDOW*WINDOW, &max, &min);
			normalize(energy, max, min, WINDOW*WINDOW, 1, 0);
	
			find_max_min(temp, WINDOW*WINDOW, &max, &min);
			normalize(temp, max, min, WINDOW*WINDOW, 1, 0);
      
			for(k = 0; k < WINDOW*WINDOW; k++){
				energy[k] += temp[k];
			}

		/*--------------------CALCULATE EXTERNAL ENERGY---------------------*/
      k = 0;
      for(r = -3; r <= 3; r++){
				for(c = -3; c <= 3; c++){
					temp[(r + 3)*WINDOW + (c + 3)] = SQR(sobel_out[(crow[i] + r)*COLS1 + (ccol[i] + c)]);
          k++;
				}
			}
	
			find_max_min(temp, WINDOW*WINDOW, &max, &min);
			normalize(temp, max, min, WINDOW*WINDOW, 1, 0);

			for(k = 0; k < WINDOW*WINDOW; k++){
				energy[k] += temp[k];
        if(i == TEST_VALUE){
          if(k%7 == 0)
            printf("\n");
          printf(" %f ", energy[k]);
          if(k == 48)
            printf("\n");
        }
			}      
      
		/*--------------------CALCULATE CHANGED CONTOUR POINT---------------------*/
			min_c = energy[0];
			for(k = 0; k < WINDOW*WINDOW; k++){
				if(min_c >= energy[k]){
					min_c = energy[k];
        }	
			}
      
      //if(i == TEST_VALUE);
        //printf("\n%f\n", min_c);

			for(r = -3; r <= 3; r++){
				for(c = -3; c <= 3; c++){
					if(energy[(r + 3)*WINDOW + (c + 3)] == min_c){
						next_row[i] = crow[i] + r;
						next_col[i] = ccol[i] + c;	
            if(i == TEST_VALUE)
            printf("\n%d %d %f\n", r, c, min_c);		
					}
				}
			}
		} // END OF LOOPING THROUGH CONTOUR POINTS

	/*--------------------CHANGE THE CONTOUR POINTS---------------------*/
		for(k = 0; k < size; k++){
			crow[k] = next_row[k];
			ccol[k] = next_col[k];		
		}
	}

	/*--------------------DRAW FINAL CONTOURS ON IMAGE---------------------*/
	final_contour = (unsigned char *)calloc(ROWS1 * COLS1, sizeof(unsigned char));

	for(i = 0; i < ROWS1 * COLS1; i++){
		final_contour[i] = image[i];
	}
	
	for(i = 0; i < size; i++){
		for(r = -3; r <= 3; r++){
			for(c = -3; c <= 3; c++){	
				if(r == 0 || c == 0){
					final_contour[(crow[i] + r)*COLS1 + (ccol[i] + c)] = 0;				
				}					
			} 		
		}
	}
	
  fpt=fopen("final_contours.txt","w");
  for(i = 0; i < 42; i++){
	  fprintf(fpt,"%d %d\n",ccol[i],crow[i]);
  }
	fclose(fpt);
 
	fpt=fopen("final_contours.ppm","w");
	fprintf(fpt,"P5 %d %d 255\n",COLS1,ROWS1);
	fwrite(final_contour,COLS1*ROWS1,1,fpt);
	fclose(fpt);

	return 0;
}

void sobel_filter(unsigned char * image, unsigned char * output, int ROWS, int COLS){

	int sobel_x[9] = {1,0,-1,2,0,-3,1,0,-1};
	int sobel_y[9] = {1,2,1,0,0,0,-1,2,-1};
	int i, r, c, sobel_r, sobel_c, sumx = 0, sumy = 0, max_out, min_out;
	float *temp, *post_normalized;
	FILE *fpt;

	temp =(float *)calloc(ROWS*COLS,sizeof(float));		
  post_normalized = (float *)calloc(ROWS*COLS,sizeof(float));		
  
	for(r = 1; r < ROWS - 1; r++){
		for(c = 1; c < COLS - 1; c++){
			sumx = sumy = 0;
			for(sobel_r = -1; sobel_r <= 1; sobel_r++){
				for(sobel_c = -1; sobel_c <= 1; sobel_c++){
					sumx += (sobel_x[(sobel_r + 1)*3 + (sobel_c + 1)] * image[(r + sobel_r)*COLS + (sobel_c + c)]);
          sumy += (sobel_y[(sobel_r + 1)*3 + (sobel_c + 1)] * image[(r + sobel_r)*COLS + (sobel_c + c)]);
				}		
			}
			temp[r*COLS + c] = (sumx*sumx) + (sumy*sumy);		
		}	
	}
	
	for(i = 0; i < ROWS*COLS; i++){
    temp[i] = sqrt(temp[i]);
    post_normalized[i] = temp[i];
    //printf(" %f ", temp[i]);
  }
  
  find_max_min(temp, ROWS*COLS, &max_out, &min_out);
	normalize(temp, max_out, min_out, ROWS*COLS, 255, 0);
  
	//find_max_min(temp, ROWS*COLS, &max_out, &min_out);
	//normalize(temp, max_out, min_out, ROWS*COLS, 1, 0);

	for(i = 0; i < ROWS*COLS; i++){
		output[i] = 255 - (unsigned char)temp[i];
    //output[i] = temp[i];
  }
	
	fpt=fopen("sobel_out.ppm","w");
	fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
	fwrite(output,COLS*ROWS,1,fpt);
	fclose(fpt);

}

void find_max_min(float energy[], int size, int *max, int *min){
	
	int i;
	*max = *min = energy[0];
 
	for(i = 0; i < size; i++){
		if(energy[i] > *max)
			*max = energy[i];
		if(energy[i] < *min)
			*min = energy[i];	
	}
}

void normalize(float energy[], int max, int min, int size, int new_max, int new_min){
	
	int i;

	for(i = 0; i < size; i++){
		if(max == min)
			energy[i] = 0;
		else
			energy[i] = new_min + (((energy[i] - min)*(new_max - new_min))/(max - min));
	}
}

void read_initial_contours(char filename[], int** ccol, int** crow, int *size){

	FILE* fpt;
	int col, row, i = 0, sz = 0, temp;

	if ((fpt=fopen(filename,"rb")) == NULL){
		  printf("Unable to open hawk.ppm for reading\n");
		  exit(0);
	}

	while(!feof(fpt)){
		fscanf(fpt, "%d", &temp);
		sz++;	
	}
	rewind(fpt);
	
	*crow = (int *)calloc(sz/2, sizeof(int));
	*ccol = (int *)calloc(sz/2, sizeof(int));
	(*size) = sz/2;
	i = 0;
	while(fscanf(fpt, "%d %d", &col, &row) != EOF){
		(*ccol)[i] = col;
		(*crow)[i] = row;
		i++;
	}

	printf("\nInitial Contours Read...\n");
	
	fclose(fpt);

}

