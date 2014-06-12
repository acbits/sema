#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <getopt.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SEMA_SUCCESS   0
#define SEMA_ERROR    -1
#define SEMA_BLOCKED   1
#define SEMA_ARG_ERROR 2

#define SEMA_SHARED 1
#define BUF_SIZE 1024


enum action_t { NONE, CREATE, DELETE, RAISE, WAIT, VALUE };


/*
 * Creates a named semaphore
 * Returns zero if successful
 */
static int sem_create(const char *sem_name, int *iv)
{
    int rv = SEMA_ERROR;
    int val = iv ? *iv : 1;

    mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP;
    sem_t *sp = sem_open(sem_name, O_CREAT|O_RDWR, mode, 0);
    
    if (sp != NULL){
	if (sem_init(sp, SEMA_SHARED, val) == 0)
	    rv = SEMA_SUCCESS;

	sem_close(sp);
    }
    return rv;
}

static int sem_delete(const char *sem_name){

    int rv = SEMA_ERROR;
    sem_t *sp = sem_open(sem_name, O_RDWR);

    if (sp != NULL){
	int value = -1;

	/* Check whether any process is blocked on it */

	if (sem_getvalue(sp, &value) == 0
	    && value < 1){
	    printf("Processe(s) blocked on [%s]?\n", sem_name);
	    rv = SEMA_BLOCKED;
	}
	else if (sem_destroy(sp) == 0
		 && sem_unlink(sem_name) == 0)
	    rv = SEMA_SUCCESS;

	sem_close(sp);
    }

    return rv;
}

static int sem_inc(const char *sem_name)
{
    int rv = SEMA_ERROR;
    sem_t *sp = sem_open(sem_name, O_RDWR);

    if (sp != NULL){
	if (sem_post(sp) == 0)
	    rv = SEMA_SUCCESS;
	sem_close(sp);
    }
    return rv;

}

static int sem_dec(const char *sem_name)
{
    int rv = SEMA_ERROR;
    sem_t *sp = sem_open(sem_name, O_RDWR);

    if (sp != NULL){
	if (sem_wait(sp) == 0)
	    rv = SEMA_SUCCESS;
	sem_close(sp);
    }

    return rv;
}

static int sem_value(const char *sem_name)
{
    int value = -1;
    sem_t *sp = sem_open(sem_name, O_RDWR);
    int rv = SEMA_ERROR;

    if (sp != NULL){
	if (sem_getvalue(sp, &value) == 0){
	    printf("%d\n", value);
	    rv = SEMA_SUCCESS;
	}
	sem_close(sp);
    }

    return rv;

}

int main(int argc, char* argv[])
{
    int rv = 0;
    enum action_t action;
    const char *sem_name = NULL;
    int initial_value = 0;
    int *ivp = NULL;
    int action_count = 0;

    while (1) {

	
	int c = getopt(argc, argv, "c:d:r:w:v:");
	
	if (c == -1)
	    break;
	
	switch(c){

	case 'c':
	    action = CREATE;
	    action_count++;
	    sem_name = optarg;
	    break;
	case 'd':
	    action = DELETE;
	    action_count++;
	    sem_name = optarg;
	    break;
	case 'r':
	    action = RAISE;
	    action_count++;
	    sem_name = optarg;
	    break;
	case 'w':
	    action = WAIT;
	    action_count++;
	    sem_name = optarg;
	    break;
	case 'v':
	    action = VALUE;
	    action_count++;
	    sem_name = optarg;
	    break;
	default:
	    action = NONE;
	    break;
	}
    }

    if (action_count > 1){
	printf("Too many arguments\n");
	rv = SEMA_ARG_ERROR;
    }
    else if (action != NONE && sem_name != NULL && strlen(sem_name) > 0){
	switch(action){
	case CREATE:
	    if (argv[optind]){
		initial_value = atoi(argv[optind]);
		ivp = &initial_value;
	    }
	    rv = sem_create(sem_name, ivp);
	    break;
	case DELETE:
	    rv = sem_delete(sem_name);
	    break;
	case RAISE:
	    rv = sem_inc(sem_name);
	    break;
	case WAIT:
	    rv = sem_dec(sem_name);
	    break;
	case VALUE:
	    rv = sem_value(sem_name);
	    break;
	default:
	    printf("Internal Error: action not set");
	    break;
	}
	if (rv == SEMA_ERROR){
	    char buf[BUF_SIZE];
	    snprintf(buf, sizeof(buf),
		     "Error operating on given semaphore:[%s]", sem_name);
	    perror(buf);
	}
	
    }
    else{
	printf("\nSYNOPSIS\n sema -c|-d|-r|-w name [initial value]\n\nDESCRIPTION\n"
	       " -c name [initial value]\n  Create a semaphore with given value. Default: 1\n"
	       " -d name\n  Delete the given semaphore.\n"
	       " -r name\n  Raise the given semaphore.\n"
	       " -v name\n  Print the current value of the given semaphore.\n"
	       " -w name\n  Wait on the given semaphore. Will block if semaphore value < 1\n");

    }

    return rv;
}

