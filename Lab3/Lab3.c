#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char *image_threshold(unsigned char *OG_IMAGE, int ROWS, int COLS)
{
  int i;
  int t = 128;
  unsigned char *T_IMAGE;
  T_IMAGE = (unsigned char *)calloc(ROWS * COLS, sizeof(unsigned char));
  for (i = 0; i < (ROWS*COLS); i++)
    {
        if (OG_IMAGE[i] <= t)
        {
            T_IMAGE[i] = 255;
        }
        else
        {
            T_IMAGE[i] = 0;
        }
    }
    
    return T_IMAGE;
}

int main (int argc, char *argv[])

{
FILE		*fpt, *fpt_tempfile, *Output_Image, *fpt_MsfImage, *fpt_thresholded;
unsigned char	*image, *Binary, *template, *Normalized_MsfImage, *Template_image, *Template_Binary, *Skeleton,*Skeleton_Branchpoint_endpoint;
char		IMAGE_HEADER[80],TEMPLATE_HEADER[80], MSFIMAGE_HEADER[80];
int		ROWS,COLS,BYTES, TEMPLATE_BYTES, TEMPLATE_ROWS, TEMPLATE_COLS,MSFIMAGE_ROWS,MSFIMAGE_COLS,MSFIMAGE_BYTES, *msf, *zero_mean;
int		 TN,FN,flag = 0,mean, letter_r,letter_c,r,c,i,j,k, sums, sum, min, max, diff, FP, TP, r2, c2 , Dummy = 1,  a, b;
char		letter,gt_letter[10], strings[2];
int     Branchpoints, Endpoints, NO_OF_NEIGHBOURS, NO_OF_TRANSACTIONS;
double 		TPR, FPR;
strcpy(strings,"e");
	/* User input arguments*/
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
    /*reading the normalized msf image that we have calculated in the previous lab*/
fpt_MsfImage=fopen("msf_e.ppm","rb");
if (fpt_MsfImage == NULL)
  {
  printf("Unable to open %s for reading\n","msf_e.ppm");
  exit(0);
  }
	/* read image header (simple 8-bit greyscale PPM only) */
i=fscanf(fpt,"%s %d %d %d ",IMAGE_HEADER,&COLS,&ROWS,&BYTES);
j=fscanf(fpt_tempfilefile,"%s %d %d %d ",TEMPLATE_HEADER,&TEMPLATE_COLS,&TEMPLATE_ROWS,&TEMPLATE_BYTES);
k=fscanf(fpt_MsfImage,"%s %d %d %d ",MSFIMAGE_HEADER,&MSFIMAGE_COLS,&MSFIMAGE_ROWS,&MSFIMAGE_BYTES);
if (i != 4 && j != 4 && k != 4 ||  strcmp(IMAGE_HEADER,"P5") != 0 && strcmp(TEMPLATE_HEADER, "P5") != 0 && strcmp(MSFIMAGE_HEADER, "P5") != 0 ||  BYTES != 255 && TEMPLATE_BYTES != 255 && MSFIMAGE_BYTES != 255)
  {
  printf("not an 8-bit PPM greyscale (P5) image\n");
  fclose(fpt);
  fclose(fpt_tempfile);
  fclose(fpt_MsfImage);
  exit(0);
  }
	/* allocate dynamic memory for image */
image=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
template=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
Normalized_MsfImage = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
Binary = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
Template_image = (unsigned char *)calloc(9*15,sizeof(unsigned char));
Template_Binary = (unsigned char *)calloc(9*15,sizeof(unsigned char));
Skeleton = (unsigned char *)calloc(9*15,sizeof(unsigned char));
Skeleton_Branchpoint_endpoint = (unsigned char *)calloc(9*15,sizeof(unsigned char));
if (image == NULL || Binary == NULL || Normalized_MsfImage ==NULL)
  {
  printf("Unable to allocate %d x %d memory\n",COLS,ROWS);
  printf("this is the error place");
  exit(0);
  }
  if(Template_Binary == NULL || Skeleton_Branchpoint_endpoint == NULL || Skeleton == NULL)
  {
    printf("Unable to allocate %d x %d memory\n",9,15);
    printf("this is the error place 2");
    exit(0);
  }
	/* read image data from file */
fread(image,1,ROWS*COLS,fpt);
fread(Normalized_MsfImage, 1, ROWS*COLS, fpt_MsfImage);
fclose(fpt);
fclose(fpt_tempfile);
fclose(fpt_MsfImage);

      /*Copying OG image data to thinned image*/

  for(k=0; k< ROWS*COLS; k++)
  {
    if(image[k] <= 128)
    {
      template[k] = 255;
    }
    else
      template[k] = 0;
  }
	/* open GT file for reading */
fpt=fopen("parenthood_gt.txt","r");

	/*looping through the threshold values */
fpt_tempfile = fopen("lab3output.txt","w");
printf("Threshold\tTP\tFP\n");
for(i=0; i<=255; i++)
{
  TP = 0;
  FP = 0;
  TN = 0;
  FN = 0;
  NO_OF_TRANSACTIONS = 0;
  NO_OF_NEIGHBOURS = 0;
  while (1)/*to check all the point locations in the file at the threshold i*/
  {
    j=fscanf(fpt,"%s %d %d",gt_letter,&letter_c,&letter_r);
    if (j != 3)
      break;
    for (r=letter_r-7; r<=letter_r+7; r++)
    {
      for(c = letter_c-4; c <= letter_c+4; c++)
      {
        if(Normalized_MsfImage[r*COLS + c] > i)
        {
          flag = 1;//the letter is detected
        }
      }
    }
    if(flag == 1)
    { 
      k = 0;
      j = 0;
        /*creating the copy of the area centered at the ground truth location*/
	  k = 0;
      j = 0;
      for (r=letter_r-7; r<=letter_r+7; r++)
      {
        for(c = letter_c-4; c <= letter_c+4; c++)
        {
          Template_Binary[k*9 + j] = template[r*COLS + c];
          j++;
        }
        k++;
        j = 0;
      }
      if(d2==1 && strcmp(gt_letter, strings) == 0){
        
      fpt_thresholded=fopen("lab3_threshold.ppm","wb");
      fprintf(fpt_thresholded,"P5 %d %d 255\n",9,15);
      fwrite(Template_Binary,1,9*15,fpt_thresholded);
      fclose(fpt_thresholded);
      d2--;}
      int ends = 1;
      int condt = 0;
      for(k=0; k< 9*15; k++)
      {
        Skeleton[k] = 1;
      }
          /*Thinning of the thresholded image*/
      while(ends == 1)
      {
        ends = 0;
        
          
          for(c = 1; c < 8; c++)//Columns
          {
              for(r = 1; r<14; r++)//Rows
              {
                NO_OF_NEIGHBOURS = 0;
                NO_OF_TRANSACTIONS = 0;
                condt = 0;
                
                if(Template_Binary[r*9 + c] == 255)
                {
                //Top Row Rotation
                r2 = r - 1;
                for(c2 = -1; c2 < 1; c2++)
                {
                  if(Template_Binary[r2*9 + c + c2] == 255 && Template_Binary[r2*9 + c + c2 + 1] == 0)
                  {
                    NO_OF_TRANSACTIONS++;
                  }
                  if(Template_Binary[r2*9 + c + c2] == 255)
                  {
                    NO_OF_NEIGHBOURS++;
                  }
                }
				//Top Column Rotation
                c2 = c + 1;
                for(r2 = -1; r2 < 1; r2++)
                {
                  if(Template_Binary[(r+r2)*9 + c2] == 255 && Template_Binary[(r+r2 + 1)*9 + c2] == 0)
                  {
                    NO_OF_TRANSACTIONS++;
                  }
                  if(Template_Binary[(r+r2)*9 + c2] == 255)
                  {
                    NO_OF_NEIGHBOURS++;
                  }
                }
                r2 = r + 1;
                for(c2 = 1; c2 > -1; c2--)
                {
                  if(Template_Binary[r2*9 + c + c2] == 255 && Template_Binary[r2*9 + c2 + c -1] == 0)
                  {
                    NO_OF_TRANSACTIONS++;
                  }
                  if(Template_Binary[r2*9 + c2 + c] == 255)
                  {
                    NO_OF_NEIGHBOURS++;
                  }
                }
                c2 = c - 1;
                for(r2 = 1; r2 > -1; r2--)
                {
                  if(Template_Binary[(r+r2)*9 + c2] == 255 && Template_Binary[(r+r2 - 1)*9 + c2] == 0)
                  {
                    NO_OF_TRANSACTIONS++;
                  }
                  if(Template_Binary[(r+r2)*9 + c2] == 255)
                  {
                    NO_OF_NEIGHBOURS++;
                  }
                }
                if(Template_Binary[(r-1)*9 + c] == 0 || Template_Binary[r*9 + c + 1] == 0 || (Template_Binary[r*9 + c - 1] == 0 && Template_Binary[(r + 1)*9 + c] == 0))
                {
                  condt = 1;
                }
                if(NO_OF_TRANSACTIONS == 1 && (NO_OF_NEIGHBOURS >= 2 && NO_OF_NEIGHBOURS<=6) && condt == 1)
                {
                  Skeleton[r*9 + c] = 0;
                  ends = 1;
                }
                else{
                  Skeleton[r*9 + c] = 1;
                }
              }
          }
          for(k=0; k<=9*15; k++)
          {
            Template_Binary[k] *= Skeleton[k];
          }
      }
      if(d3==1 && strcmp(gt_letter, strings) == 0){
        
      fpt_thresholded=fopen("lab3_thinned.ppm","wb");
      fprintf(fpt_thresholded,"P5 %d %d 255\n",9,15);
      fwrite(Template_Binary,1,9*15,fpt_thresholded);
      fclose(fpt_thresholded);
      d3--;}
      for(r=0;r<9*15;r++)
      {
        Skeleton_Branchpoint_endpoint[r] = Template_Binary[r];
      }
        Branchpoints = 0;
        Endpoints = 0;
        for(c = 1; c < 8; c++)//cols
          {
              for(r = 1; r<14; r++)//rows
              {
                NO_OF_NEIGHBOURS = 0;
                NO_OF_TRANSACTIONS = 0;
                if(Template_Binary[r*9 + c] == 255)
                {
                r2 = r - 1;
                for(c2 = -1; c2 < 1; c2++)
                {
                  if(Template_Binary[r2*9 + c + c2] == 255 && Template_Binary[r2*9 + c + c2 + 1] == 0)
                  {
                    NO_OF_TRANSACTIONS++;
                  }
                }

                //Right Column rotation
                c2 = c + 1;
                for(r2 = -1; r2 < 1; r2++)
                {
                  if(Template_Binary[(r+r2)*9 + c2] == 255 && Template_Binary[(r+r2 + 1)*9 + c2] == 0)
                  {
                    NO_OF_TRANSACTIONS++;
                  }
                }

                //Bottom Row rotation
                r2 = r + 1;
                for(c2 = 1; c2 > -1; c2--)
                {
                  if(Template_Binary[r2*9 + c + c2] == 255 && Template_Binary[r2*9 + c2 + c -1] == 0)
                  {
                    NO_OF_TRANSACTIONS++;
                  }
                }

                //Left Row rotation
                c2 = c - 1;
                for(r2 = 1; r2 > -1; r2--)
                {
                  if(Template_Binary[(r+r2)*9 + c2] == 255 && Template_Binary[(r+r2 - 1)*9 + c2] == 0)
                  {
                    NO_OF_TRANSACTIONS++;
                  }
                }
                if(NO_OF_TRANSACTIONS == 1)
                {
                  Endpoints++;
                  Skeleton_Branchpoint_endpoint[r*9 + c] = 50;//end point
                }
                else if(NO_OF_TRANSACTIONS > 2)
                {
                  Branchpoints++;
                  Skeleton_Branchpoint_endpoint[r*9 + c] = 150;//branch point
                }}
            }
          }
          if(Endpoints == 1 && Branchpoints == 1)
          {
            flag = 1;
            if(Dummy == 1){
            fpt_thresholded=fopen("lab3_thinned_Branchpoint_endpoint.ppm","wb");
            fprintf(fpt_thresholded,"P5 %d %d 255\n",9,15);
            fwrite(Skeleton_Branchpoint_endpoint,1,9*15,fpt_thresholded);
            fclose(fpt_thresholded);
            Dummy--;
            }
          }
          else{
            flag = 0;
          }
    }
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
  fprintf(fpt_tempfilefile,"%.2lf %.2lf %d\n",TPR, FPR, i);
  printf("%d\t%d\t%d\n",i, TP, FP);}
  rewind(fpt);

}
fclose(fpt);
}