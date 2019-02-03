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
	TColorImage* img = reader->GetOutput( );
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
  	std::stringstream ss( argv[ 1 ] );
  	std::string basename;
  	getline( ss, basename, '.' );

  	TWriter::Pointer writerr = TWriter::New( );
    writerr->SetInput( rImg );
    writerr->SetFileName( basename + "_R.png" );

    TWriter::Pointer writerg = TWriter::New( );
    writerg->SetInput( gImg );
    writerg->SetFileName( basename + "_G.png" );

    TWriter::Pointer writerb = TWriter::New( );
    writerb->SetInput( bImg );
    writerb->SetFileName( basename + "_B.png" );


    try
  	{
    	writerr->Update( );
    	writerg->Update( );
    	writerb->Update( );
  	}
  	catch( itk::ExceptionObject& err )
  	{
    	std::cerr << "Error: " << err << std::endl;
    	return( 1 );
    } // yrt









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

  	

  	TWriter::Pointer writer = TWriter::New( );
  	writer->SetInput( resampleFilter->GetOutput( ) );
  	writer->SetFileName( basename + "_sR.png" );

  	TWriter::Pointer writer2 = TWriter::New( );
  	writer2->SetInput( resampleFilter2->GetOutput( ) );
  	writer2->SetFileName( basename + "_sG.png" );

  	TWriter::Pointer writer3 = TWriter::New( );
  	writer3->SetInput( resampleFilter3->GetOutput( ) );
  	writer3->SetFileName( basename + "_sB.png" );
  	try
  	{
    	writer->Update( );
    	writer2->Update( );
    	writer3->Update( );
  	}
  	catch( itk::ExceptionObject& err )
  	{
    	std::cerr << "Error: " << err << std::endl;
    	return( 1 );
 	} // yrt


 	// Read an image
	TReader::Pointer reader1 = TReader::New( );
	reader1->SetFileName( basename + "_sR.png" );
	try
	{
	    reader1->Update( );
	}
	catch( itk::ExceptionObject& err )
	{
	    std::cerr << "Error: " << err << std::endl;
	    return( 1 );

	} // yrt
	TColorImage* sR = reader1->GetOutput( );
	 // Read an image
	TReader::Pointer reader2 = TReader::New( );
	reader2->SetFileName( basename + "_sG.png" );
	try
	{
	    reader2->Update( );
	}
	catch( itk::ExceptionObject& err )
	{
	    std::cerr << "Error: " << err << std::endl;
	    return( 1 );

	} // yrt
	TColorImage* sG = reader2->GetOutput( );
	 // Read an image
	TReader::Pointer reader3 = TReader::New( );
	reader3->SetFileName( basename + "_sB.png" );
	try
	{
	    reader3->Update( );
	}
	catch( itk::ExceptionObject& err )
	{
	    std::cerr << "Error: " << err << std::endl;
	    return( 1 );

	} // yrt
	TColorImage* sB = reader3->GetOutput( );

 	redInputSize = sR->GetLargestPossibleRegion( ).GetSize( );
    greenInputSize = sG->GetLargestPossibleRegion( ).GetSize( );
    blueInputSize = sB->GetLargestPossibleRegion( ).GetSize( );

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

  	
  	writer->SetInput( resampleFilter->GetOutput( ) );
  	writer->SetFileName( basename + "_ssR.png" );

  	
  	writer2->SetInput( resampleFilter2->GetOutput( ) );
  	writer2->SetFileName( basename + "_ssG.png" );

  	
  	writer3->SetInput( resampleFilter3->GetOutput( ) );
  	writer3->SetFileName( basename + "_ssB.png" );

  	try
  	{
    	writer->Update( );
    	writer2->Update( );
    	writer3->Update( );
  	}
  	catch( itk::ExceptionObject& err )
  	{
    	std::cerr << "Error: " << err << std::endl;
    	return( 1 );
 	} // yrt



 	// Read an image
	TReader::Pointer reader4 = TReader::New( );
	reader4->SetFileName( basename + "_ssR.png" );
	try
	{
	    reader1->Update( );
	}
	catch( itk::ExceptionObject& err )
	{
	    std::cerr << "Error: " << err << std::endl;
	    return( 1 );

	} // yrt
	TColorImage* ssR = reader4->GetOutput( );
	 // Read an image
	TReader::Pointer reader5 = TReader::New( );
	reader5->SetFileName( basename + "_ssG.png" );
	try
	{
	    reader5->Update( );
	}
	catch( itk::ExceptionObject& err )
	{
	    std::cerr << "Error: " << err << std::endl;
	    return( 1 );

	} // yrt
	TColorImage* ssG = reader5->GetOutput( );
	 // Read an image
	TReader::Pointer reader6 = TReader::New( );
	reader6->SetFileName( basename + "_ssB.png" );
	try
	{
	    reader6->Update( );
	}
	catch( itk::ExceptionObject& err )
	{
	    std::cerr << "Error: " << err << std::endl;
	    return( 1 );

	} // yrt
	TColorImage* ssB = reader6->GetOutput( );

 	// composite image (RGB)
  	TColorImage::Pointer RGB_2_0 = TColorImage::New( );
  	RGB_2_0->SetSpacing( img->GetSpacing( ) );
  	RGB_2_0->SetOrigin( img->GetOrigin( ) );
  	RGB_2_0->SetLargestPossibleRegion( img->GetLargestPossibleRegion( ) );
  	RGB_2_0->SetRequestedRegion( img->GetRequestedRegion( ) );
  	RGB_2_0->SetBufferedRegion( img->GetBufferedRegion( ) );
  	RGB_2_0->Allocate( );

  	RGB_2_0->FillBuffer( black );

  	TColorIterator it2( RGB_2_0, RGB_2_0->GetLargestPossibleRegion( ) );
  	TColorIterator crIt2( ssR, ssR->GetLargestPossibleRegion( ) );
  	TColorIterator cgIt2( ssG, ssG->GetLargestPossibleRegion( ) );
  	TColorIterator cbIt2( ssB, ssB->GetLargestPossibleRegion( ) );

  	it2.GoToBegin( );
  	crIt2.GoToBegin( );
  	cgIt2.GoToBegin( );
  	cbIt2.GoToBegin( );

  	for( ; !it2.IsAtEnd( ) && !crIt2.IsAtEnd( ) && !cgIt2.IsAtEnd( ) && !cbIt2.IsAtEnd( ); ++it2, ++crIt2, ++cgIt2, ++cbIt2 )
  	{
	    TRGBPixel value, pixel;
	    value = crIt2.Get( );
	    pixel.SetRed( value.GetRed( ) );

	    value = cgIt2.Get( );
	    pixel.SetGreen( value.GetGreen( ) );

	    value = cbIt2.Get( );
	    pixel.SetBlue( value.GetBlue( ) );

	    it2.Set( pixel );

  	} // rof

  	writer->SetInput( RGB_2_0 );
  	writer->SetFileName( basename + "_rRGB.png" );


  	try
  	{
    	writer->Update( );
  	}
  	catch( itk::ExceptionObject& err )
  	{
    	std::cerr << "Error: " << err << std::endl;
    	return( 1 );
 	} // yrt


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

	writer->SetInput( diff );
	writer->SetFileName( basename + "_diff.png" );

	try
	{
  	writer->Update( );
	}
	catch( itk::ExceptionObject& err )
	{
  	std::cerr << "Error: " << err << std::endl;
  	return( 1 );
	} // yrt

	return 0;
}
