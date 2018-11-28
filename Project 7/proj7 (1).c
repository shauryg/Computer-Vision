#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define WINDOW_SIZE 11
#define GYRO_THRESHOLD_R 0.03
#define GYRO_THRESHOLD_P 0.03
#define GYRO_THRESHOLD_Y 0.03

#define ACC_THRESHOLD_X 0.0009
#define ACC_THRESHOLD_Y 0.0009
#define ACC_THRESHOLD_Z 0.0009

#define COLUMNS 6
#define SQR(x) ((x)*(x))
#define SAMPLE_TIME 0.05

#define GRAVITY 9.82 // m per sec^2

void read_initial_gyro(char [], double **, double **, double **, double **, double **, double **, double **, int *);
double calculate_variance(double *, int, int, int);
double calculate_mean(double *, int, int, int);
void smooth_data_left(double*, int);
void smooth_data_center(double*, int);
int check_movement(double *, double *, double *, double *, double *, double *, int , int );

int main(int argc, char **argv){
           //sec     //gravities (G)         //rad/sec
    double *time, *accX, *accY, *accZ, *pitch, *roll, *yaw; 
    int size, i, j, index;
   	int is_moving = 0;
    double move_pitch, move_roll, move_yaw;
	double v, u, s;
	int start_index, stop_index;
	double tdX = 0, tdY = 0, tdZ = 0, tdR = 0, tdP = 0, tdYa = 0;

	FILE *fpt;

    read_initial_gyro("acc_gyro.txt", &time, &accX, &accY, &accZ, &pitch, &roll, &yaw, &size);  

	/*for(i = 0; i < size; i++){
		accZ[i] = accZ[i] + 0;
	}*/

	i = 1;

	if ((fpt=fopen("movement.csv","w")) == NULL){
		  printf("Unable to open movement.csv for writing\n");
		  exit(0);
	}
	
	fprintf(fpt, "Start Time, End Time, Pitch, Roll, Yaw, X, Y, Z\n");

	index = 0;
	start_index = stop_index = -1;
	while(index < size){
		
		is_moving = check_movement(accX, accY, accZ, pitch, roll, yaw, index, size);
		//printf("%d", is_moving);
		if(is_moving == 1){
			if(start_index == -1){
				start_index = index;
			}
			index = index + 1;
			continue;
		}
		else{
			index = index + 1;
		}

		stop_index = index;

		if(stop_index != -1 && start_index != -1){
			printf("\n%d %d", start_index, stop_index);

			//fprintf(fpt, "%d\n",i);
		fprintf(fpt, "%lf, %lf, ", start_index*0.05, stop_index*0.05);
			
			move_pitch = move_roll = move_yaw = 0;

			//Calculate Motion For Gyroscopes
			for(j = start_index; j < stop_index; j++){
				move_pitch = move_pitch + (pitch[j] * SAMPLE_TIME);
				move_roll = move_roll + (roll[j] * SAMPLE_TIME);
				move_yaw  = move_yaw + (yaw[j] * SAMPLE_TIME);
			}

			tdR = tdR + move_roll;
			tdP = tdP + move_pitch;
			tdYa = tdYa + move_yaw;

			fprintf(fpt, "%lf, %lf, %lf, ", move_pitch, move_roll, move_yaw);

			u = v = s = 0;
			//Calculate Motion For Accelerometer X
			for(j = start_index; j < stop_index; j++){
				v = u + (accX[j] * GRAVITY)*SAMPLE_TIME;
				s = s + ((v + u) / 2)*SAMPLE_TIME;
				u = v;
			}
			tdX += s;
			fprintf(fpt, "%lf, ", s);
	
			u = v = s = 0;
			//Calculate Motion For Accelerometer Y
			for(j = start_index; j < stop_index; j++){
				v = u + (accY[j] * GRAVITY)*SAMPLE_TIME;
				s = s + ((v + u) / 2)*SAMPLE_TIME;
				u = v;
			}
			tdY += s;
			fprintf(fpt, "%lf, ", s);

			u = v = s = 0;
			//Calculate Motion For Accelerometer Z
			for(j = start_index; j < stop_index; j++){
				v = u + (accZ[j] * GRAVITY)*SAMPLE_TIME;
				s = s + ((v + u) / 2)*SAMPLE_TIME;
				u = v;
			}
			tdZ += s;
			fprintf(fpt, "%lf\n", s);

			start_index = stop_index = -1;
			index = index + 1;
			is_moving = 0;		
			i++;

			fprintf(fpt, "\n");
		}

	}

	fprintf(fpt, "\n\nTotal, Distance,%lf, %lf, %lf, %lf, %lf, %lf", tdP, tdR, tdYa, tdX, tdY, tdZ);

    printf("\n\n%d\n", i);
    return 0;

}

