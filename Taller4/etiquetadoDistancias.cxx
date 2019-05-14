#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <queue>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <iterator>
#include <map>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

struct Coord {
	int x;
	int y;
};

Mat etiquetado(Mat image, String basename);
int promedio(Coord coord, Mat image);
void revisarVecinos(Coord aux, queue<Coord> &cola, Mat &intermedia, Mat image,
	 map<int, int> &mapa, int contador);
bool verificar(Coord vecino, int prom, int t, Mat image, Mat intermedia);
Mat generarRest(Mat image, Mat intermedia);

int main(int argc, char** argv )
{
	// Get command line arguments
	if ( argc < 2 )
	{
		std::cerr << "Usage: " << argv[ 0 ] << " image_file" << std::endl;
		return( -1 );

	} // fi

	// Review given command line arguments
	std::cout << "-------------------------" << std::endl;
	for( int a = 0; a < argc; a++ )
		std::cout << argv[ a ] << std::endl;
	std::cout << "-------------------------" << std::endl;

	// Read an image
	Mat image;
	image = imread( argv[1], IMREAD_GRAYSCALE );

	if ( !image.data )
	{
		std::cerr << "Error: No image data" << std::endl;
		return( -1);
	}

	stringstream ss( argv[ 1 ] );
	string basename;
	getline( ss, basename, '.' );

	Mat intermedia;

	intermedia = etiquetado(image, basename);

	imwrite( basename + "_intermedia.png", intermedia );
	//imwrite( basename + "_regiones.png", generarRest(image, intermedia) );

	return( 0 );
}

Mat etiquetado(Mat image, String basename){
	unsigned int cantidad;
	queue<Coord> cola;
	map<int, int> mapa;
	int contador = 254;
	Coord aux;
	Mat intermedia = Mat::zeros( image.size( ), CV_8UC1 );
	Mat rest = Mat::zeros( image.size( ), CV_8UC1 );

	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			if(image.at<uchar>(i, j) == 255){
				intermedia.at<uchar>(i, j) = contador;
				aux.x = i;
				aux.y = j;
				cola.push(aux);
				while (!cola.empty()) {
					aux = cola.front();
					cola.pop();
					revisarVecinos(aux, cola, intermedia, image, mapa, contador);
				}
				contador--;
			}
		}
	}

	return (intermedia);
}

int promedio(Coord coord, Mat image){
	Coord vecino;
	int prom = 0, contador = 0;

	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			vecino.x = coord.x+i;
			vecino.y = coord.y+j;
			if(vecino.x > -1 && vecino.y > -1 && vecino.x < image.rows && vecino.y < image.cols) {
				prom = prom + (int)image.at<uchar>(vecino.x, vecino.y);
				contador++;
			}
		}
	}
	if(coord.x > -1 && coord.y > -1 && coord.x < image.rows && coord.y < image.cols) {
		prom = prom - (int)image.at<uchar>(coord.x, coord.y);
		return (prom/(contador-1));
	}else{
		return (prom/contador);
	}
}

void revisarVecinos(Coord aux, queue<Coord> &cola, Mat &intermedia, Mat image,
	 map<int, int> &mapa, int contador){
	Coord vecino;

	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			vecino.x = aux.x+i;
			vecino.y = aux.y+j;

			if(intermedia.at<uchar>(i, j) == 255) {
				cola.push(vecino);
				intermedia.at<uchar>(vecino.x, vecino.y) = contador;
			}
		}
	}
}

bool verificar(Coord vecino, int prom, int t, Mat image, Mat intermedia){
	int calculo;
	//calculo = prom - (int)image.at<uchar>(vecino.x, vecino.y);
	calculo = prom - promedio(vecino, image);
	if(calculo < 0)
		calculo = calculo*(-1);

	if(vecino.x > -1 && vecino.y > -1 && vecino.x < image.rows && vecino.y < image.cols) {
		if(((int)intermedia.at<uchar>(vecino.x, vecino.y) == 0) && (calculo < t)) {
			return true;
		}
	}
	return false;
}

Mat generarRest(Mat image, Mat intermedia){
	Mat rest = Mat::zeros( image.size(), CV_8UC3 );

	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			if((int)intermedia.at<uchar>(i, j) == 255) {
				rest.at<Vec3b>(i, j) = image.at<Vec3b>(i, j);
			}
		}
	}
	return rest;
}
