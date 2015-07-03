#ifndef EMP_UI_CANVAS_H
#define EMP_UI_CANVAS_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Canvas widget.
//


#include <string>
#include <vector>

#include "Widget.h"

namespace emp {
namespace UI {

  // Base class to maintain canvas actions.
  class CanvasAction {
  protected:
    // Helper functions that may be useful to specific actions.
    void Fill(const std::string & style="") {
      if (style != "") {
        EM_ASM_ARGS({
            emp.ctx.fillStyle = Pointer_stringify($0);
          }, style.c_str());
      }
      EM_ASM({ emp.ctx.fill(); });
    }
    void Stroke(const std::string & style="") {
      if (style != "") {
        EM_ASM_ARGS({
            emp.ctx.strokeStyle = Pointer_stringify($0);
          }, style.c_str());
      }
      EM_ASM({ emp.ctx.stroke(); });
      
    }

  public:
    CanvasAction() { ; }
    virtual ~CanvasAction() { ; }

    
    virtual void Apply() = 0;            // Apply current action to emp.ctx.
    virtual CanvasAction * Clone() = 0;  // Make a copy of the current action.
  };

  class CanvasRect : public CanvasAction {
    int x; int y; int w; int h;
    std::string fill_color;
    std::string line_color;
  public:
    CanvasRect(int _x, int _y, int _w, int _h,
               const std::string & fc="", const std::string & lc="")
      : x(_x), y(_y), w(_w), h(_h), fill_color(fc), line_color(lc) { ; }

    void Apply() {
      EM_ASM_ARGS({ emp.ctx.rect($0, $1, $2, $3); }, x, y, w, h);  // Draw the rectangle
      Fill(fill_color);
      Stroke(line_color);
    }
    CanvasAction * Clone() { return new CanvasRect(*this); }
  };

  class CanvasStrokeColor : public CanvasAction {
    std::string color;
  public:
    CanvasStrokeColor(const std::string & c) : color(c) { ; }

    void Apply() {
      EM_ASM_ARGS({
          var color = Pointer_stringify($0);
          emp.ctx.strokeStyle = color;
        }, color.c_str());
    }
    CanvasAction * Clone() { return new CanvasStrokeColor(*this); }
  };


  ///////////////  Main Canvas Class ////////////////

  class Canvas : public internal::Widget<Canvas> {
  protected:
    int width;
    int height;

    std::vector<CanvasAction *> actions;
    std::size_t next_action;

    void WriteHTML(std::ostream & os) {
      os << "<canvas id=\"" << div_id << obj_ext
         << "\" width=\"" << width
         << "\" height=\"" << height << "\">";
      // @CAO We can include fallback content here for browsers that don't support canvas.
      os << "</canvas>";
    }    

    void ClearActions() {
      for (auto * a : actions) delete a;
      actions.resize(0);
      next_action = 0;
    }

    void TriggerJS() {      // Process Actions...
      // Setup the canvas
      EM_ASM_ARGS({
          var cname = Pointer_stringify($0);
          var canvas = document.getElementById(cname);
          emp.ctx = canvas.getContext('2d');
        }, GetFullID().c_str());

      while (next_action < actions.size()) {
        actions[next_action]->Apply();
        ++next_action;
      };
    }

    Canvas & AddAction(CanvasAction * new_action) {
      actions.push_back(new_action);
      return *this;
    }

  public:
    Canvas(int w, int h, const std::string & in_name="")
      : Widget(in_name), width(w), height(h), next_action(0) { ; }
    Canvas(const Canvas & in) : width(in.width), height(in.height), next_action(0) {
      actions.resize(in.actions.size());
      for (int i = 0; i < (int) actions.size(); i++) {
        actions[i] = in.actions[i]->Clone();
      }
    }
    Canvas(Canvas && in) : width(in.width), height(in.height), actions(in.actions), next_action(0) {
      in.actions.resize(0);
      in.next_action = 0;
    }

    Canvas & operator=(const Canvas &) = delete;
    ~Canvas() { ClearActions(); }

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

    // Setup Canvas Actions
    Canvas & Rect(int x, int y, int w, int h, const std::string & fc="", const std::string & lc="")
    { return AddAction( new CanvasRect(x, y, w, h, fc, lc) ); }
    Canvas & StrokeColor(std::string c) { return AddAction( new CanvasStrokeColor(c) ); }

    Canvas & Clear() { ClearActions(); return *this; }

    static std::string TypeName() { return "Canvas"; }

    bool OK(std::stringstream & ss, bool verbose=false, const std::string & prefix="") {
      bool ok = true;

      return ok;
    }
  };

};
};

#endif
