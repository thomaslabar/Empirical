//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file defines built-in population managers for use with emp::evo::World
//
//
//  Developer notes:
//  * Rather than deleting organisms ourright, run all deletions through a ClearCell function
//    so that a common signal system can also be run.

#ifndef EMP_EVO_POPULATION_MANAGER_H
#define EMP_EVO_POPULATION_MANAGER_H

#include <set>
#include "../tools/random_utils.h"
#include "PopulationIterator.h"

namespace emp {
namespace evo {

  template <typename POP_MANAGER> class PopulationIterator;

  template <typename ORG=int>
  class PopulationManager_Base {
  protected:
    using ptr_t = ORG *;
    emp::vector<ORG *> pop;

    Random * random_ptr;

  public:
    PopulationManager_Base() { ; }
    ~PopulationManager_Base() { ; }

    // Allow this and derived classes to be identified as a population manager.
    static constexpr bool emp_is_population_manager = true;
    static constexpr bool emp_has_separate_generations = false;
    using value_type = ORG*;

    friend class PopulationIterator<PopulationManager_Base<ORG> >;
    using iterator = PopulationIterator<PopulationManager_Base<ORG> >;

    ptr_t & operator[](int i) { return pop[i]; }
    const ptr_t operator[](int i) const { return pop[i]; }
    iterator begin(){return iterator(this, 0);}
    iterator end(){return iterator(this, pop.size());}

    uint32_t size() const { return pop.size(); }
    void resize(int new_size) { pop.resize(new_size); }
    int GetSize() const { return (int) pop.size(); }

    void SetRandom(Random * r) { random_ptr = r; }


    void Print(std::function<std::string(ORG*)> string_fun, std::ostream & os = std::cout,
              std::string empty="X", std::string spacer=" ") {
      for (ORG * org : pop) {
        if (org) os << string_fun(org);
        else os << empty;
        os << spacer;
      }
    }
    void Print(std::ostream & os = std::cout, std::string empty="X", std::string spacer=" ") {
      for (ORG * org : pop) {
        if (org) os << *org;
        else os << empty;
        os << spacer;
      }
    }

    // AddOrg and ReplaceOrg should be the only ways new organisms come into a population.
    // AddOrg inserts them into the end of the designated population.
    // ReplaceOrg places them at a specific position, replacing anyone who may already be there.
    int AddOrg(ORG * new_org) {
      const int pos = pop.size();
      pop.push_back(new_org);
      return pos;
    }
    int AddOrgBirth(ORG * new_org, int parent_pos) {
      const int pos = random_ptr->GetInt((int) pop.size());
      if (pop[pos]) delete pop[pos];
      pop[pos] = new_org;
      return pos;
    }

    void Clear() {
      // Delete all organisms.
      for (ORG * m : pop) delete m;
      pop.resize(0);
    }

    void Update() { ; } // Basic version of Update() does nothing, but World may trigger actions.

    // Execute() redirect to all organisms in the population, forwarding arguments.
    template <typename... ARGS>
    void Execute(ARGS... args) {
      for (ORG * m : pop) {
        if (m) m->Execute(std::forward<ARGS>(args)...);
      }
    }


    // --- POPULATION MANIPULATIONS ---

    // Run population through a bottleneck to (potentiall) shrink it.
    void DoBottleneck(const int new_size, bool choose_random=true) {
      if (new_size >= (int) pop.size()) return;  // No bottleneck needed!

      // If we are supposed to keep only random organisms, shuffle the beginning into place!
      if (choose_random) emp::Shuffle<ptr_t>(*random_ptr, pop, new_size);

      // Delete all of the organisms we are removing and resize the population.
      for (int i = new_size; i < (int) pop.size(); ++i) delete pop[i];
      pop.resize(new_size);
    }
  };

  // A standard population manager for using synchronous generations in a traditional
  // evolutionary algorithm setup.

  template <typename ORG=int>
  class PopulationManager_EA : public PopulationManager_Base<ORG> {
  protected:
    emp::vector<ORG *> next_pop;
    using PopulationManager_Base<ORG>::pop;

  public:
    PopulationManager_EA() { ; }
    ~PopulationManager_EA() { Clear(); }

    static constexpr bool emp_has_separate_generations = true;

    int AddOrgBirth(ORG * new_org, int parent_pos) {
      const int pos = next_pop.size();
      next_pop.push_back(new_org);
      return pos;
    }

    void Clear() {
      // Delete all organisms.
      for (ORG * m : pop) delete m;
      for (ORG * m : next_pop) delete m;

      pop.resize(0);
      next_pop.resize(0);
    }

    void Update() {
      for (ORG * m : pop) delete m;   // Delete the current population.
      pop = next_pop;                    // Move over the next generation.
      next_pop.resize(0);                // Clear out the next pop to refill again.
    }
  };


  // A standard population manager for using a serial-transfer protocol.  All new
  // organisms get inserted into the main population; once it is full the population
  // is shrunk down.

  template <typename ORG=int>
  class PopulationManager_SerialTransfer : public PopulationManager_Base<ORG> {
  protected:
    using PopulationManager_Base<ORG>::pop;
    using PopulationManager_Base<ORG>::random_ptr;
    using PopulationManager_Base<ORG>::DoBottleneck;

    int max_size;
    int bottleneck_size;
    int num_bottlenecks;
  public:
    PopulationManager_SerialTransfer()
      : max_size(1000), bottleneck_size(100), num_bottlenecks(0) { ; }
    ~PopulationManager_SerialTransfer() { ; }

    int GetMaxSize() const { return max_size; }
    int GetBottleneckSize() const { return bottleneck_size; }
    int GetNumBottlnecks() const { return num_bottlenecks; }

