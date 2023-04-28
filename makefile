# nmake makefile
#
# Tools used:
#  Compile::Resource Compiler
#  Compile::GNU C
#  Make: nmake or GNU make
all : run.exe

run.exe : run.obj run.def
	gcc -Zomf run.obj run.def -o run.exe
	
run.obj : run.c
	gcc -Wall -Zomf -c -O2 run.c -o run.obj

clean :
	rm -rf *exe *RES *obj