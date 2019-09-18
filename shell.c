#include <fcntl.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


/* inputBuf: Stores user input.
   cwd: Stores current working directory.
   args: Stores pointers for each argument.
   usr: Stores the username for the user.
*/
char inputBuf[1024];
char cwd[1024];
char *args[1024];
char input_file[1024];
char output_file[1024];
char *usr;
int append = 0;

/* This function prints the output in a General format
   which will be handy for debugging and for basic
   output to the user. */
void print(char *str){

  write(1, str, strlen(str));
  write(1, "\n", 1);
}


/* This function prints out the current working directory
   for the user to see before putting in input. Added a $
   to the end of the directory print out. */
void print_dir(char *str){

  write(1, usr, strlen(usr));
  write(1, "@Mic:", 5);
  write(1, str, strlen(str));
  write(1, "$ ", 2);
}


/* This function prints out the arguments the user specified
   when typing commands in a way to clearly state what is
   happening and how the shell views the input. */
void print_debug(char *str, int num){

  if(str == NULL){
    write(1, "arg: ", 5);
    write(1, "[", 1);
    write(1, "(null)", 6);
    write(1, "]\n", 2);
  } else{
    if(num == 0){
      write(1, "cmd: ", 5);
      write(1, "[", 1);
      write(1, str, strlen(str));
      write(1, "]\n", 2);

      write(1, "arg: ", 5);
      write(1, "[", 1);
      write(1, str, strlen(str));
      write(1, "]\n", 2);
    } else {
      write(1, "arg: ", 5);
      write(1, "[", 1);
      write(1, str, strlen(str));
      write(1, "]\n", 2);
    }
  }
}


/* This function takes input from the user through
   0 (stdin) line by line. */
void user_input(){
  char *ch = inputBuf;

  while(read(0, ch, 1) && *ch != '\n'){
    ch++;
  }

  *ch = '\0';
}


/* This function gets the current working directory
   for the user. */
void get_path(){

  getcwd(cwd, sizeof(cwd));
  print_dir(cwd);
}


/* This function is an intermediate function between
   parse_input and print_debug. It's not necessary,
   but just incase I want to change something in the
   future it might be handy. */
void debug_args(){

    int y = 0;
    
    while(args[y] != NULL){
      print_debug(args[y], y);
      y++;
    }

    print_debug(args[y], y);

}


/* This function changes directories for the user.
   If the directory doesn't exist it tells the user.
   Not necessary but may be nice if I want to change
   functionality. */
void change_dir(char *dir){

  if(args[2] != NULL){
    print("cd: Too many arguments.");
  }
  else if(chdir(dir) != 0){
    write(1, dir, sizeof(dir));
    print(": Is not a directory!");
  }

}


void redirect(){
  
  char *ch = inputBuf;
  int space = 1;
  int first = 1;

  while(*ch != '\0'){
    if(*ch == '<'){
      if(first == 1){
        *ch = '\0';
        first = 0;
      }
      ch++;
      while(space == 1){
        if(*ch == ' '){
          ch++;
        } else {
          space = 0;
        }
      }
      char *in = input_file;
      while(*ch != ' ' && *ch != '\0'){
        *in = *ch;
        ch++;
        in++;
      }
      *in = '\0';
      space = 1;
    }

    if(*ch == '>'){
      if(first == 1){
        *ch = '\0';
        first = 0;
      }
      ch++;
      if(*ch == '>'){
        append = 1;
        ch++;
      }
      while(space == 1){
        if(*ch == ' '){
          ch++;
        } else {
          space = 0;
        }
      }
      char *out = output_file;
      while(*ch != ' ' && *ch != '\0'){
        *out = *ch;
        ch++;
        out++;
      }
      *out = '\0';
      space = 1; 
    }

    ch++;
  }
}

void erase_files(){
  
  char *eraser = input_file;
  *eraser = '\0';
  eraser = output_file;
  *eraser = '\0';
  append = 0;
}

void execute_command(){
    
  int pid = fork(); 
  if(pid == 0){

    if(strlen(input_file) != 0){
      close(0);
      open(input_file, O_RDONLY | O_CREAT, 0666);
    }

    if(strlen(output_file) != 0){
      close(1);
      if(append == 1){
        open(output_file, O_APPEND | O_CREAT, 0666);
      } else {
        open(output_file, O_WRONLY | O_CREAT, 0666);
      }
    }

    execvp(args[0], args);
    exit(1);
  } else {
    int exitCode;
    while( wait(&exitCode) != pid);
  }
}

/* This function parses input from the user, creating
   a list of arguments with the first of those being the
   command the user wants to execute. */
int parse_input(){

  redirect();

  char *ch = inputBuf;

  int x = 0;
  int first_space = 1;
  int first_char = 1;

  while(*ch != '\0'){ 
    if(*ch == ' '){
      if(first_space){
        *ch = '\0';
        first_space = 0;
        first_char = 1;
      }
    } else {
      if(first_char){
        args[x] = ch;
        x++;
        first_space = 1;
        first_char = 0;
      }
    }
    ch++;
  }

  args[x] = NULL;

  // If the user specified command is debugargs...
  if(strcmp("debugargs", args[0]) == 0){
     debug_args();
  }
  
  // If the user specified command is cd...
  if(strcmp("cd", args[0]) == 0){
     change_dir(args[1]);
  }

  // If the user specified command is pwd...
  if(strcmp("pwd", args[0]) == 0){
    print(cwd);
  }

  // If the user specified command is exit
  // return 1 so the main function can break.
  if(strcmp("exit", args[0]) == 0){
    return 1;
  }

  // Everything went well!
  return 0;
}


int main(int argc, char **argv){

  print("Michael Shell vX.x\n");

  usr = getenv("USER");
  if(usr==NULL) return 2;

  //Main loop to continuously ask for user input.
  while(1){

    // Print out the current working directory that the user is in.
    get_path();

    // Get input from the user and what they want to do.
    user_input();

    // Parse the input given and determine what to do with it.
    if(parse_input() == 1){
      break;
    }

    execute_command();

    erase_files();
    
    //DEBUG OUTPUT 
    //print(inputBuf);

  }

  print("\nGoodbye.");

  return 0;
}
