#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <map>

using namespace cv;
using namespace std;

const int bits = 1;
const int kernelSize = 15;
const float proportion = 0.65;

Mat binarizacion(Mat imagen);
Mat intensidad(int cantb, Mat image);
Mat filtro(Mat image, int kernelSize);
Mat resaltarTableroFilas(Mat image);
Mat resaltarTableroColumnas(Mat image);

int main(int argc, char** argv )
{
	//Reportar error en parámetros
  if( argc != 2 ){
    cerr << "Uso: " << argv[ 0 ] << "nombre_imagen.extensión" << endl;
    return( -1 );

  }

  //Mostrar parámetros recibidos
  cout << "-------------------------" << endl;
  for( int a = 0; a < argc; a++ )
    cout << argv[ a ] << endl;
 	cout << "-------------------------" << endl;

  //Leer imagen
  Mat image;
  image = imread( argv[1], 1 );

  if ( !image.data ){
    cerr << "Error: No image data" << endl;
    return( -1);
  }

  cout << "Image input size: " << image.size() << endl;
  cout << "rows: " << image.rows << endl;
  cout << "columns: " << image.cols << endl;

  //Tratamiento de imagen
  //Binarización
  Mat res_img = binarizacion(image);
  //Segmentación
  res_img = resaltarTableroFilas(res_img);
  res_img = resaltarTableroColumnas(res_img);

  //Escribir imagen
  stringstream ss( argv[ 1 ] );
  string basename;
  getline( ss, basename, '.' );

  imwrite( basename + "_resultado.png", res_img );

  return( 0 );
}

Mat binarizacion(Mat image){
	Mat grey = Mat::zeros(image.size(), CV_8UC1);
	map<int,int> histogram;

	for (size_t i = 0; i < image.rows; i++){
		  for (size_t j = 0; j < image.cols; j++){
		      Vec3b pixel = image.at<Vec3b>(i, j);

		      uchar B = pixel[0];
		      uchar G = pixel[1];
		      uchar R = pixel[2];

					grey.at<uchar>(i, j) = (B + G + R) / 3;
					histogram[(B + G + R) / 3] = histogram[(B + G + R) / 3] + 1;
			}
	}

	//Limpiar
  grey = filtro(grey, kernelSize);

	int sum = 0;
	int div = image.cols*image.rows;
	for(map<int, int>::iterator it = histogram.begin(); it!=histogram.end(); it++){
		//cout << it->first << " " << it->second << endl;
		sum = it->first * it->second + sum;
	}
	float limite = sum/div;
	cout << "limite " << limite << endl;

	for (size_t i = 0; i < grey.rows; i++){
		  for (size_t j = 0; j < grey.cols; j++){
		      uchar pixel = grey.at<uchar>(i, j);

		      uchar G = pixel;

		      if(G > limite){
		      	grey.at<uchar>(i, j) = 255;
		      }else{
		      	grey.at<uchar>(i, j) = 0;
		      }
			}
	}
/*
	namedWindow("Binarizacion", WINDOW_AUTOSIZE);
	imshow("Binarizacion", grey);
	waitKey(0);
	destroyWindow("Binarizacion");
	*/
	return(grey);
}

Mat filtro(Mat image, int kernelSize){
	Mat dst, kernel;
  Point anchor;
  double delta;
  int depth;

  //filtro 2D
  depth = -1; //Igual que la imagen de origen (espesor)
  delta = 0;
  anchor = Point(-1,-1);

	kernel = Mat::ones( kernelSize, kernelSize, CV_32F )/ (float)(kernelSize*kernelSize);

  filter2D(image, dst, depth , kernel, anchor, delta, BORDER_DEFAULT );

	return(dst);
}

Mat resaltarTableroFilas(Mat image){
	Mat newImg = Mat::zeros(image.size(), CV_8UC1);
	float white = 0;

	for (size_t i = 0; i < image.rows; i++){
	  for (size_t j = 0; j < image.cols; j++){
	      uchar pixel = image.at<uchar>(i, j);

	      uchar G = pixel;

	      if(G == 255){
	      	white++;
	      }
		}
		//cout << "white " << white << " image.cols: " << image.cols << " result: " << (float)(white/image.cols) << endl;
		if((float)(white/image.cols) < proportion){
			for (size_t jj = 0; jj < image.cols; jj++){
		  	newImg.at<uchar>(i, jj) = 255;
			}
		}else{
			for (size_t jj = 0; jj < image.cols; jj++){
				uchar pixel = image.at<uchar>(i, jj);

	      uchar G = pixel;
		  	newImg.at<uchar>(i, jj) = G;
			}
		}
		white = 0;
	}
	return(newImg);
}

Mat resaltarTableroColumnas(Mat image){
	imwrite( "lol.png", image );
	Mat newImg = Mat::zeros(image.size(), CV_8UC1);
	float white = 0;

	for (size_t i = 0; i < image.cols; i++){
	  for (size_t j = 0; j < image.rows; j++){
	      uchar pixel = image.at<uchar>(j, i);

	      uchar G = pixel;

	      if(G == 255){
	      	white++;
	      }
		}
		//cout << "white " << white << " image.cols: " << image.cols << " result: " << (float)(white/image.cols) << endl;
		if((float)(white/image.rows) < proportion){
			for (size_t jj = 0; jj < image.rows; jj++){
		  	newImg.at<uchar>(jj, i) = 255;
			}
		}else{
			for (size_t jj = 0; jj < image.rows; jj++){
				uchar pixel = image.at<uchar>(jj, i);

	      uchar G = pixel;
		  	newImg.at<uchar>(jj, i) = G;
			}
		}
		white = 0;
	}
	imwrite( "lol2.png", newImg );
	return(newImg);
}

Mat identificarRenglones(Mat image){
	Mat newImg = Mat::zeros(image.size(), CV_8UC1);
	float white = 0;

	for (size_t i = 0; i < image.rows; i++){
	  for (size_t j = 0; j < image.cols; j++){
	      uchar pixel = image.at<uchar>(i, j);

	      uchar G = pixel;

	      if(G == 255){
	      	white++;
	      }
		}
		//cout << "white " << white << " image.cols: " << image.cols << " result: " << (float)(white/image.cols) << endl;
		if((float)(white/image.cols) < proportion){
			for (size_t jj = 0; jj < image.cols; jj++){
		  	newImg.at<uchar>(i, jj) = 255;
			}
		}else{
			for (size_t jj = 0; jj < image.cols; jj++){
				uchar pixel = image.at<uchar>(i, jj);

	      uchar G = pixel;
		  	newImg.at<uchar>(i, jj) = G;
			}
		}
		white = 0;
	}
	return(newImg);
}