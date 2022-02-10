#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char *argv[])

{
FILE		*fpt, *fpt_tempfile, *Output_Image;
unsigned char	*image, *Binary, *Template, *normalized_msf;
char		IMAGE_HEADER[80],TEMPLATE_HEADER[80];
int		IMAGE_ROWS,IMAGE_COLS,IMAGE_BYTES, TEMPLATE_BYTES, TEMPLATE_ROWS, TEMPLATE_COLS, *MSF, *ZeroMeanVal;
int		TN,FN,flag = 0,Mean, letter_r,letter_c,r,c,i,j, sums, sum, min, max, diff, FP, TP, r2, c2;
char		letter,gt_letter[10], strings[2];
double 		TPR, FPR;
strcpy(strings,"e");
	/* tell user how to use program if incorrect arguments */
if (argc != 1)
  {
  printf("Usage:  outline [letter]\n");
  exit(0);
  }

	/* open image for reading */
fpt=fopen("parenthood.ppm","rb");
if (fpt == NULL)
  {
  printf("Unable to open %s for reading\n","parenthood.ppm");
  exit(0);
  }

fpt_tempfile=fopen("parenthood_e_template.ppm","rb");
if (fpt_tempfile == NULL)
  {
  printf("Unable to open %s for reading\n","parenthood_e_template.ppm");
  exit(0);
  }
	/* read image header (simple 8-bit greyscale PPM only) */
i=fscanf(fpt,"%s %d %d %d ",IMAGE_HEADER,&IMAGE_COLS,&IMAGE_ROWS,&IMAGE_BYTES);
j=fscanf(fpt_tempfile,"%s %d %d %d ",TEMPLATE_HEADER,&TEMPLATE_COLS,&TEMPLATE_ROWS,&TEMPLATE_BYTES);
if (i != 4 && j != 4  ||  strcmp(IMAGE_HEADER,"P5") != 0 && strcmp(TEMPLATE_HEADER, "P5") != 0  ||  IMAGE_BYTES != 255 && TEMPLATE_BYTES != 255)
  {
  printf("not an 8-bit PPM greyscale (P5) image\n");
  fclose(fpt);
  fclose(fpt_tempfile);
  exit(0);
  }
	/* allocate dynamic memory for image */
image=(unsigned char *)calloc(IMAGE_ROWS*IMAGE_COLS,sizeof(unsigned char));
Template=(unsigned char *)calloc(IMAGE_ROWS*IMAGE_COLS,sizeof(unsigned char));
MSF = (int *)calloc(IMAGE_ROWS*IMAGE_COLS,sizeof(int));
Binary = (unsigned char *)calloc(IMAGE_ROWS*IMAGE_COLS,sizeof(unsigned char));
if (image == NULL || Template == NULL)
  {
  printf("Unable to allocate %d x %d memory\n",IMAGE_COLS,IMAGE_ROWS);
  exit(0);
  }
	/* read image data from file */
fread(image,1,IMAGE_ROWS*IMAGE_COLS,fpt);
fread(Template,1,TEMPLATE_ROWS*TEMPLATE_COLS, fpt_tempfile);
fclose(fpt);
fclose(fpt_tempfile);

	/* open GT file for reading */
fpt=fopen("parenthood_gt.txt","r");

	/*calculattion for the zero mean-centered template*/
sums = 0;
Mean = 0;
ZeroMeanVal = (int *)calloc(TEMPLATE_ROWS*TEMPLATE_COLS,sizeof(int));
for(i=0; i<TEMPLATE_ROWS*TEMPLATE_COLS; i++)
{
		sums += Template[i];
}
Mean = sums / (TEMPLATE_ROWS*TEMPLATE_COLS);
for(i=0; i<TEMPLATE_ROWS*TEMPLATE_COLS; i++)
{
		ZeroMeanVal[i] = Template[i] - Mean;
}

	/*calculation for the msf image*/
for(r=7; r<IMAGE_ROWS-7; r++)
{ 
  for(c=4; c<IMAGE_COLS-4; c++)
  {
    sum = 0;
    for (r2=-7; r2<=7; r2++)
      for (c2=-4; c2<=4; c2++)
        sum+=(image[(r+r2)*IMAGE_COLS+(c+c2)] * ZeroMeanVal[(r2+7)*TEMPLATE_COLS + (4+c2)]);
   MSF[r*IMAGE_COLS + c] = sum;

}}

	/*Minimum and maximum calculation for normalization*/
min = MSF[0];
max = MSF[0];
for(j=0; j<IMAGE_ROWS*IMAGE_COLS; j++)
{
	if(MSF[j] > max){
	max = MSF[j];}
	if(MSF[j] < min){
	min = MSF[j];}
}
	/*Normaliation of MSF image*/
normalized_msf = (unsigned char *)calloc(IMAGE_ROWS*IMAGE_COLS,sizeof(unsigned char));

for(r=0; r<IMAGE_ROWS*IMAGE_COLS; r++)
{ 
	normalized_msf[r] = ((MSF[r] - min ) * 255/(max - min));
}

	/*Output print*/
Output_Image = fopen("normalized1.ppm", "w");
    fprintf(Output_Image, "P5 %d %d 255\n", IMAGE_COLS, IMAGE_ROWS);
    fwrite(normalized_msf, IMAGE_COLS * IMAGE_ROWS, sizeof(unsigned char), Output_Image);
    fclose(Output_Image);

	/*looping through the threshold values*/
fpt_tempfile = fopen("groundtruth.txt","w");
printf("Threshold\t\tTP\tFP\n");
for(i=0; i<=255; i++)
{
	/*Binary image using the threshold*/
	for(r=0; r<IMAGE_ROWS*IMAGE_COLS; r++)
	{ 
		if(normalized_msf[r] > i)
			Binary[r] = 255;
		else
			Binary[r] = 0;
	}
  TP = 0;
  FP = 0;
  TN = 0;
  FN = 0;
  while (1)/*checking for points at threshold i*/
  {
  j=fscanf(fpt,"%s %d %d",gt_letter,&letter_c,&letter_r);
  if (j != 3)
    break;
  for (r=letter_r-7; r<=letter_r+7; r++)
    {
	for(c = letter_c-4; c <= letter_c+4; c++)
	{
		if(Binary[r*IMAGE_COLS + c] == 255)
		{
			flag = 1;
		}
	}}
	if(flag == 1 && strcmp(gt_letter, strings) == 0){
	TP++;}
	else if(flag == 1 && strcmp(gt_letter, strings) != 0)
	FP++;
	else if(flag == 0 && strcmp(gt_letter, strings) == 0)
	FN++;
	else if(flag == 0 && strcmp(gt_letter, strings) != 0)
	TN++;
    flag = 0;
    }
  TPR = (double)TP/(double)(TP + FN);
  FPR = (double)FP/(double)(FP + TN);
  if(i%5 == 0){
  fprintf(fpt_tempfile,"%.2lf %.2lf %d\n",TPR, FPR, i);}
  printf("%d\t%d\t%d\n",i, TP, FP);
  rewind(fpt);
}
fclose(fpt_tempfile);
fclose(fpt);
	/* Writing output of the result*/
fpt=fopen("lab2_Binaryimage.ppm","wb");
for(r=0; r<IMAGE_ROWS*IMAGE_COLS; r++)
        {       
                if(normalized_msf[r] > 210)
                        Binary[r] = 255;
                else    
                        Binary[r] = 0;
        }

if (fpt == NULL)
  {
  printf("Unable to open outlined.ppm for writing\n");
  exit(0);
  }
fprintf(fpt,"P5 %d %d 255\n",IMAGE_COLS,IMAGE_ROWS);
fwrite(Binary,1,IMAGE_ROWS*IMAGE_COLS,fpt);
fclose(fpt);
}

