#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <queue>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <iterator>
#include <map>
#include <math.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

struct Coord {
	int x;
	int y;
};

Mat etiquetado(Mat image, String basename,map<int,int> &mapa);
int promedio(Coord coord, Mat image);
void revisarVecinos(Coord aux, queue<Coord> &cola, Mat &intermedia, Mat image,
	 map<int, int> &mapa, int contador, int &contadorPixeles);
bool verificar(Coord vecino, int prom, int t, Mat image, Mat intermedia);
Mat generarRest(Mat image, Mat intermedia);
void descarteDeRegiones(Mat &intermedia,map<int,int >mapa);
Mat ajusteDeIntensidades( Mat intermedia, int cantRegiones);

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

	map<int,int> mapa;//contiene el numero de la region y la cantidad de pixeles que la componen

	cout<<"Luego del etiquetado se tienen las siguientes regiones: "<<endl;	

	intermedia = etiquetado(image, basename, mapa);

	cout<<"Luego del descarte se tienen las siguientes regiones: "<<endl;

	descarteDeRegiones(intermedia,mapa);


	Mat ajustada;
	ajustada = ajusteDeIntensidades(intermedia, mapa.size());

	//resalta=resaltarBlanco(intermedia);

	Mat dist;
    distanceTransform(image, dist, DIST_L2, 3);//toca corregir esto NO ES INTERMEDIA


	imwrite( basename + "_ajustada.png", ajustada );
	imwrite( basename + "_dist.png", dist );

	//imwrite( basename + "_regiones.png", generarRest(image, intermedia) );

	return( 0 );
}

Mat etiquetado(Mat image, String basename, map<int,int> &mapa){
	unsigned int cantidad;
	queue<Coord> cola;
	int contador = 1;
	int contadorPixeles = 0;
	Coord aux;
	Mat intermedia = Mat::zeros( image.size( ), CV_8UC1 );
	Mat rest = Mat::zeros( image.size( ), CV_8UC1 );
	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			if(image.at<uchar>(i, j) == 0 && intermedia.at<uchar>(i, j) == 0){//cambie 255 por 0 y agregue la condicion de intermedia
				intermedia.at<uchar>(i, j) = contador;
				aux.x = i;
				aux.y = j;
				cola.push(aux);
				contadorPixeles++;
				while (!cola.empty()) {
					aux = cola.front();
					cola.pop();
					revisarVecinos(aux, cola, intermedia, image, mapa, contador, contadorPixeles);
				}
				cout<<"Para la region ("<<contador<<") se reconocieron ("<<contadorPixeles<<") pixeles"<<endl;
				mapa.insert({ contador, contadorPixeles });
				contadorPixeles=0; 
				contador++;
			}
		}
	}

	return (intermedia);
}

void descarteDeRegiones(Mat &intermedia, map<int,int> mapa){
	int maxCant=-1;
	for (auto itr = mapa.begin(); itr != mapa.end(); ++itr) {
		 //cout<<"itr->first "<<itr->second<<endl;
         if(itr->second > maxCant){
         	maxCant = itr->second;
         }
    }
    //cout<<"maxCant"<<(double)(maxCant*0.15)<<endl;
    

    for (int i = 0; i < intermedia.rows; i++) {
		for (int j = 0; j < intermedia.cols; j++) {
			auto it=mapa.find((int)(intermedia.at<uchar>(i, j)));
			if((double)(it->second) <= (double)(maxCant*0.15) && intermedia.at<uchar>(i, j) != 0){
				intermedia.at<uchar>(i, j)=0;
				cout<<"entra"<<endl;
				mapa.erase(it);
			}
		}
	}

	for (auto itr = mapa.begin(); itr != mapa.end(); ++itr) {
		cout<<"Para la region ("<<itr->first<<") se reconocieron ("<<itr->second<<") pixeles"<<endl;
    }

}

Mat ajusteDeIntensidades(Mat intermedia, int cantRegiones){

	Mat ajustada = Mat::zeros( intermedia.size( ), CV_8UC1 );
	int factorDiferencial = floor(255/cantRegiones);
	for (int i = 0; i < intermedia.rows; i++) {
		for (int j = 0; j < intermedia.cols; j++) {
			ajustada.at<uchar>(i, j)=intermedia.at<uchar>(i, j)*((uchar)(factorDiferencial));
		}
	}
	return ajustada;
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
	 map<int, int> &mapa, int contador, int &contadorPixeles){
	Coord vecino;

	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			vecino.x = aux.x+i;
			vecino.y = aux.y+j;

			if(intermedia.at<uchar>(vecino.x, vecino.y) == 0 && image.at<uchar>(vecino.x, vecino.y) == 0) {//cambie 255 por 0 y agregue condicion de image, corregí las coordenadas(vecino)
				cola.push(vecino);
				contadorPixeles++;
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
