#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#define Maximum_length 256
#define Max_iteration 30
#define SQUARE(x) ((x) * (x))
#define Window_Size 7

/* Image Reading */
unsigned char *Read_Image(int rows, int cols, FILE *MainImage)
{
	unsigned char *image;

	image = (unsigned char *)calloc(rows * cols, sizeof(unsigned char));

	fread(image, sizeof(unsigned char), rows * cols, MainImage);

	fclose(MainImage);

	return image; 
}

/* Creation and saving a file as a ppm image */
void SaveImage(unsigned char *image, char *FileName, int rows, int cols)
{
	FILE *file;

	file = fopen(FileName, "w");
	fprintf(file, "P5 %d %d 255\n", cols, rows);
	fwrite(image, rows * cols, sizeof(unsigned char), file);
	fclose(file);
}

/* Information extraction from contour text file*/
void ReadInitialContour(char *FileName, int **Contour_Rows, int **Contour_Cols, int *File_Size)
{
	FILE *file;
	int i = 0;
	int cols, rows;
	char c;
	cols = rows = 0;
	*File_Size = 0;
	file = fopen(FileName, "r");
	if (file == NULL)
	{
		printf("Unable to read the initial contour text file\n");
		exit(1);
	}

	while((c = fgetc(file)) != EOF)
	{
		if (c == '\n')
		{
			*File_Size += 1;
		}
	}
	rewind(file);

	// Allocating memory
	*Contour_Rows = calloc(*File_Size, sizeof(int *));
	*Contour_Cols = calloc(*File_Size, sizeof(int *));
	while((fscanf(file, "%d %d\n", &cols, &rows)) != EOF)
	{
		(*Contour_Rows)[i] = rows;
		(*Contour_Cols)[i] = cols;
		i++;
	}

	fclose(file);
}

/* Code for generating hawk eye image with initial contour points */
void Contour_draw(unsigned char *image, int image_rows, int image_cols, int **Contour_Rows, int **Contour_Cols, int Array_Length, char *FileName)
{
	unsigned char *OutputImage;
	int rows, cols;
	int i = 0;

	OutputImage = (unsigned char *)calloc(image_rows * image_cols, sizeof(unsigned char));
	
	for (i = 0; i < (image_rows * image_cols); i++)
	{
		OutputImage[i] = image[i];
	}

	// Drawing "+" on image
	for (i = 0; i <Array_Length; i++)
	{
		rows = (*Contour_Rows)[i];
		cols = (*Contour_Cols)[i];
		OutputImage[(rows - 3)*image_cols + cols] = 0;
		OutputImage[(rows - 2)*image_cols + cols] = 0;
		OutputImage[(rows - 1)*image_cols + cols] = 0;
		OutputImage[(rows - 0)*image_cols + cols] = 0;
		OutputImage[(rows + 1)*image_cols + cols] = 0;
		OutputImage[(rows + 2)*image_cols + cols] = 0;
		OutputImage[(rows + 3)*image_cols + cols] = 0;
		OutputImage[(rows * image_cols) + (cols - 3)] = 0;
		OutputImage[(rows * image_cols) + (cols - 2)] = 0;
		OutputImage[(rows * image_cols) + (cols - 1)] = 0;
		OutputImage[(rows * image_cols) + (cols - 0)] = 0;
		OutputImage[(rows * image_cols) + (cols + 1)] = 0;
		OutputImage[(rows * image_cols) + (cols + 2)] = 0;
		OutputImage[(rows * image_cols) + (cols + 3)] = 0;
	}
	
	SaveImage(OutputImage, FileName, image_rows, image_cols);
	
	free(OutputImage);
}

/* calculation of minimum and maximum value in the pixels */
void Calculation_Min_Max(int *Convolution_Image, int image_rows, int image_cols, int *Minimum, int *Maximum)
{
	int i;
	
	*Minimum = Convolution_Image[0];
	*Maximum = Convolution_Image[0];
	for (i = 1; i < (image_rows * image_cols); i++)
	{
		if (*Minimum > Convolution_Image[i])
		{
			*Minimum = Convolution_Image[i];
		}
		if (*Maximum < Convolution_Image[i])
		{
			*Maximum = Convolution_Image[i];
		}
	}
}
void Calculation_Min_Max_Float(float *Convolution_Image, int image_rows, int image_cols, float *Minimum, float *Maximum)
{
	int i, j, k;
	
	*Minimum = Convolution_Image[0];
	*Maximum = Convolution_Image[0];

	for(i = 1; i < (image_rows-1); i++)
	{
		for(j = 1; j < (image_cols-1); j++)
		{
			k = (i * image_cols) + j;
			if (*Minimum > Convolution_Image[k])
			{
				*Minimum = Convolution_Image[k];
			}
			if (*Maximum < Convolution_Image[k])
			{
				*Maximum = Convolution_Image[k];
			}
		}
	}	
}

