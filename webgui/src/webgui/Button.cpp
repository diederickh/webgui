#include <webgui/Button.h>

WebGUI_Button::WebGUI_Button(std::string t, int buttonID, webgui_button_callback cb, void* user) 
  :WebGUI_Element(WG_TYPE_BUTTON)
  ,button_id(buttonID)
  ,cb_click(cb)
  ,cb_user(user)
{
  setTitle(t);
}
