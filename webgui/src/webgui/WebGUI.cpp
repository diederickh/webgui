#include <stdio.h>
#include <stdlib.h>
#include <webgui/WebGUI.h>
#include <webgui/Serializer.h>
#include <webgui/Storage.h>

// -------------------------------------------------------

/*

  This parses json that is sent by the js/web gui part. The structure
  of the json object follows: 

  ````json
  {
    i:[integer: the id of the element],
    e:[integer: the name of the event, see Types.h],
    v:[mixed the values of the type, based on the element type]
  }
  ````

 */
bool webgui_parse(struct libwebsocket* ws, 
                  unsigned char* data,
                  size_t len,
                  WebGUI* gui)
{

#if 0
  for(size_t i = 0; i < len; ++i) {
    printf("%c", data[i]);
  }
  printf("\n");
#endif
  
  json_error_t err;
  json_t* root = json_loads((const char*)data, 0, &err);

  if(!root) {
    printf("Error: cannot parse json: `%s`. Error: %s. Stopping now.\n", data,  err.text);
    json_decref(root);
    ::exit(EXIT_FAILURE);
  }

  json_t* i = json_object_get(root, "i");
  if(!i || !json_is_integer(i)) {
    printf("Error: no `i` element found in the data. This should contains the id of the element. Stopping now");
    ::exit(EXIT_FAILURE);
  }

  json_t* ev = json_object_get(root, "e");
  if(!ev || !json_is_integer(ev)) {
    printf("Error: the `e` element of data is supposed to be a numeric value. Stopping now.\n");
    ::exit(EXIT_FAILURE);
  }

  int id = json_integer_value(i);
  int event = json_integer_value(ev);

  WebGUI_Element* element = gui->findElement(id);
  if(!element) {
    printf("Error: element with id: %d not found.\n", id);
    ::exit(EXIT_FAILURE);
  }

  switch(event) {
    case WG_EVENT_VALUE_CHANGED: {
      gui->broadcast(data, len, ws);

      switch(element->type) {

        case WG_TYPE_SLIDER_FLOAT: {
          
          json_t* value = json_object_get(root, "v");
          if(!value || !json_is_number(value)) {
            printf("Received an invalid value for a WebGUI_Slider<float>. Stopping now. value: %p\n", value);
            ::exit(EXIT_FAILURE);
          }

          WebGUI_Slider<float>* sl = static_cast<WebGUI_Slider<float> *>(element);
          sl->setValue(json_real_value(value));
          break;
        }

        case WG_TYPE_TOGGLE: {
          json_t* value = json_object_get(root, "v");
          if(!value || !json_is_integer(value)) {
            printf("Received an invalid value for a WebGUI_Toggle. Stopping now.\n");
            ::exit(EXIT_FAILURE);
          }
          
          WebGUI_Toggle* toggle = static_cast<WebGUI_Toggle*>(element);
          toggle->setValue((bool)json_integer_value(value));
          break;
        }

        default: {
          printf("Unhandled WG_EVENT_VALUE_CHANGED for element type: %d\n", element->type);
          ::exit(EXIT_FAILURE);
        }

      }

      break;
    }
    case WG_EVENT_CLICKED: {
      WebGUI_Button* button = static_cast<WebGUI_Button*>(element);
      button->onClicked();
      break;
    }
    default: {
      printf("Error: unhandled gui event.\n");
      break;
    }
  }

  if(root) {
    json_decref(root);
    root = NULL;
  }

  return true;
}

// -------------------------------------------------------

void webgui_read_thread(void* user) {
  
  WebGUI* gui = static_cast<WebGUI*>(user);
  bool must_stop = false;

  while(true) {
    libwebsocket_service(gui->context, 50);

    must_stop = gui->mustStop();
    if(must_stop) {
      break;
    }
  }
}

// -------------------------------------------------------

