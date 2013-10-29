#include <uv.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <webgui/WebGUI.h>

void sighandler(int num);

bool must_run = true;

void on_click(int id, void* user);

struct GuiValues {
  bool enable_distort;
  bool enable_grid;
  float effect;
  float amount;
  float rshift;
};

int main() {

  signal(SIGINT,sighandler);
  GuiValues values;
  WebGUI* gui;
  WebGUI_Page* page;
  uint64_t delay = 5000 * 1000000;
  uint64_t timeout = uv_hrtime() +delay; 
  
  std::string msg = "this is a basic message.\n";
  unsigned int c = 0;


  gui = new WebGUI();
  page = new WebGUI_Page("Grid distort effect");
  page->push_back(new WebGUI_Toggle("Enable grid distort effect", values.enable_distort));
  page->push_back(new WebGUI_Toggle("Perlin noise", values.enable_grid));
  page->push_back(new WebGUI_Slider<float>("Effect", 0.0f, 10.0f, 0.01f, values.effect));
  page->push_back(new WebGUI_Slider<float>("Amount", 0.0f, 10.0f, 0.01f, values.amount));
  page->push_back(new WebGUI_Slider<float>("R-shift", 0.0f, 10.0f, 0.01f, values.rshift));
  page->push_back(new WebGUI_Button("Burst!", 0, on_click, NULL));
  page->push_back(new WebGUI_Button("Wind forces!", 0, on_click, NULL));
  
  gui->push_back(page);
  gui->start();


  while(must_run) {
  
    /*
    if(c % 100 == 0) {
      printf("NOW!: %d\n", c);
      if(gui) {
        printf("DELETE\n");
        delete gui;
        gui = NULL;
      }

    }
    if(gui) {
      //gui->update();
      c++;
    }
    */

  }

  delete gui;

  printf("ok.\n");
}

void sighandler(int num) {
  printf("Signal!\n");
  must_run = false;
}

void on_click(int id, void* user) {
  printf("Clicked on a button!\n");
}