    void SetMaxSize(const int m) { max_size = m; }
    void SetBottleneckSize(const int b) { bottleneck_size = b; }

    void ConfigPop(int m, int b) { max_size = m; bottleneck_size = b; }

    int AddOrgBirth(ORG * new_org, int parent_pos) {
      if (pop.size() >= max_size) {
        DoBottleneck(bottleneck_size);
        ++num_bottlenecks;
      }
      const int pos = pop.size();
      pop.push_back(new_org);
      return pos;
    }
  };

  template <typename ORG=int>
  class PopulationManager_Grid : public PopulationManager_Base<ORG> {
  protected:
    using PopulationManager_Base<ORG>::pop;
    using PopulationManager_Base<ORG>::random_ptr;

    int width;
    int height;

    int ToX(int id) const { return id % width; }
    int ToY(int id) const { return id / width; }
    int ToID(int x, int y) const { return y*width + x; }

  public:
    PopulationManager_Grid() { ConfigPop(10,10); }
    ~PopulationManager_Grid() { ; }

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

    // method to get all the von-neuman (sp?) neighbors of a particular organism
    // does not include the organism itself
    std::set<ORG *> get_org_neighbors (int org_id) {
      std::set<ORG *> neighbors;
      int org_x, org_y;
      int possx[3] = {0,0,0}, possy[3] = {0,0,0}; //arrays to hold possible offsets
      org_x = ToX(org_id);
      org_y = ToY(org_id);

      if (org_y > 0) {possy[2] = -1;} // enable going up
      if (org_y < height - 1) {possy[0] = 1;} // enable going down
      if (org_x > 0) { possx[0] = -1;} // enable going left
      if (org_x < width - 1) {possx[2] = 1;} // enable going right

      // iterate over all possible spaces && add organisms to set
      // using the set will prevent duplictes, since we *WILL* traverse the same spaces
      // multiple times.
      for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
          neighbors.insert(pop[ToID(org_x + possx[i], org_y + possy[j])]);
        }
      }

      neighbors.erase(neighbors.find(pop[org_id])); // remove focal node from set
      return neighbors;
    }

    //TODO@JGF: a) make the rest of my todo's @'d to me
    //          b) make the function naming consistent (e.g. camel, not _'s)
    
    /// This function will get all elements in a radius 'depth' from a focal node
    std::set<ORG *> * GetClusterByRadius(unsigned int focal_id, unsigned int depth, 
                                       std::set<ORG *> * lump = nullptr) {
      // if the thing exists, use it. If not, create it.
      if (lump == nullptr) {lump = new std::set<ORG *>;}
      // add ourselves
      if (lump->find(pop[focal_id]) != lump->end()) {return lump;} // stop recursion, we've been here
      lump->insert(pop[focal_id]);

      // duplicated code--joy
      // TODO@JGF: fix the duplication
      int org_x, org_y;
      int possx[3] = {0,0,0}, possy[3] = {0,0,0}; //arrays to hold possible offsets
      org_x = ToX(focal_id);
      org_y = ToY(focal_id);

      if (org_y > 0) {possy[2] = -1;} // enable going up
      if (org_y < height - 1) {possy[0] = 1;} // enable going down
      if (org_x > 0) { possx[0] = -1;} // enable going left
      if (org_x < width - 1) {possx[2] = 1;} // enable going right

      // for all 'enabled' spaces recurse this finder
      // our base case up top will prevent running in circles
      if (depth <= 0) {return lump;}
      for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
          GetClusterByRadius(ToID(org_x + possx[i], org_y + possy[j]), depth - 1, lump);
        }
      }

      return lump;
    }

    void ConfigPop(int w, int h) { width = w; height = h; pop.resize(width*height, nullptr); }

    // Injected orgs go into a random position.
    int AddOrg(ORG * new_org) {
      const int pos = random_ptr->GetInt((int) pop.size());
      if (pop[pos]) delete pop[pos];
      pop[pos] = new_org;
      return pos;
    }

    // Newly born orgs go next to their parents.
    int AddOrgBirth(ORG * new_org, int parent_pos) {
      const int parent_x = ToX(parent_pos);
      const int parent_y = ToY(parent_pos);

      const int offset = random_ptr->GetInt(9);
      const int offspring_x = emp::mod(parent_x + offset%3 - 1, width);
      const int offspring_y = emp::mod(parent_y + offset/3 - 1, height);
      const int pos = ToID(offspring_x, offspring_y);

      if (pop[pos]) delete pop[pos];

      pop[pos] = new_org;

      return pos;
    }

    void Print(std::function<std::string(ORG*)> string_fun,
               std::ostream & os = std::cout,
               const std::string & empty="-",
               const std::string & spacer=" ")
    {
      emp_assert(string_fun);
      for (int y=0; y<height; y++) {
        for (int x = 0; x<width; x++) {
          ORG * org = pop[ToID(x,y)];
          if (org) os << string_fun(org) << spacer;
          else os << empty << spacer;
        }
        os << std::endl;
      }
    }

    void Print(std::ostream & os = std::cout, std::string empty="X", std::string spacer=" ") {
      for (int y=0; y<height; y++) {
        for (int x = 0; x<width; x++) {
          ORG * org = pop[ToID(x,y)];
          if (org) os << *org << spacer;
          else os << empty << spacer;
        }
        os << std::endl;
      }
    }
  };

  using PopBasic = PopulationManager_Base<int>;
  using PopEA    = PopulationManager_EA<int>;
  using PopST    = PopulationManager_SerialTransfer<int>;
  using PopGrid  = PopulationManager_Grid<int>;

}
}


#endif
