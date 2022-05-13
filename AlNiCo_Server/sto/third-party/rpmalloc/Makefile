all: build.ninja
	ninja

build.ninja:
	./configure.py --host linux --toolchain gcc -a x86-64

clean: build.ninja
	@status=1; fails=0; while [ $$status -ne 0 ] && [ $$fails -le 10 ]; do \
		ninja -t clean -g > /dev/null 2>&1; \
		status=$$?; fails=`expr $$fails + 1`; \
	done
