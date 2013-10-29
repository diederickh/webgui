#include <webgui/Toggle.h>
#include <webgui/Types.h>

WebGUI_Toggle::WebGUI_Toggle(std::string t, bool& value) 
  :WebGUI_Element(WG_TYPE_TOGGLE)
  ,value(value)
{
  setTitle(t);
}
