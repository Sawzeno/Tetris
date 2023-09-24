#define CTRL_KEY(k) ((k) & 0x1f) // sets upper three bits to zero

void die(char *s){

  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H",4);
  perror(s);
  exit(1);
}

struct editorConfig{
  struct termios orig_termios;
};

struct editorConfig E;

void disableRawMode(){
  if(tcsetattr(STDIN_FILENO , TCSAFLUSH , &E.orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode(){

  if(tcgetattr(STDIN_FILENO , &E.orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode);

  struct termios raw = E.orig_termios ;

  //input flags 
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

  //output flags
  raw.c_oflag &= ~(OPOST);

  //control flags
  raw.c_cflag |= (CS8);

  //local flags
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  //special charachters
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  
  if(tcsetattr(STDIN_FILENO , TCSAFLUSH , &raw) == -1) die("tcsetattr");
}
