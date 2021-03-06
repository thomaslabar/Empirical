//  This file is part of Empirical, https://github.com/devosoft/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE

//This file defines an object that will listen to the signals given by a World
//object, calculate metrics of open-ended evolution, and print them out.

//Developer notes:
// * Currently assumes BitOrgs
// * Currently incompatible with pruned lineage tracker

#ifndef EMP_OEE_H
#define EMP_OEE_H

#include <set>
#include <queue>
#include <deque>
#include <algorithm>
#include <iterator>
#include <functional>

#include "../tools/vector.h"
#include "../tools/array.h"
#include "LineageTracker.h"
#include "../tools/stats.h"
#include "StatsManager.h"

namespace emp{
namespace evo{

  EMP_EXTEND_CONFIG( OEEStatsManagerConfig, StatsManagerConfig,
                     VALUE(GENERATIONS, int, 50, "How long must a lineage survive to count as persistant"),
                    )

  //Is there a way to avoid making this global but still do inheritance right?
  OEEStatsManagerConfig OeeConfig;

  template <typename POP_MANAGER>
  class OEEStatsManager : StatsManager_Base<POP_MANAGER> {
  private:
    //This prevents compilation under g++
    using org_ptr = typename POP_MANAGER::value_type;
    using ORG = typename std::remove_pointer<org_ptr>::type;
    using skeleton_type = emp::vector<int>;
    static constexpr bool separate_generations = POP_MANAGER::emp_has_separate_generations;

    std::set<skeleton_type > novel;

    int generations = OeeConfig.GENERATIONS(); //How far back do we look for persistance?

    // Historical generations needed to count stats. We only need these in
    // proportion to resolution.
    std::deque<emp::vector<int> > past_snapshots;
    using StatsManager_Base<POP_MANAGER>::resolution;
    using StatsManager_Base<POP_MANAGER>::output_location;
    using StatsManager_Base<POP_MANAGER>::delimiter;

  public:
    using StatsManager_Base<POP_MANAGER>::emp_is_stats_manager;
    using lineage_type = LineageTracker<POP_MANAGER>;
    lineage_type * lineage;
    std::function<double(org_ptr)> fit_fun;

    template <typename WORLD>
    OEEStatsManager(WORLD * w,
                    std::string location = "oee_stats.csv")
                    : StatsManager_Base<POP_MANAGER>(OeeConfig, "OEE_stats.cfg", location){
      // This isn't going to work if generations aren't a multiple of resolution
      Setup(w);
    }

    OEEStatsManager(std::string location = "oee_stats.csv")
                    : StatsManager_Base<POP_MANAGER>(OeeConfig, "OEE_stats.cfg", location){;}

    template <typename WORLD>
    void Setup(WORLD * w) {
      emp_assert(generations % resolution == 0 &&
                "ERROR: Generations required for persistance must be a multiple of resolution.",
                resolution, generations);

      OeeConfig.Read("OEE_stats.cfg");
      generations = OeeConfig.GENERATIONS();
      OeeConfig.Write("OEE_stats.cfg");

      past_snapshots = std::deque<emp::vector<int> >(2*generations/resolution + 1);

      std::function<void(int)> UpdateFun = [this] (int ud){
        Update(ud);
      };

      //Setup signal callbacks
      w->OnUpdate(UpdateFun);
      lineage = &(w->lineageM);

      output_location << "update" << delimiter << "change" << delimiter
            << "novelty" << delimiter << "ecology" << delimiter
            << "complexity" << std::endl;
    }

    void SetDefaultFitnessFun(std::function<double(org_ptr)> fit){
        fit_fun = fit;
    }

    //Update callback function handles calculating stats
    void Update(int update) {
      int change = -1;
      int novelty = -1;
      double ecology = -1;
      int complexity = -1;

      if (update % resolution == 0) {

        emp::vector<skeleton_type > persist_skeletons = Skeletonize(
                                              GetPersistLineage(lineage,
                                              past_snapshots[0],
                                              past_snapshots[generations/resolution]));
        emp::vector<skeleton_type > prev_persist_skeletons = Skeletonize(
                                              GetPersistLineage(lineage,
                                              past_snapshots[generations/resolution],
                                              past_snapshots[2*generations/resolution]));

        if (past_snapshots[2*generations/resolution].size() > 0){
          change = ChangeMetric(persist_skeletons, prev_persist_skeletons);
        }

        if (past_snapshots[generations/resolution].size() > 0){
          novelty = NoveltyMetric(persist_skeletons);
          ecology = EcologyMetric(persist_skeletons);
          complexity = ComplexityMetric(persist_skeletons,
                              [](skeleton_type skel){
                                int nulls = std::count(skel.begin(), skel.end(), -1);
                                return (double)(skel.size() - nulls);
                              });
        }
        output_location << update << delimiter << change << delimiter << novelty << delimiter << ecology << delimiter << complexity << std::endl;
        past_snapshots.pop_back();
        past_snapshots.push_front(lineage->generation_since_update);
      }
    }


