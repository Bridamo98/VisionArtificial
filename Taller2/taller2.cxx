#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

Mat filtradoGaussianoColor(Mat image, float** kernel, int tam );
Mat filtradoGaussianoGris(Mat image, float** kernel, int tam );
int main(int argc, char** argv )
{
  // Get command line arguments
  if ( argc < 2 )
  {
  	std::cerr << "Usage: " << argv[ 0 ] << " image_file"<< std::endl;
    return( -1 );

  } // fi

  // Review given command line arguments
  std::cout << "-------------------------" << std::endl;
  for( int a = 0; a < argc; a++ )
    std::cout << argv[ a ] << std::endl;
  std::cout << "-------------------------" << std::endl;

  // Read an image
  Mat image;
  image = imread( argv[1], 1 );

  if ( !image.data )
  {
    std::cerr << "Error: No image dataaaa" << std::endl;
    return( -1);
  }

  float** kernel=new float*[3];
  for(int i=0;i<3;i++){
    kernel[i]=new float[3];
  }
  kernel[0][0]=1/16;
  kernel[0][1]=2/16;
  kernel[0][2]=1/16;
  kernel[1][0]=2/16;
  kernel[1][1]=4/16;
  kernel[1][2]=2/16;
  kernel[2][0]=1/16;
  kernel[2][1]=2/16;
  kernel[2][2]=1/16;
  Mat image2=filtradoGaussianoGris(image, kernel, 3);
  //Mat image2=filtradoGaussianoColor(image, kernel, 3);
  stringstream ss( argv[ 1 ] );
  string basename;
  getline( ss, basename, '.' );
  imwrite( basename + "_gauss.png", image2 );
  
}

Mat filtradoGaussianoColor(Mat image, float** kernel,int tam){
  Mat dstc(image.size(), CV_8UC3);

  for (int i = 0; i < image.rows; i++)
  {
      for (int j = 0; j < image.cols; j++)
      {
          Vec3b pixel2;
          uchar B = 0;
          uchar G = 0;
          uchar R = 0;
          int ki=0;
          int kj=0;
          for (int ii = i-tam/2; ii < i+tam/2; ++ii)
          {
            for (int jj = j-tam/2; jj < j+tam/2; ++jj)
            {
                if(ii>=0 && jj>=0){
                  Vec3b pixel = image.at<Vec3b>(ii,jj);
                  uchar BB=pixel[0];
                  uchar GG=pixel[1];
                  uchar RR=pixel[2];
                  B=B+BB*kernel[ki][kj];
                  G=G+GG*kernel[ki][kj];
                  R=R+RR*kernel[ki][kj];
                  kj++;  
                }
                
            }
            //cout<<"joder 2"<<endl;
            kj=0;
            ki++;
          }
          dstc.at<Vec3b>(i, j) = {B, G, R};
          
      }
  }
  return dstc;
}
Mat filtradoGaussianoGris(Mat image, float** kernel, int tam ){

  cout<<"joder 1"<<endl;

  Mat dstc = Mat::zeros(image.size(), CV_8UC1);
  cout<<"joder 2"<<endl;
  for (int i = 0; i < image.rows; i++)
  {
      //cout<<"joder 3"<<endl;
      for (int j = 0; j < image.cols; j++)
      {
          //cout<<"joder 4"<<endl;
          //Vec3b pixel2;
          uchar B = 0;
          uchar G = 0;
          uchar R = 0;
          int ki=0;
          int kj=0;
          //cout<<"joder 5"<<endl;
          for (int ii = i-tam/2; ii < i+tam/2; ++ii)
          {
            for (int jj = j-tam/2; jj < j+tam/2; ++jj)
            {
                if(ii>=0 && jj>=0){
                  Vec3b pixel = image.at<Vec3b>(ii,jj);
                  uchar BB=pixel[0];
                  uchar GG=pixel[1];
                  uchar RR=pixel[2];
                  B=B+BB*kernel[ki][kj];
                  G=G+GG*kernel[ki][kj];
                  R=R+RR*kernel[ki][kj];
                  kj++;  
                }
            }
            kj=0;
            ki++;
          }
          //cout<<"joder 6"<<endl;
          dstc.at<uchar>(i, j) = (B+G+R)/3;
      }
  }
  cout<<"joder 7"<<endl;
  return dstc;
}