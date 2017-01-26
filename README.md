# MYIR Linux Application Examples

## ToolChain  

	** ARM Linux ToolChain ** -- gcc-linaro-5.3-2016.02-x86_64_arm-linux-gnueabihf.tar.xz  
	
```
	export ARCH=arm
	export CROSS_COMPILE=arm-linux-gnueabihf-
	export PATH=$PATH:/path/to/gcc-linaro-4.9-2015.05-x86_64_arm-linux-gnueabihf/bin  
```  
	 
	** PRU ToolChain **  --  ti_cgt_pru_2.1.3_linux_installer_x86.bin  
```	
	export PRU_CGT=/path/to/ti-cgt-pru_2.1.3
```  

## Compile  

```  
  $ make
```  
  
## Install  

  $ make install
	
  


