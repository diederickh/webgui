#include <webgui/Serializer.h>
#include <sstream>

std::string WebGUI_Serializer::serialize(std::vector<WebGUI_Element*>& els, bool isRoot) {

  std::string result;
  size_t i = 0;

  for(std::vector<WebGUI_Element*>::iterator it = els.begin(); it != els.end(); ++it) {

    WebGUI_Element* el = *it;

    switch(el->type) {
      case WG_TYPE_PAGE: {
        WebGUI_Page* p = static_cast<WebGUI_Page*>(el);
        result += serialize(p);
        break;
      }
      case WG_TYPE_TOGGLE: {
        WebGUI_Toggle* t = static_cast<WebGUI_Toggle*>(el);
        result += serialize(t);
        break;
      }
      case WG_TYPE_SLIDER_FLOAT: {
        WebGUI_Slider<float>* sl = static_cast<WebGUI_Slider<float>* >(el);
        result += serialize(sl);
        break;
      }
      case WG_TYPE_BUTTON: {
        WebGUI_Button* b = static_cast<WebGUI_Button*>(el);
        result += serialize(b);
        break;
      }
      default: {
        printf("Unhandled serializer type: %d. Stopping now.\n", el->type);
        ::exit(EXIT_FAILURE);
      }
    }

    if(i != els.size()-1) {
      result += ",";
    }

    ++i;
  }

  std::stringstream ss;
  
  ss << ((isRoot) ? "{" : "")
     << "\"els\":[" << result << "]"
     << ((isRoot) ? "}" : "");

  std::string json = ss.str();

  return json;
}

std::string WebGUI_Serializer::serialize(WebGUI_Page* p) {

  std::string child_json;
  if(p->elements.size()) {
    child_json = serialize(p->elements, false);
  }

  std::stringstream ss;
  ss << "{" 
     << "\"type\":" << p->type << ","
     << "\"title\":\"" << p->title << "\","
     << "\"id\":" << p->id << ","
     << "\"parent\":" << p->parent->id << ",";

  if(child_json.size()) {
    ss << child_json;
  }

  ss <<"}";
  std::string json = ss.str();
  return json;
}

std::string WebGUI_Serializer::serialize(WebGUI_Toggle* t) {
  std::stringstream ss;
  ss << "{" 
     << "\"type\":" << t->type << ","
     << "\"title\":\"" << t->title << "\","
     << "\"id\":" << t->id << ","
     << "\"parent\":" << t->parent->id
     << "}";
  return ss.str();
}

std::string WebGUI_Serializer::serialize(WebGUI_Button* b) {
  std::stringstream ss;
  ss << "{" 
     << "\"type\":" << b->type << ","
     << "\"title\":\"" << b->title << "\","
     << "\"button_id\":" << b->button_id << ","
     << "\"id\":" << b->id << ","
     << "\"parent\":" << b->parent->id 
     << "}";
  return ss.str();
}
