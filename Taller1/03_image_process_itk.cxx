#include <cmath>
#include <cstdlib>
#include <limits>
#include <iostream>
#include <string>
#include <sstream>
#include <itkImage.h>
#include <itkRGBPixel.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkIdentityTransform.h>
#include <itkResampleImageFilter.h>
#define rsize0 0.75
#define rsize1 0.5
#define rsize2 0.25
// Image type: 2-dimensional 1-byte rgb
const unsigned int Dim = 2;
typedef unsigned char                   TRGBResolution;
typedef itk::RGBPixel< TRGBResolution > TRGBPixel;
typedef itk::Image< TRGBPixel, Dim >    TColorImage;
// Types definition
typedef itk::ImageFileReader< TColorImage > TReader;
typedef itk::ImageRegionConstIteratorWithIndex< TColorImage > TIterator;
typedef itk::ImageRegionIteratorWithIndex< TColorImage > TColorIterator;
typedef itk::ImageFileWriter< TColorImage > TWriter;
typedef itk::IdentityTransform<double, 2> TransformType;
typedef itk::ResampleImageFilter< TColorImage, TColorImage > ResampleImageFilterType;
TColorImage* readImg(TReader::Pointer reader, std::string str);
void writeImg(TColorImage* Image, std::string ext);
void createImage(TColorImage* img, TColorImage::Pointer chImage);
void createFilter(ResampleImageFilterType::Pointer filter,TColorImage::Pointer img, float newSize);
std::string basename1;
// -------------------------------------------------------------------------
int main(int argc, char const *argv[])
{
  // Get command line arguments
  if( argc < 2 )
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
  TReader::Pointer reader = TReader::New( );
  reader->SetFileName( argv[ 1 ] );
  try
  {
      reader->Update( );
  }
  catch( itk::ExceptionObject& err )
  {
      std::cerr << "Error: " << err << std::endl;
      return( 1 );
  } // yrt
  TColorImage* img = reader->GetOutput( );//AQUI SE HACE EL LLAMADO A LA FUNCION DE LEER IMAGEN 
  // Create color channel images
    // red channel
    TColorImage::Pointer rImg = TColorImage::New( );
    createImage(img,rImg);
    // green channel
    TColorImage::Pointer gImg = TColorImage::New( );
    createImage(img,gImg);
    // blue channel
    TColorImage::Pointer bImg = TColorImage::New( );
    createImage(img,bImg);
    // Initialize created images in black
    TRGBPixel black;
    black.SetRed( 0 );
    black.SetGreen( 0 );
    black.SetBlue( 0 );
    rImg->FillBuffer( black );
    gImg->FillBuffer( black );
    bImg->FillBuffer( black );
    // Fill color channel images
    TIterator it( img, img->GetLargestPossibleRegion( ) );
    TColorIterator crIt( rImg, rImg->GetLargestPossibleRegion( ) );
    TColorIterator cgIt( gImg, gImg->GetLargestPossibleRegion( ) );
    TColorIterator cbIt( bImg, bImg->GetLargestPossibleRegion( ) );
    it.GoToBegin( );
    crIt.GoToBegin( );
    cgIt.GoToBegin( );
    cbIt.GoToBegin( );
    for( ; !it.IsAtEnd( ) && !crIt.IsAtEnd( ) && !cgIt.IsAtEnd( ) && !cbIt.IsAtEnd( ); ++it, ++crIt, ++cgIt, ++cbIt )
    {
      TRGBPixel value, pixel;
      pixel = it.Get( );
      value.SetRed( pixel.GetRed( ) );
      value.SetGreen( 0 );
      value.SetBlue( 0 );
      crIt.Set( value );
      value.SetRed( 0 );
      value.SetGreen( pixel.GetGreen( ) );
      value.SetBlue( 0 );
      cgIt.Set( value );
      value.SetRed( 0 );
      value.SetGreen( 0 );
      value.SetBlue( pixel.GetBlue( ) );
      cbIt.Set( value );
    } // rof
    // Write results
    //--------------------aqui se hace uso de la funcion---------------------
    std::stringstream ss( argv[ 1 ] );
    getline( ss, basename1, '.' );
    writeImg(rImg,"_R.png");
    writeImg(gImg,"_G.png");
    writeImg(bImg,"_B.png");
    ResampleImageFilterType::Pointer resampleFilter = ResampleImageFilterType::New( );
    ResampleImageFilterType::Pointer resampleFilter2 = ResampleImageFilterType::New( );
    ResampleImageFilterType::Pointer resampleFilter3 = ResampleImageFilterType::New( );
    createFilter(resampleFilter,rImg,rsize0);
    createFilter(resampleFilter2,gImg,rsize1);
    createFilter(resampleFilter3,bImg,rsize2);
    writeImg(resampleFilter->GetOutput( ),"_sR.png" );
    writeImg(resampleFilter2->GetOutput( ),"_sG.png" );
    writeImg(resampleFilter3->GetOutput( ),"_sB.png" );
  // Read an image
  TReader::Pointer reader1 = TReader::New( );
  TColorImage* sR = readImg( reader1, "_sR.png");
   // Read an image
  TReader::Pointer reader2 = TReader::New( );
  TColorImage* sG = readImg( reader2, "_sG.png");
   // Read an image
  TReader::Pointer reader3 = TReader::New( );
  TColorImage* sB = readImg( reader3, "_sB.png");
  createFilter(resampleFilter,sR,(1/rsize0));
  createFilter(resampleFilter2,sG,(1/rsize1));
  createFilter(resampleFilter3,sB,(1/rsize2));
    writeImg(resampleFilter->GetOutput( ),"_ssR.png");
    writeImg(resampleFilter2->GetOutput( ),"_ssG.png");
    writeImg(resampleFilter3->GetOutput( ),"_ssB.png");
  TReader::Pointer reader4 = TReader::New( );
  TColorImage* ssR = readImg( reader4, "_ssR.png");
  TReader::Pointer reader5 = TReader::New( );
  TColorImage* ssG = readImg( reader5, "_ssG.png");
  TReader::Pointer reader6 = TReader::New( );
  TColorImage* ssB = readImg( reader6, "_ssB.png");
  // composite image (RGB)
    TColorImage::Pointer RGB_2_0 = TColorImage::New( );
    createImage(img,RGB_2_0);
    RGB_2_0->FillBuffer( black );
    // Fill color channel images
    TColorIterator it2( RGB_2_0, RGB_2_0->GetLargestPossibleRegion( ) );
    TIterator itR( ssR, ssR->GetLargestPossibleRegion( ) );
    TIterator itG( ssG, ssG->GetLargestPossibleRegion( ) );
    TIterator itB( ssB, ssB->GetLargestPossibleRegion( ) );
    it2.GoToBegin( );
    itR.GoToBegin( );
    itG.GoToBegin( );
    itB.GoToBegin( );
    for( ; !it2.IsAtEnd() && !itR.IsAtEnd() && !itG.IsAtEnd() && !itB.IsAtEnd(); ++it2, ++itR, ++itG, ++itB )
    {
      TRGBPixel value, pixelR, pixelG, pixelB;
      pixelR = itR.Get();
      pixelG = itG.Get();
      pixelB = itB.Get();
      value.SetRed(pixelR.GetRed());
      value.SetGreen(pixelG.GetGreen());
      value.SetBlue(pixelB.GetBlue());
      it2.Set(value);
    } // rof
  writeImg( RGB_2_0, "_rRGB.png");
  TColorImage::Pointer diff = TColorImage::New( );
  createImage(img,diff);
  diff->FillBuffer( black );
  TColorIterator it3( diff, diff->GetLargestPossibleRegion( ) );
  it.GoToBegin( );
  it2.GoToBegin( );
  it3.GoToBegin( );
  for( ; !it.IsAtEnd( ) && !it2.IsAtEnd( ) && !it3.IsAtEnd( ); ++it, ++it2, ++it3 )
  {
    TRGBPixel value, value2, pixel;
    value = it.Get( );
    value2 = it2.Get( );
    pixel.SetRed( abs((int)value.GetRed( ) - (int)value2.GetRed( )) );
    pixel.SetGreen( abs((int)value.GetGreen( ) - (int)value2.GetGreen( )) );
    pixel.SetBlue( abs((int)value.GetBlue( ) - (int)value2.GetBlue( )) );
    it3.Set( pixel );
  } // rof
  writeImg( diff, "_diff.png");
  return 0;
}
TColorImage* readImg(TReader::Pointer reader, std::string str){
  reader->SetFileName( basename1+str );
  try
  {
      reader->Update( );
  }
  catch( itk::ExceptionObject& err )
  {
      std::cerr << "Error: " << err << std::endl;
      exit(1);
  }
  return reader->GetOutput( );
}
void writeImg(TColorImage* Image, std::string ext){
    TWriter::Pointer writer = TWriter::New( );
    writer->SetInput( Image );
    writer->SetFileName( basename1 + ext );
    try
    {
      writer->Update( );
    }
    catch( itk::ExceptionObject& err )
    {
      std::cerr << "Error: " << err << std::endl;
      exit(1);
    } // yrt 
}
void createImage(TColorImage* img, TColorImage::Pointer chImage){
    chImage->SetSpacing( img->GetSpacing( ) );
    chImage->SetOrigin( img->GetOrigin( ) );
    chImage->SetLargestPossibleRegion( img->GetLargestPossibleRegion( ) );
    chImage->SetRequestedRegion( img->GetRequestedRegion( ) );
    chImage->SetBufferedRegion( img->GetBufferedRegion( ) );
    chImage->Allocate( );
}
void createFilter(ResampleImageFilterType::Pointer filter,TColorImage::Pointer img, float newSize){
  TColorImage::SizeType inputSize = img->GetLargestPossibleRegion( ).GetSize( );
  std::cout << "Image input size: " << inputSize << std::endl;
  TColorImage::SizeType outputSize;
  outputSize[ 0 ] = inputSize[ 0 ] * newSize;
  outputSize[ 1 ] = inputSize[ 1 ] * newSize;
  std::cout << "Image output size: " << outputSize << std::endl;
  TColorImage::SpacingType outputSpacing;
  outputSpacing[ 0 ] = 
    img->GetSpacing( )[ 0 ] * ( static_cast< double >( inputSize[ 0 ] ) / static_cast< double >( outputSize[ 0 ] ) );
  outputSpacing[ 1 ] = 
    img->GetSpacing( )[ 1 ] * ( static_cast< double >( inputSize[ 1 ] ) / static_cast< double >( outputSize[ 1 ] ) );
  filter->SetTransform( TransformType::New( ) );
  filter->SetInput( img );
  filter->SetSize( outputSize );
  filter->SetOutputSpacing( outputSpacing );
  filter->UpdateLargestPossibleRegion( );
}
