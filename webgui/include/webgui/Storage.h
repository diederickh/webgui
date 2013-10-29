#ifndef ROXLU_WEB_GUI_STORAGE_H
#define ROXLU_WEB_GUI_STORAGE_H

#include <webgui/Element.h>
#include <webgui/Slider.h>
#include <webgui/Toggle.h>
#include <webgui/Button.h>
#include <string>
#include <sstream>
#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>

using namespace rapidxml;

static std::string webgui_get_clean_element_name(std::string title);

#define WG_VALIDATE_ATTR_NAME  1 
#define WG_VALIDATE_ATTR_VALUE 2

class WebGUI_Storage {
 public:
  bool save(std::string filepath, WebGUI_Element* el);
  bool load(std::string filepath, WebGUI_Element* el);


  std::string toXML(WebGUI_Element* el, int depth = 0);                      /* Used while saving the gui to xml */
  void parseXML(WebGUI_Element* gui, xml_node<>* parent);                    /* Used to parse the xml when loading */
  void parseSlider(WebGUI_Element* gui, xml_node<>* node);                   /* Parses a slider element */
  void parseToggle(WebGUI_Element* gui, xml_node<>* node);                   /* Parses a toggle element */
  bool validateNode(xml_node<>* node,  unsigned int what);                   /* Checks if `what` are valid in the node. */

  WebGUI_Element* findElementByCleanName(xml_node<>* node, WebGUI_Element* parent);
  WebGUI_Element* findElementByCleanName(std::string name, WebGUI_Element* parent);

  bool getBoolAttribute(xml_node<>* node, std::string attr);
  float getFloatAttribute(xml_node<>* node, std::string attr);
  int getIntAttribute(xml_node<>* node, std::string attr);
  
  template<class T> 
    T getAttribute(xml_node<>* node, std::string attr);
};


inline bool WebGUI_Storage::getBoolAttribute(xml_node<>* node, std::string attr) {
  return getAttribute<bool>(node, attr);  
}

inline float WebGUI_Storage::getFloatAttribute(xml_node<>* node, std::string attr) {
  return getAttribute<float>(node, attr);  
}

inline int WebGUI_Storage::getIntAttribute(xml_node<>* node, std::string attr) {
  return getAttribute<int>(node, attr);
}

template<class T>
inline T WebGUI_Storage::getAttribute(xml_node<>* node, std::string attr) {

  T result;

  if(!node->first_attribute(attr.c_str())) {
    printf("Cannot find the value attribute in getAttribute() of WebGUI_Storage. attribute = %s\n", attr.c_str());
    return result;
  }

  std::stringstream ss;
  ss << node->first_attribute("value")->value();
  ss >> result;

  return result;
}
#endif
