/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkImage.txx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkImage_txx
#define _itkImage_txx
#include "itkImage.h"
#include "itkProcessObject.h"
#include "itkImageRegionIterator.h"
namespace itk
{

/**
 *
 */
template<class TPixel, unsigned int VImageDimension, class TImageTraits>
Image<TPixel, VImageDimension, TImageTraits>
::Image()
: m_PixelAccessor ()
{
  m_Buffer = PixelContainer::New();

  unsigned int i;
  for (i=0; i < VImageDimension; i++)
    {
    m_Spacing[i] = 1.0;
    m_Origin[i] = 0.0;
    }
}


/**
 *
 */
template<class TPixel, unsigned int VImageDimension, class TImageTraits>
Image<TPixel, VImageDimension, TImageTraits>
::~Image()
{
}


//----------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension, class TImageTraits>
void 
Image<TPixel, VImageDimension, TImageTraits>
::Allocate()
{
  unsigned long num;

  this->ComputeOffsetTable();
  num = this->GetOffsetTable()[VImageDimension];
  
  m_Buffer->Reserve(num);
}

template<class TPixel, unsigned int VImageDimension, class TImageTraits>
void 
Image<TPixel, VImageDimension, TImageTraits>
::Initialize()
{
  //
  // We don't modify ourselves because the "ReleaseData" methods depend upon
  // no modification when initialized.
  //

  // Call the superclass which should initialize the BufferedRegion ivar.
  Superclass::Initialize();

  // Release the memory for the buffer
  m_Buffer->Initialize();
}


template<class TPixel, unsigned int VImageDimension, class TImageTraits>
void 
Image<TPixel, VImageDimension, TImageTraits>
::FillBuffer (const TPixel& value)
{
  ImageRegionIterator<Self> it(this,this->GetBufferedRegion());
  while(!it.IsAtEnd())
    {
    it.Set(value);
    ++it;
    }
}

template<class TPixel, unsigned int VImageDimension, class TImageTraits>
void 
Image<TPixel, VImageDimension, TImageTraits>
::SetPixelContainer(PixelContainer *container)
{
  if (m_Buffer != container)
    {
    m_Buffer = container;
    this->Modified();
    }
}
    


//----------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension, class TImageTraits>
void 
Image<TPixel, VImageDimension, TImageTraits>
::SetSpacing(const double spacing[ImageDimension] )
{
  unsigned int i; 
  for (i=0; i<VImageDimension; i++)
    {
    if ( spacing[i] != m_Spacing[i] )
      {
      break;
      }
    } 
  if ( i < VImageDimension ) 
    { 
    for (i=0; i<VImageDimension; i++)
      {
      m_Spacing[i] = spacing[i];
      }
    }

  this->RebuildTransforms();
}

//----------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension, class TImageTraits>
void 
Image<TPixel, VImageDimension, TImageTraits>
::SetSpacing(const float spacing[ImageDimension] )
{
  unsigned int i; 
  for (i=0; i<VImageDimension; i++)
    {
    if ( (double)spacing[i] != m_Spacing[i] )
      {
      break;
      }
    } 
  if ( i < VImageDimension ) 
    { 
    for (i=0; i<VImageDimension; i++)
      {
      m_Spacing[i] = spacing[i];
      }
    }

  this->RebuildTransforms();
}



//----------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension, class TImageTraits>
const double *
Image<TPixel, VImageDimension, TImageTraits>
::GetSpacing() const
{
  return m_Spacing;
}




//----------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension, class TImageTraits>
void 
Image<TPixel, VImageDimension, TImageTraits>
::SetOrigin(const double origin[ImageDimension] )
{
  unsigned int i; 
  for (i=0; i<VImageDimension; i++)
    {
    if ( origin[i] != m_Origin[i] )
      {
      break;
      }
    } 
  if ( i < VImageDimension ) 
    { 
    for (i=0; i<VImageDimension; i++)
      {
      m_Origin[i] = origin[i];
      }
    }

  this->RebuildTransforms();
}


//----------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension, class TImageTraits>
void 
Image<TPixel, VImageDimension, TImageTraits>
::SetOrigin(const float origin[ImageDimension] )
{
  unsigned int i; 
  for (i=0; i<VImageDimension; i++)
    {
    if ( (double)origin[i] != m_Origin[i] )
      {
      break;
      }
    } 
  if ( i < VImageDimension ) 
    { 
    for (i=0; i<VImageDimension; i++)
      {
      m_Origin[i] = origin[i];
      }
    }

  this->RebuildTransforms();
}


//----------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension, class TImageTraits>
void 
Image<TPixel, VImageDimension, TImageTraits>
::SetOrigin(const OriginOffsetType & origin )
{
  unsigned int i; 
  for (i=0; i<VImageDimension; i++)
    {
    if ( (double)origin[i] != m_Origin[i] )
      {
      break;
      }
    } 
  if ( i < VImageDimension ) 
    { 
    for (i=0; i<VImageDimension; i++)
      {
      m_Origin[i] = origin[i];
      }
    }
     
  this->RebuildTransforms();
}


//----------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension, class TImageTraits>
const double *
Image<TPixel, VImageDimension, TImageTraits>
::GetOrigin() const
{
  return m_Origin;
}

//---------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension, class TImageTraits>
void 
Image<TPixel, VImageDimension, TImageTraits>
::RebuildTransforms()
{
  typename AffineTransformType::MatrixType matrix;
  typename AffineTransformType::OffsetType offset;

  for (unsigned int i = 0; i < VImageDimension; i++)
    {
    for (unsigned int j = 0; j < VImageDimension; j++)
      {
      matrix[i][j] = 0.0;
      }
    matrix[i][i] = m_Spacing[i];
    offset[i]    = m_Origin [i];
    }

  // Create a new transform if one doesn't already exist
  if ( !m_IndexToPhysicalTransform )
    m_IndexToPhysicalTransform = AffineTransformType::New();

  m_IndexToPhysicalTransform->SetMatrix(matrix);
  m_IndexToPhysicalTransform->SetOffset(offset);

  for (unsigned int i = 0; i < VImageDimension; i++)
    {
    for (unsigned int j = 0; j < VImageDimension; j++)
      {
      matrix[i][j] = 0.0;
      }
    matrix[i][i] = 1.0 / m_Spacing[i];
    offset[i]    = -m_Origin[i] / m_Spacing[i];
    }

  // Create a new transform if one doesn't already exist
  if( !m_PhysicalToIndexTransform  )
    {
    m_PhysicalToIndexTransform = AffineTransformType::New();
    }
  
  m_PhysicalToIndexTransform->SetMatrix(matrix);
  m_PhysicalToIndexTransform->SetOffset(offset);

  this->Modified();

}


//---------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension, class TImageTraits>
typename Image<TPixel, VImageDimension, TImageTraits>::AffineTransformPointer
Image<TPixel, VImageDimension, TImageTraits>
::GetIndexToPhysicalTransform(void)
{
  
  if ( !m_IndexToPhysicalTransform )
    {
    RebuildTransforms();
    }
      
  return m_IndexToPhysicalTransform;
}


//---------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension, class TImageTraits>
typename Image<TPixel, VImageDimension, TImageTraits>::AffineTransformPointer
Image<TPixel, VImageDimension, TImageTraits>
::GetPhysicalToIndexTransform(void)
{

  if ( !m_PhysicalToIndexTransform )
    {
    RebuildTransforms();
    }

  return m_PhysicalToIndexTransform;
}

//----------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension, class TImageTraits>
void
Image<TPixel, VImageDimension, TImageTraits>
::CopyInformation(const DataObject *data)
{
  // Standard call to the superclass' method
  Superclass::CopyInformation(data);

  // Attempt to cast data to an ImageBase.  All subclasses of ImageBase
  // respond to GetSpacing(), GetOrigin()
  const ImageBase<VImageDimension> *phyData;
  
  phyData = dynamic_cast<const ImageBase<VImageDimension>*>(data);

  if (phyData)
    {
    // Copy the origin and spacing
    this->SetSpacing( phyData->GetSpacing() );
    this->SetOrigin( phyData->GetOrigin() );
    }
  else
    {
    // pointer could not be cast back down
    itkWarningMacro(<< "itk::Image::CopyInformation() cannot cast "
              << typeid(data).name() << " to "
              << typeid(ImageBase<VImageDimension>*).name() );
    }
}


/**
 *
 */
template<class TPixel, unsigned int VImageDimension, class TImageTraits>
void 
Image<TPixel, VImageDimension, TImageTraits>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  
  unsigned int i;
  os << indent << "Origin: [";
  for (i=0; i < VImageDimension - 1; i++)
    {
    os << m_Origin[i] << ", ";
    }
  os << m_Origin[i] << "]" << std::endl;
  os << indent << "Spacing: [";
  for (i=0; i < VImageDimension - 1; i++)
    {
    os << m_Spacing[i] << ", ";
    }
  os << m_Spacing[i] << "]" << std::endl;

  os << indent << "PixelContainer: " << m_Buffer << std::endl;
  if (m_IndexToPhysicalTransform)
    {       
    os << indent << "IndexToPhysicalTransform: " << m_IndexToPhysicalTransform << std::endl;
    }
  if (m_PhysicalToIndexTransform)
    {
    os << indent << "PhysicalToIndexTransform: " << m_PhysicalToIndexTransform << std::endl;
    }
}


} // end namespace itk

#endif