    //Convert a container of orgs to skeletons containing only informative sites
    //TODO: Currently assumes bit org
    template <template <typename> class C >
    C<skeleton_type> Skeletonize (C<ORG> orgs){
      C<skeleton_type> skeletons;
      for (auto org : orgs) {
        double fitness = fit_fun(&org);
        skeleton_type skeleton(org.size());
        ORG test = org;

        for (int i = 0; i < org.size(); i++) {
          test[i] = !test[i];
          if (fit_fun(&test) >= fitness){
            skeleton[i] = -1;
          } else {
            skeleton[i] = org[i];
          }
          test[i] = !test[i];
        }
        skeletons.insert(skeleton);
      }
      return skeletons;
    }

    emp::vector<skeleton_type> Skeletonize (emp::vector<ORG> orgs){
      emp::vector<skeleton_type> skeletons;
      for (auto org : orgs) {
        double fitness = fit_fun(&org);
        skeleton_type skeleton(org.size());
        ORG test = org;

        for (int i = 0; i < org.size(); i++) {
          test[i] = !test[i];
          if (fit_fun(&test) >= fitness){
            skeleton[i] = -1;
          } else {
            skeleton[i] = org[i];
          }
          test[i] = !test[i];
        }
        skeletons.push_back(skeleton);
      }
      return skeletons;
    }


    //Find most complex skeleton in the given vector.
    double ComplexityMetric(emp::vector<skeleton_type> persist,
                            std::function<double(skeleton_type)> complexity_fun) {

      double most_complex = complexity_fun(*(persist.begin()));

      for (auto org : persist) {
        if (complexity_fun(org) > most_complex) {
          most_complex = complexity_fun(org);
        }
      }
      return most_complex;
    }

    //Determine the shannon diversity of the skeletons in the given vector
    double EcologyMetric(emp::vector<skeleton_type> persist){

      return emp::evo::ShannonEntropy(persist);

    }

    //Determine how many skeletons the given vector contains that have never
    //been seen before
    int NoveltyMetric(emp::vector<skeleton_type > persist){

      int result = 0;

      for (skeleton_type lin : persist){
        if (novel.find(lin) == novel.end()){
          result++;
          novel.insert(lin);
        }
      }

      return result;
    }

    //How many skeletons are there in the first vector that aren't in the second?
    int ChangeMetric( emp::vector<skeleton_type > persist,
                      emp::vector<skeleton_type > prev_persist){

      std::set<skeleton_type > curr_set(persist.begin(), persist.end());
      std::set<skeleton_type > prev_set(prev_persist.begin(), prev_persist.end());

      std::set<skeleton_type > result;
      std::set_difference(persist.begin(), persist.end(), prev_persist.begin(),
      prev_persist.end(), std::inserter(result, result.end()));
      return result.size();
    }

  };


  //TODO: This is weird and awkward and I don't like it
  template <typename POP_MANAGER>
  using ORG = typename std::remove_pointer<typename POP_MANAGER::value_type>::type;

  //Here lies the beastiary of functions for dealing with persistant lineages

  //Takes a container of ints representing org ids (as assigned by the lineage)
  //tracker, and returns a contatiner of the genomes of those ints.
  template <typename POP_MANAGER, template <typename> class C >
  C<ORG<POP_MANAGER> > IDsToGenomes(LineageTracker<POP_MANAGER>* lineages, C<int> persist_ids) {
    C<ORG<POP_MANAGER> > persist;
    for (int id : persist_ids){
      persist.insert(persist.back(), *(lineages->org_to_genome[id]));
    }

    return persist;
  }

  //Specialization for emp::vector so we can use push_back
  template <typename POP_MANAGER>
  emp::vector<ORG<POP_MANAGER> > IDsToGenomes(LineageTracker<POP_MANAGER>* lineages, emp::vector<int> persist_ids) {
    emp::vector<ORG<POP_MANAGER> > persist;
    for (int id : persist_ids){
      persist.push_back(*(lineages->org_to_genome[id]));
    }

    return persist;
  }

