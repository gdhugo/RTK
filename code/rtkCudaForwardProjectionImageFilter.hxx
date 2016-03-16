/*=========================================================================
 *
 *  Copyright RTK Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#ifndef __rtkCudaForwardProjectionImageFilter_hxx
#define __rtkCudaForwardProjectionImageFilter_hxx

#include "rtkCudaForwardProjectionImageFilter.h"
#include "rtkCudaUtilities.hcu"
#include "rtkCudaForwardProjectionImageFilter.hcu"

#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkMacro.h>
#include "rtkMacro.h"
#include "itkCudaUtil.h"

namespace rtk
{

template <class TInputImage,
          class TOutputImage>
CudaForwardProjectionImageFilter<TInputImage,
                                 TOutputImage>
::CudaForwardProjectionImageFilter():
  m_StepSize(1),m_UseCudaTexture(true)
{
}

template <class TInputImage,
          class TOutputImage>
void
CudaForwardProjectionImageFilter<TInputImage,
                                 TOutputImage>
::GPUGenerateData()
{
  const typename Superclass::GeometryType::Pointer geometry = this->GetGeometry();
  const unsigned int Dimension = TInputImage::ImageDimension;
  const unsigned int iFirstProj = this->GetInput(0)->GetRequestedRegion().GetIndex(Dimension-1);
  const unsigned int nProj = this->GetInput(0)->GetRequestedRegion().GetSize(Dimension-1);
  const unsigned int nPixelsPerProj = this->GetOutput()->GetBufferedRegion().GetSize(0) *
    this->GetOutput()->GetBufferedRegion().GetSize(1);

  itk::Vector<double, 4> source_position;

  // Setting BoxMin and BoxMax
  // SR: we are using cuda textures where the pixel definition is not center but corner.
  // Therefore, we set the box limits from index to index+size instead of, for ITK,
  // index-0.5 to index+size-0.5.
  float boxMin[3];
  float boxMax[3];
  for(unsigned int i=0; i<3; i++)
    {
    boxMin[i] = this->GetInput(1)->GetBufferedRegion().GetIndex()[i]+0.5;
    boxMax[i] = boxMin[i] + this->GetInput(1)->GetBufferedRegion().GetSize()[i]-1.0;
    }

  // Getting Spacing
  float spacing[3];
  for(unsigned int i=0; i<3; i++)
    {
    spacing[i] = this->GetInput(1)->GetSpacing()[i];
    }

  // Cuda convenient format for dimensions
  int projectionSize[2];
  projectionSize[0] = this->GetOutput()->GetBufferedRegion().GetSize()[0];
  projectionSize[1] = this->GetOutput()->GetBufferedRegion().GetSize()[1];

  int volumeSize[3];
  volumeSize[0] = this->GetInput(1)->GetBufferedRegion().GetSize()[0];
  volumeSize[1] = this->GetInput(1)->GetBufferedRegion().GetSize()[1];
  volumeSize[2] = this->GetInput(1)->GetBufferedRegion().GetSize()[2];

  float *pin = *(float**)( this->GetInput(0)->GetCudaDataManager()->GetGPUBufferPointer() );
  float *pout = *(float**)( this->GetOutput()->GetCudaDataManager()->GetGPUBufferPointer() );
  float *pvol = *(float**)( this->GetInput(1)->GetCudaDataManager()->GetGPUBufferPointer() );

  // Go over each projection
  for(unsigned int iProj = iFirstProj; iProj < iFirstProj + nProj; iProj++)
    {
    // Account for system rotations
    typename Superclass::GeometryType::ThreeDHomogeneousMatrixType volPPToIndex;
    volPPToIndex = GetPhysicalPointToIndexMatrix( this->GetInput(1) );

    // Compute matrix to translate the pixel indices on the volume and the detector
    // if the Requested region has non-zero index
    typename Superclass::GeometryType::ThreeDHomogeneousMatrixType projIndexTranslation, volIndexTranslation;
    projIndexTranslation.SetIdentity();
    volIndexTranslation.SetIdentity();
    for(unsigned int i=0; i<3; i++)
      {
      projIndexTranslation[i][3] = this->GetOutput()->GetRequestedRegion().GetIndex(i);
      volIndexTranslation[i][3] = -this->GetInput(1)->GetBufferedRegion().GetIndex(i);

      if (m_UseCudaTexture)
        {
        // Adding 0.5 offset to change from the centered pixel convention (ITK)
        // to the corner pixel convention (CUDA).
        volPPToIndex[i][3] += 0.5;
        }
      }

    // Compute matrix to transform projection index to volume index
    typename Superclass::GeometryType::ThreeDHomogeneousMatrixType d_matrix;
    d_matrix =
      volIndexTranslation.GetVnlMatrix() *
      volPPToIndex.GetVnlMatrix() *
      geometry->GetProjectionCoordinatesToFixedSystemMatrix(iProj).GetVnlMatrix() *
      rtk::GetIndexToPhysicalPointMatrix( this->GetInput() ).GetVnlMatrix() *
      projIndexTranslation.GetVnlMatrix();
    float matrix[4][4];
    for (int j=0; j<4; j++)
      for (int k=0; k<4; k++)
        matrix[j][k] = (float)d_matrix[j][k];

    // Set source position in volume indices
    source_position = volPPToIndex * geometry->GetSourcePosition(iProj);

    int projectionOffset = iProj - this->GetOutput()->GetBufferedRegion().GetIndex(2);

    CUDA_forward_project(projectionSize,
                        volumeSize,
                        (float*)&(matrix[0][0]),
                        pin + nPixelsPerProj * projectionOffset,
                        pout + nPixelsPerProj * projectionOffset,
                        pvol,
                        m_StepSize,
                        (double*)&(source_position[0]),
                        boxMin,
                        boxMax,
                        spacing,
                        m_UseCudaTexture);
    }
}

} // end namespace rtk

#endif
