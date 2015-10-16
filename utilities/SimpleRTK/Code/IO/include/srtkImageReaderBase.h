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
#ifndef __srtkImageReaderBase_h
#define __srtkImageReaderBase_h

#include "srtkProcessObject.h"
#include "srtkPixelIDValues.h"
#include "srtkIO.h"

namespace itk {

// Forward decalaration for pointer
class ImageIOBase;

template<class T>
class SmartPointer;
}

namespace rtk {
  namespace simple {

    /** \class ImageReaderBase
     * \brief An abract base class for image readers
     *
     */
    class SRTKIO_EXPORT ImageReaderBase :
      public ProcessObject
    {
    public:
      // The default constructor and the destructor will be implicitly
      // declared as public.
      ImageReaderBase();

    protected:


      void GetPixelIDFromImageIO( const std::string &fileName,
                                  PixelIDValueType &outPixelType,
                                  unsigned int & outDimensions);

      unsigned int GetDimensionFromImageIO( const std::string &fileName, unsigned int i);

    private:

      PixelIDValueType ExecuteInternalReadScalar( int componentType );

      PixelIDValueType ExecuteInternalReadVector( int componentType );

      PixelIDValueType ExecuteInternalReadComplex( int componentType );

      itk::SmartPointer<itk::ImageIOBase> GetImageIOBase(const std::string &fileName);

    };
  }
}


#endif // __srtkImageReaderBase_h
