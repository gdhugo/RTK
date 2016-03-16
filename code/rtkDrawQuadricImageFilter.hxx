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

#ifndef __rtkDrawQuadricImageFilter_hxx
#define __rtkDrawQuadricImageFilter_hxx

#include <iostream>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>

#include "rtkDrawQuadricImageFilter.h"

namespace rtk
{

template <class TInputImage, class TOutputImage, class TSpatialObject, typename TFunction>
rtk::DrawQuadricImageFilter<TInputImage, TOutputImage, TSpatialObject, TFunction>
::DrawQuadricImageFilter()
{

}

}// end namespace rtk

#endif
