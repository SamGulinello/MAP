class Timer {
  private:
    unsigned long beginTime;
    unsigned long prevRead;
    int allowGetTime;

  public:
    Timer();
    unsigned long getTotalTime();
    void resetTimer();
    unsigned long getTime();
};
