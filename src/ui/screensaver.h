#ifndef SCREENSAVER_H
#define SCREENSAVER_H

class Screensaver {
 public:
  virtual ~Screensaver() {}

  virtual void Inhibit() = 0;
  virtual void Uninhibit() = 0;

  static Screensaver* GetScreensaver();

 private:
  static Screensaver* screensaver_;
};

#endif