  //GET PERSISTANT LINEAGE IDS FUNCTIONS:
  //These functions get the ids of orgs that went on to be the ancestor of a
  //lineage that persisted for the indicated amount of time. They will accept
  //any container of ints representing org ids and return the same container

  //The first version takes the current generation and an int indicating
  //how many generations a lineage must have survived to count as persistent.
  //It returns the ids of all orgs that were exactly that many generations
  //back in a lineage

  //Generic container version
  template <typename POP_MANAGER, template <typename> class C >
  C<int> GetPersistLineageIDs(LineageTracker<POP_MANAGER>* lineages,
               C<int> curr_generation,
               int generations){

    C<int> persist;
    for (int id : curr_generation){
      emp::vector<int> lin = lineages->TraceLineageIDs(id);
      emp_assert(lin.size() - generations > 0);
      persist.insert(persist.back(), *(lin.begin() + generations));
    }

    return persist;
  }

  //emp::vector version so we can use push_back
  template <typename POP_MANAGER>
  emp::vector<int> GetPersistLineageIDs(LineageTracker<POP_MANAGER>* lineages,
               emp::vector<int> curr_generation,
               int generations){

    emp::vector<int> persist;
    for (int id : curr_generation){
      emp::vector<int> lin = lineages->TraceLineageIDs(id);
      emp_assert(lin.size() - generations > 0);
      persist.push_back(*(lin.begin() + generations));
    }

    return persist;
  }

  //The second version takes two containers of ints representing the ids of
  //the orgnaisms that were present at one point in time and a point in time
  //the desired length of time ago, and determines which orgs in the second
  //container have descendants in the first.

  template <typename POP_MANAGER, template <typename> class C >
  C<int> GetPersistLineageIDs(LineageTracker<POP_MANAGER>* lineages,
                C<int> curr_generation,
                C<int> prev_generation){

    C<int> persist;

    for (auto id : curr_generation){
      while(id) {

        if (std::find(prev_generation.begin(), prev_generation.end(), id) != prev_generation.end()) {
          persist.insert(persist.back(), id);
          break;
        }
        id = lineages->parents[id];
      }
    }

    return persist;
  }

  //Specialization for emp::vector so we can use push_back
  template <typename POP_MANAGER>
  emp::vector<int> GetPersistLineageIDs(LineageTracker<POP_MANAGER>* lineages,
                emp::vector<int> curr_generation,
                emp::vector<int> prev_generation){

    emp::vector<int> persist;

    for (auto id : curr_generation){
      while(id) {

        if (std::find(prev_generation.begin(), prev_generation.end(), id) != prev_generation.end()) {
          persist.push_back(id);
          break;
        }
        id = lineages->parents[id];
      }
    }

    return persist;
  }

  //Whereas GetPersistLineageIDs returns the ids of the orgs that are the
  //ancestors of persistent lineages, GetPersistLineage converts the ids to
  //GENOMEs first.
  template <typename POP_MANAGER, template <typename> class C>
  C<ORG<POP_MANAGER> > GetPersistLineage(LineageTracker<POP_MANAGER>* lineages,
				       C<int> curr_generation,
				       int generations){

    C<ORG<POP_MANAGER> > persist;
    for (int id : curr_generation){
      emp::vector<ORG<POP_MANAGER>*> lin = lineages->TraceLineage(id);
      emp_assert(lin.size() - generations > 0);
      persist.insert(persist.back(), **(lin.begin() + generations));
    }

    return persist;
  }

  //Version that takes two populations
  template <typename POP_MANAGER, template <typename> class C >
  C<ORG<POP_MANAGER> > GetPersistLineage(LineageTracker<POP_MANAGER>* lineages,
                C<int> curr_generation,
                C<int> prev_generation){

    C<int> persist_ids = GetPersistLineageIDs(lineages, curr_generation, prev_generation);
    return IDsToGenomes(lineages, persist_ids);
  }

  //Version that takes two populations
  template <typename POP_MANAGER>
  emp::vector<ORG<POP_MANAGER> > GetPersistLineage(LineageTracker<POP_MANAGER>* lineages,
                emp::vector<int> curr_generation,
                emp::vector<int> prev_generation){

    emp::vector<int> persist_ids = GetPersistLineageIDs(lineages, curr_generation, prev_generation);
    return IDsToGenomes(lineages, persist_ids);
  }


  using OEEStats = OEEStatsManager<PopBasic>;
}
}


#endif
