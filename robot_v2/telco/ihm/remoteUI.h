#ifndef ROBOT_V2_REMOTEUI_H
#define ROBOT_V2_REMOTEUI_H

/**
 * Start RemoteUI and waits for the user's input until the user ask to quit
 *
 */
extern void remoteUIStart();

/**
 * Stop RemoteUI
 *
 */
extern void remoteUIStop();


/**
 * initialize in memory RemoteUI
 *
 */
extern void remoteUINew();

/**
 * destruct the RemoteUI from memory
 *
 */
extern void remoteUIFree();


/**
 *
 *
 */
extern void remoteUIDisplay();

/**
 *
 *
 */
extern void remoteUICaptureChoice();

/**
 *
 *
 */
extern void remoteUIRun();

/**
 *
 *
 */
extern void remoteUIAsk4Log();

/**
 *
 *
 */
extern void remoteUIAskClearLog();


#endif //ROBOT_V2_REMOTEUI_H
