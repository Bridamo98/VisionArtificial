#include <stdio.h>
#include <map>
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <queue>
#include <stdlib.h>
#include <unistd.h>
#include <iterator>
#include <map>
#include <math.h>
#include <limits>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

struct Coord {
	int x;
	int y;
};

struct Bordes {
	int sf;
	int inf;
	int sc;
	int ic;
};

const int bits = 1;
const int kernelSize = 15;
const float proportion = 0.65;
int IDe = 0;

void comparar(Coord* coordCentros);
float calificarDif(Mat image, Mat plantilla);
Mat resaltarTableroFilas(Mat image);
Mat resaltarTableroColumnas(Mat image);
void crearMatriz(Mat image, int supf, int inff, int supc, int infc, int ident);
Mat blancoNegro(Mat restoreImg);
Mat etiquetado(Mat image, String basename,map<int,int> &mapa);
void descarteDeRegiones(Mat &intermedia,map<int,int > &mapa,Mat &image);
Mat ajusteDeIntensidades( Mat intermedia, int cantRegiones);
void revisarVecinos(Coord aux, queue<Coord> &cola, Mat &intermedia, Mat image, map<int, int> &mapa, int contador, int &contadorPixeles);
void segmentar(Mat dist, Mat intermedia, map<int, int> mapa, Mat image, Coord* coordCentros);
void mostrarImagen(Mat image, int num){
	String s = to_string(num);
	namedWindow("Window " + s, WINDOW_NORMAL);
	imshow("Window " + s, image);
	waitKey(0);
}