/* Normalization unsigned character input image*/
unsigned char *Normalized_UnsignedChar(int *Convolution_Image, int image_rows, int image_cols, int New_Minimum, int New_Maximum, int min, int max)
{
	
	unsigned char *NormalizedImage;
	int i;
	

	NormalizedImage = (unsigned char *)calloc(image_rows * image_cols, sizeof(unsigned char));
	
	for (i = 0; i < (image_rows * image_cols); i++)
	{
		if (min == 0 && max == 0)
		{
			NormalizedImage[i] = 0;
		}
		else
		{
			NormalizedImage[i] = ((Convolution_Image[i] - min)*(New_Maximum - New_Minimum)/(max-min)) + New_Minimum;
		}
	}
	
	return NormalizedImage;
}

float *Normalized_Float(float *Convolution_Image, int image_rows, int image_cols, float New_Minimum, float New_Maximum, float min, float max)
{
	float *NormalizedImage;
	int i;

	NormalizedImage = (float *)calloc(image_rows * image_cols, sizeof(float));

	for (i = 0; i < (image_rows * image_cols); i++)
	{
		NormalizedImage[i] = ((Convolution_Image[i] - min)*(New_Maximum - New_Minimum)/(max-min)) + New_Minimum;
	}

	return NormalizedImage;
}

/* Normalized sobl image  */
float *sobel_edge_detector(unsigned char *image, int image_rows, int image_cols)
{
	int *Convolution_Image;
	float *SobelFilter_Image;
	unsigned char *NormalizedImage;
	int i, j, rows, cols;
	int Index_1 = 0;
	int Index_2 = 0;
	int x = 0;
	int y = 0;
	int min = 0;
	int max = 0;

	// sobel x and y kernels
	int g_x[9] = 	{-1, 0, 1, 
					-2, 0, 2, 
					-1, 0, 1};

	int g_y[9] = 	{-1, -2, -1
					, 0, 0, 0, 
					1, 2, 1};

	Convolution_Image = (int *)calloc(image_rows * image_cols, sizeof(int));
	SobelFilter_Image = (float *)calloc(image_rows * image_cols, sizeof(float));

	for (i = 0; i < (image_rows * image_cols); i++)
	{
		Convolution_Image[i] = image[i];
	}

	// Convolution of x and y kernels
	for (rows = 1; rows < (image_rows - 1); rows++)
	{
		for (cols = 1; cols < (image_cols - 1); cols++)
		{
			x = 0;
			y = 0;
			for (i = -1; i < 2; i++)
			{
				for (j = -1; j < 2; j++)
				{
					Index_1 = (image_cols * (rows + i)) + (cols + j);
					Index_2 = 3*(i + 1) + (j + 1);
					x += (image[Index_1] * g_x[Index_2]);
					y += (image[Index_1] * g_y[Index_2]);
				}
			}
			Index_1 = (image_cols * rows) + cols;
			Convolution_Image[Index_1] = sqrt((SQUARE(x) + SQUARE(y)));
			SobelFilter_Image[Index_1] = sqrt((SQUARE(x) + SQUARE(y)));
		}
	}

	// finding min and max values for normalization
	Calculation_Min_Max(Convolution_Image, image_rows, image_cols, &min, &max);
	
	// Normalization
	NormalizedImage = Normalized_UnsignedChar(Convolution_Image, image_rows, image_cols, 0, 255, min, max);
	SaveImage(NormalizedImage, "hawk_SobelFilter_Image.ppm", image_rows, image_cols);

	// inverting normalized image 
	for (i = 0; i < (image_rows * image_cols); i++)
	{
		NormalizedImage[i] = 255 - NormalizedImage[i];
	}
	SaveImage(NormalizedImage, "hawk_sobelinverted_Image.ppm", image_rows, image_cols);
	
	free(NormalizedImage);
	free(Convolution_Image);

	return SobelFilter_Image;
}

