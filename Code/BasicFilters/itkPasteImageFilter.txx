/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkPasteImageFilter.txx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkPasteImageFilter_txx
#define _itkPasteImageFilter_txx

#include "itkPasteImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkObjectFactory.h"
#include "itkProgressReporter.h"


namespace itk
{

/**
 *
 */
template <class TInputImage>
PasteImageFilter<TInputImage>
::PasteImageFilter() 
{
  this->ProcessObject::SetNumberOfRequiredInputs(2);

  m_DestinationIndex.Fill(0);
}


/**
 *
 */
template <class TInputImage>
void 
PasteImageFilter<TInputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);

  os << indent << "DestinationIndex: " << m_DestinationIndex << std::endl;
  os << indent << "SourceRegion: " << m_SourceRegion << std::endl;
}


/**
 *
 */
template <class TInputImage>
void
PasteImageFilter<TInputImage>
::GenerateInputRequestedRegion()
{
  // call the superclass' implementation of this method
  Superclass::GenerateInputRequestedRegion();

  // get the pointers for the inputs and output
  InputImagePointer  destPtr = 
      const_cast< TInputImage * >( this->GetInput() );
  InputImagePointer  sourcePtr = 
      const_cast< TInputImage * >( this->GetInput(1) );
  OutputImagePointer outputPtr = this->GetOutput();

  // second input must include the SourceRegion
  sourcePtr->SetRequestedRegion( m_SourceRegion );

  // first input must match the output requested region
  destPtr->SetRequestedRegion( outputPtr->GetRequestedRegion() );
}



  /** 
   * PasteImageFilter can be implemented as a multithreaded filter.
   * Therefore, this implementation provides a ThreadedGenerateData()
   * routine which is called for each processing thread. The output
   * image data is allocated automatically by the superclass prior to
   * calling ThreadedGenerateData().  ThreadedGenerateData can only
   * write to the portion of the output image specified by the
   * parameter "outputRegionForThread"
   *
   * \sa ImageToImageFilter::ThreadedGenerateData(),
   *     ImageToImageFilter::GenerateData() 
   */
