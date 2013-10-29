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

  # WebGUI

  Lots of projects we do at [Apollo](http://www.apollomedia.nl) are controlled
  using a gui to tweak everything exactly to our needs. Though more and more projects
  need to be controlled remotely. Because of this we created a gui that we can use
  to control the parameters of the application through a website which makes it a 
  lot easier to control our apps! 

*/

#ifndef ROXLU_WEB_GUI_H
#define ROXLU_WEB_GUI_H

extern "C" {
#  include <uv.h>
#  include <libwebsockets.h>
#  include <jansson.h>
}

#include <vector>
#include <string>
#include <webgui/Types.h>
#include <webgui/Page.h>
#include <webgui/Panel.h>
#include <webgui/Toggle.h>
#include <webgui/Slider.h>
#include <webgui/Button.h>

// -------------------------------------------------------
class WebGUI;

void webgui_write_thread(void* user);
void webgui_read_thread(void* user);

int webgui_proto_callback(struct libwebsocket_context* ctx,
                          struct libwebsocket* ws,
                          enum libwebsocket_callback_reasons reason,
                          void* user,
                          void* in,
                          size_t len);

bool webgui_parse(struct libwebsocket* ws,            /* Parses the data it receives from the websocket */
                  unsigned char* data,            
                  size_t len, 
                  WebGUI* gui);

// -------------------------------------------------------

std::string webgui_callback_reason_to_string(int r);

// -------------------------------------------------------


class WebGUI : public WebGUI_Element {
 public:
  WebGUI();
  ~WebGUI();
  bool start();
  void stop();
  bool save(std::string filepath);                      /* Save the current state to a file at the given location (full path) */
  bool load(std::string filepath);                      /* Load the stored settings from the given filepath */

 private:
  void addTask(WebGUI_Task* t);                          /* Adds a task to the task queue that is handled in a separate thread */
  std::string serialize();                               /* Serializes the elements to a json string, used to communicate with the websocket */

 public:
  void addConnection(WebGUI_Connection* c);              /* Add a new connection */
  void removeConnection(struct libwebsocket* ws);        /* Removes the connection form our list of currently connected clients */
  void removeConnections();                              /* Remove all connections */
  void removeTasks();                                    /* Remove all tasks */
  void sendElements(WebGUI_Connection* c);               /* Sends the serialized gui to the client */
  void broadcast(unsigned char* data, size_t nbytes,     /* Broadcasts a message to all clients, when `except` is a valid pointer do not broadcast to this connected as this is probably the one which is responsible for this broadcast  */
                 struct libwebsocket* except = NULL);   
  bool mustStop();

 public:

  /* websockets */
  std::vector<libwebsocket_protocols> protocols;       /* The protocols that we're handling ... basically only one, the gui protocol */
  std::vector<WebGUI_Connection*> connections;         /* List with the current connections */
  struct libwebsocket_context* context;                /* Our lib websocket context */

  /* threading */
  uv_thread_t read_thread;                             /* We read in a separate thread; but the read and write threads are synchronized */
  uv_thread_t write_thread;                            /* The write thread handle */
  uv_mutex_t mutex;                                    /* Used to synchronize with the thread */
  uv_mutex_t stop_mutex;                               /* Only used to handle the stop flag */
  uv_cond_t cv;                                        /* Used to signal when we have new work for the thread */
  std::vector<WebGUI_Task*> tasks;                     /* The tasks which are handled by the thread */
  bool must_stop;

  /* gui elements */
  std::string json;                                    /* Contains the serialized version of the gui that is send to clients when the connect */
};


inline void WebGUI::addConnection(WebGUI_Connection* c) {
  uv_mutex_lock(&mutex);
  {
    connections.push_back(c);
  }
  uv_mutex_unlock(&mutex);
}

inline void WebGUI::removeConnection(struct libwebsocket* ws) {
  uv_mutex_lock(&mutex);
  {
    for(std::vector<WebGUI_Connection*>::iterator it = connections.begin(); it != connections.end(); ++it) {

      WebGUI_Connection* c = *it;
      if(c->ws != ws) {
        continue;
      }

      connections.erase(it);

      delete c;
      c = NULL;
      break;
    }
  }
  uv_mutex_unlock(&mutex);
}

inline void WebGUI::sendElements(WebGUI_Connection* c) {

#if !defined(NDEBUG)
  if(!json.size()) {
    printf("Error: cannot send the gui information to the client because it seems that you didn't add any gui elements.\n");
    return;
  }
#else 
  if(!json.size()) {
    return;
  }
#endif

  WebGUI_Task* t = new WebGUI_Task();
  t->setType(WG_TASK_SEND);
  t->setConnection(c);
  t->setData((unsigned char*)json.c_str(), json.size());
  addTask(t);
}

inline void WebGUI::addTask(WebGUI_Task* t) {
  uv_mutex_lock(&mutex);
  {
    tasks.push_back(t);
    uv_cond_signal(&cv);
  }
  uv_mutex_unlock(&mutex);
}


inline bool WebGUI::mustStop() {
  bool m = false;
  uv_mutex_lock(&stop_mutex);
    m = must_stop;
  uv_mutex_unlock(&stop_mutex);
  return m;
}


#endif