int main(int argc, char** argv )
{
	//Reportar error en parámetros
	if( argc != 2 ) {
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

	if ( !image.data ) {
		cerr << "Error: No image data" << endl;
		return( -1);
	}

	cout << "Tamaño de la imagen: " << image.size() << endl;
	cout << "Filas: " << image.rows << endl;
	cout << "Columnas: " << image.cols << endl;

	stringstream ss( argv[ 1 ] );
	string basename;
	getline( ss, basename, '.' );

	//Tratamiento de imagen
	//Binarización
	Mat res_img, kernel, intermedia;
	Mat ajustada, dist;
	map<int,int> mapa;

	res_img = blancoNegro(image);

	threshold(res_img, res_img, 177, 255, THRESH_OTSU);

	kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
	{//APERTURA
		erode(res_img, res_img, kernel);
		dilate(res_img, res_img, kernel);
	}

	//dilate(res_img, res_img, kernel);
	erode(res_img, res_img, kernel);
	//erode(res_img, res_img, kernel);
	//erode(res_img, res_img, kernel);

	//GaussianBlur(res_img, res_img, Size(33, 33), 0);
	GaussianBlur(res_img, res_img, Size(11, 11), 0);
	GaussianBlur(res_img, res_img, Size(11, 11), 0);

	threshold(res_img, res_img, 177, 255, THRESH_OTSU);

	//Segmentación
	char respuesta;
	cout << "¿Hay superficies externas al tablero?(s/n)" << endl;
	do{
			cin >> respuesta;
	}while(respuesta!='s' && respuesta!='n');
	if(respuesta == 's'){
		res_img = resaltarTableroFilas(res_img);
		res_img = resaltarTableroColumnas(res_img);
	}
	threshold(res_img, res_img, 177, 255, THRESH_BINARY_INV);

	intermedia = etiquetado(res_img, basename, mapa);
	descarteDeRegiones(intermedia,mapa,res_img);

	ajustada = ajusteDeIntensidades(intermedia, mapa.size());

	distanceTransform(res_img, dist, DIST_L2, 3);
	Coord* coordCentros=new Coord[mapa.size()];
	segmentar(dist, intermedia, mapa, ajustada, coordCentros);

	comparar(coordCentros);

	return( 0 );
}

void comparar(Coord* coordCentros){


	float resultados[IDe][2];

	for (int i = 0; i < IDe; i++) {
		resultados[i][0] = -numeric_limits<float>::max();
		resultados[i][1] = -numeric_limits<float>::max();
	}

	for (int i = 0; i < IDe; i++) {

		Mat image;
		int aux;
		image = imread( to_string(i) + "_resultado.png", 1 );
		for (int j = 0; j < 167; j++) {


			Mat plantilla;
			plantilla = imread( to_string(j) + "_plantilla.png", 1 );

			Mat dst, dst2;

			double nx, ny, nx2, ny2;
			nx = (50.0)/(plantilla.rows*1.);
			ny = (50.0)/(plantilla.cols*1.);
			nx2 = (50.0)/(image.rows*1.);
			ny2 = (50.0)/(image.cols*1.);
			resize(plantilla, dst, Size(), nx, ny, INTER_LINEAR);
			resize(image, dst2, Size(), nx2, ny2, INTER_LINEAR);
			aux = calificarDif(dst2,dst);
			if(aux > resultados[i][0]) {
				resultados[i][0] = aux;
				resultados[i][1] = j;
			}
		}

	}
	int nuevoIDe[IDe][2];
	int count = 0, count2 = 0;
	for (int i = 0; i < IDe; i++) {
		cout << "Se emparejó " << i << " con " << resultados[i][1] << " con un valor de " << resultados[i][0] << endl;
	}

	/*Producir el texto*/
	stringstream finalText;

	ifstream infile("plantillas.txt");
	string line;

	for (int i = 0; i < IDe; i++) {

		while (getline(infile, line))
		{
			istringstream iss(line);
			int num;
			string let;
			if (!(iss >> num >> let)) {break; }     // error
			if(resultados[i][1] == num) {
				finalText<<let;
			}
		}
		infile.close();
		infile.open("plantillas.txt");
	}
	cout<<endl;
	cout << "La respuesta final es: "<<finalText.str()<<endl;
	infile.close();

	ofstream myfile ("result.txt");
	if (myfile.is_open())
	{
		myfile <<finalText.str();
		myfile.close();
	}
}

float calificarDif(Mat image, Mat plantilla){
	float rest = 0;

	int minf, minc;
	if(image.rows < plantilla.rows)
		minf = image.rows;
	else
		minf = plantilla.rows;

	if(image.cols < plantilla.cols)
		minc = image.cols;
	else
		minc = plantilla.cols;

	for (int i = 0; i < minf; i++)
	{
		for (int j = 0; j < minc; j++)
		{

			int valimage = (int)(image.at<uchar>(i, j));
			int valplantilla = (int)(plantilla.at<uchar>(i, j));
			if( (valimage != 0 && valplantilla != 0) ) {
				rest+=1.0;
			}
			else if(valimage == 0 && valplantilla == 0) {
				rest+=1.0;
			}else if(valimage != 0 && valplantilla == 0) {
				rest-=2.0;
			}

		}
	}
	return(rest);
}

Mat resaltarTableroFilas(Mat image){
	Mat newImg = Mat::zeros(image.size(), CV_8UC1);
	float white = 0;

	for (size_t i = 0; i < image.rows; i++) {
		for (size_t j = 0; j < image.cols; j++) {
			uchar pixel = image.at<uchar>(i, j);

			uchar G = pixel;

			if(G == 255) {
				white++;
			}
		}
		if((float)(white/image.cols) < proportion) {
			for (size_t jj = 0; jj < image.cols; jj++) {
				newImg.at<uchar>(i, jj) = 255;
			}
		}else{
			for (size_t jj = 0; jj < image.cols; jj++) {
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
	Mat newImg = Mat::zeros(image.size(), CV_8UC1);
	float white = 0;

	for (size_t i = 0; i < image.cols; i++) {
		for (size_t j = 0; j < image.rows; j++) {
			uchar pixel = image.at<uchar>(j, i);

			uchar G = pixel;

			if(G == 255) {
				white++;
			}
		}
		if((float)(white/image.rows) < proportion) {
			for (size_t jj = 0; jj < image.rows; jj++) {
				newImg.at<uchar>(jj, i) = 255;
			}
		}else{
			for (size_t jj = 0; jj < image.rows; jj++) {
				uchar pixel = image.at<uchar>(jj, i);

				uchar G = pixel;
				newImg.at<uchar>(jj, i) = G;
			}
		}
		white = 0;
	}
	return(newImg);
}

void crearMatriz(Mat image, int supf, int inff, int supc, int infc, int ident){
	Mat resimg((inff-supf), (infc-supc), CV_8UC1);
	int ii = 0, jj = 0;
	for (size_t i = supf; i < inff; i++) {
		for (size_t j = supc; j < infc; j++) {
			uchar pixel = image.at<uchar>(i, j);
			resimg.at<uchar>(ii, jj) = pixel;
			jj++;
		}
		jj = 0;
		ii++;
	}
	imwrite( to_string(IDe) + "_resultado.png", resimg);
	IDe++;
}

Mat blancoNegro(Mat restoreImg){
	Mat dif(restoreImg.size(), CV_8UC1);
	for (size_t i = 0; i < restoreImg.rows; i++)
	{
		for (size_t j = 0; j < restoreImg.cols; j++)
		{
			Vec3b pixel1 = restoreImg.at<Vec3b>(i, j);
			uchar B = pixel1[0];
			uchar G = pixel1[1];
			uchar R = pixel1[2];
			dif.at<uchar>(i, j) = (B+G+R)/3;
		}
	}
	return(dif);
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
			if(image.at<uchar>(i, j) == 255 && intermedia.at<uchar>(i, j) == 0) {
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
				//cout<<"Para la region ("<<contador<<") se reconocieron ("<<contadorPixeles<<") pixeles"<<endl;
				mapa.insert({ contador, contadorPixeles });
				contadorPixeles=0;
				contador++;
			}
		}
	}

	return (intermedia);
}

void descarteDeRegiones(Mat &intermedia, map<int,int> &mapa, Mat &image){
	int maxCant=image.rows*image.cols;

	for (int i = 0; i < intermedia.rows; i++) {
		for (int j = 0; j < intermedia.cols; j++) {
			auto it=mapa.find((int)(intermedia.at<uchar>(i, j)));
			if((double)(it->second) <= (double)(maxCant*0.0003) && intermedia.at<uchar>(i, j) != 0
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
/*
   for (auto itr = mapa.begin(); itr != mapa.end(); ++itr) {
    if(itr->second > 0)
      cout<<"Para la region ("<<itr->first<<") se reconocieron ("<<itr->second<<") pixeles"<<endl;
   }*/

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

void revisarVecinos(Coord aux, queue<Coord> &cola, Mat &intermedia, Mat image, map<int, int> &mapa, int contador, int &contadorPixeles){
	Coord vecino;

	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			vecino.x = aux.x+i;
			vecino.y = aux.y+j;
			if(vecino.x > -1 && vecino.y > -1 && vecino.x < image.rows && vecino.y < image.cols &&
			   intermedia.at<uchar>(vecino.x, vecino.y) == 0 && image.at<uchar>(vecino.x, vecino.y) == 255) {
				cola.push(vecino);
				contadorPixeles++;
				intermedia.at<uchar>(vecino.x, vecino.y) = contador;
			}
		}
	}
}

void segmentar(Mat dist, Mat intermedia, map<int, int> mapa, Mat image, Coord* coordCentros){
	map<int, Coord> mapaCentros;
	map<int, Bordes> mapaBordes;

	for (map<int,int>::iterator it=mapa.begin(); it!=mapa.end(); ++it) {
		if(it->second > 0) {
			int max = -numeric_limits<int>::max();
			int promF = 0, promC = 0;
			int count = 0;
			Coord coord;
			int mini = numeric_limits<int>::max(), maxi = -numeric_limits<int>::max();
			int minj = numeric_limits<int>::max(), maxj = -numeric_limits<int>::max();

			for (int i = 0; i < intermedia.rows; i++) {
				for (int j = 0; j < intermedia.cols; j++) {
					if((int)(intermedia.at<uchar>(i, j)) == it->first) {
						if(i > maxi) {
							maxi = i;
						}else if(i < mini) {
							mini = i;
						}
						if(j > maxj) {
							maxj = j;
						}else if(j < minj) {
							minj = j;
						}
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
			Bordes bordes;
			bordes.sf = mini;
			bordes.inf = maxi;
			bordes.sc = minj;
			bordes.ic = maxj;
			coord.x = promC/count;
			coord.y = promF/count;
			mapaCentros.insert({it->first, coord});
			mapaBordes.insert({it->first, bordes});
		}
	}

	/*for (map<int,Coord>::iterator it=mapaCentros.begin(); it!=mapaCentros.end(); ++it) {
	   cout << "Para la region " << it->first << " el centro está en la fila "
	     << it->second.x << " columna " << it->second.y << endl;
	   }*/

	for (map<int,Bordes>::iterator it=mapaBordes.begin(); it!=mapaBordes.end(); ++it) {
		crearMatriz(image, it->second.sf, it->second.inf, it->second.sc, it->second.ic, IDe);
		coordCentros[IDe-1]=(Coord)(mapaCentros.find(it->first)->second);
	}
}
