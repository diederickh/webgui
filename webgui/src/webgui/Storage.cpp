#include <stdio.h>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <webgui/Storage.h>
#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>

using namespace rapidxml;

// -----------------------------------------------------------

static std::string webgui_get_clean_element_name(std::string title) {
  std::string result;
  unsigned char* ptr = (unsigned char*)title.c_str();
  for(size_t i = 0; i < title.size(); ++i) {
    if(ptr[i] < 48 
       || (ptr[i] > 58 && ptr[i] < 65) 
       || (ptr[i] > 90 && ptr[i] < 97)
       || (ptr[i] > 122)
       )
      {
        result.push_back('_');
      }
    else {
         result.push_back(title[i]);
    }
  }
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  return result;
}

// -----------------------------------------------------------

bool WebGUI_Storage::save(std::string filepath, WebGUI_Element* el) {

  if(!el) {
    printf("Error: trying to save the webgui but invalid element given. Stopping now.\n");
    return false;
  }

  if(!filepath.size()) {
    printf("Error: invalid size.\n");
    return false;
  }

  std::ofstream ofs(filepath.c_str(), std::ios::out);
  if(!ofs.is_open()) {
    printf("Error: cannot open the xml file for writing: %s\n", filepath.c_str());
    return false;
  }

  std::string xml = toXML(el);  
  if(!xml.size()) {
    printf("Error: trying to save a gui but the generated xml is emtpy (!?).\n");
    ofs.close();
    return false;
  }

  ofs.write((char*)xml.c_str(), xml.size());

  ofs.close();

  return true;
}

bool WebGUI_Storage::load(std::string filepath, WebGUI_Element* el) {
  
  if(!filepath.size()) {
    printf("Error: invalid path given to WebGUI_Storage.\n");
    return false;
  }
  
  file<> rf(filepath.c_str());
  xml_document<> doc;
  xml_node<>* root;

  try {

    doc.parse<0>(rf.data());

    root = doc.first_node("elements");
    if(!root) {
      printf("Error: cannot find elements node in gui file.\n");
      return false;
    }

    parseXML(el, root);

  }
  catch(const rapidxml::parse_error& e) {
    printf("Error while parsing the gui xml: %s\n", e.what());
    return false;
  }

  return true;
}

void WebGUI_Storage::parseXML(WebGUI_Element* gui, xml_node<>* parent ) {

  if(!parent) {
    printf("Warning: invalid parent given to parseXML().\n");
    return ;
  }

  for(xml_node<>* child = parent->first_node(); child != NULL; child = child->next_sibling()) {
    if(child->first_node()) {
      parseXML(gui, child);
    }
    else {

      std::string type = child->name();
      std::string name = "";

      if(type == "sliderf") {
        parseSlider(gui, child);
      }
      else if(type == "toggle") {
        parseToggle(gui, child);
      }
      else {
        printf("Unhandled gui element while loading. Must implement or actively ignore this! Stopping now\n");
        ::exit(EXIT_FAILURE);
      }

    }
  }
}

bool WebGUI_Storage::validateNode(xml_node<>* node, unsigned int what) {

  if( (what & WG_VALIDATE_ATTR_NAME) == WG_VALIDATE_ATTR_NAME) {
    if(!node->first_attribute("name")) {
      printf("Cannot find the  name field. Invalid XML!.\n");
      return false;
    }
  }

  if( (what & WG_VALIDATE_ATTR_VALUE) == WG_VALIDATE_ATTR_VALUE) {
    if(!node->first_attribute("value")) {
      printf("Cannot find the  name field. Invalid XML!.\n");
      return false;
    }
  }

  return true;
}