void webgui_write_thread(void* user) {

  WebGUI* gui = static_cast<WebGUI*>(user);

  std::vector<WebGUI_Task*> work;
  std::vector<WebGUI_Connection*> connections;
  bool must_stop = false;

  while(true) {
    uv_mutex_lock(&gui->mutex);
    {
      while(gui->tasks.size() == 0) {
        uv_cond_wait(&gui->cv, &gui->mutex);
      }
      std::copy(gui->tasks.begin(), gui->tasks.end(), std::back_inserter(work));
      std::copy(gui->connections.begin(), gui->connections.end(), std::back_inserter(connections));
      gui->tasks.clear();
    }
    uv_mutex_unlock(&gui->mutex);

    std::vector<WebGUI_Task*>::iterator it = work.begin(); 
    while(it != work.end()) {

      WebGUI_Task* task = *it;

      switch(task->getType()) {

        // send to one connection
        case WG_TASK_SEND: {
          libwebsocket_write(task->getConnection()->ws, 
                             task->getPtr(), 
                             task->getSize(), 
                             LWS_WRITE_TEXT);
          break;
        }

        // broadcast to all connections.
        case WG_TASK_BROADCAST: {

          for(size_t i = 0; i < connections.size(); ++i) {

            if(connections[i]->ws == task->getExceptWebSocket()) {
              continue;
            }
            
            uv_mutex_lock(&gui->mutex);
            {
              libwebsocket_write(connections[i]->ws, 
                                 task->getPtr(), 
                                 task->getSize(),  
                                 LWS_WRITE_TEXT);
            }
            uv_mutex_unlock(&gui->mutex);
          };
          break;
        }

        // stop!
        case WG_TASK_STOP: {
          must_stop = true;
          break;
        }

        default: {
          printf("Unhandled web gui task: %d. Stopping now.\n", task->getType());
          ::exit(EXIT_FAILURE);
          break;
        }
      }
      
      delete task;
      task = NULL;
      it = work.erase(it);
    }

    // cleanup if we've stopped
    for(std::vector<WebGUI_Task*>::iterator it = work.begin(); it != work.end(); ++it) {
      WebGUI_Task* task = *it;
      delete task;
    }
    
    work.clear();
    connections.clear();

    if(must_stop) {
      break;
    }
  }
}

// -------------------------------------------------------

int webgui_proto_callback(struct libwebsocket_context* ctx,
                          struct libwebsocket* ws,
                          enum libwebsocket_callback_reasons reason,
                          void* user,
                          void* in,
                          size_t len)
{
  WebGUI* gui = static_cast<WebGUI*>(libwebsocket_context_user(ctx));

  if(!gui) {
    printf("Error: the user member of the libwesockets_context is not a valid WebGUI pointer. Stopping now.\n");
    ::exit(EXIT_FAILURE);
  }

  switch(reason) {

    case LWS_CALLBACK_ESTABLISHED:  {
      WebGUI_Connection* c = new WebGUI_Connection();
      c->ws = ws;
      gui->addConnection(c);
      gui->sendElements(c);
      break;
    };

    case LWS_CALLBACK_RECEIVE: {
      webgui_parse(ws, (unsigned char*)in, len, gui);
      break;
    };

    case LWS_CALLBACK_CLOSED: {
      gui->removeConnection(ws);
      break;
    }

    default: break;
  }
  return 0;
}

// -------------------------------------------------------

WebGUI::WebGUI() 
  :context(NULL)
  ,WebGUI_Element(WG_TYPE_GUI)
  ,must_stop(true)
{
  uv_mutex_init(&mutex);
  uv_mutex_init(&stop_mutex);
  uv_cond_init(&cv);
}

WebGUI::~WebGUI() {

  if(context) {
    stop();
  }

  uv_mutex_destroy(&mutex);
  uv_mutex_destroy(&stop_mutex);
  uv_cond_destroy(&cv);
}

bool WebGUI::start() {

  if(context) {
    printf("Erorr: Cannot start because it seems that we've already started. Call stop() first.\n");
    return false;
  }
  
  json = serialize();

  libwebsocket_protocols proto;
  proto.name = "webgui";
  proto.callback = webgui_proto_callback;
  proto.per_session_data_size = 0;
  proto.rx_buffer_size = 0;
  proto.no_buffer_all_partial_tx = 0;
  protocols.push_back(proto);

  libwebsocket_protocols proto_end;
  proto_end.name = NULL;
  proto_end.callback = NULL;
  proto_end.per_session_data_size = 0;
  proto_end.no_buffer_all_partial_tx = 0;
  protocols.push_back(proto_end);

  struct lws_context_creation_info info;
  memset((char*)&info, 0x00, sizeof(info));
  
  info.port = 7681;
  info.iface = NULL;
  info.protocols = &protocols.front();
  info.extensions = NULL;
  info.gid = -1;
  info.uid = -1;
  info.user = this;

  context = libwebsocket_create_context(&info);
  if(!context) {
    printf("Cannot create the websocket context.\n");
    return false;
  }

  must_stop = false;

  uv_thread_create(&write_thread, webgui_write_thread, this);
  uv_thread_create(&read_thread, webgui_read_thread, this);
  return true;
}

void WebGUI::broadcast(unsigned char* data, size_t nbytes, struct libwebsocket* except) {
  WebGUI_Task* t = new WebGUI_Task();
  t->setType(WG_TASK_BROADCAST);
  t->setData(data, nbytes);
  t->setExceptWebSocket(except);
  addTask(t);
}

std::string WebGUI::serialize() {
  WebGUI_Serializer s;
  std::string json = s.serialize(elements);
  return json;
}

