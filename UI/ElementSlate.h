#ifndef EMP_UI_ELEMENT_SLATE_H
#define EMP_UI_ELEMENT_SLATE_H

///////////////////////////////////////////////////////////////////////////////////////////
//
//  Manage a section of the current web page
//

#include <map>
#include <string>

#include "emscripten.h"

#include "../tools/assert.h"
#include "../tools/alert.h"

#include "Element.h"
#include "ElementText.h"
#include "ElementWrapper.h"

namespace emp {
namespace UI {

  using ElementButton = emp::UI::ElementWrapper<emp::UI::Button>;
  using ElementImage = emp::UI::ElementWrapper<emp::UI::Image>;
  using ElementTable = emp::UI::ElementWrapper<emp::UI::Table>;

  class ElementSlate : public Element {
  protected:
    std::map<std::string, Element *> element_dict;  // By-name lookup for elements.
    bool initialized;                               // Is element hooked into HTML DOM hierarchy.

    void InitializeChild(Element * child) {
      EM_ASM_ARGS({
          var slate_name = Pointer_stringify($0);
          var elem_name = Pointer_stringify($1);
          $( '#' + slate_name ).append('<span id=\'' + elem_name + '\'></span>');
        }, GetName().c_str(), child->GetName().c_str() );
    }

    // Return a text element for appending, either the current last element or build new one.
    ElementText & GetTextElement() {
      // If the final element is not text, add one.
      if (children.size() == 0 || children.back()->IsText() == false)  {
        std::string new_name = name + std::string("__") + std::to_string(children.size());
        Element * new_child = new ElementText(new_name, this);
        children.push_back(new_child);

        // If this slate is already initialized, we should immediately initialize the child.
        if (initialized) InitializeChild(new_child);
      }
      return *((ElementText *) children.back());
    }
    
    virtual bool Register(Element * new_element) {
      // @CAO Make sure that name is not already used?
      element_dict[new_element->GetName()] = new_element;
      
      // Also register in parent, if available.
      if (parent) parent->Register(new_element);
      
      return true; // Registration successful.
    }

    std::string CalcNextName() const {
      return name + std::string("__") + std::to_string(children.size());
    }

    void UpdateHTML() {
      HTML.str("");                               // Clear the current stream.
      for (auto * element : children) {
        HTML << "<span id=\"" << element->GetName() << "\"></span>\n";
      }
    }

  
public:
    ElementSlate(const std::string & name, Element * in_parent=nullptr)
      : Element(name, in_parent), initialized(false)
    { ; }
    ~ElementSlate() { ; }
    
    // Do not allow Managers to be copied
    ElementSlate(const ElementSlate &) = delete;
    ElementSlate & operator=(const ElementSlate &) = delete;

    bool Contains(const std::string & test_name) {
      return element_dict.find(test_name) != element_dict.end();
    }
    Element & FindElement(const std::string & test_name) {
      emp_assert(Contains(test_name));
      return *(element_dict[test_name]);
    }
    Element & operator[](const std::string & test_name) {
      return FindElement(test_name);
    }
    ElementButton & Button(const std::string & test_name) {
      // Assert that we have the correct type, then return it.
      emp_assert(dynamic_cast<ElementButton *>( &(FindElement(test_name)) ) != NULL);
      return dynamic_cast<ElementButton&>( FindElement(test_name) );
    }
    ElementImage & Image(const std::string & test_name) {
      // Assert that we have the correct type, then return it.
      emp_assert(dynamic_cast<ElementImage *>( &(FindElement(test_name)) ) != NULL);
      return dynamic_cast<ElementImage&>( FindElement(test_name) );
    }
    ElementSlate & Slate(const std::string & test_name) {
      // Assert that we have the correct type, then return it.
      emp_assert(dynamic_cast<ElementSlate *>( &(FindElement(test_name)) ) != NULL);
      return dynamic_cast<ElementSlate&>( FindElement(test_name) );
    }
    ElementTable & Table(const std::string & test_name) {
      // Assert that we have the correct type, then return it.
      emp_assert(dynamic_cast<ElementTable *>( &(FindElement(test_name)) ) != NULL);
      return dynamic_cast<ElementTable&>( FindElement(test_name) );
    }
    ElementText & Text(const std::string & test_name) {
      // Assert that we have the correct type, then return it.
      emp_assert(dynamic_cast<ElementText *>( &(FindElement(test_name)) ) != NULL);
      return dynamic_cast<ElementText&>( FindElement(test_name) );
    }


    // Add additional children on to this element.
    Element & Append(const std::string & in_text) {
      return GetTextElement().Append(in_text);
    }

    Element & Append(const std::function<std::string()> & in_fun) {
      return GetTextElement().Append(in_fun);
    }

    // Default to passing specialty operators to parent.
    Element & Append(emp::UI::Button info) {
      // If a name was passed in, use it.  Otherwise generate a default name.
      if (info.GetTempName() == "") info.TempName( CalcNextName() );

      ElementButton * new_child = new ElementButton(info, this);
      children.push_back(new_child);
      
      // If this slate is already initialized, we should immediately initialize the child.
      if (initialized) InitializeChild(new_child);
      
      return *new_child;
    }
    Element & Append(emp::UI::Image info) {
      // If a name was passed in, use it.  Otherwise generate a default name.
      // @CAO should we default name to URL??
      if (info.GetTempName() == "") info.TempName( CalcNextName() );

      ElementImage * new_child = new ElementImage(info, this);
      children.push_back(new_child);
      
      // If this slate is already initialized, we should immediately initialize the child.
      if (initialized) InitializeChild(new_child);
      
      return *new_child;
    }
    Element & Append(emp::UI::Table info) {
      // If a name was passed in, use it.  Otherwise generate a default name.
      if (info.GetTempName() == "") info.TempName( CalcNextName() );

      ElementTable * new_child = new ElementTable(info, this);
      children.push_back(new_child);
      
      // If this slate is already initialized, we should immediately initialize the child.
      if (initialized) InitializeChild(new_child);
      
      return *new_child;
    }


  };

};
};

#endif