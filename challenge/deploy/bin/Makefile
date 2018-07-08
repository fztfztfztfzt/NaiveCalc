clear:
	- rm *.o
	- rm core
	- rm *.txt
	- rm rpc
	- rm *.pyc
	echo "CISCN{This is a text for test}" > flag
comp:
	gcc -c calc.c -fPIC -o calc.o -lm
	gcc -c RPCcalc.c -fPIC -o RPCcalc.o 
	gcc -c WjCryptLib_Sha256.c -fPIC -o WjCryptLib_Sha256.o -I .
	gcc -c WjCryptLib_Md5.c -fPIC -o WjCryptLib_Md5.o -I .
	gcc -Wl,-z,relro,-z,now -fPIE -pie calc.o WjCryptLib_Sha256.o WjCryptLib_Md5.o RPCcalc.o -o rpc -lm -pthread 
test:
	gcc -g -c calc.c -o calc.o -lm
	gcc -g -c RPCcalc.c -o RPCcalc.o 
	gcc -c WjCryptLib_Sha256.c -o WjCryptLib_Sha256.o -I .
	gcc -c WjCryptLib_Md5.c -o WjCryptLib_Md5.o -I .
	gcc -g -Wl,-z,relro,-z,now calc.o WjCryptLib_Sha256.o WjCryptLib_Md5.o RPCcalc.o -o rpc -lm -pthread 