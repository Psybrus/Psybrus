rem this script is mainly to create distributable visual studio project file


genie --with-opencl-nvidia vs2008
rename vs2008 vs2008_opencl_nvidia

genie --with-opencl-intel vs2008
rename vs2008 vs2008_opencl_intel

genie --with-opencl-amd vs2008
rename vs2008 vs2008_opencl_amd

genie --with-opencl-nvidia vs2010
rename vs2010 vs2010_opencl_nvidia

genie --with-opencl-intel vs2010
rename vs2010 vs2010_opencl_intel

genie --with-opencl-amd vs2010
rename vs2010 vs2010_opencl_amd

genie --with-dx11 vs2008
rename vs2008 vs2008_dx11

genie --with-dx11 vs2010
rename vs2010 vs2010_dx11

genie --with-dx11 vs2005
rename vs2005 vs2005_dx11

genie vs2005
genie vs2008
genie vs2010


pause