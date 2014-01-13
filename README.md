ArrayFire-Examples
==================

This repository contains example applications built using ArrayFire. 

The source code in this repository released under BSD 3-clause license. 
Please refer to the **LICENSE** file for more information.

Usage
------------------
### Linux

Linux makefiles have been provided with this repo. To compile an example,
go the the directory of the file and enter the following command:

For CUDA  : **`make cuda`**

For OpenCL: **`make opencl`**

The executables filenames generated are in the format **example_cuda/ocl**.

### Windows

#### Generate the Project Files

Once the repo is cloned, open the command prompt into the root folder of the
repo, ie. ArrayFire-Examples.
Now enter the following command:

**`.\vs-files\update.bat .\`**

This command will generate project and solution files for Visual Studio 2008,
2010 and 2012 for all the examples. These projects are also necessary if
using the consolidated solution files (AF_Examples_20*.sln).

### Running the Examples
Refer to the [ArrayFire Documentation]
(http://www.accelereyes.com/arrayfire/c/installation.htm#verifywindows) to see
the execution procedure.
