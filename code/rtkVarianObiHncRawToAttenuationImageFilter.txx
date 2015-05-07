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

#ifndef __rtkVarianObiHncRawToAttenuationImageFilter_txx
#define __rtkVarianObiHncRawToAttenuationImageFilter_txx

#include <itkImageFileWriter.h>
#include <itksys/SystemTools.hxx>
#include <itkRegularExpressionSeriesFileNames.h>

#include "rtkMacro.h"

namespace rtk
{

template <class TInputImage, class TOutputImage>
VarianObiHncRawToAttenuationImageFilter<TInputImage, TOutputImage>
::VarianObiHncRawToAttenuationImageFilter() :
  m_FloodProjectionsReader( HncImageSeries::New() )
{
}

template<class TInputImage, class TOutputImage>
void
VarianObiHncRawToAttenuationImageFilter<TInputImage, TOutputImage>
::BeforeThreadedGenerateData()
{
  if( m_FileNames.size() != 1 )
    {
    itkGenericExceptionMacro(<< "Error, more than one norm file found.");
    }

  std::string path = itksys::SystemTools::GetFilenamePath(m_FileNames[0]);
  std::vector<std::string> pathComponents;
  itksys::SystemTools::SplitPath(m_FileNames[0].c_str(), pathComponents);
  std::string fileName = pathComponents.back();

  // Reference image (flood field)
  FileNamesContainer floodFilenames;
  floodFilenames.push_back( path + std::string("/norm.hnc") );
  m_FloodProjectionsReader->SetFileNames( floodFilenames );
  m_FloodProjectionsReader->Update();
}

template<class TInputImage, class TOutputImage>
void
VarianObiHncRawToAttenuationImageFilter<TInputImage, TOutputImage>
::ThreadedGenerateData( const OutputImageRegionType& outputRegionForThread, ThreadIdType itkNotUsed(threadId) )
{
  // Flood image iterator
  OutputImageRegionType floodRegion = outputRegionForThread;

  floodRegion.SetSize(2,1);
  floodRegion.SetIndex(2,0);
  itk::ImageRegionConstIterator<InputImageType> itFlood(m_FloodProjectionsReader->GetOutput(), floodRegion);

  // Projection region
  OutputImageRegionType outputRegionSlice = outputRegionForThread;
  outputRegionSlice.SetSize(2,1);

  for(int k = outputRegionForThread.GetIndex(2);
      k < outputRegionForThread.GetIndex(2) + (int)outputRegionForThread.GetSize(2);
      k++)
    {
    outputRegionSlice.SetIndex(2,k);

    // Create iterators
    itk::ImageRegionConstIterator<InputImageType> itIn(this->GetInput(), outputRegionSlice);
    itk::ImageRegionIterator<OutputImageType>     itOut(this->GetOutput(), outputRegionSlice);

    itFlood.GoToBegin();
    while( !itFlood.IsAtEnd() )
      {
      // The reference has been exactly acquired at the same position
      itOut.Set( -log( (double)itIn.Get() / (double)itFlood.Get() ) );
      ++itIn;
      ++itOut;
      ++itFlood;
      }
    }

}

} // end namespace rtk

#endif
