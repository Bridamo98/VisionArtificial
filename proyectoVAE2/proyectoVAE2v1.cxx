#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <map>
#include <sstream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

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

	//Tratamiento de imagen
	//Binarización
  Mat res_img, kernel;

  int thres = 180, max = 255;
  res_img = blancoNegro(image);
  //cvtColor(image, gray, COLOR_BGR2GRAY);

  threshold(res_img, res_img, thres, max, 2);

	threshold(res_img, res_img, 177, 255, THRESH_OTSU);

	//res_img = binarizacion(res_img);

	kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
  erode(res_img, res_img, kernel);
	dilate(res_img, res_img, kernel);

	GaussianBlur(res_img, res_img, Size(11, 11), 0);
	GaussianBlur(res_img, res_img, Size(11, 11), 0);

	threshold(res_img, res_img, 177, 255, THRESH_OTSU);


	//Segmentación
	res_img = resaltarTableroFilas(res_img);
	res_img = resaltarTableroColumnas(res_img);

	segmentacion(res_img, 0, res_img.rows, 0, res_img.cols);

	//Escribir imagen
	stringstream ss( argv[ 1 ] );
	string basename;
	getline( ss, basename, '.' );

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
	imwrite( to_string(ident) + "_resultado.png", resimg);
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
