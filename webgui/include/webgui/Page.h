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

 # WebGUI_Page

 This is a container element to which elements like buttons, sliders, 
 etc.. are added. Use push_back() to add new elements.  A WebGUI* can 
 contain multiple WebGUI_Page* elements.

*/

#ifndef ROXLU_WEB_GUI_PAGE_H
#define ROXLU_WEB_GUI_PAGE_H

#include <webgui/Types.h>
#include <webgui/Element.h>
#include <vector>
#include <string>

class WebGUI_Page : public WebGUI_Element {
 public:
  WebGUI_Page(std::string title);
  ~WebGUI_Page();
};

#endif
