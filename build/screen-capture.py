#!/usr/bin/env python3

# https://stackoverflow.com/questions/12978846/python-get-screen-pixel-value-in-os-x
# http://neverfear.org/blog/view/156/OS_X_Screen_capture_from_Python_PyObjC
# python3 -m venv env, source env/bin/activate, python3 -m pip install numpy

# For error: cannot link directly with dylib/framework, your binary is not an allowed client of 
# /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks//Python.framework/Python.tbd
# Add to allowable-clients list

# Uncomment numpy and imageio for writing image to file
#import numpy as np
#import imageio
import os
import Quartz.CoreGraphics as CG
import struct

# Create screenshot as CGImage
image = CG.CGWindowListCreateImage(CG.CGRectInfinite, CG.kCGWindowListOptionOnScreenOnly, CG.kCGNullWindowID, CG.kCGWindowImageDefault)

# Intermediate step, get pixel data as CGDataProvider
prov = CG.CGImageGetDataProvider(image)
# Copy data out of CGDataProvider, becomes string of bytes
_data = CG.CGDataProviderCopyData(prov)

# Get width/height of image
width = CG.CGImageGetWidth(image)
height = CG.CGImageGetHeight(image)

# Pixel data is unsigned char (8bit unsigned integer),
# and there are for (blue,green,red,alpha)
data_format = "BBBB"
# Pixel location coordinates
x = 1505
y = 10

# Calculate offset, based on http://www.markj.net/iphone-uiimage-pixel-color/
offset = 4 * ((width*int(round(y))) + int(round(x)))

# Unpack data from string into Python'y integers
b, g, r, a = struct.unpack_from(data_format, _data, offset=offset)
print(b, g, r, a);

# Delete previous indicator files
if os.path.exists('dark.true'):
    os.remove('dark.true')
if os.path.exists('dark.false'):
    os.remove('dark.false')

# If icon is black or white, write respective indicator file
if (b == 255 and r == 255 and g == 255):
    file_object = open('dark.true', 'a')
    file_object.close()
elif (b == 0 and r == 0 and g == 0):
    file_object = open('dark.false', 'a')
    file_object.close()
else:
    # Open a file with access mode 'a'
    file_object = open('/Users/luv/Desktop/wxpassman.log', 'a')
    # Append string at the end of file
    file_object.write('screen-capture.py error: unexpected color value')
    # Close the file
    file_object.close()

# Write image to file
#imgdata = np.fromstring(_data,dtype=np.uint8).reshape(len(_data)//4,4)
#numpy_img = imgdata[:width*height,:-1].reshape(height,width,3)
#imageio.imwrite('screen-capture.png', numpy_img)
