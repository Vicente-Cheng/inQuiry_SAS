
make: sg_lib.c sg_lib_data.c sg_cmds_basic.c sg_cmds_basic2.c sg_cmds_extra.c sg_cmds_mmc.c sg_pt_common.c sg_pt_linux.c sg_io_linux.c
	@if test -f ".libs/libsgutils2.so.2.0.0"; then \
                make clean ; \
        fi;
	mkdir -p .libs
	gcc -I ../include -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -W -g -O2 -c sg_lib.c  -fPIC -DPIC -o .libs/sg_lib.o
	gcc -I ../include -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -W -g -O2 -c sg_lib_data.c  -fPIC -DPIC -o .libs/sg_lib_data.o
	gcc -I ../include -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -W -g -O2 -c sg_cmds_basic.c  -fPIC -DPIC -o .libs/sg_cmds_basic.o
	gcc -I ../include -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -W -g -O2 -c sg_cmds_basic2.c  -fPIC -DPIC -o .libs/sg_cmds_basic2.o
	gcc -I ../include -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -W -g -O2 -c sg_cmds_extra.c  -fPIC -DPIC -o .libs/sg_cmds_extra.o
	gcc -I ../include -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -W -g -O2 -c sg_cmds_mmc.c  -fPIC -DPIC -o .libs/sg_cmds_mmc.o
	gcc -I ../include -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -W -g -O2 -c sg_pt_common.c  -fPIC -DPIC -o .libs/sg_pt_common.o
	gcc -I ../include -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -W -g -O2 -c sg_pt_linux.c  -fPIC -DPIC -o .libs/sg_pt_linux.o
	gcc -I ../include -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -W -g -O2 -c sg_io_linux.c  -fPIC -DPIC -o .libs/sg_io_linux.o
	gcc -shared  -fPIC -DPIC  .libs/sg_lib.o .libs/sg_lib_data.o .libs/sg_cmds_basic.o .libs/sg_cmds_basic2.o .libs/sg_cmds_extra.o .libs/sg_cmds_mmc.o .libs/sg_pt_common.o .libs/sg_pt_linux.o .libs/sg_io_linux.o    -O2   -Wl,-soname -Wl,libsgutils2.so.2 -o .libs/libsgutils2.so.2.0.0
	ln -s libsgutils2.so.2.0.0 .libs/libsgutils2.so
	@if ! test -f "/usr/lib/libsgutils2.so.2.0.0"; then \
		mv libsgutils2.so.2.0.0 /usr/lib ; \
		ln -s libsgutils2.so.2.0.0 /usr/lib/libsgutils2.so.2 ; \
		ln -s libsgutils2.so.2.0.0 /usr/lib/libsgutils2.so ; \
	fi ;
clean:
	rm -rf .libs
