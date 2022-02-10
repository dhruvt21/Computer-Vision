#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main()

{
FILE		*fpt;
unsigned char	*image;
unsigned char	*smoothed3;
float 		*smoothed1;
char		header[320];
int		ROWS,COLS,BYTES;
int		r,c,r2,c2,r3,c3,sum,temp1;
struct timespec	tp1,tp2;

	/* read image */
if ((fpt=fopen("einstein-gaussnoise.ppm","rb")) == NULL)
  {
  printf("Unable to open einstein-gaussnoise.ppm for reading\n");
  exit(0);
  }
fscanf(fpt,"%s %d %d %d",header,&COLS,&ROWS,&BYTES);
if (strcmp(header,"P5") != 0  ||  BYTES != 255)
  {
  printf("Not a greyscale 8-bit PPM image\n");
  exit(0);
  }
image=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
header[0]=fgetc(fpt);	/* read white-space character that separates header */
fread(image,1,COLS*ROWS,fpt);
fclose(fpt);

	/* allocate memory for smoothed version of image */
smoothed3=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
smoothed1=(float *)calloc(ROWS*COLS,sizeof(float));

	/* query timer */
clock_gettime(CLOCK_REALTIME,&tp1);
printf("%ld %ld\n",(long int)tp1.tv_sec,tp1.tv_nsec);

	/* smooth image, skipping the border points */

for(r=0; r<COLS; r++)
{
  sum = 0;
  for(c=0; c<COLS; c++)
  {
  	if(c<=6){
	  sum += image[r*COLS + c];
	  if(c==6)
		smoothed1[r*COLS + 3] = sum;
	}
	else{
	sum = sum + image[r*COLS +c] - image[r*COLS + c - 7];
	smoothed1[(r*COLS) + c - 3] = sum;
	}
}}

for(c=0; c<COLS; c++)
{
  sum = 0;
  temp1 = 3;
  for(r=0; r<COLS; r++)
  {
	if(r<=6){
	sum += smoothed1[r*COLS +c];
	if(r==6){
		smoothed3[temp1*COLS+c] = (int)sum/49;
		temp1++;
	}}
	else{
	sum = sum + smoothed1[r*COLS+c] - smoothed1[(r-7) * COLS + c];
	smoothed3[temp1*COLS + c] = (int)sum/49;
	temp1++;
	}
}}
	
	/* query timer */
clock_gettime(CLOCK_REALTIME,&tp2);
printf("%ld %ld\n",(long int)tp2.tv_sec,tp2.tv_nsec);

	/* report how long it took to smooth */
printf("%ld\n",tp2.tv_nsec-tp1.tv_nsec);

	/* write out smoothed image to see result */
fpt=fopen("smoothed3.ppm","w");
fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
fwrite(smoothed3,COLS*ROWS,1,fpt);
fclose(fpt);
}