void WebGUI::stop() {

#if !defined(NDEBUG)
  if(must_stop) {
    printf("You're calling stop but we're not even running.\n");
    return;
  }
#endif

  WebGUI_Task* t = new WebGUI_Task();
  t->setType(WG_TASK_STOP);
  addTask(t);

  uv_mutex_lock(&stop_mutex);
    must_stop = true;
  uv_mutex_unlock(&stop_mutex);

  uv_thread_join(&read_thread);
  uv_thread_join(&write_thread);

  removeConnections();
  removeTasks();

  libwebsocket_context_destroy(context);
  context = NULL;
}

void WebGUI::removeConnections() {
  uv_mutex_lock(&mutex);
  {
    for(std::vector<WebGUI_Connection*>::iterator it = connections.begin(); it != connections.end(); ++it) {
      WebGUI_Connection* c = *it;
      delete c;
    }
    connections.clear();
  }
  uv_mutex_unlock(&mutex);
}

void WebGUI::removeTasks() {
  uv_mutex_lock(&mutex);
  {
    for(std::vector<WebGUI_Task*>::iterator it = tasks.begin(); it != tasks.end(); ++it) {
      WebGUI_Task* t = *it;
      delete t;
    }
    tasks.clear();
  }
  uv_mutex_unlock(&mutex);
}

bool WebGUI::save(std::string filepath) {
  
  if(!filepath.size()) {
    printf("Error: no filepath given.\n");
    return false;
  }

  WebGUI_Storage storage;
  return storage.save(filepath, this);
}

bool WebGUI::load(std::string filepath) {

  if(!filepath.size()) {
    printf("Error: no filepath given.\n");
    return false;
  }

  WebGUI_Storage storage;
  return storage.load(filepath, this);
}

// ----------------------------------------------------------------------------

std::string webgui_callback_reason_to_string(int r) {
  switch(r) {
    case LWS_CALLBACK_ESTABLISHED: return "LWS_CALLBACK_ESTABLISHED"; 
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR: return "LWS_CALLBACK_CLIENT_CONNECTION_ERROR"; 
    case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH: return "LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH"; 
    case LWS_CALLBACK_CLIENT_ESTABLISHED: return "LWS_CALLBACK_CLIENT_ESTABLISHED"; 
    case LWS_CALLBACK_CLOSED: return "LWS_CALLBACK_CLOSED"; 
    case LWS_CALLBACK_CLOSED_HTTP: return "LWS_CALLBACK_CLOSED_HTTP"; 
    case LWS_CALLBACK_RECEIVE: return "LWS_CALLBACK_RECEIVE"; 
    case LWS_CALLBACK_CLIENT_RECEIVE: return "LWS_CALLBACK_CLIENT_RECEIVE"; 
    case LWS_CALLBACK_CLIENT_RECEIVE_PONG: return "LWS_CALLBACK_CLIENT_RECEIVE_PONG"; 
    case LWS_CALLBACK_CLIENT_WRITEABLE: return "LWS_CALLBACK_CLIENT_WRITEABLE"; 
    case LWS_CALLBACK_SERVER_WRITEABLE: return "LWS_CALLBACK_SERVER_WRITEABLE"; 
    case LWS_CALLBACK_HTTP: return "LWS_CALLBACK_HTTP"; 
    case LWS_CALLBACK_HTTP_FILE_COMPLETION: return "LWS_CALLBACK_HTTP_FILE_COMPLETION"; 
    case LWS_CALLBACK_HTTP_WRITEABLE: return "LWS_CALLBACK_HTTP_WRITEABLE"; 
    case LWS_CALLBACK_FILTER_NETWORK_CONNECTION: return "LWS_CALLBACK_FILTER_NETWORK_CONNECTION"; 
    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION: return "LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION"; 
    case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS: return "LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS"; 
    case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS: return "LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS"; 
    case LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION: return "LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION"; 
    case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER: return "LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER"; 
    case LWS_CALLBACK_CONFIRM_EXTENSION_OKAY: return "LWS_CALLBACK_CONFIRM_EXTENSION_OKAY"; 
    case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED: return "LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED"; 
    case LWS_CALLBACK_PROTOCOL_INIT: return "LWS_CALLBACK_PROTOCOL_INIT"; 
    case LWS_CALLBACK_PROTOCOL_DESTROY: return "LWS_CALLBACK_PROTOCOL_DESTROY"; 
    case LWS_CALLBACK_ADD_POLL_FD: return "LWS_CALLBACK_ADD_POLL_FD"; 
    case LWS_CALLBACK_DEL_POLL_FD: return "LWS_CALLBACK_DEL_POLL_FD"; 
    case LWS_CALLBACK_SET_MODE_POLL_FD: return "LWS_CALLBACK_SET_MODE_POLL_FD"; 
    case LWS_CALLBACK_CLEAR_MODE_POLL_FD: return "LWS_CALLBACK_CLEAR_MODE_POLL_FD"; 
    default: return "UNKNOWN";
  }
}


