#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include "constants.h"
#include "mini-shell-parser.c"

int count_p;
void ejecutar_cmd(char** cmd) {
  execvp(cmd[0], cmd);
}

void ejecutar_hijo_n(int pipe_fd[count_p-1][2], char **progs, int my_numb) {

	if(my_numb == 0){
		// Cerrar lectura pipe actual
		close(pipe_fd[my_numb][0]);
		// Conectar escritura a stdout
		dup2(pipe_fd[my_numb][1], STDOUT_FILENO);
		ejecutar_cmd(progs);
	}

	if(my_numb == count_p-1 && count_p == 2){
		// Cerrar escritura del pipe-actual
		close(pipe_fd[my_numb-1][1]);
		// Conectar lectura a stdin actual
  		dup2(pipe_fd[my_numb-1][0], STDIN_FILENO);
		ejecutar_cmd(progs);
	}

	for (size_t p = 0; p < my_numb; p++)
	{
		if(p != my_numb -1){
			// Cerrar lectura de pipes sin uso
			close(pipe_fd[p][0]);
		}
		// Cerrar escritura del pipe-1
		close(pipe_fd[p][1]);
	}
	

	if(my_numb == count_p-1){

		// Conectar lectura a stdin actual
  		dup2(pipe_fd[my_numb-1][0], STDIN_FILENO);
		ejecutar_cmd(progs);
	}


	// Conectar lectura al pipe -1
	dup2(pipe_fd[my_numb-1][0], STDIN_FILENO);
	
	// Conectar escritura a stdout pipe actual
	dup2(pipe_fd[my_numb][1], STDOUT_FILENO);

	// Cerrar lectura a  pipe actual
	close(pipe_fd[my_numb][0]);

	// Ejecutar programa
	ejecutar_cmd(progs);
}


static int run(char ***progs, size_t count)
{	
	int r, status;
	count_p = count;
	//Reservo memoria para el arreglo de pids
	//TODO: Guardar el PID de cada proceso hijo creado en children[i]
	pid_t *children = malloc(sizeof(*children) * count);

	//TODO: Pensar cuantos pipes necesito.
	int pipe_fd[count-1][2];
	

	//TODO: Pensar cuantos procesos necesito
	pid_t children_actual;
	int i;
    for(i=0; i <count; i++){
		if(i != count-1){
	  		pipe(pipe_fd[i]);
		}
		children_actual = fork();
		if (children_actual == 0){
			ejecutar_hijo_n(pipe_fd, progs[i], i);
		}
		children[i] = children_actual;
	}
	for (size_t p = 0; p < count-1; p++)
	{
		// Cerrar lectura de pipes sin uso
		close(pipe_fd[p][0]);
		// Cerrar escritura de pipes sin uso
		close(pipe_fd[p][1]);
	}
	



	//TODO: Para cada proceso hijo:
			//1. Redireccionar los file descriptors adecuados al proceso
			//2. Ejecutar el programa correspondiente

	//Espero a los hijos y verifico el estado que terminaron
	for (int j = 0; j < count; j++) {
		waitpid(children[j], &status, 0);

		if (!WIFEXITED(status)) {
			fprintf(stderr, "proceso %d no terminó correctamente [%d]: ",
			    (int)children[j], WIFSIGNALED(status));
			perror("");
			return -1;
		}
	}
	r = 0;
	free(children);

	return r;
}


int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("El programa recibe como parametro de entrada un string con la linea de comandos a ejecutar. \n"); 
		printf("Por ejemplo ./mini-shell 'ls -a | grep anillo'\n");
		return 0;
	}
	int programs_count;
	char*** programs_with_parameters = parse_input(argv, &programs_count);

	printf("status: %d\n", run(programs_with_parameters, programs_count));

	fflush(stdout);
	fflush(stderr);

	return 0;
}
