simple_filter: ses_inq_all.c
	gcc -fPIC -DHAVE_CONFIG_H -I./include -I./lib -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -c -o inQuiry_SAS.o inQuiry_SAS.c
	gcc -shared -o libqsas.so inQuiry_SAS.o -L./lib/.libs/ -lsgutils2
clean:
	rm -rf inQuiry_SAS.o libqsas.so
