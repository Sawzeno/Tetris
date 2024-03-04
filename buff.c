#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdint.h>

typedef struct Row Row;
typedef struct Buffer Buffer;

struct Row{
	char*		string;
	Row*		next;
	uint64_t size;
};

struct Buffer{
	char*			string;
	Row**			rows;
	uint64_t	numrows;
	uint64_t	numcols;
};

void maler(void* e){
	if(e == NULL){
		perror("MALLOC ERROR !");
		exit(EXIT_FAILURE);
	}
}

Buffer* createBuffer(uint64_t numrows , uint64_t numcols , char type)
{
	Buffer* buffer = malloc(sizeof(Buffer));
	maler(buffer);

	buffer->numrows = numrows;
	buffer->numcols = numcols;

	buffer->string = calloc(numrows * numcols , sizeof(char));
	maler(buffer->string);
	
	for(uint64_t i = 0 ; i < numrows * numcols; ++i){
		buffer->string[i] = type;
	}

	buffer->rows = calloc(numrows , sizeof(Row*));
	maler(buffer->rows);

	for(uint64_t i = 0 ; i < numrows ; ++i){
		Row* row = buffer->rows[i];

		row = malloc(sizeof(Row));
		maler(row);

		row->string = &(buffer->string[i * numcols]);
		row->size = numcols;
		row->next = NULL;
		buffer->rows[i] = row;
	}

	return buffer;
}


int main(){

	uint64_t resx = 64;
	uint64_t resy	=	64;

	Buffer* screenA	=	createBuffer(20, 40 , '0');
	Buffer* screenB	=	createBuffer(20, 10 , 'X');

	Buffer* output	=	createBuffer(resx,resy,' ');

	
	for(uint64_t i = 0 ; i < screenA->numrows ; ++i){
		Row* currentrow = screenA->rows[i];
		while(currentrow != NULL){
			write(STDOUT_FILENO , currentrow->string , currentrow->size);
			currentrow = currentrow->next;
		}
		write(STDOUT_FILENO , "\n", 1);
	}	

	return 0;
}
