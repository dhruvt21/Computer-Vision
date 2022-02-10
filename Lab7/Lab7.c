#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define Maximum_Length 256
#define AccelerationThreshold_x 0.0009
#define AccelerationThreshold_y 0.0009
#define AccelerationThreshold_z 0.0009
#define GyroThreshold_p 0.03
#define GyroThreshold_r 0.03
#define GyroThreshold_y 0.03
#define SmoothWindowSize 25
#define VarianceWindowSize 20
#define SamplingTime 0.05
#define Gravity 9.81

// Reading the text file to extract data
void read_text_file(char *FileName, int *FileSize, double **Time, double **ACC_X, double **ACC_Y,
					double **ACC_Z, double **PITCH, double **ROLL, double **YAW)
{
	FILE *file;
	int i = 0;
	double d1, d2, d3, d4, d5, d6, d7;
	char c;
	char line[Maximum_Length];
	*FileSize = 0;
 
	// Obtaining file length and rewind it to the beginning
	file = fopen(FileName, "r");
	if (file == NULL)
	{
		printf("Error, could not read in initial contour text file\n");
		exit(1);
	}
	while((c = fgetc(file)) != EOF)
	{
		if (c == '\n')
		{
			*FileSize += 1;
		}
	}
	rewind(file);
	fgets(line, sizeof(line), file);

	// Memory Allocation
	*Time = calloc(*FileSize, sizeof(double *));
	*ACC_X = calloc(*FileSize, sizeof(double *));
	*ACC_Y = calloc(*FileSize, sizeof(double *));
	*ACC_Z = calloc(*FileSize, sizeof(double *));
	*PITCH = calloc(*FileSize, sizeof(double *));
	*ROLL = calloc(*FileSize, sizeof(double *));
	*YAW = calloc(*FileSize, sizeof(double *));
	
	// Extracting data from text file	
	while((fscanf(file, "%lf %lf %lf %lf %lf %lf %lf\n", &d1, &d2, &d3, &d4, &d5, &d6, &d7)) != EOF)
	{
		(*Time)[i] = d1;
		(*ACC_X)[i] = d2;
		(*ACC_Y)[i] = d3;
		(*ACC_Z)[i] = d4;
		(*PITCH)[i] = d5;
		(*ROLL)[i] = d6;
		(*YAW)[i] = d7;
		i++;
	}
	fclose(file);

	// Creating CSV file
	file = fopen("InitialFile.csv", "w");
	fprintf(file, "Time[s],Acceleration_X[m/s],Acceleration_Y[m/s],Acceleration_Z[m/s],Pitch,Roll,Yaw\n");
	for (i = 0; i < *FileSize; i++)
	{
		fprintf(file, "%lf,%lf,%lf,%lf,%lf,%lf,%lf\n", (*Time)[i], (*ACC_X)[i], (*ACC_Y)[i], (*ACC_Z)[i], (*PITCH)[i], (*ROLL)[i], (*YAW)[i]);
	}
	fclose(file);
}

// Smoothing the data points 
void smooth_data(int ArrayLength, double **data, double **SmoothData)
{
	int i, j;
	double sum;
	
	// Memory Allocation
	*SmoothData = calloc(ArrayLength, sizeof(double *));

	for (i = 0; i < SmoothWindowSize; i++)
	{
		(*SmoothData)[i] = (*data)[i];
	}

	for (i = SmoothWindowSize - 1; i < ArrayLength; i++)
	{
		sum = 0;
		for (j = 1; j < SmoothWindowSize; j++)
		{
			sum += (*data)[i - j];
		}
		(*SmoothData)[i] = (sum + (*data)[i]) / SmoothWindowSize;
	}
}
double CalculateVariance(double **data, int ArrayLength, int Index)
{
	int i;
	int window = 0;
	double mean = 0;
	double variance = 0;

	// Error handling incase of big windowsize
	if ((Index + VarianceWindowSize) < ArrayLength)
	{
		window = VarianceWindowSize;
	}
	else
	{
		window = abs(ArrayLength - Index);
	}

	// Sum calculation
	for (i = Index; i < (Index + window); i++)
	{
		variance += (*data)[i];
	}
	mean = variance / window;
	variance = 0;
	for (i = Index; i < (Index + window); i++)
	{
		variance += pow(((*data)[i] - mean), 2);
	}
	variance = variance / (window - 1);

	return variance;
}

