#include <iostream>
#include <unistd.h>


using namespace std;
void pipeline(const char* process1, const char* process2){
    int fd[2];
    pipe(fd);
    int id = fork();

    if(id != 0){  //parent process
        close(fd[0]); //close the READ end of the pipe
        dup2(fd[1], STDOUT_FILENO);  //redirect standard output to the write end of the pipe
        close(fd[1]);  //close the WRITE end of the pipe

        //execute process1
        execlp("/bin/cat", "cat", "001_pipeline.cpp", nullptr);
        std::cerr << "failed to execute" << process1 << std::endl;

    }
    else{
        close(fd[1]); //close the write end of the pipe
        dup2(fd[0], STDIN_FILENO); //redirect standard input to the READ end of the pipe
        close(fd[0]); //close the READ end of the pipe

        //execute process2
        execlp("/usr/bin/grep", "grep", "hello", nullptr);
        std::cerr << "failed to execute" << process2 << std::endl;

    }
}

int main(){
    pipeline("cat 001_pipeline.cpp", "grep hello");
    return 0;
}