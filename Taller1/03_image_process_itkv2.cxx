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

#define rsize0 0.25
#define rsize1 0.25
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
    rImg->SetSpacing( img->GetSpacing( ) );
    rImg->SetOrigin( img->GetOrigin( ) );
    rImg->SetLargestPossibleRegion( img->GetLargestPossibleRegion( ) );
    rImg->SetRequestedRegion( img->GetRequestedRegion( ) );
    rImg->SetBufferedRegion( img->GetBufferedRegion( ) );
    rImg->Allocate( );

    // green channel
    TColorImage::Pointer gImg = TColorImage::New( );
    gImg->SetSpacing( img->GetSpacing( ) );
    gImg->SetOrigin( img->GetOrigin( ) );
    gImg->SetLargestPossibleRegion( img->GetLargestPossibleRegion( ) );
    gImg->SetRequestedRegion( img->GetRequestedRegion( ) );
    gImg->SetBufferedRegion( img->GetBufferedRegion( ) );
    gImg->Allocate( );

    // blue channel
    TColorImage::Pointer bImg = TColorImage::New( );
    bImg->SetSpacing( img->GetSpacing( ) );
    bImg->SetOrigin( img->GetOrigin( ) );
    bImg->SetLargestPossibleRegion( img->GetLargestPossibleRegion( ) );
    bImg->SetRequestedRegion( img->GetRequestedRegion( ) );
    bImg->SetBufferedRegion( img->GetBufferedRegion( ) );
    bImg->Allocate( );

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
  

    TColorImage::SizeType redInputSize = rImg->GetLargestPossibleRegion( ).GetSize( );
    TColorImage::SizeType greenInputSize = gImg->GetLargestPossibleRegion( ).GetSize( );
    TColorImage::SizeType blueInputSize = bImg->GetLargestPossibleRegion( ).GetSize( );

    std::cout << "Image input red size: " << redInputSize << std::endl;
    std::cout << "Image input  green size: " << greenInputSize << std::endl;
    std::cout << "Image input blue size: " << blueInputSize << std::endl;

    TColorImage::SizeType redOutputSize;
    redOutputSize[ 0 ] = redInputSize[ 0 ] * rsize0;
    redOutputSize[ 1 ] = redInputSize[ 1 ] * rsize0;

  TColorImage::SizeType greenOutputSize;
    greenOutputSize[ 0 ] = greenInputSize[ 0 ] * rsize1;
    greenOutputSize[ 1 ] = greenInputSize[ 1 ] * rsize1;    

  TColorImage::SizeType blueOutputSize;
    blueOutputSize[ 0 ] = blueInputSize[ 0 ] * rsize2;
    blueOutputSize[ 1 ] = blueInputSize[ 1 ] * rsize2;

    std::cout << "Image output red size: " << redOutputSize << std::endl;
    std::cout << "Image output green size: " << greenOutputSize << std::endl;
    std::cout << "Image output blue size: " << blueOutputSize << std::endl;

    TColorImage::SpacingType redOutputSpacing, greenOutputSpacing, blueOutputSpacing;

    redOutputSpacing[ 0 ] = 
      rImg->GetSpacing( )[ 0 ] * ( static_cast< double >( redInputSize[ 0 ] ) / static_cast< double >( redOutputSize[ 0 ] ) );
    redOutputSpacing[ 1 ] = 
      rImg->GetSpacing( )[ 1 ] * ( static_cast< double >( redInputSize[ 1 ] ) / static_cast< double >( redOutputSize[ 1 ] ) );

    greenOutputSpacing[ 0 ] = 
      gImg->GetSpacing( )[ 0 ] * ( static_cast< double >( greenInputSize[ 0 ] ) / static_cast< double >( greenOutputSize[ 0 ] ) );
    greenOutputSpacing[ 1 ] = 
      gImg->GetSpacing( )[ 1 ] * ( static_cast< double >( greenInputSize[ 1 ] ) / static_cast< double >( greenOutputSize[ 1 ] ) );

    blueOutputSpacing[ 0 ] = 
      bImg->GetSpacing( )[ 0 ] * ( static_cast< double >( blueInputSize[ 0 ] ) / static_cast< double >( blueOutputSize[ 0 ] ) );
    blueOutputSpacing[ 1 ] = 
      bImg->GetSpacing( )[ 1 ] * ( static_cast< double >( blueInputSize[ 1 ] ) / static_cast< double >( blueOutputSize[ 1 ] ) );

    // Rescale image
    ResampleImageFilterType::Pointer resampleFilter = ResampleImageFilterType::New( );


    resampleFilter->SetTransform( TransformType::New( ) );
    resampleFilter->SetInput( rImg );
    resampleFilter->SetSize( redOutputSize );
    resampleFilter->SetOutputSpacing( redOutputSpacing );
    resampleFilter->UpdateLargestPossibleRegion( );

    ResampleImageFilterType::Pointer resampleFilter2 = ResampleImageFilterType::New( );

    resampleFilter2->SetTransform( TransformType::New( ) );
    resampleFilter2->SetInput( gImg );
    resampleFilter2->SetSize( greenOutputSize );
    resampleFilter2->SetOutputSpacing( greenOutputSpacing );
    resampleFilter2->UpdateLargestPossibleRegion( );

    ResampleImageFilterType::Pointer resampleFilter3 = ResampleImageFilterType::New( );

    resampleFilter3->SetTransform( TransformType::New( ) );
    resampleFilter3->SetInput( bImg );
    resampleFilter3->SetSize( blueOutputSize );
    resampleFilter3->SetOutputSpacing( blueOutputSpacing );
    resampleFilter3->UpdateLargestPossibleRegion( );




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

   redInputSize = sR->GetLargestPossibleRegion( ).GetSize( );
    greenInputSize = sG->GetLargestPossibleRegion( ).GetSize( );
    blueInputSize = sB->GetLargestPossibleRegion( ).GetSize( );

  std::cout << " s redInputSize " << std::endl;

  redOutputSize[ 0 ] = redInputSize[ 0 ] * (1/rsize0);
    redOutputSize[ 1 ] = redInputSize[ 1 ] * (1/rsize0);

  
    greenOutputSize[ 0 ] = greenInputSize[ 0 ] * (1/rsize1);
    greenOutputSize[ 1 ] = greenInputSize[ 1 ] * (1/rsize1);

  
    blueOutputSize[ 0 ] = blueInputSize[ 0 ] * (1/rsize2);
    blueOutputSize[ 1 ] = blueInputSize[ 1 ] * (1/rsize2);

    redOutputSpacing[ 0 ] = 
      sR->GetSpacing( )[ 0 ] * ( static_cast< double >( redInputSize[ 0 ] ) / static_cast< double >( redOutputSize[ 0 ] ) );
    redOutputSpacing[ 1 ] = 
      sR->GetSpacing( )[ 1 ] * ( static_cast< double >( redInputSize[ 1 ] ) / static_cast< double >( redOutputSize[ 1 ] ) );

    greenOutputSpacing[ 0 ] = 
      sG->GetSpacing( )[ 0 ] * ( static_cast< double >( greenInputSize[ 0 ] ) / static_cast< double >( greenOutputSize[ 0 ] ) );
    greenOutputSpacing[ 1 ] = 
      sG->GetSpacing( )[ 1 ] * ( static_cast< double >( greenInputSize[ 1 ] ) / static_cast< double >( greenOutputSize[ 1 ] ) );

    blueOutputSpacing[ 0 ] = 
      sB->GetSpacing( )[ 0 ] * ( static_cast< double >( blueInputSize[ 0 ] ) / static_cast< double >( blueOutputSize[ 0 ] ) );
    blueOutputSpacing[ 1 ] = 
      sB->GetSpacing( )[ 1 ] * ( static_cast< double >( blueInputSize[ 1 ] ) / static_cast< double >( blueOutputSize[ 1 ] ) );

    resampleFilter->SetTransform( TransformType::New( ) );
    resampleFilter->SetInput( sR );
    resampleFilter->SetSize( redOutputSize );
    resampleFilter->SetOutputSpacing( redOutputSpacing );
    resampleFilter->UpdateLargestPossibleRegion( );

    resampleFilter2->SetTransform( TransformType::New( ) );
    resampleFilter2->SetInput( sG );
    resampleFilter2->SetSize( greenOutputSize );
    resampleFilter2->SetOutputSpacing( greenOutputSpacing );
    resampleFilter2->UpdateLargestPossibleRegion( );

    resampleFilter3->SetTransform( TransformType::New( ) );
    resampleFilter3->SetInput( sB );
    resampleFilter3->SetSize( blueOutputSize );
    resampleFilter3->SetOutputSpacing( blueOutputSpacing );
    resampleFilter3->UpdateLargestPossibleRegion( );

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
    RGB_2_0->SetSpacing( img->GetSpacing( ) );
    RGB_2_0->SetOrigin( img->GetOrigin( ) );
    RGB_2_0->SetLargestPossibleRegion( img->GetLargestPossibleRegion( ) );
    RGB_2_0->SetRequestedRegion( img->GetRequestedRegion( ) );
    RGB_2_0->SetBufferedRegion( img->GetBufferedRegion( ) );
    RGB_2_0->Allocate( );

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
  diff->SetSpacing( img->GetSpacing( ) );
  diff->SetOrigin( img->GetOrigin( ) );
  diff->SetLargestPossibleRegion( img->GetLargestPossibleRegion( ) );
  diff->SetRequestedRegion( img->GetRequestedRegion( ) );
  diff->SetBufferedRegion( img->GetBufferedRegion( ) );
  diff->Allocate( );

  diff->FillBuffer( black );

  TColorIterator it3( diff, diff->GetLargestPossibleRegion( ) );

  it.GoToBegin( );
  it2.GoToBegin( );
  it3.GoToBegin( );
  std::cout << "img size " << img->GetLargestPossibleRegion( ).GetSize( ) << " rgb size " << RGB_2_0->GetLargestPossibleRegion( ).GetSize( ) << std::endl;

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
  std::cout << " buscando " << basename1+str << std::endl;
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
    std::cout << " escribiendo " << basename1+ext << std::endl;
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
