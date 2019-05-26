#include <stdio.h>
#include <map>
#include <sstream>
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
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

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

Mat binarizacion(Mat imagen);
Mat intensidad(int cantb, Mat image);
Mat filtro(Mat image, int kernelSize);
Mat resaltarTableroFilas(Mat image);
Mat resaltarTableroColumnas(Mat image);
void segmentacion(Mat imagen, int superiorf, int inferiorf, int superiorc, int inferiorc);
int buscarFila(Mat image, int supf, int inff, int supc, int infc);
int buscarColumna(Mat image, int supf, int inff, int supc, int infc);
void crearMatriz(Mat image, int supf, int inff, int supc, int infc, int ident);
//Mat binarizacion(Mat imagen);
float analisis(Mat image, map<int,int> histogram, int val);
Mat ecualizacion(Mat src);
Mat blancoNegro(Mat restoreImg);
Mat etiquetado(Mat image, String basename,map<int,int> &mapa);
void revisarVecinos(Coord aux, queue<Coord> &cola, Mat &intermedia, Mat image,
                    map<int, int> &mapa, int contador, int &contadorPixeles);
void descarteDeRegiones(Mat &intermedia,map<int,int > &mapa,Mat &image);
Mat ajusteDeIntensidades( Mat intermedia, int cantRegiones);
void centros(Mat dist, Mat intermedia, map<int, int> mapa, Mat image);
float calificarDif(Mat image, Mat plantilla);
void comparar();
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

	cout << "Image input size: " << image.size() << endl;
	cout << "rows: " << image.rows << endl;
	cout << "columns: " << image.cols << endl;

	stringstream ss( argv[ 1 ] );
	string basename;
	getline( ss, basename, '.' );

	//Tratamiento de imagen
	//Binarización
  Mat res_img, kernel, intermedia;
	map<int,int> mapa;

  int thres = 180, max = 255;
  res_img = blancoNegro(image);
  //cvtColor(image, res_img, COLOR_BGR2GRAY);

  threshold(res_img, res_img, thres, max, 2);

	threshold(res_img, res_img, 177, 255, THRESH_OTSU);
	//adaptiveThreshold(res_img, res_img, 177, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 3, 2);

	//res_img = binarizacion(res_img);

	kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
  erode(res_img, res_img, kernel);
	dilate(res_img, res_img, kernel);
	//dilate(res_img, res_img, kernel);
	//erode(res_img, res_img, kernel);
	//erode(res_img, res_img, kernel);
	//erode(res_img, res_img, kernel);

	//GaussianBlur(res_img, res_img, Size(33, 33), 0);
	GaussianBlur(res_img, res_img, Size(11, 11), 0);
	GaussianBlur(res_img, res_img, Size(11, 11), 0);

	threshold(res_img, res_img, 177, 255, THRESH_OTSU);


	//Segmentación
	//res_img = resaltarTableroFilas(res_img);
	//res_img = resaltarTableroColumnas(res_img);
	threshold(res_img, res_img, 177, 255, THRESH_BINARY_INV);

	intermedia = etiquetado(res_img, basename, mapa);
	descarteDeRegiones(intermedia,mapa,res_img);
	Mat ajustada;
	ajustada = ajusteDeIntensidades(intermedia, mapa.size());
	Mat dist;
	distanceTransform(res_img, dist, DIST_L2, 3);
	centros(dist, intermedia, mapa, ajustada);

	imwrite( basename + "_ajustada.png", ajustada );
	imwrite( basename + "_dist.png", dist );

	//segmentacion(res_img, 0, res_img.rows, 0, res_img.cols);
	comparar();

	//Escribir imagen
	imwrite( basename + "_resultado.png", res_img );

	return( 0 );
}
/*
Mat binarizacion(Mat image){
	Mat grey = Mat::zeros(image.size(), CV_8UC1);
	map<int,int> histogram;

	for (size_t i = 0; i < image.rows; i++) {
		for (size_t j = 0; j < image.cols; j++) {
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
	for(map<int, int>::iterator it = histogram.begin(); it!=histogram.end(); it++) {
		//cout << it->first << " " << it->second << endl;
		sum = it->first * it->second + sum;
	}
	float limite = sum/div;
	cout << "limite " << limite << endl;

	for (size_t i = 0; i < grey.rows; i++) {
		for (size_t j = 0; j < grey.cols; j++) {
			uchar pixel = grey.at<uchar>(i, j);

			uchar G = pixel;

			if(G > limite) {
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

	return(grey);
}
*/

void comparar(){
	float resultados[IDe][2];

	for (int i = 0; i < IDe; i++) {
		resultados[i][0] = -99999.0;
		resultados[i][1] = -99999.0;
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
				//cout << aux << endl;
				if(aux > resultados[i][0]){
					resultados[i][0] = aux;
					resultados[i][1] = j;
				}
		}

	}

	for (int i = 0; i < IDe; i++) {
		cout << "Se emparejó " << i << " con " << resultados[i][1] << " con un valor de " << resultados[i][0] << endl;
	}
}

