// idea of a grammer , to prse and print accordingly (can be implimented in the global buffer)
// circular buffer
// make multiple buffers with the power of the flex tape
//multithreading

typedef struct matrix{
   int rows;
   int cols;
   char* ptr;
   char * head;
}matrix;

void createMatrix(matrix* m , int rows , int cols){
  m->rows = rows;
  m->cols = cols;
  int res  = (m->rows) * (m->cols);
  m->ptr  = (char*)malloc(res * sizeof(char));
  m->head = m->ptr;
}

void printMatrix(matrix* m){

  for(int i  = 0 ; i < m->rows ; i++){
    for(int j = 0 ; j < m->cols ; j++){
      printf("%c ",m->ptr[i * (m->cols) + j ]);
    }
    printf("\n");
  }
}

void appendString(matrix* m , char* string){

  int len = strlen(string);
  memcpy(m->head , string , len * sizeof(char));
  m->head += len * sizeof(char);
}  

void swap(int *a , int *b){

  int temp = *a;
  *a = *b;
  *b = temp;
}

char* cell(matrix* m  , int x , int y ){

  char* temp;
  temp = &(m->ptr[x * (m->cols) + y]);
  return temp;
}

void rotate(matrix* m , int d){

  int n = m->rows;
  for(int i = 0 ; i < n/2 ; i++){
    for(int j = i ; j < (n - ( i + 1)) ; j ++ ){

      int temp = *cell(m,i,j);
      *cell(m,i,j) = *cell(m , j , n-1-i );
      *cell(m,j,n-1-i) = *cell(m,n-1-i,n-1-j);
      *cell(m,n-1-i,n-1-j) = *cell(m,n-1-j,i);
      *cell(m,n-1-j,i) = temp;
    }
  }

  printf("rotated left");
}

