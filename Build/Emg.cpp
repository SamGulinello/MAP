#include <stdint.h>

extern const int EMG_ARRAY_lENGTH;

class Emg {
  private:
    int32_t emgValue;
    int32_t emgArray;
    int32_t total;
    int32_t readIndex;

    //stuff for getMaxElement

    int32_t maxValue;
    int32_t* first;
    int32_t* last;
    int32_t* maxNum;
    

  public:
    int32_t emgRead(int);
    void emgCal();
    int32_t rms(int32_t);
    int32_t* getMaxElement(int32_t*, int32_t*);
};
