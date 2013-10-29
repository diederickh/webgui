#include <webgui/Page.h>

WebGUI_Page::WebGUI_Page(std::string t)
  :WebGUI_Element(WG_TYPE_PAGE)
{
  setTitle(t);
}

WebGUI_Page::~WebGUI_Page() {

}
