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


  # WebGUI_Button
  
  Creates a button element in the web gui. When you click on this in the web gui
  this set callback function will be called with the ID you've passed to the 
  constructor.

*/

#ifndef ROXLU_WEB_GUI_BUTTON_H
#define ROXLU_WEB_GUI_BUTTON_H

#include <webgui/Element.h>
#include <webgui/Types.h>

// -------------------------------------------------------

typedef void(*webgui_button_callback)(int id, void* user);  /* Your callback function must follow this typedef */

// -------------------------------------------------------

class WebGUI_Button : public WebGUI_Element {
 public:
  WebGUI_Button(std::string t,                     /* The title for the element */
                int buttonID,                      /* The ID of that is passed into your callback function */
                webgui_button_callback callback,   /* The function that we call when someone presses the button */
                void* user);                       /* The user pointer that gets passed into the function */
  void onClicked();                                /* Call this when the user clicked on the button; this will call the set callback */
 public:
  int button_id;                                   /* Each button has it's own id that is given through the constructor. This ID is passed to the button callback function so you can act upon the received id. */
  webgui_button_callback cb_click;                 /* The callback function that is called when `onClicked()` is called - so someone presses a button */
  void* cb_user;                                   /* The user pointer that is passed into the callback function */
};

inline void WebGUI_Button::onClicked() {

#if !defined(NDEBUG)
  if(!cb_click) {
    printf("Error: clicked on a button, but no valid callback set.\n");
    return;
  }
#endif

  cb_click(button_id, cb_user);
}

#endif
