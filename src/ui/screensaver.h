#ifndef SCREENSAVER_H
#define SCREENSAVER_H

class Screensaver {
 public:
  virtual ~Screensaver() {}

  static const char* kGnomeService;
  static const char* kGnomePath;
  static const char* kGnomeInterface;

  static const char* kKdeService;
  static const char* kKdePath;
  static const char* kKdeInterface;

  virtual void Inhibit() = 0;
  virtual void Uninhibit() = 0;

  static Screensaver* GetScreensaver();

 private:
  static Screensaver* screensaver_;
};

#endif
