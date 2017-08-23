/*=========================================================================
*
*  Copyright Insight Software Consortium
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
#ifndef __srtkImageSeriesWriter_h
#define __srtkImageSeriesWriter_h

#include "srtkMacro.h"
#include "srtkImage.h"
#include "srtkProcessObject.h"
#include "srtkIO.h"
#include "srtkMemberFunctionFactory.h"

namespace rtk {
  namespace simple {

    /** \class ImageSeriesWriter
     * \brief Writer series of image from a SimpleITK image.
     *
     *
     * \sa itk::simple::WriteImage for the procedural interface
     **/
    class SRTKIO_EXPORT ImageSeriesWriter
      : public ProcessObject
    {
    public:
      typedef ImageSeriesWriter Self;

      ImageSeriesWriter();

      /** Print ourselves to string */
      virtual std::string ToString() const;

      /** return user readable name fo the filter */
      virtual std::string GetName() const { return std::string("ImageSeriesWriter"); }

      /** \brief Enable compression if available for file type.
       *
       * These methods Set/Get/Toggle the UseCompression flag which
       * get's passed to image file's itk::ImageIO object. This is
       * only a request as not all file formatts support compression.
       * @{ */
      SRTK_RETURN_SELF_TYPE_HEADER SetUseCompression( bool UseCompression );
      bool GetUseCompression( void ) const;

      SRTK_RETURN_SELF_TYPE_HEADER UseCompressionOn(void) { return this->SetUseCompression(true); }
      SRTK_RETURN_SELF_TYPE_HEADER UseCompressionOff(void) { return this->SetUseCompression(false); }
      /** @} */

      /** The filenames to where the image slices are written.
        *
        * The number of filenames must match the number of slices in
        * the image.
        * @{ */
      SRTK_RETURN_SELF_TYPE_HEADER SetFileNames(const std::vector<std::string> &fileNames);
      const std::vector<std::string> &GetFileNames() const;
      /** @} */


      SRTK_RETURN_SELF_TYPE_HEADER Execute(const Image&);
      SRTK_RETURN_SELF_TYPE_HEADER Execute(const Image &image, const std::vector<std::string> &inFileNames, bool inUseCompression);

    protected:

      template <class TImageType> Self &ExecuteInternal ( const Image& inImage );

    private:

      // function pointer type
      typedef Self& (Self::*MemberFunctionType)( const Image& );

      // friend to get access to executeInternal member
      friend struct detail::MemberFunctionAddressor<MemberFunctionType>;
      nsstd::auto_ptr<detail::MemberFunctionFactory<MemberFunctionType> > m_MemberFactory;

      bool m_UseCompression;
      std::vector<std::string> m_FileNames;
    };

  SRTKIO_EXPORT void WriteImage ( const Image & image, const std::vector<std::string> &fileNames, bool inUseCompression=false );
  }
}

#endif
