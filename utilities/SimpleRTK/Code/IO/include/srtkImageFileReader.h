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
#ifndef __srtkImageFileReader_h
#define __srtkImageFileReader_h

#include "srtkMacro.h"
#include "srtkImage.h"
#include "srtkImageReaderBase.h"
#include "srtkMemberFunctionFactory.h"

namespace rtk {
  namespace simple {

    /** \class ImageFileReader
     * \brief Read a 2D or 3D image and return a smart pointer to a SimpleRTK
     * image
     *
     * This reader handles scalar and vector images and returns an image with
     * the same type as the file on disk.
     *
     * \sa rtk::simple::ReadImage for the procedural interface
     */
    class SRTKIO_EXPORT ImageFileReader
      : public ImageReaderBase
    {
    public:
      typedef ImageFileReader Self;

      /** Print ourselves to string */
      virtual std::string ToString() const;

      /** return user readable name fo the filter */
      virtual std::string GetName() const { return std::string("ImageFileReader"); }

      SRTK_RETURN_SELF_TYPE_HEADER SetFileName ( std::string fn );
      std::string GetFileName() const;

      Image Execute();

      ImageFileReader();

    protected:

      template <class TImageType> Image ExecuteInternal ( itk::ImageIOBase * );

    private:

      // function pointer type
      typedef Image (Self::*MemberFunctionType)( itk::ImageIOBase * );

      // friend to get access to executeInternal member
      friend struct detail::MemberFunctionAddressor<MemberFunctionType>;
      nsstd::auto_ptr<detail::MemberFunctionFactory<MemberFunctionType> > m_MemberFactory;

      std::string m_FileName;
    };

  SRTKIO_EXPORT Image ReadImage ( std::string filename, PixelIDValueEnum outputPixelType = srtkUnknown );
  }
}

#endif
