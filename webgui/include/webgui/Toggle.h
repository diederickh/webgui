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

 # WebGUI_Toggle

 Creates a on/off toggle for boolean values. 

*/

#ifndef ROXLU_WEB_GUI_TOGGLE_H
#define ROXLU_WEB_GUI_TOGGLE_H

#include <webgui/Element.h>
#include <string>

class WebGUI_Toggle : public WebGUI_Element {
 public:
  WebGUI_Toggle(std::string title, bool& value);
  void setValue(bool v); /* Update the value */
 public:
  bool value;
};

inline void WebGUI_Toggle::setValue(bool v) {
  value = v;
}

#endif