void WebGUI_Storage::parseToggle(WebGUI_Element* gui, xml_node<>* node) {
  
  if(!validateNode(node, WG_VALIDATE_ATTR_NAME | WG_VALIDATE_ATTR_VALUE)) {
    ::exit(EXIT_FAILURE);
  }

  WebGUI_Element* gui_el = findElementByCleanName(node, gui);
  if(!gui_el) {
    printf("Cannot find a gui element which is found in that xml file.\n");
    return;
  }
  
  WebGUI_Toggle* toggle = static_cast<WebGUI_Toggle*>(gui_el);
  toggle->setValue(getBoolAttribute(node, "value"));
}

void WebGUI_Storage::parseSlider(WebGUI_Element* gui, xml_node<>* node) {

  if(!validateNode(node, WG_VALIDATE_ATTR_NAME | WG_VALIDATE_ATTR_VALUE)) {
    ::exit(EXIT_FAILURE);
  }

  WebGUI_Element* gui_el = findElementByCleanName(node, gui);
  if(!gui_el) {
    printf("Cannot find a gui element which is found in that xml file.\n");
    return;
  }

  // @todo - implement int slider
  WebGUI_Slider<float>* slider = static_cast<WebGUI_Slider<float>* >(gui_el);
  slider->setValue(getFloatAttribute(node, "value"));
}

WebGUI_Element* WebGUI_Storage::findElementByCleanName(xml_node<>* node, WebGUI_Element* parent) {

  if(!validateNode(node, WG_VALIDATE_ATTR_NAME | WG_VALIDATE_ATTR_VALUE)) {
    ::exit(EXIT_FAILURE);
  }

  std::string name = node->first_attribute("name")->value();

  return findElementByCleanName(webgui_get_clean_element_name(name), parent);
}

WebGUI_Element* WebGUI_Storage::findElementByCleanName(std::string name, WebGUI_Element* parent) {

  if(!parent) {
    printf("Error: trying to find an element by a clean name in WebGUI_Storage but the given element is invalid.\n");
    return NULL;
  }

  std::string el_name = webgui_get_clean_element_name(parent->title);
  if(el_name == name) {
    return parent;
  }

  for(std::vector<WebGUI_Element*>::iterator it = parent->elements.begin(); it != parent->elements.end(); ++it) {
    WebGUI_Element* result = findElementByCleanName(name, *it);
    if(result) {
      return result;
    }
  }

  return NULL;
}

std::string WebGUI_Storage::toXML(WebGUI_Element* el, int depth) {

  if(!el) {
    return "";
  }

  std::stringstream xml;

  std::string tabs = "";
  for(int i = 0; i < depth; ++i) {
    tabs += "\t";
  }

  if(el->elements.size()) {
    xml << tabs << "<elements>\n";
  }


  for(std::vector<WebGUI_Element*>::iterator it = el->elements.begin(); it != el->elements.end(); ++it) {
    WebGUI_Element* element = *it;
    std::string label = webgui_get_clean_element_name(element->title);

    switch(element->type) {

      case WG_TYPE_PAGE: {
        xml << tabs << "\t<page name=\"" << label << "\">\n" ;
        xml << tabs << toXML(element, depth+2);
        xml << tabs << "\t</page>\n";
        break;
      }

      case WG_TYPE_TOGGLE: {
        WebGUI_Toggle* toggle = static_cast<WebGUI_Toggle*>(element);

        xml << tabs << "\t<toggle "
            <<   "name=\"" << label << "\" " 
            <<   "value=\"" << toggle->getValue() << "\" "
            << "/>\n";
        break;
      }

      case WG_TYPE_SLIDER_FLOAT: {

        WebGUI_Slider<float>* slider = static_cast<WebGUI_Slider<float>* >(element);
        char value[128];
        sprintf(value, "%.07f", slider->getValue());

        xml << tabs << "\t<sliderf "
            << "name=\"" << label << "\" "
            << "value=\"" << value << "\" "
            << "/>\n";
        break;
      }
    }
  }

  if(el->elements.size()) {
    xml << tabs <<  "</elements>\n";
  }
  return xml.str();
}
