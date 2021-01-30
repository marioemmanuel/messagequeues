/*
 * client.c
 *
 * WRITTEN BY:		M.EMMANUEL
 * WRITTEN FOR:		AD USUM PROPIUM
 * DATE:		29/01/2021
 * REV:			1
 *
 * THIS PROGRAM IMPLEMENTS APPLICATION BUS AND IS INTENDED TO RECEIVE AND DISTRIBUTE 
 * ALL MESSAGES ACROSS THE APPLICATION.
 */

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>

void createWindow();
void displayMessage(char *textLineBuffer);
void *listenQueue(void *arg);
void registerButtonCB(Widget w, XtPointer  client_data, XtPointer call_data);
void sendButtonCB(Widget w, XtPointer  client_data, XtPointer call_data);

/* X11 RELATED VARIABLES */
XtAppContext	context;

Widget		toplevel;
Widget		busForm;
Widget		textWidget;
Widget		registerButton;
Widget		sendButton;
pthread_mutex_t textWidgetMutex;
long		textWidgetLength = 0;

/* BUS HANDLER SYS V QUEUE VARIABLES */
int bus_queue_id;
key_t bus_queue_key;
int bus_message_id;
	
/* BUS CLIENT SYS V QUEUE VARIABLES */
int client_queue_id;
key_t client_queue_key;
int client_message_id;

/* BUS AND CLIENT BUFFER */
struct message {
	long mtype;
	char mtext[20];
};

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

	/* MESSAGE STRUCT */
	struct message	message_buffer;
	char 		message_in[100];
	char 		textLineBuffer[1000];

	/* GET BUS QUEUE_KEY */
	bus_queue_key = ftok("/etc/xchart", 1);
	bus_queue_id = msgget(bus_queue_key, 0);
	sprintf(textLineBuffer, "Bus Handler Queue key retrieved %ld.\n", bus_queue_key);
	displayMessage(textLineBuffer); 

	/* GET CLIENT QUEUE_KEY */
	client_queue_key = ftok("/etc/xchart", getpid());
	client_queue_id = msgget(client_queue_key, IPC_CREAT|0666);
	sprintf(textLineBuffer, "Client Queue key retrieved %ld.\n", client_queue_key);
	displayMessage(textLineBuffer); 

	/* READ CLIENT QUEUE MESSAGES */
	while(1) {
		client_message_id = msgrcv(client_queue_id, &message_buffer, 20, 0, 0);
		sprintf(message_in, "[IN] Message from client %ld: %s\n", message_buffer.mtype, message_buffer.mtext); 
		displayMessage(message_in);
	}

}

void createWindow(int argc, char *argv[]){
	
	Arg		al[10];
	int		ac;
	XmStringCharSet	char_set = XmSTRING_DEFAULT_CHARSET;
	char		pidString[50];

	/* CREATE TOP LEVEL */
	sprintf(pidString, "Bus Client %d", getpid());
	toplevel = XtAppInitialize(&context, pidString, NULL, 0, &argc, argv, NULL, NULL, 0);

	/* RESIZE TOP LEVEL */
	ac = 0;
	XtSetArg(al[ac], XmNwidth, 300); ac++;
	XtSetArg(al[ac], XmNheight, 300); ac++;
	XtSetValues(toplevel, al, ac);

	/* CREATE TOP LEVEL FORM MANAGER */
	ac = 0;
	busForm  = XmCreateForm(toplevel, "busForm", al, ac);
	XtManageChild(busForm);

	/* CREATE REGISTER BUTTON WIDGET */
	ac = 0;
	XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
	XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
	XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
	XtSetArg(al[ac], XmNrightPosition, 50); ac++;
	XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++;
	XtSetArg(al[ac], XmNlabelString, XmStringCreateLtoR("Register", char_set)); ac++;
	registerButton = XmCreatePushButton(busForm, "registerButton", al, ac);
	XtManageChild(registerButton);
	XtAddCallback(registerButton, XmNactivateCallback, registerButtonCB, NULL); 
	
	/* CREATE SEND BUTTON WIDGET */
	ac = 0;
	XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
	XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
	XtSetArg(al[ac], XmNleftWidget, registerButton); ac++;
	XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
	XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++;
	XtSetArg(al[ac], XmNlabelString, XmStringCreateLtoR("Send", char_set)); ac++;
	sendButton = XmCreatePushButton(busForm, "sendButton", al, ac);
	XtManageChild(sendButton);
	XtAddCallback(sendButton, XmNactivateCallback, sendButtonCB, NULL); 

	/* CREATE TEXT WIDGET */
	ac = 0;
	XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
	XtSetArg(al[ac], XmNtopWidget, registerButton); ac++;
	XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
	XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
	XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
	XtSetArg(al[ac], XmNeditMode, XmMULTI_LINE_EDIT); ac++;
	XtSetArg(al[ac], XmNscrollingPolicy, XmVARIABLE); ac++;
	XtSetArg(al[ac], XmNscrollBarDisplayPolicy, XmSTATIC); ac++;
	XtSetArg(al[ac], XmNscrollHorizontal, FALSE); ac++;
	XtSetArg(al[ac], XmNeditable, FALSE); ac++;
	textWidget = XmCreateScrolledText(busForm, "textWidget", al, ac);
	XtManageChild(textWidget);

	/* DISPLAY TOP LEVEL */
	XtRealizeWidget(toplevel);
}

void registerButtonCB(Widget w, XtPointer  client_data, XtPointer call_data) {

	struct message message_buffer;
	char status_text[100];

	message_buffer.mtype = getpid();
	sprintf(message_buffer.mtext, "R%ld", client_queue_key);
	bus_message_id = msgsnd(bus_queue_id, &message_buffer, 20, 0);
	sprintf(status_text, "REGISTER CLIENT %d KEY %ld STATUS %d\n", getpid(), client_queue_key, bus_message_id);
	displayMessage(status_text); 

}

void sendButtonCB(Widget w, XtPointer  client_data, XtPointer call_data) {

	struct message message_buffer;
	char status_text[100];

	message_buffer.mtype = getpid();
	sprintf(message_buffer.mtext, "M%d", getpid());
	bus_message_id = msgsnd(bus_queue_id, &message_buffer, 20, IPC_NOWAIT);
	sprintf(status_text, "[OUT] MESSAGE\n");
	/*displayMessage(status_text); */

}

void displayMessage(char *textLineBuffer) {
	pthread_mutex_lock(&textWidgetMutex);
	XmTextInsert(textWidget, textWidgetLength, textLineBuffer); 
	textWidgetLength += strlen(textLineBuffer);
	pthread_mutex_unlock(&textWidgetMutex);
	XmUpdateDisplay(textWidget);
}
