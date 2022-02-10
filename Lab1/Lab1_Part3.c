#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main()

{
FILE		*fpt;
unsigned char	*image;
unsigned char	*smoothed3;
char		header[320];
int		ROWS,COLS,BYTES;
int		r,c,r2,c2,sum;
struct timespec	tp1,tp2;
float *smoothed2;

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
smoothed2=(float *)calloc(ROWS*COLS,sizeof(float));
smoothed3=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));

	/* query timer */
clock_gettime(CLOCK_REALTIME,&tp1);
printf("%ld %ld\n",(long int)tp1.tv_sec,tp1.tv_nsec);

	/* smooth image, skipping the border points */

for (r=3; r<ROWS-3; r++)
  for (c=0; c<COLS; c++)
    {
    sum=0;
    for (r2=-3; r2<=3; r2++)
	sum+=image[(r+r2)*COLS+(c)];
    smoothed2[r*COLS+c]=sum;
    }

for (r=0; r<ROWS; r++)
  for (c=3; c<COLS-3; c++)
    {
    sum=0;
      for (c2=-3; c2<=3; c2++)
        sum+=smoothed2[(r*COLS)+(c+c2)];
    smoothed3[r*COLS+c]=(int)sum/49;
    }

	/* query timer */
clock_gettime(CLOCK_REALTIME,&tp2);
printf("%ld %ld\n",(long int)tp2.tv_sec,tp2.tv_nsec);

	/* report how long it took to smooth */
printf("%ld\n",tp2.tv_nsec-tp1.tv_nsec);

	/* write out smoothed image to see result */
fpt=fopen("smoothed1.ppm","w");
fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
fwrite(smoothed3,COLS*ROWS,1,fpt);
fclose(fpt);
}
