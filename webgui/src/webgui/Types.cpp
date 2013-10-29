#include <webgui/Types.h>

WebGUI_Connection::WebGUI_Connection()
  :ws(NULL)
{
}

// -------------------------------------------------------

WebGUI_Task::WebGUI_Task()
  :type(WG_TASK_NONE)
  ,connection(NULL)
  ,nbytes(0)
  ,except_websocket(NULL)
{
}
