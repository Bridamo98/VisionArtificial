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
void revisarVecinos(Coord aux, queue<Coord> &cola, Mat &intermedia, Mat image,
                    map<int, int> &mapa, int contador, int &contadorPixeles);
void descarteDeRegiones(Mat &intermedia,map<int,int > &mapa,Mat &image);
Mat ajusteDeIntensidades( Mat intermedia, int cantRegiones);
void centros(Mat dist, Mat intermedia, map<int, int> mapa);

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

	descarteDeRegiones(intermedia,mapa,image);

	Mat ajustada;
	ajustada = ajusteDeIntensidades(intermedia, mapa.size());

	Mat dist;
	distanceTransform(image, dist, DIST_L2, 3);  //toca corregir esto NO ES INTERMEDIA

	cout << "Los centros de las regiones son: " << endl;

	centros(dist, intermedia, mapa);

	imwrite( basename + "_ajustada.png", ajustada );
	imwrite( basename + "_dist.png", dist );

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
			if(image.at<uchar>(i, j) == 255 && intermedia.at<uchar>(i, j) == 0) {//cambie 255 por 0 y agregue la condicion de intermedia
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

void descarteDeRegiones(Mat &intermedia, map<int,int> &mapa, Mat &image){
	int maxCant=-1;
	for (auto itr = mapa.begin(); itr != mapa.end(); ++itr) {
		if(itr->second > maxCant) {
			maxCant = itr->second;
		}
	}

	for (int i = 0; i < intermedia.rows; i++) {
		for (int j = 0; j < intermedia.cols; j++) {
			auto it=mapa.find((int)(intermedia.at<uchar>(i, j)));
			if((double)(it->second) <= (double)(maxCant*0.15) && intermedia.at<uchar>(i, j) != 0
			   && it != mapa.end()) {
				if(it->second != 0) {
					mapa.erase(it);
					mapa.insert({ (int)(intermedia.at<uchar>(i, j)), 0 });
				}
				intermedia.at<uchar>(i, j)=0;
				image.at<uchar>(i, j)=0;
			}
		}
	}

	for (auto itr = mapa.begin(); itr != mapa.end(); ++itr) {
		if(itr->second > 0)
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

void revisarVecinos(Coord aux, queue<Coord> &cola, Mat &intermedia, Mat image,
                    map<int, int> &mapa, int contador, int &contadorPixeles){
	Coord vecino;

	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			vecino.x = aux.x+i;
			vecino.y = aux.y+j;
			if(vecino.x > -1 && vecino.y > -1 && vecino.x < image.rows && vecino.y < image.cols &&
			   intermedia.at<uchar>(vecino.x, vecino.y) == 0 && image.at<uchar>(vecino.x, vecino.y) == 255) {//cambie 255 por 0 y agregue condicion de image, corregí las coordenadas(vecino)
				cola.push(vecino);
				contadorPixeles++;
				intermedia.at<uchar>(vecino.x, vecino.y) = contador;
			}
		}
	}
}

void centros(Mat dist, Mat intermedia, map<int, int> mapa){
	map<int, Coord> mapaCentros;

	for (map<int,int>::iterator it=mapa.begin(); it!=mapa.end(); ++it) {
		if(it->second > 0) {
			int max = -1;
			int promF = 0, promC = 0;
			int count = 0;
			Coord coord;

			for (int i = 0; i < intermedia.rows; i++) {
				for (int j = 0; j < intermedia.cols; j++) {
					if((int)(intermedia.at<uchar>(i, j)) == it->first) {
						if((int)(dist.at<uchar>(i, j)) > max) {
							max = (int)(dist.at<uchar>(i, j));
							promF = i;
							promC = j;
							count = 1;
						}else if((int)(dist.at<uchar>(i, j)) == max) {
							promF = promF+i;
							promC = promC+j;
							count++;
						}
					}
				}
			}
			coord.x = promC/count;
			coord.y = promF/count;
			mapaCentros.insert({it->first, coord});
		}
	}

	for (map<int,Coord>::iterator it=mapaCentros.begin(); it!=mapaCentros.end(); ++it) {
		cout << "Para la region " << it->first << " el centro está en la fila "
		     << it->second.x << " columna " << it->second.y << endl;
	}
}
