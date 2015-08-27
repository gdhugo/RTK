/*=========================================================================
 *
 *  Copyright Insight Software Consortium & RTK Consortium
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
#ifdef _MFC_VER
#pragma warning(disable:4996)
#endif

#include "srtkImageFileReader.h"

#include <rtkConfiguration.h>
#ifdef RTK_USE_CUDA
# include <itkCudaImage.h>
#endif
#include <itkImageFileReader.h>


namespace rtk {
  namespace simple {

    Image ReadImage ( std::string filename ) {
      ImageFileReader reader; return reader.SetFileName ( filename ).Execute();
    }

    ImageFileReader::ImageFileReader() {

      // list of pixel types supported
      typedef NonLabelPixelIDTypeList PixelIDTypeList;

      this->m_MemberFactory.reset( new detail::MemberFunctionFactory<MemberFunctionType>( this ) );

      this->m_MemberFactory->RegisterMemberFunctions< PixelIDTypeList, 3 > ();
      this->m_MemberFactory->RegisterMemberFunctions< PixelIDTypeList, 2 > ();
    }

    std::string ImageFileReader::ToString() const {

      std::ostringstream out;
      out << "rtk::simple::ImageFileReader";
      out << std::endl;

      out << "  FileName: \"";
      this->ToStringHelper(out, this->m_FileName);
      out << "\"" << std::endl;

      return out.str();
    }

    ImageFileReader& ImageFileReader::SetFileName ( std::string fn ) {
      this->m_FileName = fn;
      return *this;
    }

    std::string ImageFileReader::GetFileName() const {
      return this->m_FileName;
    }

    Image ImageFileReader::Execute () {

      PixelIDValueType type = srtkUnknown;
      unsigned int dimension = 0;

      this->GetPixelIDFromImageIO( this->m_FileName, type, dimension );

      if ( dimension != 2 && dimension != 3 )
        {
        srtkExceptionMacro( "The file in the series have unsupported " << dimension - 1 << " dimensions." );
        }

      if ( !this->m_MemberFactory->HasMemberFunction( type, dimension ) )
        {
        srtkExceptionMacro( << "PixelType is not supported!" << std::endl
                            << "Pixel Type: "
                            << GetPixelIDValueAsString( type ) << std::endl
                            << "Refusing to load! " << std::endl );
        }

      return this->m_MemberFactory->GetMemberFunction( type, dimension )();
    }

  template <class TImageType>
  Image
  ImageFileReader::ExecuteInternal( void )
  {

    typedef TImageType                      ImageType;
    typedef itk::ImageFileReader<ImageType> Reader;

    // if the InstantiatedToken is correctly implemented this should
    // not occour
    assert( ImageTypeToPixelIDValue<ImageType>::Result != (int)srtkUnknown );

    typename Reader::Pointer reader = Reader::New();
    reader->SetFileName( this->m_FileName.c_str() );

    this->PreUpdate( reader.GetPointer() );

    reader->Update();

    return Image( reader->GetOutput() );
  }

  }
}
