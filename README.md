# A HelloWorld project written in OpenCL
## compile
If you just want to compile main.cpp which is run as host program: 
```
make host
```
You can also compile fpga kernel only:
```
make fpga
```
Or build them all:
```
make all
```
## run
```
./main.exe HelloWorld_Kernel.aocx
```

you can edit **Makefile** to choose whether to use software emulation or generate hardware binary file.
