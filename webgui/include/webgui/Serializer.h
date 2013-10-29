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

 # WebGUI_Serializer

 The WebGUI_Serializer takes care of all communication between the websocket
 and the application. It will serialize the GUI and send over the gui definition
 to the web app that generates the gui. All communication over the websocket is 
 done using json.

 See: https://gist.github.com/roxlu/e9d5d96f3be521907fa3  for a example of the 
      serialized json that is used to generate a gui. The type values are from 
      Types.h

*/
#ifndef ROXLU_WEB_GUI_SERIALIZER_H
#define ROXLU_WEB_GUI_SERIALIZER_H

#include <webgui/Element.h>
#include <webgui/Page.h>
#include <webgui/Panel.h>
#include <webgui/Toggle.h>
#include <webgui/Slider.h>
#include <webgui/Button.h>
#include <sstream>
#include <vector>
#include <string>

class WebGUI_Serializer {
 public:
  std::string serialize(std::vector<WebGUI_Element*>& els, bool isRoot = true); /* serialize the given elements and return a buffer as string */
  std::string serialize(WebGUI_Page* el);
  std::string serialize(WebGUI_Toggle* el);
  std::string serialize(WebGUI_Button* el);

  template<class T>
  std::string serialize(WebGUI_Slider<T>* el);
};

template<class T>
std::string WebGUI_Serializer::serialize(WebGUI_Slider<T>* el) {

  std::stringstream ss;
  ss << "{" 
     << "\"type\":" << el->type << ","
     << "\"title\":\"" << el->title << "\","
     << "\"id\":" << el->id << ","
     << "\"parent\":" << el->parent->id  << ","
     << "\"min\":" << el->min_value << ","
     << "\"max\":" << el->max_value << ","
     << "\"step\":" << el->step << ""
     << "}";

  return ss.str();
}

#endif
