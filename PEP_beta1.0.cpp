/*
Copyright (c) <2012>, <Luca Shindler>
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, 
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ipl.h"
#include "cv.h"
#include <time.h>
#include "highgui.h"
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cvtypes.h>
#include <iostream>
#include <windows.h>

using namespace std;

typedef unsigned char  byte;
typedef unsigned long  ulong;
typedef unsigned short ushort;

//  ---------------------------------------------------------------------------
//	  Main
//  ---------------------------------------------------------------------------	

void main (short argc, char *argv[])
{
IplImage	*img8U[3], *src=0, *src2=0, *eigImage,*curr_pyr, *prev_pyr, *tempImage, *dst;
char	*matrice, filename[500], filename2[500], dirname[500], images_root[500],string[500],filename_FEB[500],status[10000],filename_out[500], AVIname[20];;
int		n_row, n_col, band, i, j, avanza, kk, ll, k, fr, x_offset,y_offset,y_off,width,height,startfr,finalfr, fradvance;
int		maxcornerCount=40000, cornerCount, max[20000], img_loc, scritti, comet, *bar_per_frame, window_size, half_window_size;
int		n_row_enlarged, n_col_enlarged, answer_roi, ss, x_min, x_max, y_min, y_max, pyrLev, winTrack;
int		im_type, dpmin;
ulong	n_frames;
float	qualityLevel, minDistance, **x_coord, **y_coord, **x_coord2, **y_coord2, x_float, y_float, xp, yp;
double	Nx,Ny,Dx,Dy;
ushort	spotnumber, count;
byte	**img;
FILE	*file,*outfile;
CvPoint	punto,punto2;
CvVideoWriter *writer;	

	
	MessageBox(0, "Calculate the barycentre of particles from a sequence of 'bmp' images.\n\nOutput:\n1) .feb file cointaning the particle centre coordinates;\n2) .ofp file with the corrispondent displacement vector predictor."
		   , "Particle Extraction and Prediction calculation algorithm, vBeta, 2012",MB_OK);

	printf("\nMaximum number of particles = %d (Press any key)\n", maxcornerCount);
	getch();
	
	scritti=0;
	comet=3;
	x_coord = (float **) malloc(comet * sizeof(float *));
	y_coord = (float **) malloc(comet * sizeof(float *));
	x_coord2 = (float **) malloc(comet * sizeof(float *));
	y_coord2 = (float **) malloc(comet * sizeof(float *));
	for(i=0;i<comet;i++) {
		x_coord[i] = (float *) malloc(maxcornerCount * sizeof(float));
		y_coord[i] = (float *) malloc(maxcornerCount * sizeof(float));
		x_coord2[i] = (float *) malloc(maxcornerCount * sizeof(float));
		y_coord2[i] = (float *) malloc(maxcornerCount * sizeof(float));
	}
	bar_per_frame=(int *) malloc(comet * sizeof(int));
	

//  ---------------------------------------------------------------------------
//    Data input
//  ---------------------------------------------------------------------------

	printf("\nInsert the filename root of output files: ");
	scanf("%s",string);
	wsprintf(filename_FEB,"output\\%s.feb",string);
     
    if((file = fopen(filename_FEB,"wb"))==NULL) {
		perror(filename);
		exit(33);
	} 
	wsprintf(filename_out,"output\\%s.ofp",string);
	wsprintf(AVIname,"output\\%s.avi",string);

	if((outfile=fopen(filename_out,"wb"))==NULL) {
		perror(filename);
		exit(33);
	}

	printf("\nInsert the file directory that contains the image sequence (Only BMP format): ");
	scanf("%s",dirname);

	printf("\nInsert the filename root of the image sequence (03d or 04d or 05d):");
	scanf("%s",images_root);

	printf("\nType '3', '4' or '5' if the file name is like respectively '03d', '04d', '05d':");
	scanf("%d",&im_type);

	printf("\nInsert starting frame: ");
	scanf("%d", &startfr);

	printf("\nInsert final frame: ");
	scanf("%d", &finalfr);

	printf("\nInsert frame advance: ");
	scanf("%d", &fradvance);
	
	if (im_type==3)
	wsprintf(filename, "%s\\%s%03d.bmp", dirname, images_root, startfr);
	
	if (im_type==4)
	wsprintf(filename, "%s\\%s%04d.bmp", dirname, images_root, startfr);
	
	if (im_type==5)
	wsprintf(filename, "%s\\%s%05d.bmp", dirname, images_root, startfr);

	src=cvLoadImage(filename, -1);
	n_row=src->height;
	n_col=src->width;
	band=src->nChannels;
	dst=cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,3);

	for(i=0;i<3;i++){
		img8U[i] = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	}
	cvReleaseImage(&src);

	printf("\nNumber of image rows= %d", n_row);
	printf("\nNumber of image columns= %d\n", n_col);

	printf("\nInsert the minimum distance between features [pixel]:");
	scanf("%f",&minDistance);

	printf("\nInsert the minimum distance between particles [pixel]:");
	scanf("%d",&dpmin);

	printf("\nInsert the number of pyramidal levels (typical value 3):");
	scanf("%d",&pyrLev);

	printf("\nInsert the half dimension of tracking window [pixel]:");
	scanf("%d",&winTrack);

	printf("\nInsert the quality level for feature extraction(typical value 0.01):");
	scanf("%f",&qualityLevel);

	printf("\nAnalyze a Region Of Interest (Y=1, N=0)? ");
	scanf("%d", &answer_roi);

	if(answer_roi==1){
		printf("\nInsert x-offset ROI:");
		scanf("%d",&x_offset);

		printf("\nInsert y-offset ROI:");
		scanf("%d",&y_off);

		printf("\nInsert ROI width:");
		scanf("%d",&width);

		printf("\nInsert ROI height:");
		scanf("%d",&height);
		y_offset=n_row-(y_off+height);
	}
	else{
		x_offset=0;
		y_offset=0;
		width=n_col;
		height=n_row;
	}
	
	x_min=x_offset;
	x_max=x_offset+width-1;
	y_min=y_offset;
	y_max=y_offset+height-1;
	window_size=minDistance; //2L+1
	half_window_size=floor(window_size/2.f); //L

// Image resizing
	n_row_enlarged=n_row+2*half_window_size+2;
	n_col_enlarged=n_col+2*half_window_size+2;
	img = (byte **) calloc(n_row_enlarged, sizeof(byte *));
	for(i=0;i<n_row_enlarged;i++) {
		img[i] = (byte *) calloc(n_col_enlarged, sizeof(byte));
	}	

//  ---------------------------------------------------------------------------
//    Write data on .feb header file
//  ---------------------------------------------------------------------------
	n_frames=finalfr-startfr;
	fwrite(&n_frames, sizeof(ulong), 1, file);		//number of frames to analyse
	fwrite(&startfr, sizeof(int), 1, file);			//starting frame
	fwrite(&fradvance, sizeof(int), 1, file);		//frame advance
	fwrite(&window_size, sizeof(int), 1, file);     //size of window for local gray level search
	fwrite(&n_row,sizeof(int),1,file);              //img rows
	fwrite(&n_col,sizeof(int),1,file);              //img cols
	fwrite(&x_offset,sizeof(int),1,file);			//x0
	fwrite(&y_offset,sizeof(int),1,file);			//y0
	fwrite(&width,sizeof(int),1,file);				//width
	fwrite(&height,sizeof(int),1,file);				//height
	fwrite(&minDistance, sizeof(float), 1, file);   //minimum distance between features
	fwrite(&dpmin, sizeof(int), 1, file);           //minimum distance between particles
	fwrite(&qualityLevel, sizeof(float), 1, file);  //quality level of features
	fwrite(&pyrLev, sizeof(int), 1, file);          //number of pyramids levels
	fwrite(&winTrack, sizeof(int), 1, file);        //size of window tracking
//  ---------------------------------------------------------------------------

//  ---------------------------------------------------------------------------
//	  OpenCV variable declaration
//  ---------------------------------------------------------------------------
	CvRect roi=cvRect(x_offset,y_offset,width,height);
	CvPoint2D32f* corners;
	CvPoint2D32f* corners2;
	CvPoint2D32f* delta;
	CvPoint2D32f* deltaDEF;
	CvPoint2D32f* max_loc;
	CvPoint2D32f* centers;
	cvNamedWindow( "Output", 0); // working window
	cvResizeWindow("Output", 640, 640);
	corners=(CvPoint2D32f*) calloc(maxcornerCount, sizeof(CvPoint2D32f));
	writer=cvCreateVideoWriter(AVIname, -1, 20, cvSize(n_col,n_row), 1);
//  ---------------------------------------------------------------------------

//  ---------------------------------------------------------------------------
//	  Image sequence analysis
//  ---------------------------------------------------------------------------
	for (fr=startfr; fr<(n_frames+startfr); fr+=fradvance) {
		
		corners2=(CvPoint2D32f*) calloc(maxcornerCount, sizeof(CvPoint2D32f));
		max_loc=(CvPoint2D32f*) calloc(maxcornerCount, sizeof(CvPoint2D32f));
		centers=(CvPoint2D32f*) calloc(maxcornerCount, sizeof(CvPoint2D32f));
		delta=(CvPoint2D32f*) calloc(maxcornerCount, sizeof(CvPoint2D32f));
		deltaDEF=(CvPoint2D32f*) calloc(maxcornerCount, sizeof(CvPoint2D32f));

		fseek(file,0,SEEK_END);

		// Choosing types of images 03d, 04d, 05d		
		if (im_type==3){
			wsprintf(filename, "%s\\%s%03d.bmp", dirname, images_root, fr);
			wsprintf(filename2, "%s\\%s%03d.bmp", dirname, images_root, fr+fradvance);
		}
		if (im_type==4){
			wsprintf(filename, "%s\\%s%04d.bmp", dirname, images_root, fr);
			wsprintf(filename2, "%s\\%s%04d.bmp", dirname, images_root, fr+fradvance);
		}
		if (im_type==5){
			wsprintf(filename, "%s\\%s%05d.bmp", dirname, images_root, fr);
			wsprintf(filename2, "%s\\%s%05d.bmp", dirname, images_root, fr+fradvance);
		}

		printf("\nFile name= %s", filename);

//Read image fr
		src=cvLoadImage(filename, 0);
//Smooth image (optional)
		//cvSmooth(src, src, CV_GAUSSIAN, 5, 5);
//Read image fr+1
		src2=cvLoadImage(filename2, 0);
//Smooth image (optional)
		//cvSmooth(src2, src2, CV_GAUSSIAN, 5, 5);	
//Set image ROI
		cvSetImageROI(src,roi);
		cvSetImageROI(src2,roi);

		matrice=src->imageData;
		n_row=src->height;
		n_col=src->width;
		band=src->nChannels;
		img8U[0]->imageData=matrice;
		cvCvtPlaneToPix(img8U[0], img8U[0],img8U[0],NULL, dst);
		
		printf("\nAnalysing ...\n");
//Defining pyramidal images
		curr_pyr=cvCreateImage(cvSize(src->width,src->height),8,1);
		prev_pyr=cvCreateImage(cvSize(src->width,src->height),8,1);
//Defining eigenvalues images
		eigImage = cvCreateImage(cvSize(src->width,src->height),IPL_DEPTH_32F,1);
		tempImage = cvCreateImage(cvSize(src->width,src->height),IPL_DEPTH_32F,1);
		cornerCount=maxcornerCount;
//Good feature to track identification	
		cvGoodFeaturesToTrack(src,eigImage,tempImage,corners,&cornerCount,qualityLevel,minDistance);

//Optical flow calculation		
		cvCalcOpticalFlowPyrLK(src, src2, prev_pyr, curr_pyr,corners, corners2, cornerCount, cvSize(winTrack,winTrack),pyrLev, status,
							NULL, cvTermCriteria (CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 3, 0.01), NULL );
		
		printf("\nNumber of features identified = %d\n", cornerCount);
		avanza=0;

		for (kk=0; kk<n_row; kk++){
			for(ll=0; ll<n_col; ll++){		
				img[kk+half_window_size+1][ll+half_window_size+1]=matrice[avanza];
				avanza++;
			}
		}

		for(i=0; i<cornerCount; i++){
			delta[i].x=corners2[i].x-corners[i].x;
			delta[i].y=corners2[i].y-corners[i].y;
			corners[i].x=corners[i].x+x_offset+half_window_size+1;
			corners[i].y=corners[i].y+y_offset+half_window_size+1;
		}
	
//  ---------------------------------------------------------------------------
//	  Calculation of particle barycentre using three point Gaussian fitting
//  ---------------------------------------------------------------------------	
	
		count=0;
		for(i=0; i<cornerCount; i++){
			max[i]=-1000;		
			for (kk=(corners[i].y-half_window_size); kk <= (corners[i].y+half_window_size) ; kk++){
				for (ll=(corners[i].x-half_window_size); ll <= (corners[i].x+half_window_size); ll++) {
					img_loc=(int)img[kk][ll];
					if (img_loc>max[i]) {
						max[i]=img_loc;
						max_loc[i].x=ll;
						max_loc[i].y=kk;
					}
				}
			}

			int	jj=max_loc[i].y;
			int	ii=max_loc[i].x;

			Dx=(2*log((float)img[jj][ii-1]+1)-4*log((float)img[jj][ii]+1)+2*log((float)img[jj][ii+1]+1));
			Dy=(2*log((float)img[jj-1][ii]+1)-4*log((float)img[jj][ii]+1)+2*log((float)img[jj+1][ii]+1));
			Nx=(log((float)img[jj][ii-1]+1)-log((float)img[jj][ii+1]+1));
			Ny=(log((float)img[jj-1][ii]+1)-log((float)img[jj+1][ii]+1));

			if (Dx!=0 && Dy!=0) {
				if (abs (Nx/Dx)<=3 && abs (Ny/Dy)<=3) { // Control of the barycentre coordinates with respect to the local maximum intensity
					centers[count].x=(max_loc[i].x+0.5f)+(Nx/Dx);
					centers[count].y=(max_loc[i].y+0.5f)+(Ny/Dy);
					deltaDEF[count].x=delta[i].x;
					deltaDEF[count].y=delta[i].y;
					count=count+1;
				}
			}
		}
//  ---------------------------------------------------------------------------

//  ---------------------------------------------------------------------------
//	  Discarding barycentre within minimum particle distance 'dpmin'
//  ---------------------------------------------------------------------------	

		for (k=0;k<count;k++) {		
			for (i=0;i<count;i++) {				
				if ((centers[k].x==centers[i].x && k!=i && centers[k].y==centers[i].y) || (abs(centers[k].x-centers[i].x)<dpmin && k!=i && abs(centers[k].y-centers[i].y)<dpmin) ){
					for (ss=i;ss<count-1;ss++) {
						centers[ss].x = centers[ss+1].x;
						centers[ss].y = centers[ss+1].y;
						deltaDEF[ss].x=deltaDEF[ss+1].x;
						deltaDEF[ss].y=deltaDEF[ss+1].y;
					}
					count=count-1; 
					i=i-1;
				}
			}
		}
//  ---------------------------------------------------------------------------

		printf("\nNumber of barycentres identified= %d\n", count);

		fwrite(&count, sizeof(ushort), 1, file); // writing number of barycentres identified


//  ---------------------------------------------------------------------------
//	  Output image visualization
//  ---------------------------------------------------------------------------	

		cvRectangle(dst, cvPoint(x_offset,y_offset), cvPoint(x_offset+width,y_offset+height), CV_RGB(0,0,255), 1 );
		cvShowImage("Output",dst);
		cvWaitKey(1);
		
		spotnumber = 0;
		for(i=0;i<count;i++){
			x_coord[scritti][spotnumber]=centers[i].x-half_window_size-1;
			y_coord[scritti][spotnumber]=centers[i].y-half_window_size-1;
			x_coord2[scritti][spotnumber]=x_coord[scritti][spotnumber]+deltaDEF[i].x;
			y_coord2[scritti][spotnumber]=y_coord[scritti][spotnumber]+deltaDEF[i].y;
			spotnumber++;
		}
		bar_per_frame[scritti]=spotnumber;
		for(i=0; i<=scritti; i++){
			for(j=0; j<bar_per_frame[i]; j++){
				punto.x=x_coord[i][j];
				punto.y=y_coord[i][j];
				punto2.x=x_coord2[i][j];
				punto2.y=y_coord2[i][j];
				//cvCircle(dst, punto, 1, CV_RGB(255,(255/(comet-1)*i),0), -1);  //CV_FILLED
				cvLine(dst,punto,punto2,CV_RGB(255,(255/(comet-1)*i),0),1,8,0);
			}		
		}
		cvShowImage("Output",dst);
		cvWaitKey(1); 
		cvWriteFrame(writer, dst);
		scritti++;
		if(scritti==comet){
			for(i=1; i<scritti; i++){
				for(j=0; j<bar_per_frame[i]; j++){
					x_coord[i-1][j]=x_coord[i][j];
					y_coord[i-1][j]=y_coord[i][j];
					x_coord2[i-1][j]=x_coord2[i][j];
					y_coord2[i-1][j]=y_coord2[i][j];
				}
				bar_per_frame[i-1]=bar_per_frame[i];
			}
			scritti=comet-1;
		}

//  ---------------------------------------------------------------------------

//  ---------------------------------------------------------------------------
//	  Write data output in .feb & .ofp files
//  ---------------------------------------------------------------------------	

		for(i=0; i<count; i++){
			x_float = (centers[i].x-half_window_size-1);
			y_float = (centers[i].y-half_window_size-1);
			if(x_float<x_min)
				x_float=(float)x_min;
			if(x_float>x_max)
				x_float=(float)x_max;
			if(y_float<y_min)
				y_float=(float)y_min;
			if(y_float>y_max)
				y_float=(float)y_max;
			xp=x_float+deltaDEF[i].x;
			yp=y_float+deltaDEF[i].y;
			fwrite(&(xp), sizeof(float),1,outfile);
			fwrite(&(yp), sizeof(float),1,outfile);
			fwrite(&x_float,sizeof(float),1,file);
			fwrite(&y_float,sizeof(float),1,file);			
		}
  // --------------------------------------------------------------------------

  // --------------------------------------------------------------------------
  //   Closing
  // --------------------------------------------------------------------------		
		cvReleaseImage(&src2);	
		cvReleaseImage(&src);
		cvReleaseImage(&eigImage);
		cvReleaseImage(&tempImage);
		cvReleaseImage(&curr_pyr);
		cvReleaseImage(&prev_pyr);
		corners->x=corners2->x;
		corners->y=corners2->y;
		free(corners2);
		free(delta);
		free(deltaDEF);
		free(centers);
		free(max_loc);
	}

	fclose(file);
    fclose(outfile);
	cvReleaseVideoWriter(&writer);

	for (i=0;i<n_row_enlarged;i++) {
   		free(img[i]);		
	}
	free(img);	

	for(i=0;i<comet;i++) {
		free(x_coord[i]);
		free(y_coord[i]);
	}
	free(x_coord);
	free(y_coord);
	free(bar_per_frame);
	
}
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

