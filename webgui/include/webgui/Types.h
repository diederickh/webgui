/*
---------------------------------------------------------------------------------
 
                                               oooo
                                               `888
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo
                `888""8P d88' `88b  `88b..8P'   888  `888  `888
                 888     888   888    Y888'     888   888   888
                 888     888   888  .o8"'88b    888   888   888
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P'
 
                                                  www.roxlu.com
                                             www.apollomedia.nl
                                          www.twitter.com/roxlu
 
---------------------------------------------------------------------------------

  # WebGUI_Task

  A WebGUI_Task is a tiny piece of data which is used to perform some (heavy)
  operations in a sperate thread.  See WebGUI which creates a thread.

*/

#ifndef ROXLU_WEB_GUI_TYPES_H
#define ROXLU_WEB_GUI_TYPES_H

extern "C" {
#  include <libwebsockets.h>
}

#include <vector>
#include <string>

#define WG_TYPE_NONE 0           /* The default gui type, WG_TYPE_NONE means an invalid type */
#define WG_TYPE_GUI 1            /* The main GUI element .. this is basically "a gui" and you only need to create one */
#define WG_TYPE_PAGE 2           /* A Page can hold multiple gui elements like toggles, sliders, etc. */
#define WG_TYPE_PANEL 3          /* _NOT USED_ */
#define WG_TYPE_TOGGLE 4         /* An on/off boolean toggle */
#define WG_TYPE_SLIDER_FLOAT 5   /* A float slider */
#define WG_TYPE_SLIDER_INT 6     /* An int slider */
#define WG_TYPE_BUTTON 7         /* Button + callback function */

#define WG_TASK_NONE 0
#define WG_TASK_BROADCAST 1      /* Broadcasting task */
#define WG_TASK_SEND 2           /* Send data to the connection data has been set to the "connection" pointer in WebGUI_Task */
#define WG_TASK_STOP 3           /* Because we're using a condition var for the worker thread we need to send it a task so it knows that it needs to stop */

#define WG_EVENT_NONE 0          /* Default event ID. We receive "events" from the GUI/webapp */
#define WG_EVENT_VALUE_CHANGED 1 /* Is sent by the JS part, when a value has change */
#define WG_EVENT_CLICKED 2       /* Button clicked */

struct WebGUI_Connection {
  WebGUI_Connection();
  struct libwebsocket* ws;
};

struct WebGUI_Task {
  WebGUI_Task();

  void setData(unsigned char* ptr, size_t nbytes);   /* Set the data;  this function takes care of the pre/post padding */
  void setType(int t);                               /* Set the task type */
  void setConnection(WebGUI_Connection* c);          /* When a task is related to a specific connection this can be used to set it */
  void setExceptWebSocket(struct libwebsocket* ws);  /* Used when broadcasting, this will broadcast to all connectetions except this one */

  unsigned char* getPtr();                   /* Get pointer to the data part in buffer */
  size_t getSize();                          /* Get the size of the actual data we want to send (see pre/post padding notes in the libwebsockets.h file) */
  int getType();                             /* Get the type of the task */
  WebGUI_Connection* getConnection();        /* When a task is related to a specific connection, this will return the connection */
  struct libwebsocket* getExceptWebSocket(); /* Used by the broadcast task - we broadcast to all sockets except this one. */

private:
  int type;                                   /* The task size */
  size_t nbytes;                              /* Size of the actual data in the buffer. this is not the same as buffer.size() because the buffer.size() has some extra space for the pre and post padding (See libwebsockets.h for more info about this) */
  std::vector<unsigned char> buffer;          /* The buffer which contains the data to be send */
  WebGUI_Connection* connection;              /* The wrapper for our websocket connection */
  struct libwebsocket* except_websocket;      /* Used for the broadcast task type; when broadcasting an event that was triggered by a webgui the triggering gui does not need to receive the event itself because it's the source of it */
};


inline void WebGUI_Task::setExceptWebSocket(struct libwebsocket* ws) {

#if !defined(NDEBUG)
  if(type != WG_TASK_BROADCAST) {
    printf("Error: You're trying to set the except websocket but this is only used when the type of the task is WG_TASK_BROADCAST. Make sure to set this type first. Stopping now.\n");
    ::exit(EXIT_FAILURE);
  }
  if(!ws) {
    printf("Error: trying to set the except websocket but the given pointer is not valid. Stopping now.\n");
    ::exit(EXIT_FAILURE);
  }
#endif

  except_websocket = ws;
  
}

inline void WebGUI_Task::setData(unsigned char* ptr, size_t len) {

#if !defined(NDEBUG)
  if(!ptr) {
    printf("Error: trying to set data but the given data pointer is invalid. Stopping now.\n");
    ::exit(EXIT_FAILURE);
  }
  if(!len) {
    printf("Error: trying to set data but the len is invalid. Stopping now.\n");
    ::exit(EXIT_FAILURE);
  }
#endif

  nbytes = len;
  buffer.resize(nbytes + LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING);
  buffer.insert(buffer.begin() + LWS_SEND_BUFFER_PRE_PADDING, ptr, ptr + nbytes);
}

inline size_t WebGUI_Task::getSize() {
  return nbytes;
}

inline unsigned char* WebGUI_Task::getPtr() {
  return (unsigned char*)&buffer.front() + LWS_SEND_BUFFER_PRE_PADDING;
}

inline int WebGUI_Task::getType() {
  return type;
}

inline void WebGUI_Task::setType(int t) {
  type = t;
}

inline void WebGUI_Task::setConnection(WebGUI_Connection* c) {

#if !defined(NDEBUG)
  if(!c) {
    printf("Error: trying to set the connection but the connection is invalid/NULL. Stopping now.\n");
    ::exit(EXIT_FAILURE);
  }
#endif

  connection = c;
}

inline WebGUI_Connection* WebGUI_Task::getConnection() {
  return connection;
}

inline struct libwebsocket* WebGUI_Task::getExceptWebSocket() {
  return except_websocket;
}
#endif