float calificarDif(Mat image, Mat plantilla){
	//cout << "dimensiones" << endl;
	//cout << image.rows << " " << plantilla.rows << " " << image.cols << " " << plantilla.cols << endl;
	//mostrarImagen(image,1);
	//mostrarImagen(plantilla,2);
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
				if( (valimage != 0 && valplantilla != 0) ){
					rest+=1.0;
				}
				else if(valimage == 0 && valplantilla == 0){
					rest+=1.0;
				}else if(valimage != 0 && valplantilla == 0){
					rest-=2.0;
				}

	    }
	}
	return(rest);
}

Mat filtro(Mat image, int kernelSize){
	Mat dst, kernel;
	Point anchor;
	double delta;
	int depth;

	//filtro 2D
	depth = -1;       //Igual que la imagen de origen (espesor)
	delta = 0;
	anchor = Point(-1,-1);

	kernel = Mat::ones( kernelSize, kernelSize, CV_32F )/ (float)(kernelSize*kernelSize);

	filter2D(image, dst, depth, kernel, anchor, delta, BORDER_DEFAULT );

	return(dst);
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
		//cout << "white " << white << " image.cols: " << image.cols << " result: " << (float)(white/image.cols) << endl;
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
	//imwrite( "lol.png", image );
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
		//cout << "white " << white << " image.cols: " << image.cols << " result: " << (float)(white/image.cols) << endl;
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
	//imwrite( "lol2.png", newImg );
	return(newImg);
}