// Storing data to the CSV file if the object moves
void Move_Hold(double **ACC_X, double **ACC_Y, double **ACC_Z, double **PITCH, double **ROLL, double **YAW, int ArrayLength)
{
	FILE *text_file, *csv_file;
	int i, j, k;
	int Starting_Movement = 0;
	int End_of_Movement = 0;
	int moving = 0;
	double StartTime = 0;
	double EndTime = 0;
	double AccelerationVariance[3];
	double GyroscopeVariance[3];
	double GyroscopeDistance[3];
	double velocity[3];
	double Distance_Covered[3];
	double Prev_Velocity[3];
	double sum[6];
	char file_name[Maximum_Length];
	k = 1;

	sprintf(file_name, "movement-%d.txt", VarianceWindowSize);
	text_file = fopen(file_name, "w");
	sprintf(file_name, "movement-%d.csv", VarianceWindowSize);
	csv_file = fopen(file_name, "w");
	fprintf(csv_file, "Start Index,Stop Index,Start Time,End Time,X [m],Y [m],Z [m],PITCH [radians],ROLL [radians],YAW [radians]\n");
	
	for (i = 0; i < 6; i++)
	{
		sum[i] = 0;
	}


	for (i = 0; i < ArrayLength; i++)
	{
		for(j = 0; j < 3; j++)
		{
			AccelerationVariance[j] = 0;
			GyroscopeVariance[j] = 0;
			GyroscopeDistance[j] = 0;
			velocity[j] = 0;
			Distance_Covered[j] = 0;
			Prev_Velocity[j] = 0;
		}

		// Variance Calculation for acc and gyro
		AccelerationVariance[0] = CalculateVariance(ACC_X, ArrayLength, i);
		AccelerationVariance[1] = CalculateVariance(ACC_Y, ArrayLength, i);
		AccelerationVariance[2] = CalculateVariance(ACC_Z, ArrayLength, i);
		GyroscopeVariance[0] = CalculateVariance(PITCH, ArrayLength, i);
		GyroscopeVariance[1] = CalculateVariance(ROLL, ArrayLength, i);
		GyroscopeVariance[2] = CalculateVariance(YAW, ArrayLength, i);

		if ((AccelerationVariance[0] > AccelerationThreshold_x) || (AccelerationVariance[1] > AccelerationThreshold_y) 
			|| (AccelerationVariance[2] > AccelerationThreshold_z))
		{
			moving = 1;	
		}
		if ((GyroscopeVariance[0] > GyroThreshold_p) || (GyroscopeVariance[1] > GyroThreshold_r) 
			|| (GyroscopeVariance[2] > GyroThreshold_y))
		{
			moving = 1;
		}

		// Tracking of movement start and end time
		if (Starting_Movement == 0 && moving == 1)
		{
			Starting_Movement = i;
			StartTime = Starting_Movement * SamplingTime;
		}
		if (End_of_Movement == 0 && moving == 0 && Starting_Movement != 0)
		{
			End_of_Movement = i;
			EndTime = End_of_Movement * SamplingTime;
		}

		// Integration of gyro and acc
		if (Starting_Movement != 0 && End_of_Movement != 0)
		{
			// Gyro integration
			for (j = Starting_Movement; j < End_of_Movement; j++)
			{
				GyroscopeDistance[0] += ((*PITCH)[j] * SamplingTime);
				GyroscopeDistance[1] += ((*ROLL)[j] * SamplingTime);
				GyroscopeDistance[2] += ((*YAW)[j] * SamplingTime);
			}

			// acc double integration
			for (j = Starting_Movement; j < End_of_Movement; j++)
			{		
				Prev_Velocity[0] = velocity[0];		
				velocity[0] += ((*ACC_X)[j] * Gravity * SamplingTime);
				Distance_Covered[0] += (((velocity[0] + Prev_Velocity[0]) / 2) * SamplingTime);
				
				Prev_Velocity[1] = velocity[1];
				velocity[1] += ((*ACC_Y)[j] * Gravity * SamplingTime);
				Distance_Covered[1] += (((velocity[1] +Prev_Velocity[1]) / 2) * SamplingTime);
				
				Prev_Velocity[2] = velocity[2];
				velocity[2]	+= ((*ACC_Z)[j] * Gravity * SamplingTime);
				Distance_Covered[2] += (((velocity[2] +Prev_Velocity[2]) / 2) * SamplingTime);	
				
			}

			sum[0] += Distance_Covered[0];
			sum[1] += Distance_Covered[1];
			sum[2] += Distance_Covered[2];
			sum[3] += GyroscopeDistance[0];
			sum[4] += GyroscopeDistance[1];
			sum[5] += GyroscopeDistance[2];
			fprintf(text_file, "-------------------------------------\n");
			fprintf(text_file, "Movement #%d:\n", k);
			fprintf(text_file, "Movement X-axis: %.6f[m]\nMovement Y-axis: %.6f[m]\nMovement Z-axis: %.6f[m]\n", Distance_Covered[0], Distance_Covered[1], Distance_Covered[2]);
			fprintf(text_file, "Movement PITCH: %.6f[radians]\nMovement ROLL: %.6f[radians]\nMovement YAW: %.6f[radians]\n", GyroscopeDistance[0], GyroscopeDistance[1], GyroscopeDistance[2]);
			fprintf(text_file, "Movement Start Time: %.2f | Movement End Time: %.2f\n", StartTime, EndTime);
			fprintf(text_file, "Movement Start Index: %d | Movement End Index: %d\n", Starting_Movement, End_of_Movement);
			fprintf(text_file, "-------------------------------------\n\n");

			// CSV printing
			fprintf(csv_file, "%d,%d,%.2f,%.2f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n", 
					Starting_Movement + 1, End_of_Movement + 1, StartTime, EndTime, 
					Distance_Covered[0], Distance_Covered[1], Distance_Covered[2], 
					GyroscopeDistance[0], GyroscopeDistance[1], GyroscopeDistance[2]);

			Starting_Movement = 0;
			End_of_Movement = 0;
			StartTime = 0;
			EndTime = 0;
			k++;
		}
		moving = 0;
	}
	fprintf(csv_file,"Total Distance:,,,,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n", sum[0],sum[1],sum[2],sum[3],sum[4],sum[5]);
	fclose(text_file);
	fclose(csv_file);
}