/* Active contour algorithm to input image*/
void ActiveContour(unsigned char *image, float *SobelFilter_Image, int image_rows, int image_cols, int **Contour_Rows, int **Contour_Cols, int Array_Length)
{
	float *InvertedSobel;
	float *Internal_Energy1;
	float *Internal_Energy2;
	float *External_Energy;
	float *Sum_of_Energies;
	float min, max, New_Minimum, New_Maximum;
	float *Internal_Energy1_normalized, *Internal_Energy2_normalized, *External_Energy_normalized;
	float Avg_dist_X = 0;
	float Avg_dist_Y = 0;
	float Avg_dist = 0;
	int i, j, k, l, rows, cols;
	int index = 0;
	int Index_2 = 0;
	int index3 = 0;
	int new_x[Array_Length];
	int new_y[Array_Length];
	int temp = 0;
	New_Minimum = 0.0;
	New_Maximum = 1.0;


	Internal_Energy1 = (float *)calloc(49, sizeof(float));
	Internal_Energy2 = (float *)calloc(49, sizeof(float));
	External_Energy = (float *)calloc(49, sizeof(float));
	Sum_of_Energies = (float *)calloc(49, sizeof(float));
	InvertedSobel = (float *)calloc(image_rows * image_cols, sizeof(float));

	Calculation_Min_Max_Float(SobelFilter_Image, image_rows, image_cols, &min, &max);
	
	// Creates an invereted Sobel image for External Energy calculation
	for ( i = 0; i < (image_rows * image_cols); i++)
	{
		InvertedSobel[i] = SobelFilter_Image[i];
		InvertedSobel[i] = (float)max - InvertedSobel[i];
	}

	// Calculates first Internal Energy
	for (l = 0; l < Max_iteration; l++)
	{

		Avg_dist_X = 0.0;
		Avg_dist_Y = 0.0;
		Avg_dist = 0.0;

		// Calculation of average distance
		for (i = 0; i <Array_Length; i++)
		{
			if ((i + 1) <Array_Length)
			{
				Avg_dist_X = SQUARE((*Contour_Cols)[i] - (*Contour_Cols)[i + 1]);
				Avg_dist_Y = SQUARE((*Contour_Rows)[i] - (*Contour_Rows)[i + 1]);
			}
			else
			{
				Avg_dist_X = SQUARE((*Contour_Cols)[i] - (*Contour_Cols)[0]);
				Avg_dist_Y = SQUARE((*Contour_Rows)[i] - (*Contour_Rows)[0]);
			}
			Avg_dist +=sqrt(Avg_dist_X + Avg_dist_Y);
			new_x[i] = 0;
			new_y[i] = 0;
		}
		Avg_dist /=Array_Length;

		for (i = 0; i <Array_Length; i++)
		{
			rows = (*Contour_Rows)[i];
			cols = (*Contour_Cols)[i];
			index = 0;

			// first and second internal energy calculation
			for (j = (rows - 3); j <= (rows + 3); j++)
			{
				for (k = (cols - 3); k <= (cols + 3); k++)
				{
					if ((i + 1) <Array_Length)
					{
						Internal_Energy1[index] = SQUARE(k - (*Contour_Cols)[i + 1]) + SQUARE(j - (*Contour_Rows)[i + 1]); 
						Internal_Energy2[index] = SQUARE(sqrt(Internal_Energy1[index]) - Avg_dist); 
						Index_2 = (j * image_cols) + k;
						External_Energy[index] = SQUARE(InvertedSobel[Index_2]);
					}
					else
					{
						Internal_Energy1[index] = SQUARE(k - (*Contour_Cols)[0]) + SQUARE(j - (*Contour_Rows)[0]); 
						Internal_Energy2[index] = SQUARE(sqrt(Internal_Energy1[index]) - Avg_dist); 
						Index_2 = (j * image_cols) + k;
						External_Energy[index] = SQUARE(InvertedSobel[Index_2]);
						
					}
					index++;
				}
			}

			// find min and max of each enegery and normalize it to 0-1
			Calculation_Min_Max_Float(Internal_Energy1, Window_Size, Window_Size, &min, &max);
			Internal_Energy1_normalized = Normalized_Float(Internal_Energy1, Window_Size, Window_Size, New_Minimum, New_Maximum, min, max);
			Calculation_Min_Max_Float(Internal_Energy2, Window_Size, Window_Size, &min, &max);
			Internal_Energy2_normalized = Normalized_Float(Internal_Energy2, Window_Size, Window_Size, New_Minimum, New_Maximum, min, max);
			Calculation_Min_Max_Float(External_Energy, Window_Size, Window_Size, &min, &max);
			External_Energy_normalized = Normalized_Float(External_Energy, Window_Size, Window_Size, New_Minimum, New_Maximum, min, max);

			for (j = 0; j < 49; j++)
			{
				Sum_of_Energies[j] = Internal_Energy1_normalized[j] + Internal_Energy2_normalized[j] + External_Energy_normalized[j];
			}
			free(Internal_Energy1_normalized);
			free(Internal_Energy2_normalized);
			free(External_Energy_normalized);
			min = Sum_of_Energies[0];
			index = 0;
			for (j = 0; j < 49; j++)
			{
				if (min > Sum_of_Energies[j])
				{
					min = Sum_of_Energies[j];
					index = j;
				}
			}
			temp = 0;
			Index_2 = (index / Window_Size); // row
			if (Index_2 < 3)
			{
				temp = (*Contour_Rows)[i] - abs(Index_2 - 3);
				new_y[i] = temp;
			}
			else if (Index_2 > 3)
			{
				temp = (*Contour_Rows)[i] + abs(Index_2 - 3);
				new_y[i]  = temp;
			}
			else
			{
				new_y[i] = (*Contour_Rows)[i];
			}
			

			index3 = (index % Window_Size); // col
			if (index3 < 3)
			{
				new_x[i] = (*Contour_Cols)[i] - abs(index3 - 3);
				
			}
			else if (index3 > 3)
			{
				new_x[i] = (*Contour_Cols)[i] + abs(index3 - 3);
			}
			else
			{
				new_x[i] = (*Contour_Cols)[i];
			}
		}
		for (i = 0; i <Array_Length; i++)
		{
			(*Contour_Cols)[i] = new_x[i];
			(*Contour_Rows)[i] = new_y[i];
		}
		
	}

	// Draws image wit hfinal contour points 
	Contour_draw(image, image_rows, image_cols, Contour_Rows, Contour_Cols,Array_Length, "Final_Contour.ppm");

	// CREATE FILE WITH FINAL CONTOUR POINTS
	FILE *file;
	file = fopen("final_contour_points.csv", "w");
	fprintf(file, "COLS,ROWS\n");
	for(i = 0; i <Array_Length; i++)
	{
		fprintf(file, "%d,%d\n", (*Contour_Cols)[i], (*Contour_Rows)[i]);
	}
	fclose(file);
	free(Internal_Energy1);
	free(Internal_Energy2);
	free(External_Energy);
	free(Sum_of_Energies);
	free(InvertedSobel);
}

