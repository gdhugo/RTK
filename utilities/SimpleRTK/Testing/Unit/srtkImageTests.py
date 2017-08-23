#==========================================================================
#
#   Copyright Insight Software Consortium & RTK Consortium
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#          http://www.apache.org/licenses/LICENSE-2.0.txt
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#==========================================================================*/

from __future__ import print_function
import unittest

import SimpleRTK as srtk
import sys


class ImageTests(unittest.TestCase):
    """These tests are suppose to test the python interface to the srtk::Image"""



    def setUp(self):
        pass

    def test_legacy(self):
        """ This is old testing cruft before tehe unittest enlightenment """

        image = srtk.Image( 10, 10, srtk.srtkInt32 )

        image + image
        image + 1
        1 + image
        image - image
        image - 1
        1 - image
        image * image
        image * 1
        1 * image
        image / image
        1.0 / image
        image / 1.0
        image // image
        image // 1
        1 // image
        image & image
        image | image
        image ^ image
        ~image

        image += image
        image -= image
        image *= image
        image /= image
        image //= image

        image = image * 0

        image.SetPixel( 0, 0, 1 )
        image[ [0,1] ]  = 2
        image[ 9,9 ]  = 3

        image.GetPixel( 1,1 )
        #image.GetPixel( [1,1] )
        image[1,1]
        image[ [ 1,1 ] ]

        self.assertEqual(sum( image ), 6)

        self.assertEqual(len( image ), 100)


if __name__ == '__main__':
    unittest.main()
