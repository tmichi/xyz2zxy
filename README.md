# xyz2zxy

Convert XY cross-sectional images to ZX cross-sectional images.

![Teaser image of xyz2zxy](xyz2zxy_teaser.png "Example of the result.")


## Overview
* This program converts 3D images defined by XY corss-sectional images to different cross-sectional images (ZX).
* Such images are easy to convert by reading all images at once, however it requires huge memory usage. 
* This program reduces memory usage by  divide-and-conquer approach (or using HDD for temporary data).
* This can be used for observation of very large serial-sectioning images in a different axis-aligned cross-section.

## Requirements
* CMake (> v.3.*.*)
* C++17 or later (filesystem)
* OpenCV (> v.4.5.0)
* ~~fmt (https://fmt.dev) (v.8.0.1 or later)~~ 

Lower version may work fine. 
## Build and Test 
### Unix-like System with CMake 
```bash
% mkdir build
% cd build
% cmake ..
% make  
%
```
### Validation
```bash
% ./make_sample 
% ./xyz2zxy -i sample -o output -n 4
Step1 divide:[********************] (256/256)
Step2 concat:[********************] (256/256)
% ./validate output
validation ok
%
```
* ``make check`` creates sasmple data and validates the computation result. 
### Windows (Visual Studio )
* Use CMake to create the solution file.
* Set CMAKE_PREFIX_PATH to OpenCV and fmt in order to call find_package().
### Other systems. 
* See CMakeLists.txt
## Usage
* ``xyz2zxy -i {input_dir} -o {output_dir} -n {n} -pitch {px} {pz}``
  * ``{input_dir}`` : the directory where images are contained.
  * ``{output_dir}`` : the directory where converted images are saved.
  * ``{px} {pz}`` : Custom pitch size 
  * ``n`` : the number of images that are loaded in the memory (Default : 100). Larger n computes faster, but requires large memory size.
*  ``make_sample, validate`` : executables for validation.
## License 
* MIT License
## Authors
* Takashi Michikawa <ram-axed-0b@icloud.com>, Image Processing Research Team, RIKEN Center for Advanced Photonics.
## Copyrights 
* Main codes : (c)2021 - now RIKEN
* mi/*.hpp : (c)2007 - now Takashi Michikawa
## Acknowlegments
* This work was supported by Council for Science, Technology and Innovation(CSTI), Cross-ministerial Strategic Innovation Promotion Program (SIP), “Materials Integration for revolutionary design system of structural materials”(Funding agency:JST).
