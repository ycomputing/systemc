# SystemC examples

## Folder structure
One folder contains one SystemC example, and folders are independent to each other.

## Makefile
Each folder contains ```Makefile```, which assumes
[SystemC](https://github.com/accellera-official/systemc) is configured using the following configuration
,in linux64 environment.

```bash
../configure CXX=g++ 'CXXFLAG=-std=c++17' --prefix=/opt/systemc 
```

If your environment is different, then you will have to modify ```Makefile``` in each folder.
For more information, please refer
[Installation Notes for SystemC](https://github.com/accellera-official/systemc/blob/main/INSTALL.md)

