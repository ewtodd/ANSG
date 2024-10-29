#ifndef ACTION_HH
#define ACTION_HH

#include "G4VUserActionInitialization.hh"
#include "event.hh"
#include "generator.hh"
#include "run.hh"
#include "stepping.hh"

class ActionInitialization : public G4VUserActionInitialization {
public:
  ActionInitialization();
  ~ActionInitialization();

  virtual void Build() const;
  virtual void BuildForMaster() const;
};

#endif