int main(int argc, char *argv[])
{
	FILE *MainImage;
	int IMAGE_ROWS, IMAGE_COLS, IMAGE_BYTES;
	char file_header[Maximum_length];
	unsigned char *input_image;
	float *SobelFilter_Image;
	int *Contour_Rows, *Contour_Cols;		
	int File_Size;
	if (argc != 3)
	{
		printf("Usage: ./executable MainImage.ppm initial_contour_file.txt");
		exit(1);
	}

	/* Opens image for reading  */
	MainImage = fopen(argv[1], "rb");
	if (MainImage == NULL)
	{
		printf("Error, could not read input image\n");
		exit(1);
	}
	fscanf(MainImage, "%s %d %d %d\n", file_header, &IMAGE_COLS, &IMAGE_ROWS, &IMAGE_BYTES);
	if ((strcmp(file_header, "P5") != 0) || (IMAGE_BYTES !=  255))
	{
		printf("Error, not a greyscale 8-bit PPM image\n");
		fclose(MainImage);
		exit(1);
	}

	/* Memory allocation for input image */
	input_image = Read_Image(IMAGE_ROWS, IMAGE_COLS, MainImage);

	/* get info from text file */
	ReadInitialContour(argv[2], &Contour_Rows, &Contour_Cols, &File_Size);
	
	/* + sign*/
	Contour_draw(input_image, IMAGE_ROWS, IMAGE_COLS, &Contour_Rows, &Contour_Cols, File_Size, "hawk_initial_contour.ppm");
	
	/* Un-normalized sobel image*/
	SobelFilter_Image = sobel_edge_detector(input_image, IMAGE_ROWS, IMAGE_COLS);
	
	/* Contour algorithm */
	ActiveContour(input_image, SobelFilter_Image, IMAGE_ROWS, IMAGE_COLS, &Contour_Rows, &Contour_Cols, File_Size);
	
	free(input_image);
	free(SobelFilter_Image);
	free(Contour_Rows);
	free(Contour_Cols);
	
	return 0;
}