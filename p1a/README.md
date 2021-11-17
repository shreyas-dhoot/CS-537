Approach 
- database.txt contains all the <key, value> pairs.
- When kv is executed, if database exists, then we read all the pairs and store it in Linked List data structure.
- The linked list structure contains a struct node with key, value and node * that points to the next node
- When all the instructions are executed, the program iterates over the linked list and store all the values in the db.

For p command,
- Check if there are 2 arguments and the first one is string
- Add a node at the end of the list with the new value.
- However, this will create duplicates.
- Therefore, iterate over the list to find if that particular key is present, delete it and then store the new key,value pair

For g command
- Check if there is one argument and it is integer
- Iterate over the list to find the key, value pair and print it
- If it doesn't find it, then print key not found.

For d command
- Check if there is one argument and it is integer
- Iterate over the list to find if that particular key is present and delete it
- The deletion should be in such a way that the previous node should point the next node after deletion

For a command
- Iterate over every element in the list and print it.
- 