Mat identificarRenglones(Mat image){
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
		//cout << "white " << white << " image.cols: " << image.cols << " result: " << (float)(white/image.cols) << endl;
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

void segmentacion(Mat imagen, int superiorf, int inferiorf, int superiorc, int inferiorc){
	uchar pixel;
	int superior2f = -1, inferior2f = -1;
	int superior2c = -1, inferior2c = -1;
	int bandera = 0;
	//cout << superiorf << " " << inferiorf << endl;
	//cout << superiorc << " " << inferiorc << endl;

	for(int i = superiorf; i<inferiorf; i++) {
		for(int j = superiorc; j<inferiorc; j++) {
			pixel = imagen.at<uchar>(i, j);
			if(pixel == 0 && superior2f == -1) {
				superior2f = i;
				break;
			}
		}
		if(superior2f > -1) {
			int resultado = buscarFila(imagen, superior2f, inferiorf, superiorc, inferiorc);
			if(resultado > -1) {
				i = resultado;
				inferior2f = i;
				//crearMatriz(imagen, superior2f, inferior2f, superiorc, inferiorc, IDe);
				segmentacion(imagen, superior2f, inferior2f, superiorc, inferiorc);
				superior2f = -1;
				bandera = 1;
			}
		}
	}

	if(bandera == 0) {
		for(int j = superiorc; j<inferiorc; j++) {
			for(int i = superiorf; i<inferiorf; i++) {
				pixel = imagen.at<uchar>(i, j);
				if(pixel == 0 && superior2c == -1) {
					superior2c = j;
					break;
				}
			}
			if(superior2c > -1) {
				int resultado = buscarColumna(imagen, superiorf, inferiorf, superior2c, inferiorc);
				if(resultado > -1) {
					j = resultado;
					inferior2c = j;
					//crearMatriz(imagen, superior2f, inferior2f, superiorc, inferiorc, IDe);
					segmentacion(imagen, superiorf, inferiorf, superior2c, inferior2c);
					superior2c = -1;
					bandera = 1;
				}
			}
		}
	}
	if(bandera == 0) {
		IDe++;
		crearMatriz(imagen, superiorf, inferiorf, superiorc, inferiorc, IDe);
	}
}

int buscarFila(Mat image, int supf, int inff, int supc, int infc){
	uchar pixel;
	int bandera = 0;
	for(int i = supf; i<inff; i++) {
		for(int j = supc; j<infc; j++) {
			pixel = image.at<uchar>(i, j);
			if(pixel == 0) {
				bandera = 1;
				break;
			}
		}
		if(bandera == 0) {
			return(i);
		}
		bandera = 0;
	}
	return(-1);
}

int buscarColumna(Mat image, int supf, int inff, int supc, int infc){
	uchar pixel;
	int bandera = 0;
	for(int j = supc; j<infc; j++) {
		for(int i = supf; i<inff; i++) {
			pixel = image.at<uchar>(i, j);
			if(pixel == 0) {
				bandera = 1;
				break;
			}
		}
		if(bandera == 0) {
			return(j);
		}
		bandera = 0;
	}
	return(-1);
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

//histograma<intensidad, cantidad>
Mat binarizacion(Mat image){
	//Mat grey = Mat::zeros(image.size(), CV_8UC1);
  Mat grey = image;
	map<int,int> histogram;
	int inten;
	int maxInt = -1;
	int minInt = 800;

	for (size_t i = 0; i < image.rows; i++){
		  for (size_t j = 0; j < image.cols; j++){
		      uchar pixel = image.at<uchar>(i, j);
/*
		      uchar B = pixel[0];
		      uchar G = pixel[1];
		      uchar R = pixel[2];

		      inten = (B + G + R) / 3;*/
					//grey.at<uchar>(i, j) = pixel;
					histogram[pixel] = histogram[pixel] + 1;

					if(inten > maxInt){
						maxInt = pixel;
					}
					if(inten < minInt){
						minInt = pixel;
					}

			}
	}

	float result;
	float min = 9999;
	int t;
	for(int i = 0; i<256; i++){
		result = analisis(image, histogram, i);
		if(result < min){
			min = result;
			t = i;
		}
	}

	cout << "T = " << t << " con un resultado de " << min << endl;

	//Crear imagen binarizada
	for (size_t i = 0; i < grey.rows; i++){
		  for (size_t j = 0; j < grey.cols; j++){
		      uchar pixel = grey.at<uchar>(i, j);

		      uchar G = pixel;

		      if(G > t){
		      	grey.at<uchar>(i, j) = 255;
		      }else{
		      	grey.at<uchar>(i, j) = 0;
		      }
			}
	}
	return(grey);
}

float analisis(Mat image, map<int,int> histogram, int val){
	int suma = 0, sumb = 0;
	int sum2a = 0, sum2b = 0;
	int canta = 0, cantb = 0;
	int div = image.cols*image.rows;

	for(map<int, int>::iterator it = histogram.begin(); it!=histogram.end(); it++){

		if(it->first < val){
			suma = it->first * it->second + suma;
			sum2a = it->second + sum2a;
			canta = it->second + canta;
		}else{
			sumb = it->first * it->second + sumb;
			sum2b = it->second + sum2b;
			cantb = it->second + cantb;
		}
	}

	float pesoa = (float)sum2a/div;
	float mediaa = (float)suma/canta;
	float pesob = (float)sum2b/div;
	float mediab = (float)sumb/cantb;

	float sum3a = 0, sum3b = 0;

	for(map<int, int>::iterator it = histogram.begin(); it!=histogram.end(); it++){

		if(it->first < val){
			sum3a = ((it->first - mediaa) * (it->first - mediaa) * it->second) + sum3a;
		}else{
			sum3b = ((it->first - mediab) * (it->first - mediab) * it->second) + sum3b;
		}
	}

	float varianzaa = sum3a/canta, varianzab = sum3b/cantb;
	float result = varianzaa + varianzab;

	return(result);
}

Mat ecualizacion(Mat img){
  // Convertir BGR image a YCrCb
  Mat ycrcb, result;
  cvtColor(img, ycrcb, COLOR_BGR2YCrCb);

  // Extarer cada uno de los canales
  vector<Mat> channels;
  split(ycrcb, channels);

  // Ecualizar histograma del canal Y (luminosidad)
  equalizeHist(channels[0], channels[0]);

  // Unir los canales nuevamente
  merge(channels, ycrcb);

  // Convertir color YCrCb a BGR
  cvtColor(ycrcb, result, COLOR_YCrCb2BGR);
  return(result);
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
	int maxCant=image.rows*image.cols;
	/*
	for (auto itr = mapa.begin(); itr != mapa.end(); ++itr) {
		if(itr->second > maxCant) {
			maxCant = itr->second;
		}
	}*/

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

void centros(Mat dist, Mat intermedia, map<int, int> mapa, Mat image){
	map<int, Coord> mapaCentros;
	map<int, Bordes> mapaBordes;

	for (map<int,int>::iterator it=mapa.begin(); it!=mapa.end(); ++it) {
		if(it->second > 0) {
			int max = -1;
			int promF = 0, promC = 0;
			int count = 0;
			Coord coord;
			int mini = 99999, maxi = -1;
			int minj = 99999, maxj = -1;

			for (int i = 0; i < intermedia.rows; i++) {
				for (int j = 0; j < intermedia.cols; j++) {
					if((int)(intermedia.at<uchar>(i, j)) == it->first) {
						if(i > maxi){
							maxi = i;
						}else if(i < mini){
							mini = i;
						}
						if(j > maxj){
							maxj = j;
						}else if(j < minj){
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
/*
	for (map<int,Coord>::iterator it=mapaCentros.begin(); it!=mapaCentros.end(); ++it) {
		cout << "Para la region " << it->first << " el centro está en la fila "
		     << it->second.x << " columna " << it->second.y << endl;
	}*/

		for (map<int,Bordes>::iterator it=mapaBordes.begin(); it!=mapaBordes.end(); ++it) {
			crearMatriz(image, it->second.sf, it->second.inf, it->second.sc, it->second.ic, IDe);
		}
}
