/*
 * bus.c
 *
 * WRITTEN BY:		M.EMMANUEL
 * WRITTEN FOR:		AD USUM PROPIUM
 * DATE:		26/01/2021
 * REV:			1
 *
 * THIS PROGRAM IMPLEMENTS APPLICATION BUS AND IS INTENDED TO RECEIVE AND DISTRIBUTE 
 * ALL MESSAGES ACROSS THE APPLICATION.
 */

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Text.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>

struct message {
	long mtype;
	char mtext[20];
};

void createWindow();
void displayMessage(char *textLineBuffer);
void *listenQueue(void *arg);
void parseMessage(struct message* msg);

XtAppContext	context;

Widget		toplevel;
Widget		busForm;
pthread_mutex_t textWidgetMutex;
Widget		textWidget;
long		textWidgetLength = 0;
key_t		client_queue[100];
int		number_of_clients;
	
int main(int argc, char *argv[]) {

	pthread_t	thread;

	/* INITIALIZE MULTITHREAD X11 ENVIRONMENT */
	XInitThreads();

	/* CREATE X WINDOW */
	createWindow(argc, argv);

	/* GET NEW THREAD TO PROCESS QUEUES */
	/* PARENTS HANDLES X11 */
	pthread_create(&thread, NULL, listenQueue, NULL);

	/* BEGIN X11 APPLICATION */
	XtAppMainLoop(context);

}

void *listenQueue(void *arg) {

	int 		queue_id;
	key_t 		queue_key;
	int 		message_id;
	struct message  message_buffer;
	char		textLineBuffer[1000];

	/* GET QUEUE_KEY */
	queue_key = ftok("/etc/xchart", 1);
	sprintf(textLineBuffer, "Queue key retrieved %ld\n", queue_key);
	displayMessage(textLineBuffer);

	/* GET QUEUE IDENTIFIER */
	queue_id = msgget(queue_key, IPC_CREAT|0666);
	sprintf(textLineBuffer, "Queue created. Waiting for messages...\n");
	displayMessage(textLineBuffer);

	while(1) {
		message_id = msgrcv(queue_id, &message_buffer, 20, 0, 0);
		parseMessage(&message_buffer);
	}

}

void parseMessage(struct message* msg) {
	
	char		textLineBuffer[1000];

	/* REGISTER MESSAGE */
	if(msg->mtext[0]=='R') {

		/* PRINT MESSAGE ON X11 TEXT WIDGET */
		sprintf(textLineBuffer, "Client %ld registered at queue %s.\n", msg->mtype, msg->mtext);
		displayMessage(textLineBuffer);

		/* REGISTER QUEUE */
		char* ptr; 
		key_t queue_key_register;
		sscanf(msg->mtext + 1, "%ld", &queue_key_register);
		client_queue[number_of_clients] = msgget(queue_key_register, 0);
		number_of_clients++;
	}

	/* CLIENT MESSAGE */
	if(msg->mtext[0]=='M') {

		/* PRINT RECEIVED MESSAGE */
		sprintf(textLineBuffer, "Received message %s from client %ld.\n", msg->mtext, msg->mtype);
		displayMessage(textLineBuffer);

		/* FORWARD TO ALL PARTICIPANTS */
		int i;
		for(i=0; i<number_of_clients; i++) {
			msgsnd(client_queue[i], msg, 10, 0);
		}
	}

}

void createWindow(int argc, char *argv[]){
	
	Arg		al[10];
	int		ac;

	/* CREATE TOP LEVEL */
	toplevel = XtAppInitialize(&context, "Bus Handler", NULL, 0, &argc, argv, NULL, NULL, 0);

	/* RESIZE TOP LEVEL */
	ac = 0;
	XtSetArg(al[ac], XmNwidth, 300); ac++;
	XtSetArg(al[ac], XmNheight, 600); ac++;
	XtSetValues(toplevel, al, ac);

	/* CREATE TOP LEVEL FORM MANAGER */
	ac = 0;
	busForm  = XmCreateForm(toplevel, "busForm", al, ac);
	XtManageChild(busForm);

	/* CREATE TEXT WIDGET */
	ac = 0;
	XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
	XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
	XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
	XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
	XtSetArg(al[ac], XmNeditMode, XmMULTI_LINE_EDIT); ac++;
	XtSetArg(al[ac], XmNscrollingPolicy, XmVARIABLE); ac++;
	XtSetArg(al[ac], XmNscrollBarDisplayPolicy, XmSTATIC); ac++;
	XtSetArg(al[ac], XmNscrollHorizontal, FALSE); ac++;
	XtSetArg(al[ac], XmNeditable, FALSE); ac++;
	textWidget = XmCreateScrolledText(busForm, "testWidget", al, ac);
	XtManageChild(textWidget);

	/* DISPLAY TOP LEVEL */
	XtRealizeWidget(toplevel);
}

void displayMessage(char *textLineBuffer) {
	pthread_mutex_lock(&textWidgetMutex);
	XmTextInsert(textWidget, textWidgetLength, textLineBuffer); 
	textWidgetLength += strlen(textLineBuffer);
	pthread_mutex_unlock(&textWidgetMutex);
	XmUpdateDisplay(textWidget);
}
