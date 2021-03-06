/**************Statement of Policy****************************/
// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Haojian Jin
/*************************************************************/


#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>


int s;			//socket file descriptor
long rangein15sec=0;
long rangein15secCounter=0;
int isTerminated = 0;
int cur_test_number =0;


int check_perfect(int num)
{
	int sum, cur_factor;
	int result = 0;
	sum = 0;
	for(cur_factor = 1; cur_factor <=(num -1); cur_factor++)
	{
		if((num%cur_factor) ==0)
		{
			sum += cur_factor;
		}
	}
	if(sum == num)
	{
		return sum;
	}
	else
		return 0;
}

void start_compute(int _start, int _end)
{
	
	int j=0;
	char buf[100];
	for(j=_start; j<=_end; j++)
	{
		cur_test_number = j;
		int res;
		if((res = check_perfect(j))!=0)
		{
			//report to manager
			//printf("%ld\n", res);
			if(res == 6 || res == 28||res ==496)
				usleep(1000);
			snprintf(buf, sizeof(buf), "p|%ld", res);
			
			write(s, buf, strlen(buf)+1);
		}
		rangein15secCounter++;
	}
	if(rangein15sec == 0)
		rangein15sec = rangein15secCounter;
	//printf("%ld",rangein15sec);
	snprintf(buf, sizeof(buf), "u|%ld|%ld", _end, rangein15sec);
	write(s, buf, strlen(buf)+1);
	
}

void process_cmd(char *cmd)
{
	char buf[100];
	char *pch = strtok(cmd,"|");
	switch(*pch)
	{
	case 'r': 	//register
		pch = strtok (NULL, "|");
		long start_point = atol(pch); 
		pch = strtok (NULL, "|");
		long range_size = atol(pch);
		printf("Client registered successfully. Start compute from %ld to %ld\n", start_point, start_point+range_size);
		start_compute(start_point, start_point+range_size);
		break;
	case 's':       // for status
		break;
	case 'k':       //kill by reporter
		printf("kill message received\n");
		//char buf[100];
		snprintf(buf, sizeof(buf), "l|%ld", cur_test_number);
		write(s, buf, strlen(buf)+1);
		exit(0);
		break;
	case 'u':
		pch = strtok (NULL, "|");
		long u_start_point = atol(pch); 
		pch = strtok (NULL, "|");
		long u_range_size = atol(pch);
		printf("new range starts! Start from %ld to %ld\n", u_start_point, u_start_point+u_range_size);
		start_compute(u_start_point, u_start_point+u_range_size);		
		break;				
	}
}

void time_handler(int signum)
{
	//printf("counter: rangein15sec");
	rangein15sec = rangein15secCounter;
	rangein15secCounter = 0;
}	


void handler(int signum)
{
	printf("sig invoked\n");
	//isTerminated = 1;
	char buf[100];
	snprintf(buf, sizeof(buf), "l|%ld", cur_test_number);
	write(s, buf, strlen(buf)+1);
	exit(0);
}

int handle_signals(void)
{
	sigset_t set;
	struct sigaction act;
	
	sigfillset(&set);
	sigprocmask(SIG_SETMASK, &set, NULL);
	memset(&act, 0, sizeof(act));
	sigfillset(&act.sa_mask);
	act.sa_handler = handler;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);
	sigaction(SIGHUP, &act, NULL);
	sigemptyset(&set);
	sigprocmask(SIG_SETMASK, &set, NULL);
	return 1;
}



int main (int argc,char *argv[])
{

	handle_signals();

	struct itimerval itv;
	struct sigaction act;
	
	memset(&act, 0, sizeof(act));
	act.sa_handler = time_handler;
	sigaction(SIGALRM, &act, NULL);
	memset(&itv, 0, sizeof(itv));
	itv.it_interval.tv_sec = 15;
	itv.it_interval.tv_usec =0;
	itv.it_value.tv_sec = 1;
	itv.it_value.tv_usec=0;
	setitimer(ITIMER_REAL, &itv, NULL);	

	struct sockaddr_in sin; /* socket address for destination */
	int len;		//length of address
	long address;
	int i;


	/* Fill in Manager's Address */
	address = *(long *) gethostbyname(argv[1])->h_addr;
	sin.sin_addr.s_addr= address;
	sin.sin_family= AF_INET;
	sin.sin_port = atoi(argv[2]);

	while(1) { /*loop waiting for Manager if Necessary */		
			/* create the socket */
	if ((s = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		perror("Socket");
		exit(1);
		}

		/* try to connect to Manager */
	if (connect (s, (struct sockaddr *) &sin, sizeof (sin)) <0) {
		printf("Where is that Manager!\n");
		close(s);
		sleep(10);
		continue;
		}
	break; /* connection successful */
	}
	printf("connection successful \n");
	//for (i=1; i<= atoi(argv[3]); i++ ) write(s,&i,sizeof (i));

	/* Register on the manager, and get the range */
	char buf[100];
	long range_size = -1;
	long start_point = atol(argv[3]);
	snprintf(buf, sizeof(buf), "r|%ld|%ld", start_point, range_size);
	write(s, buf, strlen(buf)+1);
	while(1){
		read(s, buf, 100);
		//printf("passlog: %s\n", buf);
		process_cmd(buf);
	}
}
