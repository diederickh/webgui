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

 # WebGUI_Slider

 Creates a slider for floats, ints, etc...

 ```c++
 WebGUI_Page* page = ..
 page->push_back(new WebGUI_Slider<float>("Effect", 0.0f, 10.0f, 0.01f, value));
 ````

*/

#ifndef ROXLU_WEB_GUI_SLIDER_H
#define ROXLU_WEB_GUI_SLIDER_H

#include <webgui/Types.h>
#include <webgui/Element.h>

template<class T>
class WebGUI_Slider : public WebGUI_Element {
 public:
  WebGUI_Slider(std::string title, T min, T max, T step, T& value);
  void setValue(T v);                                        /* Updates the value */
 private:
  void initialize(WebGUI_Slider<float>* slider);              /* Initializes the float slider */
  void initialize(WebGUI_Slider<int>* slider);                /* Initializes the integer slider */
 public:
  T min_value;                                                /* Minimum value */
  T max_value;                                                /* Maximum value */
  T step;                                                     /* Step size, see gui */
  T& value;
};


template<class T>
WebGUI_Slider<T>::WebGUI_Slider(std::string t, T min, T max, T step, T& value) 
  :WebGUI_Element(WG_TYPE_NONE)
  ,min_value(min)
  ,max_value(max)
  ,step(step)
  ,value(value)
{
  setTitle(t);
  initialize(this);
}

template<class T>
void WebGUI_Slider<T>::initialize(WebGUI_Slider<float>* slider) {
  type = WG_TYPE_SLIDER_FLOAT;
}

template<class T>
void WebGUI_Slider<T>::initialize(WebGUI_Slider<int>* slider) {
  type = WG_TYPE_SLIDER_INT;
}

template<class T>
void WebGUI_Slider<T>::setValue(T v) {
  value = v;
}

#endif