int main(int argc, char *argv[])
{
	FILE *file;
	int file_size;
	int i;
	double *Time, *ACC_X, *ACC_Y, *ACC_Z, *PITCH, *ROLL, *YAW;
	double *smooth_ACC_X, *smooth_ACC_Y, *smooth_ACC_Z, *smooth_PITCH, *smooth_ROLL, *smooth_YAW;

	if (argc != 2)
	{
		printf("Usage: ./executable text_file.txt\n");
		exit(1);
	}

	//data reading from text file
	read_text_file(argv[1], &file_size, &Time, &ACC_X, &ACC_Y, &ACC_Z, &PITCH, &ROLL, &YAW);

	//performing smoothing on the extracted data
	smooth_data(file_size, &ACC_X, &smooth_ACC_X);
	smooth_data(file_size, &ACC_Y, &smooth_ACC_Y);
	smooth_data(file_size, &ACC_Z, &smooth_ACC_Z);
	smooth_data(file_size, &PITCH, &smooth_PITCH);
	smooth_data(file_size, &ROLL, &smooth_ROLL);
	smooth_data(file_size, &YAW, &smooth_YAW);

	//smooth data storage in CSV
	file = fopen("SmoothData.csv", "w");
	fprintf(file, "Time[s],Acceletarion-X[m/s],Acceletarion-Y[m/s],Acceleration-Z[m/s],PITCH,ROLL,YAW\n");
	for (i = 0; i < file_size; i++)
	{
		fprintf(file, "%lf,%lf,%lf,%lf,%lf,%lf,%lf\n", Time[i], smooth_ACC_X[i], smooth_ACC_Y[i], smooth_ACC_Z[i], smooth_PITCH[i], smooth_ROLL[i], smooth_YAW[i]);
	}
	fclose(file);

	/* Movement determination */
	Move_Hold(&ACC_X, &ACC_Y, &ACC_Z, &PITCH, &ROLL, &YAW, file_size);

	return 0;
}