int check_movement(double *accX, double *accY, double *accZ, double *pitch, double *roll, double *yaw, int index, int size){

	double variance[6];

	if(index + WINDOW_SIZE > size)
		index = size - index;

	variance[0] = calculate_variance(accX, size, index, WINDOW_SIZE);
	variance[1] = calculate_variance(accY, size, index, WINDOW_SIZE);
	variance[2] = calculate_variance(accZ, size, index, WINDOW_SIZE);
	variance[3] = calculate_variance(pitch, size, index, WINDOW_SIZE);
	variance[4] = calculate_variance(roll, size, index, WINDOW_SIZE);
	variance[5] = calculate_variance(yaw, size, index, WINDOW_SIZE);

	printf("\nindex %d %f %f %f\n", index, variance[3], variance[4], variance[5]);

	if(variance[0] > ACC_THRESHOLD_X || variance[1] > ACC_THRESHOLD_Y || variance[2] > ACC_THRESHOLD_Z){
		return 1;
	}
	if(variance[3] > GYRO_THRESHOLD_P || variance[4] > GYRO_THRESHOLD_R || variance[5] > GYRO_THRESHOLD_Y){
		return 1;
	}

	return 0;
}

void smooth_data_left(double *data, int size){
    
    int i, j;    
    int ws = 25;
    double smoothed[size];

    memset(smoothed,0,size * sizeof(double));

    for(i = 0; i < ws; i++)
        smoothed[i] = data[i];
	
    for(i = ws - 1; i < size; i++){
        for(j = 0; j < ws; j++){
            smoothed[i] += data[i - j];
        }
        smoothed[i] = smoothed[i] / ws;
    } 

    for(i = 0; i < size; i++){
      data[i] = smoothed[i];
    } 
}

void smooth_data_center(double *data, int size){
    
    int i, j;    
    int ws = 11;
    double smoothed[size];

    memset(smoothed,0,size * sizeof(double));

    for(i = 0; i < ws/2; i++)
        smoothed[i] = data[i];

	printf("\n%d", ws/2);
	
	for(i = ws/2; i < size - ws/2; i++){
		for(j = -ws/2; j > ws/2; j++){
			smoothed[i] += data[i + j];
		}	
		smoothed[i] = smoothed[i] / ws;
	}
}


double calculate_variance(double *data, int size, int index, int ws){
    int i = 0;
    double mean;
    double variance = 0;

    mean = calculate_mean(data, size, index, ws);    
    
    if(index + ws > size)
        ws = size - index;

    for(i = index; i < index + ws; i++){     
        variance =  variance + SQR(data[i] - mean);   
    }
    
    variance = variance / (double)(ws - 1);
    return variance;
}

double calculate_mean(double *data, int size, int index, int ws){

    int i = 0;
    double sum = 0.0;

    if(index + ws > size)
        ws = size - index;

    for(i = index; i < index + ws; i++){
        sum = sum + data[i];
    }
    
    return sum/(double)ws;
}

void read_initial_gyro(char filename[], double **time, double **accX, double **accY, double **accZ, double **pitch, double **roll, double **yaw, int *size){

	FILE* fpt;
	int i = 0, sz = 0;
	double temp_f;
	char temp_s[256];

	if ((fpt=fopen(filename,"rb")) == NULL){
		  printf("Unable to open acc_gyro.ppm for reading\n");
		  exit(0);
	}	
	
	fgets(temp_s, sizeof(temp_s), fpt);	

	while(!feof(fpt)){
		fscanf(fpt, "%lf", &temp_f);
		sz++;	
	}
    sz = sz/7;
	(*size) = sz;
    
    rewind(fpt);
	fgets(temp_s, sizeof(temp_s), fpt);	

    *time = (double *)calloc(sz, sizeof(double));
    *accX = (double *)calloc(sz, sizeof(double));
    *accY = (double *)calloc(sz, sizeof(double));
    *accZ = (double *)calloc(sz, sizeof(double));
    *pitch = (double *)calloc(sz, sizeof(double));
    *roll = (double *)calloc(sz, sizeof(double));
    *yaw = (double *)calloc(sz, sizeof(double));

	i = 0;
    while(fscanf(fpt, "%lf %lf %lf %lf %lf %lf %lf", &(*time)[i], &(*accX)[i], &(*accY)[i], &(*accZ)[i], &(*pitch)[i], &(*roll)[i], &(*yaw)[i]) != EOF){
		i++;
	}

	fclose(fpt);
}





