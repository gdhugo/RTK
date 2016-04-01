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
#ifndef __rtkWarpProjectionStackToFourDImageFilter_hxx
#define __rtkWarpProjectionStackToFourDImageFilter_hxx

#include "rtkWarpProjectionStackToFourDImageFilter.h"
#include "rtkGeneralPurposeFunctions.h"

#include <itkObjectFactory.h>
#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>

namespace rtk
{

template< typename VolumeSeriesType, typename ProjectionStackType, typename TMVFImageSequence, typename TMVFImage>
WarpProjectionStackToFourDImageFilter< VolumeSeriesType, ProjectionStackType, TMVFImageSequence, TMVFImage>::WarpProjectionStackToFourDImageFilter()
{
  this->SetNumberOfRequiredInputs(3);
  m_MVFInterpolatorFilter = MVFInterpolatorType::New();

  this->m_UseCudaSplat = true;
  this->m_UseCudaSources = true;

#ifdef RTK_USE_CUDA
  this->m_BackProjectionFilter = rtk::CudaWarpBackProjectionImageFilter::New();
#else
  this->m_BackProjectionFilter = rtk::BackProjectionImageFilter<VolumeType, VolumeType>::New();
  itkWarningMacro("The warp back project image filter exists only in CUDA. Ignoring the displacement vector field and using CPU voxel-based back projection")
#endif
}

template< typename VolumeSeriesType, typename ProjectionStackType, typename TMVFImageSequence, typename TMVFImage>
void
WarpProjectionStackToFourDImageFilter< VolumeSeriesType, ProjectionStackType, TMVFImageSequence, TMVFImage>::SetDisplacementField(const TMVFImageSequence* DisplacementField)
{
  this->SetNthInput(2, const_cast<TMVFImageSequence*>(DisplacementField));
}

template< typename VolumeSeriesType, typename ProjectionStackType, typename TMVFImageSequence, typename TMVFImage>
typename TMVFImageSequence::ConstPointer
WarpProjectionStackToFourDImageFilter< VolumeSeriesType, ProjectionStackType, TMVFImageSequence, TMVFImage>::GetDisplacementField()
{
  return static_cast< const TMVFImageSequence * >
          ( this->itk::ProcessObject::GetInput(2) );
}

template< typename VolumeSeriesType, typename ProjectionStackType, typename TMVFImageSequence, typename TMVFImage>
void
WarpProjectionStackToFourDImageFilter< VolumeSeriesType, ProjectionStackType, TMVFImageSequence, TMVFImage>
::SetSignalFilename(const std::string _arg)
{
  itkDebugMacro("setting SignalFilename to " << _arg);
  if ( this->m_SignalFilename != _arg )
    {
    this->m_SignalFilename = _arg;
    this->Modified();

    std::ifstream is( _arg.c_str() );
    if( !is.is_open() )
      {
      itkGenericExceptionMacro(<< "Could not open signal file " << m_SignalFilename);
      }

    double value;
    while( !is.eof() )
      {
      is >> value;
      m_Signal.push_back(value);
      }
    }
  m_MVFInterpolatorFilter->SetSignalVector(m_Signal);
}

template< typename VolumeSeriesType, typename ProjectionStackType, typename TMVFImageSequence, typename TMVFImage>
void
WarpProjectionStackToFourDImageFilter< VolumeSeriesType, ProjectionStackType, TMVFImageSequence, TMVFImage>
::GenerateOutputInformation()
{
  m_MVFInterpolatorFilter->SetInput(this->GetDisplacementField());
  m_MVFInterpolatorFilter->SetFrame(0);

#ifdef RTK_USE_CUDA
  dynamic_cast< rtk::CudaWarpBackProjectionImageFilter* >
      (this->m_BackProjectionFilter.GetPointer())->SetDisplacementField(m_MVFInterpolatorFilter->GetOutput());
#endif

  Superclass::GenerateOutputInformation();
}

template< typename VolumeSeriesType, typename ProjectionStackType, typename TMVFImageSequence, typename TMVFImage>
void
WarpProjectionStackToFourDImageFilter< VolumeSeriesType, ProjectionStackType, TMVFImageSequence, TMVFImage>
::GenerateData()
{
  int Dimension = ProjectionStackType::ImageDimension;

  // Set the Extract filter
  typename ProjectionStackType::RegionType extractRegion;
  extractRegion = this->GetInputProjectionStack()->GetLargestPossibleRegion();
  extractRegion.SetSize(Dimension-1, 1);

  // Declare the pointer to a VolumeSeries that will be used in the pipeline
  typename VolumeSeriesType::Pointer pimg;

  int NumberProjs = this->GetInputProjectionStack()->GetLargestPossibleRegion().GetSize(Dimension-1);
  int FirstProj = this->GetInputProjectionStack()->GetLargestPossibleRegion().GetIndex(Dimension-1);

  // Get an index permutation that sorts the signal values. Then process the projections
  // in that permutated order. This way, projections with identical phases will be
  // processed one after the other. This will save some of the MVF interpolation operations.
  std::vector<unsigned int> IndicesOfProjectionsSortedByPhase = GetSortingPermutation<double>(this->m_Signal);

  bool firstProjectionProcessed = false;

  // Process the projections in permutated order
  for (int i = 0 ; i < this->m_Signal.size(); i++)
    {
    // Make sure the current projection is in the input projection stack's largest possible region
    this->m_ProjectionNumber = IndicesOfProjectionsSortedByPhase[i];
    if ((this->m_ProjectionNumber >= FirstProj) && (this->m_ProjectionNumber<FirstProj+NumberProjs))
      {
      // After the first update, we need to use the output as input.
      if(firstProjectionProcessed)
        {
        pimg = this->m_SplatFilter->GetOutput();
        pimg->DisconnectPipeline();
        this->m_SplatFilter->SetInputVolumeSeries( pimg );
        }

      // Set the Extract Filter
      extractRegion.SetIndex(Dimension-1, this->m_ProjectionNumber);
      this->m_ExtractFilter->SetExtractionRegion(extractRegion);

      // Set the MVF interpolator
      m_MVFInterpolatorFilter->SetFrame(this->m_ProjectionNumber);

      // Set the splat filter
      this->m_SplatFilter->SetProjectionNumber(this->m_ProjectionNumber);

      // Update the last filter
      this->m_SplatFilter->Update();

      // Update condition
      firstProjectionProcessed = true;
      }
    }

  // Graft its output
  this->GraftOutput( this->m_SplatFilter->GetOutput() );

  // Release the data in internal filters
  if(pimg.IsNotNull())
    pimg->ReleaseData();

  this->m_DisplacedDetectorFilter->GetOutput()->ReleaseData();
  this->m_BackProjectionFilter->GetOutput()->ReleaseData();
  this->m_ExtractFilter->GetOutput()->ReleaseData();
  this->m_ConstantVolumeSource->GetOutput()->ReleaseData();
  this->m_MVFInterpolatorFilter->GetOutput()->ReleaseData();
}

}// end namespace


#endif
