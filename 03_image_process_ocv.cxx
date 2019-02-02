#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <opencv2/opencv.hpp>

#define rsize0 0.25
#define rsize1 0.25
#define rsize2 0.25

using namespace cv;
using namespace std;

int main(int argc, char** argv )
{
  if ( argc < 2 )
  {
  	cerr << "Usage: " << argv[ 0 ] << " image_file" << endl;
    return( -1 );
  }

  Mat image;
  image = imread( argv[1], 1 );

  if ( !image.data )
  {
    cerr << "Error: No image data" << endl;
    return( -1);
  }


//--------------------------------------
	vector<Mat> channels;
 	split(image, channels);

	Mat zero = Mat::zeros(image.size(), CV_8UC1);

 	vector<Mat> B = { channels[0], zero, zero };
	vector<Mat> G = { zero, channels[1], zero };
 	vector<Mat> R = { zero, zero, channels[2] };

	Mat rImg, gImg, bImg;

	merge(R, rImg);
	merge(G, gImg);
	merge(B, bImg);

  stringstream ss( argv[ 1 ] );
  string basename;
  getline( ss, basename, '.' );

  imwrite( basename + "_R.png", rImg );
  imwrite( basename + "_G.png", gImg );
  imwrite( basename + "_B.png", bImg );
//--------------------------------------


  Mat res_imgR, res_imgG, res_imgB;
  resize(rImg, res_imgR, Size(), rsize0, rsize0, INTER_LINEAR);
  resize(gImg, res_imgG, Size(), rsize1, rsize1, INTER_LINEAR);
  resize(bImg, res_imgB, Size(), rsize2, rsize2, INTER_LINEAR);

  imwrite( basename + "_sR.png", res_imgR );
  imwrite( basename + "_sG.png", res_imgG );
  imwrite( basename + "_sB.png", res_imgB );

  Mat rres_imgR, rres_imgG, rres_imgB;
  resize(res_imgR, rres_imgR, Size(), (1/rsize0), (1/rsize0), INTER_LINEAR);
  resize(res_imgG, rres_imgG, Size(), (1/rsize1), (1/rsize1), INTER_LINEAR);
  resize(res_imgB, rres_imgB, Size(), (1/rsize2), (1/rsize2), INTER_LINEAR);

  imwrite( basename + "_ssR.png", rres_imgR );
  imwrite( basename + "_ssG.png", rres_imgG );
  imwrite( basename + "_ssB.png", rres_imgB );
//--------------------------------------


	vector<Mat> channels1, channels2, channels3;
 	split(rres_imgR, channels1);
 	split(rres_imgG, channels2);
 	split(rres_imgB, channels3);

 	vector<Mat> resV = { channels3[0], channels2[1] , channels1[2] };

	Mat restoreImg(image.size(), CV_8UC3);

	merge(resV, restoreImg);

  imwrite( basename + "_rRGB.png", restoreImg );
//--------------------------------------


	Mat dif(restoreImg.size(), CV_8UC3);

	cout << "cols ori " << image.cols << "rows ori " << image.rows << endl;
	cout << "cols res " << restoreImg.cols << "rows res " << restoreImg.rows << endl;

	for (size_t i = 0; i < restoreImg.rows; i++)
	{
		  for (size_t j = 0; j < restoreImg.cols; j++)
		  {
		      Vec3b pixel = image.at<Vec3b>(i, j);
		      Vec3b pixel1 = restoreImg.at<Vec3b>(i, j);

		      uchar B = abs(pixel[0]-pixel1[0]);
		      uchar G = abs(pixel[1]-pixel1[1]);
		      uchar R = abs(pixel[2]-pixel1[2]);

					dif.at<Vec3b>(i, j) = {B, G, R};
		  }
	}

	cout << "cols dif " << dif.cols << "rows dif " << dif.rows << endl;

  imwrite( basename + "_diff.png", dif );
//--------------------------------------
/**
	Mat dif;
	absdiff(image, restoreImg, dif);
  imwrite( basename + "_diff.png", dif );
**/
	return 0;
}
