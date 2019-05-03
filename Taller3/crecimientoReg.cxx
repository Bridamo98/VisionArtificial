#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <queue>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

struct Coord {
	int x;
	int y;
};

Mat crecimientoRegiones(Mat image, int t, String basename);
int promedio(Coord coord, Mat image);
void revisarVecinos(Coord aux, int prom, int t, queue<Coord> &cola, Mat &intermedia, Mat image);
bool verificar(Coord vecino, int prom, int t, Mat image, Mat intermedia);
Mat generarRest(Mat image, Mat intermedia);

int main(int argc, char** argv )
{
	// Get command line arguments
	if ( argc < 3 )
	{
		std::cerr << "Usage: " << argv[ 0 ] << " image_file T" << std::endl;
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
		std::cerr << "Error: No image data" << std::endl;
		return( -1);
	}

	stringstream ss( argv[ 1 ] );
	string basename;
	getline( ss, basename, '.' );

	Mat grey;
	Mat rest;
	Mat intermedia;
	cvtColor(image, grey, COLOR_BGR2GRAY);

	intermedia = crecimientoRegiones(grey, atoi(argv[2]), basename);

	imwrite( basename + "_intermedia.png", intermedia );
	imwrite( basename + "_region.png", generarRest(image, intermedia) );

	return( 0 );
}

Mat crecimientoRegiones(Mat image, int t, String basename){
	unsigned int cantidad;
	queue<Coord> cola;
	Coord semilla, aux;
	Mat intermedia = Mat::zeros( image.size( ), CV_8UC1 );
	Mat rest = Mat::zeros( image.size( ), CV_8UC1 );
	int prom;
	char respuesta;

	cout << intermedia.rows << " " << intermedia.cols << endl;

	cout << "Ingrese la cantidad de semillas: ";
	cin >> cantidad;
	do {
		cout << endl << "¿Generar semillas automáticas? s/n: ";
		cin >> respuesta;
		if(respuesta == 's') {
			for (unsigned int i = 0; i < cantidad; i++) {
				srand(time(NULL));
				semilla.x = rand()%(image.rows);
				semilla.y = rand()%(image.cols);
				usleep(100);
				cola.push(semilla);
			}
		}else if(respuesta == 'n') {
			for (unsigned int i = 0; i < cantidad; i++) {
				cout << endl << "Fila: ";
				cin >> semilla.x;
				cout << endl << "Columna: ";
				cin >> semilla.y;
				cola.push(semilla);
			}
		}else{
			cout << "Respuesta inválida" << endl;
		}
	} while(respuesta != 's' && respuesta != 'n');

	while(!cola.empty()) {
		aux = cola.front();
		cola.pop();
		intermedia.at<uchar>(aux.x, aux.y) = (uchar)255;
		//prom = image.at<uchar>(aux.x, aux.y);
		prom = promedio(aux, image);
		revisarVecinos(aux, prom, t, cola, intermedia, image);
	}
	cout << endl;

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

void revisarVecinos(Coord aux, int prom, int t, queue<Coord> &cola, Mat &intermedia, Mat image){
	Coord vecino;

	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			vecino.x = aux.x+i;
			vecino.y = aux.y+j;

			if(verificar(vecino, prom, t, image, intermedia)) {
				cola.push(vecino);
				intermedia.at<uchar>(vecino.x, vecino.y) = 177;
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
