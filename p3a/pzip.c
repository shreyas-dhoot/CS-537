#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <sys/types.h>

int qsize = 40;

typedef struct {
	char *start;
	int index;
	int size;
} ChunkInfo;

typedef struct {
	char *encoded;
	int size;
} encodedBlocks;

int pgsize;
ChunkInfo *chunkInfoQueue;
int fillptr = 0;
int useptr = 0;
encodedBlocks *allEncodings;
int chunks = 0;
int isProducerAlive;
int numfull = 0;

pthread_mutex_t m  = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

void saveToMemory(encodedBlocks *eBlocks, char *op,  int size) {
	char *res = malloc(size);
	memcpy(res, op, size);
	eBlocks -> size = size;
	eBlocks -> encoded = res;
}

void RunLengthEncoding(ChunkInfo chunk, encodedBlocks *eBlocks) {
	char startChar;
	char* endPtr = chunk.start;
	int len = chunk.size;
	int flag = 0;
	char *output = malloc(chunk.size * 8);
	char *curr = output;
	int freq = 0;

	int i;
	for(i = 0; i < len; i++) {
		if(endPtr[i] != '\0') {
			flag = 1;
			startChar = endPtr[i];
			break;
		}
	}
	// All are \0, therefore ignore
	if(flag == 0) {
		saveToMemory(eBlocks, output, curr-output);
		munmap(chunk.start, chunk.size);
		free(output);
		return;
	}
	
	i = 0;
	while (i < len) {
            if(endPtr[i]=='\0') {
                //Do nothing;
            }
            else if(endPtr[i] != startChar) {
                *((int*)curr) = freq;
                curr[4] = startChar;
                curr += 5;
                startChar = endPtr[i];
                freq = 1;
            } else 
		    freq++;
	    i++;
        }

	if(startChar!='\0') {
            *((int*)curr) = freq;
            curr[4] = startChar;
            curr += 5;
        }

	saveToMemory(eBlocks, output, curr-output);
	munmap(chunk.start, chunk.size);
	
	free(output);
	return;
}

void do_fill(ChunkInfo chunk) {
	chunkInfoQueue[fillptr] = chunk;
	fillptr = (fillptr + 1) % qsize;
	numfull++;
}

ChunkInfo do_get() {
	ChunkInfo ci = chunkInfoQueue[useptr];
	useptr = (useptr + 1) % qsize;
	numfull--;
	return ci;
}

void *consumer(void *ptr) {
	while (1) 
	{
		ChunkInfo cInfo;
		pthread_mutex_lock(&m);
		// Waiting for producer
		while (numfull == 0 && isProducerAlive) {
			pthread_cond_wait(&fill, &m);
		}
		// As producer is not alive, exit this thread after giving up the lock
		if (numfull == 0 && !isProducerAlive) {
			pthread_mutex_unlock(&m);
			pthread_exit(0);
		}
		cInfo = do_get();
		// Signaling producer
		pthread_cond_signal(&empty);
		pthread_mutex_unlock(&m);
        	RunLengthEncoding(cInfo, &allEncodings[cInfo.index]);
	}
	pthread_exit(0);
}

void producer(ChunkInfo chunkInfo) {
	pthread_mutex_lock(&m);
	while (numfull == qsize) {
		pthread_cond_wait(&empty, &m);
	}
	do_fill(chunkInfo);
	pthread_cond_signal(&fill);
	pthread_mutex_unlock(&m);
}

int main(int argc, char **argv)
{
	if (argc <= 1) {
		printf("pzip: file1 [file2 ...]\n");
		exit(1);
	}
	int numfiles = argc - 1;
	isProducerAlive = 1;
	pgsize = getpagesize() * 8;
	int processors = get_nprocs();
	int chunkCounter = 0;
	int fileLeft = 0;
	int offset = 0;
	int chunksize = 0;
	chunkInfoQueue = malloc(sizeof(ChunkInfo) * qsize);

	char *validFiles[argc];
	int j = 0;
	int total = numfiles;
 	validFiles[j] = argv[j];
 	j++;

 	for (int i = 1; i < total + 1; i++) {
        	int fd = open(argv[i], O_RDONLY);

        	if(fd == -1){ 
            		numfiles--;
        	} else {
            		validFiles[j] = argv[i];
            		j++;
        	}
	}

	//calculate total number of chunks to be processed
	for (int i = 1; i < numfiles + 1; i++) 
	{
		int fd = open(validFiles[i], O_RDONLY);

		struct stat statbuf;
		fstat(fd, &statbuf);
		double fsize = (double) statbuf.st_size;

		int a = fsize / ((double)pgsize);
		if ( ((int) fsize) % pgsize != 0) {
			a++;
		}
		if (fsize != 0)
			chunks += a;
		close(fd);
	}

	//allocate
	allEncodings = malloc(sizeof(encodedBlocks) * chunks);

	//create threads
	pthread_t consumerThreads[processors];
	for (int i = 0; i < processors; i++) {
		pthread_create(&consumerThreads[i], NULL, consumer, NULL);
	}
	
	//loop over files and prepare chunks to be added to queue
	int fileCounter = 1;
	
	int f = 0;
	while(fileCounter < numfiles + 1) {
		f = open(validFiles[fileCounter], O_RDONLY);
		struct stat statbuf;
		fstat(f, &statbuf);
		fileLeft = (int) statbuf.st_size;
		offset = 0;

		while(fileLeft > 0) {

			chunksize = fileLeft > pgsize ? pgsize : fileLeft;
			//Empty files
			if (chunksize == 0) 
				break;

			//map chunks
		
			ChunkInfo cInfo;
			cInfo.start = mmap(NULL, chunksize, PROT_READ , MAP_PRIVATE, f, offset);
			cInfo.size  = chunksize;
			cInfo.index = chunkCounter;
			
			// No additional threads created for producer. Producer will add in the queue
			producer(cInfo);
		
			fileLeft -= chunksize;
			offset  += chunksize;
			chunkCounter++;
		}
		fileCounter++;
		close(f);
	}

	isProducerAlive = 0;

	//Wake all consumer threads. As Producer is not alive, will exit threads if queue empty
	pthread_cond_broadcast(&fill);
	for (int i = 0; i < processors; i++)
	{
		pthread_join(consumerThreads[i], NULL);
	}

	char *end = NULL;

	for (int i = 0; i < chunks; i++) 
	{
		char *binary = allEncodings[i].encoded;
		if(binary[4]=='\0')
		 	continue;
		int n = allEncodings[i].size;
		if (end && end[4] == binary[4]) 
			*((int*)binary) += *((int*)end);
		 else if(end) 
			fwrite(end, 5, 1, stdout);
		fwrite(binary, n - 5, 1, stdout);
		end = binary + n - 5;
	}
	fwrite(end, 5, 1, stdout);

	return 0;
}
