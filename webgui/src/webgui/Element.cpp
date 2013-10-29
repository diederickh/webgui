#include <webgui/Element.h>
#include <webgui/Types.h>

int WebGUI_Element::num_els = 0;

WebGUI_Element::WebGUI_Element(int type)
  :type(type)
  ,parent(NULL)
{

}

WebGUI_Element::~WebGUI_Element() {

  for(std::vector<WebGUI_Element*>::iterator it = elements.begin(); it != elements.end(); ++it) {
    delete *it;
  }

  elements.clear();
  id = -1;
  type = WG_TYPE_NONE;
  title.clear();
  parent = NULL;
  WebGUI_Element::num_els = 0;
}


WebGUI_Element* WebGUI_Element::findElement(int elementID, WebGUI_Element* parent) {

  std::vector<WebGUI_Element*>& els = (!parent) ? elements : parent->elements;
  WebGUI_Element* found = NULL;

  for(std::vector<WebGUI_Element*>::iterator it = els.begin(); it != els.end(); ++it) {

    WebGUI_Element* el = *it;

    if(el->id == elementID) {
      found = el;
      break;
    }

    if(el->elements.size()) {
      return findElement(elementID, el);
    }
  }

  return found;
}
