Algorithm
	- Checked if the arguments are greater than equal to 1
	- Checked if all the files are present or not
	- Calculated the total number of chunks to be processed
		- Opened file one by one and calculated no. of chunks present in each file
		- Max chunk size is defined (pagesize * 8)
	- We start consumer threads
	- For every chunk, we create a cInfo which contains start index, size and index.
	- This cInfo is added in the queue using the producer thread (main thread)
	- The producer thread notified the consumer threads to consume the cInfo
	- If the queue is not empty, remove one cInfo and run RunLengthEncoding function
	- If the queue is empty, then
		- If the producer thread is alive, then the consumer waits for more cInfo
		- Else the thread exits
	- After the producer finishes, wait for all the consumers to complete
	- Store all encodings of the chunks
	- If the character of the end of first chunk is equal to the character of the start of next chunk, then merge the count and then write encoding to stdout
	
RunLengthEncoding
	- if \0 found ignore
	- startChar contains the character 'a' (example)
	- EndPointer should be incremented till a different character (in this case 'a') is found
	- When found store it in curr block and copy it to res
	- Then do munmap





Time taken = 23 secs
