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

  # WebGUI_Element

  The WebGUI_Element is the base type for all of the available gui elements. It
  defines a couple of common properties like the title and the `push_back()` and 
  `size()` function. 

*/
#ifndef ROXLU_WEB_GUI_ELEMENT_H
#define ROXLU_WEB_GUI_ELEMENT_H

#include <string>
#include <vector>

class WebGUI_Element {
 public:
  WebGUI_Element(int type);                  /* Create with one of the types as defined in Types.h */
  virtual ~WebGUI_Element(); 

  void setTitle(std::string t);              /* Set the title for this element */
  void push_back(WebGUI_Element* el);        /* Add a child element */
  size_t size();                             /* Number of child elements */

  WebGUI_Element* findElement(int elementID, WebGUI_Element* parent = NULL); /* Find an elemetnt by id recursively*/

 public:
  static int num_els;                        /* We auto generate an unique ID for each of the elements. This ID is used when communication with the server/ over the socket. */
  int id;                                    /* The unique ID that is used when sending data over the network to indicate this particular element */
  int type;                                  /* The type of element, see Types.h */
  std::string title;                         /* The title/label for the element. */
  std::vector<WebGUI_Element*> elements;     /* The child elements, used by containers; e.g. WebGUI_Page */
  WebGUI_Element* parent;                    /* The parent element */
};

inline void WebGUI_Element::setTitle(std::string t) {
  title = t;
}

inline size_t WebGUI_Element::size() {
  return elements.size();
}

inline void WebGUI_Element::push_back(WebGUI_Element* el) {

  el->parent = this;
  el->id = num_els;
  num_els++;
  
  elements.push_back(el);
}

#endif
