# release version
REL=0.3

all:
	gcc -Wall -O3 -o ventriloctrl ventriloctrl.c -lX11 -lXevie
	gcc -Wall -O3 -o findkey findkey.c

clean:
	rm -f ventriloctrl findkey

realclean: clean
	rm -rf ventriloctrl-$(REL)/ ventriloctrl-$(REL).tar.gz

release:
	rm -f ventriloctrl findkey
	mkdir ventriloctrl-$(REL)/
	cp `find * -type f` ventriloctrl-$(REL)/
	tar -zcvvf ventriloctrl-$(REL).tar.gz ventriloctrl-$(REL)/
	rm -rf ventriloctrl-$(REL)/
