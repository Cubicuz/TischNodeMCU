
/*
 * wants: 
 *  mapping name -> functionpointer
 *  string array containing function names
 *  timerindependant
 *  ref to led memory
 *    bitcount
 *    ledcount
 *    colorcount (RGB RGBW)
 *    LED-independant
 *  structoriented
 *  selectedFunction
 *  currentAnimationState
 */

struct Animation{
  int ledCount;
  uint8_t tick;

};

struct NameAndFunction {
  const char* name;
  void *fkt(Animation);
} availableAnimations[5];

void animate(Animation anim);

void setFunction(Animation anim, int index);

void setFunction(Animation anim, const char* name);

void setFunction(Animation anim, void (*fkt));

void initAnimation(Animation anim);
