#ifndef INCLUDED_TIMER_H
#define INCLUDED_TIMER_H

class Timer {
private:
  struct Data;
  Data* d;
public:
  /*! default constructor starts immediately. */
  Timer();
  ~Timer();
  void restart();
  /*! gets the elapsed time since start in milliseconds.
  */
  double getElapsedTimeMs() const;
}; // end class Timer


#endif