template <class TInputImage>
void 
PasteImageFilter<TInputImage>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                       int threadId)
{
  itkDebugMacro(<<"Actually executing");

  // Get the input and output pointers
  typename Superclass::InputImageConstPointer  destPtr = this->GetInput();
  typename Superclass::InputImageConstPointer  sourcePtr = this->GetInput(1);
  typename Superclass::OutputImagePointer outputPtr = this->GetOutput();

  // support progress methods/callbacks
  ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());
  
  // What is the region on the destination image would be overwritten by the source?
  // Do we need to use the source image at all for the region generated by this thread?
  bool useSource;
  InputImageRegionType sourceRegionInDestinationImage;
  InputImageRegionType sourceRegionInDestinationImageCropped;
  sourceRegionInDestinationImage.SetIndex( m_DestinationIndex );
  sourceRegionInDestinationImage.SetSize( m_SourceRegion.GetSize() );

  if ( sourceRegionInDestinationImage.Crop( outputRegionForThread ) )
    {
    // paste region is inside this thread
    useSource = true;
    sourceRegionInDestinationImageCropped = sourceRegionInDestinationImage;
    }
  else
    {
    // paste region is outside this thread
    useSource = false;
    }

  // If the source image needs to be used to generate the output image, does the
  // destination image need to be used? i.e. will the source region completely
  // overlap the destination region for this thread?
  bool useOnlySource;
  if ( useSource && (sourceRegionInDestinationImageCropped == outputRegionForThread) )
    {
    // sourceRegionInDestinationImage completely overlaps the output
    // region for this thread, so we'll only copy data from the source
    useOnlySource = true;
    }
  else
    {
    // sourceRegionInDestinationImage only partially overlaps the
    // output region for this thread so we need to copy from both
    // inputs
    useOnlySource = false;
    }

  // If the source needs to be used, what part of the source needs to copied
  // by this thread?
  InputImageRegionType sourceRegionInSourceImageCropped;
  if ( useSource )
    {
    // what is the proposed shift from destination to source?
    Offset<InputImageDimension> originalOffsetFromDestinationToSource;
    originalOffsetFromDestinationToSource = m_SourceRegion.GetIndex() - m_DestinationIndex;

    // transform the cropped index back into the source image
    InputImageIndexType sourceIndexInSourceImageCropped;
    sourceIndexInSourceImageCropped = sourceRegionInDestinationImageCropped.GetIndex()
      + originalOffsetFromDestinationToSource;

    // set the values in the region
    sourceRegionInSourceImageCropped.SetIndex( sourceIndexInSourceImageCropped );
    sourceRegionInSourceImageCropped.SetSize( sourceRegionInDestinationImageCropped.GetSize() );
    }
  
  
  // Define iterators types
  typedef ImageRegionIterator<TInputImage> OutputIterator;
  typedef ImageRegionConstIterator<TInputImage> InputIterator;

  // There are three cases that we need to consider:
  //
  // 1. source region does not impact this thread, so copy data from
  //    from the destination image to the output
  //
  // 2. source region completely overlaps the output region for this
  //    thread, so copy data from the source image to the output
  //
  // 3. source region partially overlaps the output region for this
  //    thread, so copy data as needed from both the source and
  //    destination.
  //
  if ( !useSource )
    {
    // paste region is outside this thread, so just copy the destination
    // input to the output
    OutputIterator outIt(outputPtr, outputRegionForThread);
    InputIterator destIt(destPtr, outputRegionForThread);
    
    // walk the output region, and sample the destination image
    while( !outIt.IsAtEnd() )
      {
      // copy the input pixel to the output
      outIt.Set( destIt.Get());
      ++outIt; 
      ++destIt; 
      progress.CompletedPixel();
      }
    }
  else if ( useOnlySource )
    {
    // paste region completely overlaps the output region
    // for this thread, so copy data from the second input
    // to the output
    OutputIterator outIt(outputPtr, outputRegionForThread);
    InputIterator sourceIt(sourcePtr, sourceRegionInSourceImageCropped);
    
    // walk the output region, and sample the source image
    while( !outIt.IsAtEnd() )
      {
      // copy the input pixel to the output
      outIt.Set( sourceIt.Get());
      ++outIt; 
      ++sourceIt; 
      progress.CompletedPixel();
      }
    }
  else
    {
    // paste region partially overlaps the output region for the
    // thread, so we need copy data from both inputs as necessary. the
    // following code could be optimized.  this case could be
    // decomposed further such the output is broken into a set of
    // regions where each region would get data from either the
    // destination or the source images (but not both).  but for the
    // sake of simplicity and running under the assumption that the
    // source image is smaller than the destination image, we'll just
    // copy the destination to the output then overwrite the
    // appropriate output pixels with the source.

    // Copy destination to output
    //
    OutputIterator outIt(outputPtr, outputRegionForThread);
    InputIterator destIt(destPtr, outputRegionForThread);
    
    // walk the output region, and sample the destination image
    while( !outIt.IsAtEnd() )
      {
      // copy the input pixel to the output
      outIt.Set( destIt.Get());
      ++outIt; 
      ++destIt; 
      progress.CompletedPixel();
      }


    // Copy source to output
    //
    InputIterator sourceIt(sourcePtr, sourceRegionInSourceImageCropped);

    // reset the output iterator to walk the section of the image covered
    // by the source image
    outIt = OutputIterator(outputPtr, sourceRegionInDestinationImageCropped);
    
    // walk the output region, and sample the source image
    while( !outIt.IsAtEnd() )
      {
      // copy the input pixel to the output
      outIt.Set( sourceIt.Get());
      ++outIt; 
      ++sourceIt; 
      progress.CompletedPixel();
      }
      
    }
}

} // end namespace itk

#endif
